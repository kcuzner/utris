/**
 * uTris
 *
 * Main gameplay routines
 *
 * Kevin Cuzner
 */

#ifndef _UTRIS_H_
#define _UTRIS_H_

#include <stdint.h>

typedef enum { UTRIS_ROTATE, UTRIS_LEFT, UTRIS_RIGHT, UTRIS_DOWN } UtrisCommand;

void utris_init(void);

void utris_start(void);

void utris_tick(void);

void utris_command(UtrisCommand command);


#endif //_UTRIS_H_

