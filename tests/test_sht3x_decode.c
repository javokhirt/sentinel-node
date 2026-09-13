// Host-compiled unit tests for drivers/sht3x_decode.c 

#include "../drivers/sht3x_decode.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

/* Floats are never exactly equal. Compare within a tolerance. */
static int close_enough(float a, float b, float tol)
{
    return fabsf(a - b) < tol;
}

static void test_crc_datasheet_vector(void)
{
    const uint8_t data[2] = { 0xBE, 0xEF };
    assert(sht3x_crc8(data, 2) == 0x92);
}

static void test_crc_init_value_is_not_zero(void)
{
    const uint8_t zeros[2] = { 0x00, 0x00 };
    assert(sht3x_crc8(zeros, 2) == 0x81);
}

static void test_conversion_endpoints(void)
{
    assert(close_enough(sht3x_raw_to_celsius(0x0000), -45.0f, 0.01f));
    assert(close_enough(sht3x_raw_to_celsius(0xFFFF), 130.0f, 0.01f));
    assert(close_enough(sht3x_raw_to_rh(0x0000), 0.0f, 0.01f));
    assert(close_enough(sht3x_raw_to_rh(0xFFFF), 100.0f, 0.01f));
}

static void test_decode_valid_frame(void)
{
    const uint8_t frame[6] = { 0x62, 0xBE, 0xAD, 0x7A, 0xE1, 0xA4 };
    float t = 0.0f, h = 0.0f;

    assert(sht3x_decode_measurement(frame, &t, &h) == true);
    assert(close_enough(t, 22.5f, 0.01f));
    assert(close_enough(h, 48.0f, 0.01f));
}

static void test_decode_rejects_corrupted_frame(void)
{
    uint8_t frame[6] = { 0x62, 0xBE, 0xAD, 0x7A, 0xE1, 0xA4 };
    frame[1] ^= 0x01;

    assert(sht3x_decode_measurement(frame, NULL, NULL) == false);

    uint8_t frame2[6] = { 0x62, 0xBE, 0xAD, 0x7A, 0xE1, 0xA4 };
    frame2[4] ^= 0x80;
    assert(sht3x_decode_measurement(frame2, NULL, NULL) == false);
}

int main(void)
{
    test_crc_datasheet_vector();
    test_crc_init_value_is_not_zero();
    test_conversion_endpoints();
    test_decode_valid_frame();
    test_decode_rejects_corrupted_frame();

    printf("all sht3x_decode tests passed\n");
    return 0;
}
