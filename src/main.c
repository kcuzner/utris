/**
 * 1-Wire LED
 */

#include "utris.h"
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

#define FLAG_UTRIS_TICK 0x1

static uint8_t flags;

int main(void)
{
    utris_init();

    TCCR0A = 0;
    TIMSK0 = (1<<TOIE0); //enable all interrupts for the timer
    TCCR0B = (1<<CS02); //clock source is clk/256

    utris_start();

    sei();

    while(1)
    {
        if (flags & FLAG_UTRIS_TICK)
        {
            utris_tick();
        }
        flags = 0;
        display_write_row();
    }

    return 0;
}

ISR(TIM0_OVF_vect)
{
    flags |= FLAG_UTRIS_TICK;
}

