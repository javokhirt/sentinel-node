# Host tests. Plain gcc, no ARM toolchain, no board.
#
#   make        build and run
#   make clean
#
# Only files with zero hardware dependencies can be built here. Anything
# that includes stm32f4xx.h will not compile on the host, by design.

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Werror -g -I../drivers
LDLIBS  := -lm

BUILD   := build
SRC     := ../drivers/sht3x_decode.c

TESTS   := $(BUILD)/test_sht3x_decode

.PHONY: all clean
all: $(TESTS)
	@for t in $(TESTS); do echo "--- $$t"; ./$$t || exit 1; done

$(BUILD)/test_%: test_%.c $(SRC) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
