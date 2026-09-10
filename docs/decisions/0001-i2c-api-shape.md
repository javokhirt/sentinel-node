# 0001 — I2C API takes raw bytes, not a register address

**Date:** 2026-09-10
**Status:** done

## What I changed

`platform/i2c.c` used to expose three functions:

```c
void I2C1_byteRead (char saddr, char maddr, char* data);
void I2C1_burstRead (char saddr, char maddr, int numBytes, char* data);
void I2C1_burstWrite(char saddr, char maddr, int n, char* data);
```

Now it exposes two:

```c
void i2c1_read (uint8_t saddr, uint16_t n, uint8_t *data);
void i2c1_write(uint8_t saddr, uint16_t n, const uint8_t *data);
```

The `maddr` parameter is gone.

## Why

I wrote the first version following the pattern I learned from an MPU6050
tutorial. That pattern assumes the device behaves like a small memory: you
write a register address, do a repeated START, then read back whatever is at
that address. `maddr` is that register address. It's baked into all three
functions.

The SHT3x does not work that way. It has no registers. You send it a 16-bit
*command* — two raw bytes — and depending on the command it either does
something or gives data back. Reading a measurement is:

1. one write transaction: `0x24 0x00`
2. a pause of 15 ms with nothing on the bus at all
3. a completely separate read transaction of 6 bytes

There is no register address anywhere in that, and no repeated START either,
because step 2 means the two transactions can't be joined.

So the old API couldn't express what I needed. I could have added a fourth
function and kept the old three, but nothing in this project uses the
register-address pattern — the SHT31 is the only I2C device on the board, and
the SX126x radio is SPI. Keeping ~200 lines of untested code for a device I
don't have seemed worse than deleting it. Git still has it if I need it.

## What I gave up

If I ever add a chip that *does* have registers (very likely — most of them do),
I'll need the write-then-repeated-START-then-read pattern back. That's one
function, `i2c1_write_read()`, not three. I'll write it when I need it.

## Related

- `bug-hunts/0001-ack-race-on-last-byte.md` — a separate bug I found while
  rewriting `i2c1_read`
- SHT3x-DIS datasheet §4.2–4.4
