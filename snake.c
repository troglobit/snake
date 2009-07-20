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

#ifdef DEBUG
#define DBG(fmt, args...) fprintf (stderr, fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

/* Default 0.2 sec between snake movement. */
unsigned int usec_delay = DEFAULT_DELAY;

void alarm_handler (int signal __attribute__ ((unused)))
{
   static struct itimerval val;

   if (!signal)
   {
      sigset_t set;
      struct sigaction action;

      /* Set up signal set with just SIGALRM. */
      sigemptyset(&set);
      sigaddset(&set, SIGALRM);

      /* Trap SIGALRM. */
      sigemptyset(&action.sa_mask);
      sigaddset(&action.sa_mask, SIGALRM);
      action.sa_flags = 0;
      action.sa_handler = alarm_handler;
      sigaction(SIGALRM, &action, NULL);
   }

   val.it_value.tv_sec  = 0;
   val.it_value.tv_usec = usec_delay;

   setitimer (ITIMER_REAL, &val, NULL);
} 

void show_score (screen_t *screen)
{
   textcolor (LIGHTCYAN);
   gotoxy (2, MAXROW + 3);
   printf ("Level: %05d", screen->level);

   gotoxy (20, MAXROW + 3);
   printf ("Gold Left: %05d", screen->gold);

   gotoxy (40, MAXROW + 3);
   textcolor (LIGHTGREEN);
   printf ("Score: %05d", screen->score);

   gotoxy (60, MAXROW + 3);
   textcolor (LIGHTMAGENTA);
   printf ("High Score: %05d", screen->high_score);
}

void draw_line (int col, int row)
{
   int i;

   gotoxy (col, row);
   textcolor (LIGHTBLUE);

   for (i = 0; i < MAXCOL + 2; i++)
   {
      if (i == 0 || i == MAXCOL + 1)
         printf ("+");
      else
         printf ("-");
   }
}

/* If level==0 then just move on to the next level
 * if level==1 restart game
 * Otherwise start game at that level. */
void setup_level (screen_t *screen, snake_t *snake, char keys[], int level)
{
   int i, row, col;

   srand (getpid ());

   /* Initialize on (re)start */
   if (1 == level)
   {
      screen->obstacles = 4;
      screen->level = 1;
      screen->score = 0;
      snake->speed = 14;
   }
   else
   {
      screen->score += screen->level * 1000;
      screen->obstacles += 2;
      screen->level++;          /* add to obstacles */

      if ((screen->level % 5 == 0) && (snake->speed > 1))
      {
         snake->speed--;        /* increase snake->speed every 5 levels */
      }
   }

   /* Set up global variables for new level */
   screen->gold = 0;
   snake->len = level + 4;
   usec_delay = DEFAULT_DELAY - level * 10000;

   /* Fill grid with blanks */
   for (row = 0; row < MAXROW; row++)
   {
      for (col = 0; col < MAXCOL; col++)
      {
         screen->grid[row][col] = ' ';
      }
   }

   /* Fill grid with objects */
   for (i = 0; i < screen->obstacles * 2; i++)
   {
      row = rand () % MAXROW;
      col = rand () % MAXCOL;

      if (i < screen->obstacles)
      {
         screen->grid[row][col] = CACTUS;
      }
      else
      {
         screen->gold++;
         screen->grid[row][col] = GOLD;
      }
   }

   /* Create snake array of length snake->len */
   for (i = 0; i < snake->len; i++)
   {
      snake->body[i].row = START_ROW;
      snake->body[i].col = START_COL + i;
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
         printf ("%c", screen->grid[row][col]);
      }

      textcolor (LIGHTBLUE);
      printf ("|");
   }

   draw_line (1, MAXROW + 3);

   show_score (screen);

   gotoxy (1, 1);
   textcolor (LIGHTRED);
   printf ("Micro Snake -- Key Left: %c, Right: %c, Up: %c, Down: %c, Exit: x. Any key to start",
           keys[LEFT], keys[RIGHT], keys[UP], keys[DOWN]);

   snake->dir = RIGHT;
}

void move (snake_t *snake)
{
   switch (snake->dir)
   {
      case RIGHT:
         snake->body[snake->len].row = snake->body[snake->len - 1].row;
         snake->body[snake->len].col = snake->body[snake->len - 1].col + 1;
         break;

      case LEFT:
         snake->body[snake->len].row = snake->body[snake->len - 1].row;
         snake->body[snake->len].col = snake->body[snake->len - 1].col - 1;
         break;

      case UP:
         snake->body[snake->len].row = snake->body[snake->len - 1].row - 1;
         snake->body[snake->len].col = snake->body[snake->len - 1].col;
         break;

      case DOWN:
         snake->body[snake->len].row = snake->body[snake->len - 1].row + 1;
         snake->body[snake->len].col = snake->body[snake->len - 1].col;
         break;

      default:
         /* NOP */
         break;
   }
}


void update (snake_t *snake)
{
   int i;

   /* Blank last segment of snake */
   gotoxy (snake->body[0].col, snake->body[0].row + 1);
   printf (" ");

   /* ... and remove it from the array */
   for (i = 1; i <= snake->len; i++)
   {
      snake->body[i - 1] = snake->body[i];
   }

   /* Display snake in yellow */
   textcolor (YELLOW);
   for (i = 0; i <= snake->len; i++)
   {
      gotoxy (snake->body[i].col, snake->body[i].row + 1);
      printf ("O");
   }
}

int collide_walls (snake_t *snake)
{
   snake_segment_t *head = &snake->body[snake->len - 1];

   if ((head->row > MAXROW + 1) || (head->row <= 1) ||
       (head->col > MAXCOL + 1) || (head->col <= 1))
   {
      DBG("Wall collision.\n");
      return 1;
   }

   return 0;
}

int collide_object (snake_t *snake, screen_t *screen, char object)
{
   snake_segment_t *head = &snake->body[snake->len - 1];
   
   if (screen->grid[head->row - 2][head->col - 2] == object)
   {
      DBG("Object '%c' collision.\n", object);
      return 1;
   }

   return 0;
}

int collide_self (snake_t *snake)
{
   int i;
   snake_segment_t *head = &snake->body[snake->len - 1];

   for (i = 0; i < snake->len - 1; i++)
   {
      snake_segment_t *body = &snake->body[i];

      if (head->row == body->row && head->col == body->col)
      {
         DBG("Self collision.\n");
         return 1;
      }
   }

   return 0;
}

int main (void)
{
   char keypress;
   snake_t snake;
   screen_t screen;
   char keys[NUM_KEYS] = DEFAULT_KEYS;

   if (WEXITSTATUS(system ("stty cbreak -echo stop u")))
   {
      fprintf (stderr, "Failed setting up the screen, is 'stty' missing?\n");
      return 1;
   }

   /* Call it once to initialize the timer. */
   alarm_handler (0);

   do                            /* restart game loop */
   {
      setup_level (&screen, &snake, keys, 1);

      /* main loop */
      do
      {
         /* If key has been hit, then check it is a direction key - if so,
          * change direction */
         keypress = (char)getchar ();
         if (keypress == keys[RIGHT])
            snake.dir = RIGHT;
         else if (keypress == keys[LEFT])
            snake.dir = LEFT;
         else if (keypress == keys[UP])
            snake.dir = UP;
         else if (keypress == keys[DOWN])
            snake.dir = DOWN;

         /* Move the snake one position. */
         move (&snake);

         update (&snake);

         /* keeps cursor flashing in one place instead of following snake */
         gotoxy (1, 1);

         if (collide_walls (&snake) || 
             collide_object (&snake, &screen, CACTUS) ||
             collide_self (&snake))
         {
            keypress = 'x';     /* game over */
         }

         if (collide_object (&snake, &screen, GOLD))
         {
            /* increase score and length of snake */
            screen.gold--;
            screen.score += snake.len * screen.obstacles;
            show_score (&screen);
            snake.len++;
            move (&snake);
            update (&snake);

            if (!screen.gold)
            {
               /* Go to next level */
               setup_level (&screen, &snake, keys, 0);
            }
         }
      }
      while (keypress != 'x');

      if (screen.score > screen.high_score)
      {
         /* New high score! */
         screen.high_score = screen.score;
      }

      show_score (&screen);

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

   clrscr ();

   return WEXITSTATUS(system ("stty sane"));
}


/**
 * Local Variables:
 *  version-control: t
 *  c-file-style: "ellemtel"
 * End:
 */
