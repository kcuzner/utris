/**
 * uTris
 *
 * Main gameplay routines
 *
 * Kevin Cuzner
 */

#include "utris.h"

#include "display.h"

#include <avr/pgmspace.h>

static uint8_t board[8];

/**
 * Double buffered display
 *
 * The unused buffer is used as a scratchpad for potential moves
 */
static uint8_t display[2][8];
static uint8_t scratchpad;

#define PIECE_SIZE 3

/**
 * Format:
 * 1 Byte: Number of orientations
 * ---For each orientation:
 *   1 Byte: [7:4] Width of piece, [3:0] Height of piece
 *   PIECE_SIZE Bytes: Orientation bitmap, origin LSB
 */
static const uint8_t PROGMEM pieces[] = {
    //L-tromino
    0x4,
    //L-tromino, orientation 1
    0x22,
    0x01,
    0x03,
    0x00,
    //L-tromino, orientation 2
    0x22,
    0x02,
    0x03,
    0x00,
    //L-tromino, orientation 3
    0x22,
    0x03,
    0x02,
    0x00,
    //L-tromino, orientation 4
    0x22,
    0x03,
    0x01,
    0x00,
    //I-tromino
    0x2,
    //I-tromino, orientation 1
    0x13,
    0x01,
    0x01,
    0x01,
    //I-tromino, orientation 2
    0x31,
    0x07,
    0x00,
    0x00
};

static const uint8_t PROGMEM * const pieceMap[] = {
    &pieces[0],
    &pieces[1 + (PIECE_SIZE + 1)*4]
};

#define INIT_POSX 3;
#define INIT_POSY 0;

static uint8_t level = 0x80;
static uint8_t posX = INIT_POSX;
static uint8_t posY = INIT_POSY;
static uint8_t piece;
static uint8_t orientation;
static uint16_t rows;

void utris_init(void)
{
    display_init(display[0]);
    scratchpad = 1;
}

static void utris_clear(uint8_t *buf)
{
    for (uint8_t i = 0; i < 8; i++)
        buf[i] = 0;
}

/**
 * Blits src onto dest, ORing the results
 */
static void utris_blit(uint8_t *dest, uint8_t *src, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        dest[i] |= src[i];
    }
}

static uint8_t piece_buffer[PIECE_SIZE];
/**
 * Attempts to blit the piece onto the passed desination buffer
 *
 * Returns 0 if the piece will not fit fully within the buffer
 */
static uint8_t utris_try_blit_piece(uint8_t *dest)
{
    uint8_t *pieceStart = (uint8_t *)pgm_read_word(&pieceMap[piece]);
    uint8_t *orientationStart = pieceStart + 1 + ((PIECE_SIZE + 1) * orientation);
    uint8_t size = pgm_read_byte(&orientationStart[0]);
    uint8_t width = size >> 4;
    uint8_t height = size & 0x0F;
    if (posX + width > 8)
        return 0;
    if (posX > 8) //catches negative numbers
        return 0;
    if (posY + height > 8)
        return 0;
    if (posY > 8) //catches negative numbers
        return 0;

    for (uint8_t i = 0; i < PIECE_SIZE; i++)
    {
        piece_buffer[i] = pgm_read_byte(&orientationStart[1+i]) << posX;
    }

    utris_blit(&dest[posY], piece_buffer, PIECE_SIZE);

    return 1;
}

/**
 * Checks for collisions between the two passed 8-byte buffers
 */
static uint8_t utris_check_collisions(uint8_t *a, uint8_t *b)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if ((~(a[i] ^ b[i])) & a[i])
            return 0;
    }

    return 1;
}

void utris_start(void)
{
    posX = posY = rows = 0;
    piece = 0;
}

void utris_tick(void)
{
    static uint8_t count = 0;

    if (count-- != level)
        return;

    count = 0xFF;

    uint8_t *buf = display[scratchpad];
    utris_clear(buf);
    utris_try_blit_piece(buf);
    display_set_buffer(buf);
    posX++;
    if (posX == 7)
    {
        posX = 0;
        posY++;
        if (posY == 7)
        {
            posY = 0;
        }
    }
    orientation++;
    orientation &= 0x3;
    scratchpad ^= 1;

    /*if (count == level)
    {
        count = 0xFF;
        uint8_t *buf = display[scratchpad];
        utris_clear(buf);
        //attempt to move the piece downwards
        posY++;
        if (!utris_try_blit_piece(buf) || !utris_check_collisions(board, buf))
        {
            //reverse move, add it to the background
            utris_clear(buf);
            posY--;
            if (!utris_try_blit_piece(buf))
            {
                //game over!
                posX = INIT_POSX;
                posY = INIT_POSY;
                utris_clear(board);
                level = 0;
                return;
            }
            utris_blit(board, buf, 8);
        }
        display_set_buffer(display[scratchpad]);
        scratchpad ^= 1;
    }*/
}

void utris_command(UtrisCommand command)
{
}
