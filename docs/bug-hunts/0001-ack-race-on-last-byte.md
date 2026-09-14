# 0001 — The last byte of an I2C read was a race, and I didn't know it

**Date:** 2026-09-10, confirmed on hardware 2026-09-13
**Found by:** reading RM0383 §18.3.3 carefully, before it ever ran on hardware

## The code I wrote

My first `i2c1_read` drained the bus one byte at a time and handled the last
byte inside the loop:

```c
while (n > 0U) {
    if (n == 1) {
        I2C1->CR1 &= ~I2C1_CR1_ACK;   /* NACK the last byte */
        I2C1->CR1 |=  I2C1_CR1_STOP;
        while (!(I2C1->SR1 & I2C1_SR1_RXNE)) { }
        *data = I2C1->DR;
        break;
    } else {
        while (!(I2C1->SR1 & I2C1_SR1_RXNE)) { }
        *data++ = I2C1->DR;
        n--;
    }
}
```

This looks fine to me and it probably would have worked most of the time, which
is the worrying part.

## What's actually wrong

The F4's I2C peripheral has two places a received byte can sit: the shift
register (bits arriving off the wire) and DR (where I read from). When a byte
finishes arriving it moves to DR and `RXNE` goes high — and **the next byte
immediately starts arriving into the now-empty shift register.** The wire never
pauses.

The hardware decides ACK or NACK at the end of every byte, by checking the ACK
bit *at that instant*.

So on a 6-byte read: when byte 5 lands in DR and I read it, byte 6 is already
being clocked in. I then loop back, check `n == 1`, and clear ACK — but byte 6
may already have finished arriving, and got ACKed. The sensor then thinks I want
a 7th byte and keeps driving the bus.

At 100 kHz a byte takes 90 µs and my loop takes maybe 1 µs, so I'd win the race
almost always. Almost. Change the clock, add an interrupt, and it breaks — and
the failure mode is a bad CRC or a stuck BUSY flag that hangs the *next* call,
nowhere near the actual cause.

## The fix

The key thing I didn't know: **when both DR and the shift register are full, the
peripheral stretches the clock.** It holds SCL low and the sensor physically
cannot send another bit until I drain DR. That state is the `BTF` flag.

So instead of racing the wire, let the pipeline fill up on purpose — the bus
freezes — and clear ACK while nothing is moving:

```c
while (n > 3U) {
    while (!(I2C1->SR1 & I2C1_SR1_RXNE)) { }
    *data++ = I2C1->DR;
    n--;
}

while (!(I2C1->SR1 & I2C1_SR1_BTF)) { }   /* bus frozen, N-2 in DR, N-1 in shift reg */
I2C1->CR1 &= ~I2C1_CR1_ACK;               /* byte N hasn't started arriving yet */
*data++ = I2C1->DR;                       /* releasing DR restarts the clock */
I2C1->CR1 |= I2C1_CR1_STOP;
*data++ = I2C1->DR;
while (!(I2C1->SR1 & I2C1_SR1_RXNE)) { }
*data = I2C1->DR;
```

Same intent, completely different guarantee. Before, I cleared ACK while byte N
was in flight. Now I clear it before byte N has been requested.

## What I'm not implementing

This whole approach needs the pipeline to hold two bytes ahead of me, which
doesn't exist at small sizes:

- **n == 1** — the byte starts arriving the moment ADDR is cleared, so ACK has
  to be cleared *before* that. Different sequence.
- **n == 2** — needs the `POS` bit, which tells the hardware "this ACK setting
  applies to the next byte, not the current one."

SHT3x reads are 3 bytes (status) or 6 bytes (measurement), so I never hit
either. I documented the limit and added a guard that returns early instead of
running the tail sequence and overrunning the caller's buffer.

I'll write those cases when I add a second device to the bus. Most chips want a
1-byte read for a WHO_AM_I check, so it'll probably be soon.

## Confirmed on hardware

Captured a measurement read on a Saleae with the I2C analyzer on:

![I2C read capture](../media/i2c-capture.png)

Six bytes come back and the last one is NAKed. Every earlier byte decodes as
`+ ACK`, byte six as `0x0A + NAK`. That is what the BTF sequence exists to
guarantee, and it is the part I could not have verified any other way.

The frame itself checks out end to end:

```
6A 68 38   B4 2E 0A
│  │  │    │  │  └── CRC over B4 2E
│  │  │    └──┴───── raw RH  = 0xB42E = 46126  ->  70.38 %RH
│  │  └───────────── CRC over 6A 68
└──┴──────────────── raw T   = 0x6A68 = 27240  ->  27.74 °C
```

Both checksums recompute correctly against the bytes on the wire.

The analog traces are worth a look too. SDA falls sharply and rises on a curve
— that is open-drain I2C made visible. The bus can only be pulled down actively;
the rise is passive, charging through the pull-up resistor on the breakout.

Still to check: whether SCL genuinely sits low through the BTF window, and
whether the old loop can be caught ACKing byte 6 if I push the bus to 400 kHz
to tighten the race. This capture starts mid-transaction, so it does not show
the START, the address byte, or the STOP either.

Worth saying plainly: **I never saw the old version fail.** The code went from
the fix straight to working hardware, so I have no evidence it would have broken
in practice. It would almost certainly have worked on my desk. That is the
reason it is worth writing down — it would have worked until it didn't,
somewhere else, for a reason that looked unrelated.

## What I take from this

I would not have found this by testing. It would have worked on my desk, and
broken later, somewhere else, for a reason that looked unrelated. The only
reason I know about it is that I went and read the master receiver section of
RM0383 instead of assuming my loop was obviously correct.

Reference manual over intuition, for anything touching hardware timing.
