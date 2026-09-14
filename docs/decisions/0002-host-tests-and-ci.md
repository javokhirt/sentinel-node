# 0002 — Split the SHT3x driver so the logic can be tested

**Date:** 2026-09-13
**Status:** done

## What I changed

`drivers/sht3x.c` became two files:

- `sht3x.c` — I2C transactions. Includes `stm32f4xx.h`. Needs hardware.
- `sht3x_decode.c` — CRC-8 and raw-to-physical conversion. Includes nothing
  beyond `<stdint.h>` and `<stdbool.h>`.

`tests/test_sht3x_decode.c` compiles the second one with plain gcc and runs it
on the host. CI runs it on every push.

## Why

I wanted to verify the CRC against the datasheet vector before touching
hardware, and I couldn't. `crc8` was `static`, and even after removing that,
`sht3x.c` pulls in `stm32f4xx.h` — full of Cortex-M register definitions that
laptop gcc can't compile.

So the test I wanted was impossible until the file was split. That's the useful
part: the need for a test is what showed me the file was doing two unrelated
jobs. Talking to a bus and doing arithmetic on bytes have nothing in common
except that they both happen to involve this sensor.

The line is whether a function touches a register. If it does, it can only be
tested on the board. If it doesn't, it's just math and runs anywhere.

## What the tests actually check

Not "does it run". Each one catches a specific way the code could be wrong and
still look fine:

- `CRC(0xBEEF) == 0x92` — the only value Sensirion publishes. If it passes, the
  algorithm is correct, not probably correct.
- CRC of `00 00` is `0x81`, not zero. Guards the init value. If someone
  "simplifies" init from 0xFF to 0x00, a dead bus reading all zeros produces a
  matching checksum and decodes as a valid -45 °C.
- Conversion endpoints. A 65536-instead-of-65535 divisor only shows up at the
  top of the range, and the error is ~0.003 °C, so normal testing never finds it.
- A real frame with genuine checksums decoding to known values.
- One flipped bit being rejected. A single bit flip still gives a plausible
  temperature. Nothing but the checksum catches it.

## Cost

Bare `assert()` instead of Unity or Ceedling. Five tests don't justify a
dependency, and a failing assert exits non-zero, which is all CI needs to mark
the job red. If this grows to where "which assert failed" becomes an annoying
question, I'll move to Unity then.

## What this does not prove

The tests say nothing about whether the driver works. They say the math is
right. Timing, the I2C sequence, the sensor itself — none of that is covered,
and none of it can be without hardware in the loop.

The firmware job in CI is the same kind of partial: it proves the code still
compiles from a clean clone, not that it runs.
