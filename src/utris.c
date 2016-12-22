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
 * Single-buffered display. We have a low rate game, so it shouldn't get
 * in the way.
 *
 * Double buffering was a 9-byte RAM investment with little return.
 */
uint8_t display[8];

#define PIECE_SIZE 3

/**
 * Format:
 * 1 Byte: orientation mask
 * ---For each orientation:
 *   1 Byte: [7:4] Width of piece, [3:0] Height of piece
 *   PIECE_SIZE Bytes: Orientation bitmap, origin LSB
 *
 * Every piece must have four orientations!
 */
static const uint8_t PROGMEM pieces[] = {
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
    //I-tromino, orientation 1
    0x13,
    0x01,
    0x01,
    0x01,
    //I-tromino, orientation 2
    0x31,
    0x07,
    0x00,
    0x00,
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
    &pieces[(PIECE_SIZE + 1)*4]
};

#define INIT_POS 0x03
#define INIT_POS_NEWPIECE 0xF3 //preempt a y+1 move
#define INIT_LEVEL 0x80

typedef enum { COLLISION_NONE, COLLISION_BOTTOM, COLLISION_SIDE } CollisionDirection;

typedef union {
    uint16_t all;
    struct {
        unsigned x:4; //avr-gcc packs this lsb-first
        unsigned y:4;
        uint8_t o;
    };
} Point;

static uint8_t level;
static Point pos;
static Point lastPos;
static uint8_t piece;

void utris_init(void)
{
    display_init(display);
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

static CollisionDirection utris_check_piece_bounds(void)
{
    uint8_t *pieceStart = (uint8_t *)pgm_read_word(&pieceMap[piece]);
    uint8_t *orientationStart = pieceStart + ((PIECE_SIZE + 1) * pos.o);
    uint8_t size = pgm_read_byte(&orientationStart[0]);
    uint8_t width = size >> 4;
    uint8_t height = size & 0x0F;
    //the following takes advantage of 0-1=0xF > 8. Will not work if x or y
    //is less than 4 bits wide.
    if (pos.x + width > 8)
        return COLLISION_SIDE;
    if (pos.y + height > 8)
        return COLLISION_BOTTOM;

    return COLLISION_NONE;
}

static uint8_t piece_buffer[PIECE_SIZE];
/**
 * Blits the pice onto the passed buffer
 */
static void utris_blit_piece(uint8_t *dest)
{
    uint8_t *pieceStart = (uint8_t *)pgm_read_word(&pieceMap[piece]);
    uint8_t *orientationStart = pieceStart + ((PIECE_SIZE + 1) * pos.o);

    for (uint8_t i = 0; i < PIECE_SIZE; i++)
    {
        piece_buffer[i] = pgm_read_byte(&orientationStart[1+i]) << pos.x;
    }

    utris_blit(&dest[pos.y], piece_buffer, PIECE_SIZE);
}

/**
 * Checks for collisions between the two passed 8-byte buffers,
 * returning 0 if there are collisions and 1 if there is no intersection
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

static __attribute__ ((noinline)) void utris_new_piece(void)
{
    pos.all = lastPos.all = INIT_POS;
    piece ^= 1;
}

static void utris_row_reduce(void)
{
    for (int8_t i = 0; i < 8; i++)
    {
        if (board[i] == 0xFF)
        {
            board[0] = 0;
            for (int8_t j = i; j > 0; j--)
            {
                board[j] = board[j-1];
            }
            level++;
        }
    }
}

void utris_start(void)
{
    utris_clear(board);
    utris_new_piece();
    level = INIT_LEVEL;
}

void utris_tick(void)
{
    static uint8_t count = 0;

    uint8_t *buf = display;
    utris_clear(buf);

    //if the piece is out of bounds on the bottom or collides with the
    //board, then the piece at its previous position is added to the
    //board.
    //
    //if the piece is out of bounds on the side then we just reverse the
    //position and render it there.

    CollisionDirection dir = utris_check_piece_bounds();
    utris_blit_piece(buf); //yes it could have been blitted outside
    if (dir != COLLISION_NONE || !utris_check_collisions(board, buf))
    {
        pos = lastPos;
        utris_clear(buf);
        utris_blit_piece(buf);
        if (dir != COLLISION_SIDE)
        {
            if (!utris_check_collisions(board, buf))
            {
                //can't add to background? Game over!
                utris_start();
            }
            else
            {
                //actually add it to the background
                utris_blit(board, buf, 8);
                //it fit in the background, so get a new piece
                utris_new_piece();
                utris_row_reduce();
            }
        }
    }
    utris_blit(buf, board, 8);

    //make any downward move, if needed
    if (count-- != level)
        return;
    count = 0xFF;

    utris_command(UTRIS_DOWN); //they could press the button before we loop again and "skip over" parts of the board by dropping a lastPos
}

void utris_command(UtrisCommand command)
{
    if (command == UTRIS_NONE)
        return;

    lastPos = pos;
    switch (command)
    {
        case UTRIS_ROTATE:
            pos.o++;
            return;
        case UTRIS_DOWN:
            pos.y++;
            return;
        case UTRIS_LEFT:
            pos.x--;
            return;
        case UTRIS_RIGHT:
            pos.x++;
            return;
        default:
            return;
    }
}

