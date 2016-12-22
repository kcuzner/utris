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

#define FLAG_TICK 0x1
//the bit order of the following masks is an optimization, they are the same order as the button resistor values
#define FLAG_RIGHT 0x2
#define FLAG_LEFT 0x4
#define FLAG_ROTATE 0x8
#define FLAG_DOWN 0x10

static uint8_t flags;

extern uint8_t display[2][8];

static inline void evaluate_adc_buttons(void)
{
    static uint8_t flagsMask = 0;
    //if (!(ADCSRA & _BV(ADIF)))
    //    return;

    //ADCSRA |= _BV(ADIF);
    uint8_t val = ADCH >> 5;

    //the following magic numbers were tuned to my resistor values
    if (val == 0x7)
    {
        flags |= flagsMask;
        flagsMask = 0;
    }
    else if (val == 0x6)
    {
        flagsMask |= FLAG_DOWN;
    }
    else if (val == 0x4)
    {
        flagsMask |= FLAG_ROTATE;
    }
    else if (val == 0x1)
    {
        flagsMask |= FLAG_LEFT;
    }
    else if (val == 0x0)
    {
        flagsMask |= FLAG_RIGHT;
    }
}

int main(void)
{
    utris_init();

    utris_start();

    //set up clock for utris ticks
    TCCR0A = 0;
    TIMSK0 = _BV(TOIE0); //enable all interrupts for the timer
    TCCR0B = _BV(CS02); //clock source is clk/256

    //set up adc for input aquisition
    ADMUX = _BV(ADLAR) | _BV(MUX1); //ADC2 (PB4) is our input
    ADCSRB = 0; //free-running mode
    DIDR0 = _BV(ADC2D); //disable digital buffer on PB4
    ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE);// | _BV(ADIE); //enabled, interrupts, start conversion, auto trigger

    sei();

    while(1)
    {
        cli();
        evaluate_adc_buttons();
        UtrisCommand command = UTRIS_NONE;
        if (flags & FLAG_RIGHT)
            command = UTRIS_RIGHT;
        else if (flags & FLAG_LEFT)
            command = UTRIS_LEFT;
        else if (flags & FLAG_ROTATE)
            command = UTRIS_ROTATE;
        else if (flags & FLAG_DOWN)
            command = UTRIS_DOWN;
        utris_command(command);

        if (flags & FLAG_TICK)
            utris_tick();

        flags = 0;
        sei();

        display_write_row();
    }

    return 0;
}

ISR(TIM0_OVF_vect)
{
    flags |= FLAG_TICK;
}

/*ISR(ADC_vect)
{
    static uint8_t flagsMask = 0;
    uint8_t val = ADCH >> 5;
    if (val == 0x7)
    {
        flags |= flagsMask;
        flagsMask = 0;
    }
    else if (val == 0x6)
    {
        flagsMask |= FLAG_DOWN;
    }
    else if (val == 0x4)
    {
        flagsMask |= FLAG_ROTATE;
    }
    else if (val == 0x1)
    {
        flagsMask |= FLAG_LEFT;
    }
    else if (val == 0x0)
    {
        flagsMask |= FLAG_RIGHT;
    }
}*/

