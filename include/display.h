/**
 * uTris
 *
 * Dual shift register 8x8 dot matrix display writing routines
 *
 * Kevin Cuzner
 */
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <avr/io.h>

#define DISPLAY_DS_PIN PB0
#define DISPLAY_STCP_PIN PB1
#define DISPLAY_SHCP_PIN PB2

/**
 * Initializes the display
 *
 * buffer: Pointer to initial display buffer (uint8_t[8])
 */
void display_init(uint8_t *buffer);

/**
 * Sets the current display buffer, resetting the current row
 *
 * buffer: Pointer to display buffer (uint8_t[8])
 */
void display_set_buffer(uint8_t *buffer);

/**
 * Displays a new row
 */
void display_write_row(void);

#endif //_DISPLAY_H_

