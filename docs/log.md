# Build log

Newest at the top.

---

## 2026-09-10 — I2C layer rewritten, ready for the sensor driver

Spent today on `platform/i2c.c`. Went in thinking it was basically done and came
out having rewritten most of it.

Three things happened:

**The API was the wrong shape.** I'd written it around a register address
(`maddr`), copying the pattern from an MPU6050 tutorial. The SHT3x has no
registers — it's command-driven, and the measurement read is two separate
transactions with a 15 ms gap in between. Deleted `byteRead`, rewrote the other
two without `maddr`. Wrote it up in `decisions/0001`.

**The last-byte NACK was a race.** Found this by reading RM0383 §18.3.3 rather
than by testing. My loop cleared the ACK bit while the final byte was already
arriving on the wire. Fixed with the BTF approach. Full writeup in
`bug-hunts/0001` — this is the one I'd want to talk about.

**Types.** Buffers were `char`, which is signed on ARM GCC. Every byte above
0x7F would have sign-extended when I shifted it into a `uint16_t`, so my
humidity readings would have been quietly wrong. Changed to `uint8_t`. Nothing
dramatic, but it's the kind of bug that produces plausible numbers instead of
an error, which is worse.

Also read the SHT3x datasheet properly for the first time. Sections 1–3 and 5–9
are almost entirely irrelevant to the driver; §4 is the whole thing. Useful
things I pulled out:

- address 0x44 (ADDR pin low, which is what the breakout does)
- `0x2400` = single shot, high repeatability, no clock stretching
- 15 ms max measurement time at high repeatability (Table 4)
- 6 bytes back: T MSB, T LSB, CRC, RH MSB, RH LSB, CRC
- CRC-8, poly 0x31, init 0xFF, test vector CRC(0xBEEF) = 0x92
- `T = -45 + 175 * raw / 65535` and `RH = 100 * raw / 65535` — note 65535, not
  65536

Skipping clock stretching on purpose. It would let me read immediately without
the delay, but it means the sensor holds my clock line for 15 ms with no timeout
and no way out. That's fine on a desk and bad under an RTOS. Might turn this
into its own decision note when I get to the FreeRTOS work.

**Not done:** no timeouts anywhere. Every `while` loop in this file spins
forever if the sensor doesn't respond, so one loose wire hangs the node. There's
a TODO at the top of the file. Wanted one clean reading before adding error
paths, which I still think is the right order, but it can't stay this way.

**Next:** `drivers/sht3x.c` on top of this. Plan is to write the CRC function
first and check it against 0x92 on my laptop before any of it touches the board.

Nothing has run on hardware yet. Everything here is "compiles and I believe it
is correct," which is not the same thing.
