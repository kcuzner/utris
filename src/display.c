/**
 * uTris
 *
 * Dual shift register 8x8 dot matrix display writing routines
 *
 * Kevin Cuzner
 */

#include "display.h"
#include <avr/io.h>

static uint8_t *display_buffer;
static uint8_t current_row = 0;
static uint8_t row_mask = 0x7F;

void display_init(uint8_t *buffer)
{
    DDRB |= _BV(DISPLAY_DS_PIN) | _BV(DISPLAY_STCP_PIN) | _BV(DISPLAY_SHCP_PIN);
    display_set_buffer(buffer);
}

void display_set_buffer(uint8_t *buffer)
{
    display_buffer = buffer;
    current_row = 0;
    row_mask = 0x7F;
}

static void display_shift_byte(uint8_t byte)
{
    //shift byte MSB first
    for (uint8_t i = 0; i < 8; i++)
    {
        if (byte & 0x80)
            PORTB |= _BV(DISPLAY_DS_PIN);
        else
            PORTB &= ~_BV(DISPLAY_DS_PIN);
        PORTB |= _BV(DISPLAY_SHCP_PIN);
        PORTB &= ~_BV(DISPLAY_SHCP_PIN);
        byte <<= 1;
    }
}

void display_write_row(void)
{
    //shift out buffer first, row mask last
    uint8_t row = display_buffer[current_row];
    display_shift_byte(row);
    display_shift_byte(row_mask);

    //toggle STCP
    PORTB |= _BV(DISPLAY_STCP_PIN);
    PORTB &= ~_BV(DISPLAY_STCP_PIN);

    current_row++;
    current_row &= 0x7;
    row_mask >>= 1;
    row_mask |= 0x80;
    if (row_mask == 0xFF)
        row_mask = 0x7F;
}
