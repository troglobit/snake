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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "snake.h"

char keys[NUM_KEYS] = {'o', 'p', 'a', 'z'};

int score, snake_length, speed, obstacles, level, firstpress, high_score = 0;
char screen_grid[MAXROW][MAXCOL];
snake_segment_t snake[100];

void alarm_handler (int signal __attribute__ ((unused)))
{
   static struct itimerval val;

   val.it_value.tv_sec  = 0;
   val.it_value.tv_usec = 200000 - level * 10000;

   setitimer (ITIMER_REAL, &val, NULL);
} 

void show_score (void)
{
   textcolor (LIGHTCYAN);
   gotoxy (2, MAXROW + 3);
   printf ("Level: %05d", level);

   gotoxy (40, MAXROW + 3);
   textcolor (LIGHTGREEN);
   printf ("Score: %05d", score);

   gotoxy (60, MAXROW + 3);
   textcolor (LIGHTMAGENTA);
   printf ("High Score: %05d", high_score);
}

void draw_line (int col, int row)
{
   int i;

   gotoxy (col, row);
   textcolor (LIGHTBLUE);

   for (i = 0; i < MAXCOL + 2; i++)
   {
      printf ("=");
   }
}

direction_t setup_level (void)
{
   int i, row, col;

   /* Set up global variables for new level */
   snake_length = level + 4;

   firstpress = 1;

   /* Fill grid with blanks */
   for (row = 0; row < MAXROW; row++)
   {
      for (col = 0; col < MAXCOL; col++)
      {
         screen_grid[row][col] = ' ';
      }
   }

   /* Fill grid with Xs and food */
   for (i = 0; i < obstacles * 2; i++)
   {
      row = rand () % MAXROW;
      col = rand () % MAXCOL;

      if (i < obstacles)
      {
         screen_grid[row][col] = CACTUS;
      }
      else
      {
         screen_grid[row][col] = GOLD;
      }
   }

   /* Create snake array of length snake_length */
   for (i = 0; i < snake_length; i++)
   {
      snake[i].row = START_ROW;
      snake[i].col = START_COL + i;
   }

   /* Draw playing board */
   draw_line (1, 2);

   for (row = 0; row < MAXROW; row++)
   {
      gotoxy (1, row + 3);

      textcolor (LIGHTBLUE);
      printf ("|");

      textcolor (WHITE);
      for (col = 0; col < MAXCOL; col++)
      {
         printf ("%c", screen_grid[row][col]);
      }

      textcolor (LIGHTBLUE);
      printf ("|");
   }

   draw_line (1, MAXROW + 3);

   show_score ();

   gotoxy (1, 1);
   textcolor (LIGHTRED);
   printf ("Micro Snake -- Key Left: %c, Right: %c, Up: %c, Down: %c, Exit: x. Any key to start",
           keys[LEFT], keys[RIGHT], keys[UP], keys[DOWN]);

   return RIGHT;
}

void add_segment (direction_t dir)
{
   switch (dir)
   {
      case RIGHT:
         snake[snake_length].row = snake[snake_length - 1].row;
         snake[snake_length].col = snake[snake_length - 1].col + 1;
         break;

      case LEFT:
         snake[snake_length].row = snake[snake_length - 1].row;
         snake[snake_length].col = snake[snake_length - 1].col - 1;
         break;

      case UP:
         snake[snake_length].row = snake[snake_length - 1].row - 1;
         snake[snake_length].col = snake[snake_length - 1].col;
         break;

      case DOWN:
         snake[snake_length].row = snake[snake_length - 1].row + 1;
         snake[snake_length].col = snake[snake_length - 1].col;
         break;

      default:
         /* NOP */
         break;
   }
}


int main (void)
{
   int i;
   char keypress;
   sigset_t set;
   struct sigaction action;
   direction_t dir;

   srand (getpid ());
   if (WEXITSTATUS(system ("stty cbreak -echo stop u")))
   {
      return 1;
   }

   /* Set up signal set with just SIGALRM. */
   sigemptyset(&set);
   sigaddset(&set, SIGALRM);

   /* Trap SIGALRM. */
   sigemptyset(&action.sa_mask);
   sigaddset(&action.sa_mask, SIGALRM);
   action.sa_flags = 0;
   action.sa_handler = alarm_handler;
   sigaction(SIGALRM, &action, NULL);

   /* Call it once to start the timer. */
   alarm_handler (0);

   do                            /* restart game loop */
   {
      obstacles = 4;
      level = 1;
      score = 0;
      speed = 14;

      dir = setup_level ();

      /* main loop */
      do
      {
         /* If key has been hit, then check it is a direction key - if so,
          * change direction */
         keypress = (char)getchar ();
         if (keypress == keys[RIGHT])
            dir = RIGHT;
         else if (keypress == keys[LEFT])
            dir = LEFT;
         else if (keypress == keys[UP])
            dir = UP;
         else if (keypress == keys[DOWN])
            dir = DOWN;

         /* Add a segment to the end of the snake */
         add_segment (dir);

         /* Blank last segment of snake */
         gotoxy (snake[0].col, snake[0].row + 1);
         printf (" ");

         /* ... and remove it from the array */
         for (i = 1; i <= snake_length; i++)
         {
            snake[i - 1] = snake[i];
         }

         /* Display snake in yellow */
         textcolor (YELLOW);
         for (i = 0; i <= snake_length; i++)
         {
            gotoxy (snake[i].col, snake[i].row + 1);
            printf ("O");
         }

         /* keeps cursor flashing in one place instead of following snake */
         gotoxy (1, 1);

         /* Collision detection - walls (bad!) */
         if ((snake[snake_length - 1].row > MAXROW + 1) ||
             (snake[snake_length - 1].row <= 1) ||
             (snake[snake_length - 1].col > MAXCOL + 1) || 
             (snake[snake_length - 1].col <= 1) ||
             /* Collision detection - obstacles (bad!) */
             (screen_grid[snake[snake_length - 1].row - 2][snake[snake_length - 1].col - 2] == CACTUS))
         {
            keypress = 'x';      /* i.e. exit loop - game over */
         }

         /* Collision detection - snake (bad!) */
         for (i = 0; i < snake_length - 1; i++)
         {
            if ((snake[snake_length - 1].row) == (snake[i].row) && (snake[snake_length - 1].col) == (snake[i].col))
            {
               keypress = 'x';   /* i.e. exit loop - game over */
               break;            /* no need to check any more segments */
            }
         }

         /* Collision detection - food (good!) */
         if (screen_grid[snake[snake_length - 1].row - 2][snake[snake_length - 1].col - 2] == GOLD)
         {
            /* increase score and length of snake */
            score += snake_length * obstacles;
            show_score ();
            snake_length++;
            add_segment (dir);

            /* if length of snake reaches certain size, onto next level */
            if (snake_length == (level + 3) * 2)
            {
               score += level * 1000;
               obstacles += 2;
               level++;          /* add to obstacles */

               if ((level % 5 == 0) && (speed > 1))
               {
                  speed--;       /* increase speed every 5 levels */
               }

               setup_level ();   /* display next level */
            }
         }
      }
      while (keypress != 'x');

      if (score > high_score)
      {
         /* New high score! */
         high_score = score;
      }

      show_score ();

      gotoxy (30, 6);
      textcolor (LIGHTRED);
      printf ("G A M E   O V E R");

      gotoxy (30, 9);
      textcolor (YELLOW);
      printf ("Another Game (y/n)? ");

      do
      {
         keypress = getchar ();
      }
      while ((keypress != 'y') && (keypress != 'n'));
   }
   while (keypress == 'y');

   return WEXITSTATUS(system ("stty sane"));
}


/**
 * Local Variables:
 *  version-control: t
 *  c-file-style: "ellemtel"
 * End:
 */
