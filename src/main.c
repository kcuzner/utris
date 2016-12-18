/**
 * 1-Wire LED
 */

#include "display.h"

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

/**
 * Fuse data for this program to work properly on the ATTiny13A
 */
#ifdef __AVR_ATtiny13A__
FUSES = {
    .low = (FUSE_SPIEN & FUSE_SUT0 & FUSE_CKSEL0), //9.6MHz, no ckdiv8
    .high = (FUSE_BODLEVEL1) //brown out at 2.7V
};
#else
#error "No fuses defined for selected processor"
#endif

static uint8_t display[8] = {
    0x82,
    0xC6,
    0xE0,
    0xF0,
    0xF8,
    0xFC,
    0xFE,
    0xFF
};

int main(void)
{
    display_init(display);

    TCCR0A = 0;
    TCCR0B = (1<<CS02) | (1<<CS00); //clock source is clk/64
    TIMSK0 = (1<<TOIE0); //enable all interrupts for the timer

    sei();

    while(1)
    {
        display_write_row();
    }

    return 0;
}

ISR(TIM0_OVF_vect)
{
}

