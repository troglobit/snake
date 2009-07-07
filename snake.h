/* Micro Snake, based on a simple simple snake game by Simon Huggins
 * 
 * Copyright (c) 2003, 2004 Simon Huggins <webmaster@simonhuggins.com>
 * Copyright (c) 2009, Joachim Nilsson <joachim.nilsson@member.fsf.org>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Original Borland Builder C/C++ snake code available at Simon's home page
 * http://www.simonhuggins.com/courses/cbasics/course_notes/snake.htm
 */

#ifndef __SNAKE_H__
#define __SNAKE_H__

#define MAXROW    15
#define MAXCOL    77
#define START_COL 33
#define START_ROW  7

/* Esc[Line;ColumnH     - Moves the cursor to the specified position (coordinates) */
#define gotoxy(x,y)       printf("\033[%d;%dH", y, x)
/* Esc[Value;...;Valuem - Set Graphics Mode */
#define textcolor(color)  printf("\033[%sm", color)

#define BLACK         "0;30"
#define LIGHTRED      "1;31"    /* Red */
#define LIGHTGREEN    "1;32"    /* Green */
#define YELLOW        "0;33"
#define LIGHTBLUE     "1;34"    /* Blue */
#define LIGHTMAGENTA  "1;35"    /* Magenta */
#define LIGHTCYAN     "1;36"    /* Cyan */
#define WHITE         "0;37"

typedef enum { LEFT=0, RIGHT, UP, DOWN, NUM_KEYS } direction_t;

typedef struct snake_segment
{
   int row, col;
} snake_segment_t;

#endif /* __SNAKE_H__ */

/**
 * Local Variables:
 *  version-control: t
 *  c-file-style: "ellemtel"
 * End:
 */
