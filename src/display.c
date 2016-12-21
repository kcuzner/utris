/**
 * uTris
 *
 * Dual shift register 8x8 dot matrix display writing routines
 *
 * Kevin Cuzner
 */

#include "display.h"
#include <avr/io.h>
#include <util/atomic.h>

static uint8_t *display_buffer;
static uint8_t *next_buffer;
static uint8_t current_row = 0;

void display_init(uint8_t *buffer)
{
    DDRB |= _BV(DISPLAY_DS_PIN) | _BV(DISPLAY_STCP_PIN) | _BV(DISPLAY_SHCP_PIN);
    display_set_buffer(buffer);
}

void display_set_buffer(uint8_t *buffer)
{
    next_buffer = buffer;
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
    //shift out columns first, row mask last
    uint8_t columns = display_buffer[current_row];
    display_shift_byte(~columns); //note that the columns are on the cathodes
    display_shift_byte(1 << current_row);

    //toggle STCP
    PORTB |= _BV(DISPLAY_STCP_PIN);
    PORTB &= ~_BV(DISPLAY_STCP_PIN);

    current_row++;
    current_row &= 0x7;
    cli();
    if (current_row == 0 && next_buffer)
    {
        display_buffer = next_buffer;
        /**
         * Note that we could clear the next buffer value to avoid doing this
         * step in every frame after the first call to set_buffer, but that
         * adds several bytes to the program size and isn't strictly necesary.
         */
    }
    sei();
}

