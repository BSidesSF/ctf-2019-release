#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#include "blocks.h"

#define disable_buffering(_fd) setvbuf(_fd, NULL, _IONBF, 0)

/* Note: It's important that these are prime numbers, otherwise it might be
 * impossible to build shellcode. */
#define BOARD_WIDTH  10
#define BOARD_HEIGHT 21
#define BOARD_SIZE (BOARD_WIDTH * BOARD_HEIGHT / 8)

#define FILLED "#"
#define PIECE  "*"
#define BLANK  " "

typedef struct {
  /* Treat the board as a 1d array */
  uint8_t board[BOARD_SIZE];
  piece_t piece;
  int8_t piece_x, piece_y;
  int8_t piece_rotation;
  uint32_t score;
  int8_t force;
  int8_t ticked;
  int8_t debug;

  pthread_t ui_thread;
} game_t;

/* Global game object */
game_t game;

void print_board() {
  size_t i;

  for(i = 0; i < BOARD_SIZE; i++) {
    printf("%02x", game.board[i]);
  }
  printf("\n");
}

void set_square(int x, int y, int on) {
  int byte = ((y * BOARD_WIDTH) + x) / 8;
  int bit  = ((y * BOARD_WIDTH) + x) % 8;

  if(on) {
    game.board[byte] = game.board[byte] | (1 << bit);
  } else {
    game.board[byte] = game.board[byte] & ~(1 << bit);
  }
}

int8_t get_square(int x, int y) {
  int byte = ((y * BOARD_WIDTH) + x) / 8;
  int bit = ((y * BOARD_WIDTH) + x) % 8;

  return (game.board[byte] & (1 << bit)) ? 1 : 0;
}

piece_structure_t real_piece() {
  piece_structure_t base;
  switch(game.piece_rotation) {
    case 0:
      base = pieces_r0[game.piece];
      break;
    case 1:
      base = pieces_r1[game.piece];
      break;
    case 2:
      base = pieces_r2[game.piece];
      break;
    case 3:
      base = pieces_r3[game.piece];
      break;
  }

   return (piece_structure_t) {
     base.a_x + game.piece_x, base.a_y + game.piece_y,
     base.b_x + game.piece_x, base.b_y + game.piece_y,
     base.c_x + game.piece_x, base.c_y + game.piece_y,
     base.d_x + game.piece_x, base.d_y + game.piece_y,
   };
}

void draw_board() {
  size_t i, j;

  /* Superimpose the current piece on it */
  piece_structure_t current_piece = real_piece();

  /* "Clear" the board */
  printf("Score: %d\n\n", game.score);

  /* Print the header */
  printf("+");
  for(j = 0; j < BOARD_WIDTH; j++) {
    printf("-");
  }
  printf("+\n");

  /* Print the board */
  for(i = 0; i < BOARD_HEIGHT; i++) {
    printf("|");
    for(j = 0; j < BOARD_WIDTH; j++) {
      if(get_square(j, i)) {
        printf(FILLED);
      } else if (current_piece.a_x == j && current_piece.a_y == i) {
        printf(PIECE);
      } else if (current_piece.b_x == j && current_piece.b_y == i) {
        printf(PIECE);
      } else if (current_piece.c_x == j && current_piece.c_y == i) {
        printf(PIECE);
      } else if (current_piece.d_x == j && current_piece.d_y == i) {
        printf(PIECE);
      } else {
        printf(BLANK);
      }
    }
    printf("|");
    printf("\n");
  }

  /* Print the footer */
  printf("+");
  for(j = 0; j < BOARD_WIDTH; j++) {
    printf("-");
  }
  printf("+\n");
  if(game.debug) {
    print_board();
  }
}

int8_t collision_left() {
  piece_structure_t piece = real_piece();

  /* Check if it's off the end. */
  if(piece.a_x - 1 < 0 || piece.b_x - 1 < 0 || piece.c_x - 1 < 0 || piece.d_x - 1 < 0) {
    return 1;
  }

  /* Check if it's gonna hit another piece */
  if(get_square(piece.a_x - 1, piece.a_y) || get_square(piece.b_x - 1, piece.b_y) || get_square(piece.c_x - 1, piece.c_y) || get_square(piece.d_x - 1, piece.d_y)) {
    return 1;
  }

  return 0;
}

int8_t collision_right() {
  piece_structure_t piece = real_piece();

  /* Check if it's off the end. */
  if(piece.a_x + 1 >= BOARD_WIDTH || piece.b_x + 1 >= BOARD_WIDTH || piece.c_x + 1 >= BOARD_WIDTH || piece.d_x + 1 >= BOARD_WIDTH) {
    return 1;
  }

  if(get_square(piece.a_x + 1, piece.a_y) || get_square(piece.b_x + 1, piece.b_y) || get_square(piece.c_x + 1, piece.c_y) || get_square(piece.d_x + 1, piece.d_y)) {
    return 1;
  }

  return 0;
}

int8_t collision_down() {
  piece_structure_t piece = real_piece();

  /* Check if it's off the bottom */
  if(piece.a_y + 1 >= BOARD_HEIGHT || piece.b_y + 1 >= BOARD_HEIGHT || piece.c_y + 1 >= BOARD_HEIGHT || piece.d_y + 1 >= BOARD_HEIGHT) {
    return 1;
  }

  /* Check if it's hitting another piece */
  if(get_square(piece.a_x, piece.a_y + 1) || get_square(piece.b_x, piece.b_y + 1) || get_square(piece.c_x, piece.c_y + 1) || get_square(piece.d_x, piece.d_y + 1)) {
    return 1;
  }

  return 0;
}

int8_t collision_rotate_r() {
  int8_t rotation = game.piece_rotation;

  /* Temporarily rotate. */
  game.piece_rotation = (game.piece_rotation + 1) % 4;

  /* Get the structure. */
  piece_structure_t piece = real_piece();

  /* Rotate back. */
  game.piece_rotation = rotation;

  if(piece.a_x < 0 || piece.b_x < 0 || piece.c_x < 0 || piece.d_x < 0) {
    return 1;
  }

  if(piece.a_x >= BOARD_WIDTH || piece.b_x >= BOARD_WIDTH || piece.c_x >= BOARD_WIDTH || piece.d_x >= BOARD_WIDTH) {
    return 1;
  }

  /* Check if there are any overlaps */
  if(get_square(piece.a_x, piece.a_y) || get_square(piece.b_x, piece.b_y) || get_square(piece.c_x, piece.c_y) || get_square(piece.d_x, piece.d_y)) {
    return 1;
  }

  return 0;
}

int8_t collision_rotate_l() {
  int8_t rotation = game.piece_rotation;

  /* Temporarily rotate. */
  game.piece_rotation = game.piece_rotation == 0 ? 3 : game.piece_rotation - 1;

  /* Get the structure. */
  piece_structure_t piece = real_piece();

  /* Rotate back. */
  game.piece_rotation = rotation;

  if(piece.a_x < 0 || piece.b_x < 0 || piece.c_x < 0 || piece.d_x < 0) {
    return 1;
  }

  if(piece.a_x >= BOARD_WIDTH || piece.b_x >= BOARD_WIDTH || piece.c_x >= BOARD_WIDTH || piece.d_x >= BOARD_WIDTH) {
    return 1;
  }

  /* Check if there are any overlaps */
  if(get_square(piece.a_x, piece.a_y) || get_square(piece.b_x, piece.b_y) || get_square(piece.c_x, piece.c_y) || get_square(piece.d_x, piece.d_y)) {
    return 1;
  }

  return 0;
}

static void *ui_thread(void *dummy) {
  for(;;) {
    char move = getchar();

    if(move == 'e') {
      if(collision_rotate_r()) {
        continue;
      }
      game.piece_rotation = (game.piece_rotation + 1) % 4;
    }

    if(move == 'q') {
      if(collision_rotate_l()) {
        continue;
      }
      game.piece_rotation = game.piece_rotation == 0 ? 3 : game.piece_rotation - 1;
    }

    if(move == 'a') {
      /* Make sure we can move it left */
      if(collision_left()) {
        continue;
      }
      game.piece_x--;
    } else if(move == 'd') {
      /* Make sure we can move it right */
      if(collision_right()) {
        continue;
      }
      game.piece_x++;
    } else if(move == 's') {
      game.force = 1;
    }
  }
  return NULL;
}

void write_piece() {
  piece_structure_t piece = real_piece(game);

  set_square(piece.a_x, piece.a_y, 1);
  set_square(piece.b_x, piece.b_y, 1);
  set_square(piece.c_x, piece.c_y, 1);
  set_square(piece.d_x, piece.d_y, 1);
}

void reroll_piece() {
  game.piece = rand() % 7;
  game.piece_x = 3;
  game.piece_y = 0;
  game.piece_rotation = 0;
}

int8_t row_is_full(int8_t row) {
  size_t i;

  for(i = 0; i < BOARD_WIDTH; i++) {
    if(!get_square(i, row)) {
      return 0;
    }
  }
  return 1;
}

void row_remove(int8_t row) {
  ssize_t i, j;

  for(i = row; i > 0; i--) {
    for(j = 0; j < BOARD_WIDTH; j++) {
      set_square(j, i, get_square(j, i - 1));
    }
  }
}

void handle_lines() {
  ssize_t i;

  int bonus = 1000;
  for(i = BOARD_HEIGHT - 1; i >= 0; i--) {
    while(row_is_full(i)) {
      bonus *= 2;
      row_remove(i);
    }
  }

  game.score += bonus;
}

void tick() {
  game.piece_y++;
}

int main(int argc, char *argv[])
{
  srand(0);

  disable_buffering(stdout);
  disable_buffering(stderr);

  reroll_piece();

  memset(game.board, 0, sizeof(game.board));

#if 0
  set_square(1, 16, 1);
  set_square(6, 16, 1);
  set_square(7, 16, 1);

  set_square(1, 18, 1);
  set_square(4, 18, 1);
  set_square(5, 18, 1);
  set_square(6, 18, 1);
  set_square(7, 18, 1);

  set_square(0, 17, 1);
  set_square(1, 17, 1);
  set_square(2, 17, 1);
  set_square(3, 17, 1);
  set_square(4, 17, 1);
  set_square(5, 17, 1);
  set_square(6, 17, 1);
  set_square(7, 17, 1);
  set_square(8, 17, 1);


  set_square(0, 19, 1);
  set_square(1, 19, 1);
  set_square(2, 19, 1);
  set_square(3, 19, 1);
  set_square(4, 19, 1);
  set_square(5, 19, 1);
  set_square(6, 19, 1);
  set_square(7, 19, 1);
  set_square(8, 19, 1);


  set_square(0, 20, 1);
  set_square(1, 20, 1);
  set_square(2, 20, 1);
  set_square(3, 20, 1);
  set_square(4, 20, 1);
  set_square(5, 20, 1);
  set_square(6, 20, 1);
  set_square(7, 20, 1);
  set_square(8, 20, 1);
#endif
  printf("In case it helps or whatever, system() is at %p and the game object is at %p. :)\n", system, &game);
  //game.board[BOARD_SIZE-9] = 0x00;
  //game.board[BOARD_SIZE-8] = 0x00;
  //game.board[BOARD_SIZE-7] = 0x00;
  //game.board[BOARD_SIZE-6] = 0x00;
  //game.board[BOARD_SIZE-5] = ~'s';
  //game.board[BOARD_SIZE-4] = ~'h';
  //game.board[BOARD_SIZE-3] = ~';';
  //game.board[BOARD_SIZE-2] = 0x00;
  //game.board[BOARD_SIZE-1] = 0x00;

  /* Create a UI thread. */
  if(pthread_create(&(game.ui_thread), NULL, ui_thread, NULL)) {
    printf("Error creating thread\n");
    return 1;
  }

  for(;;) {
    if(game.force) {
      while(!collision_down()) {
        game.ticked = 1;
        game.piece_y++;
      }
      game.force = 0;
    }
    if(collision_down()) {
      if(game.ticked == 0) {
        break;
      }
      write_piece();
      reroll_piece();
      handle_lines();
      game.ticked = 0;
    } else {
      tick();
      game.ticked = 1;
    }

    draw_board();
    usleep(650000);
  }
  pthread_cancel(game.ui_thread);

  /* The intended solution is to change this to system(), and add an offset to game.board */
  memset(game.board, 0, BOARD_SIZE);

  printf("Game over! You got %d points!\n", game.score);

  alarm(60);

  return 0;
}
