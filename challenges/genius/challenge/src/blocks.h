#ifndef __BLOCKS_H__
#define __BLOCKS_H__

#include <stdint.h>

typedef enum {
  BLOCK_I,
  BLOCK_O,
  BLOCK_T,
  BLOCK_S,
  BLOCK_Z,
  BLOCK_J,
  BLOCK_L,
} piece_t;

typedef struct {
  int8_t a_x, a_y, b_x, b_y, c_x, c_y, d_x, d_y;
} piece_structure_t;

piece_structure_t pieces_r0[7] = {
  (piece_structure_t){
    0, 1,
    1, 1,
    2, 1,
    3, 1,
  }, /* BLOCK_I */

  (piece_structure_t){
    0, 0,
    1, 0,
    0, 1,
    1, 1,
  }, /* BLOCK_O */

  (piece_structure_t){
    1, 0,
    0, 1,
    1, 1,
    2, 1,
  }, /* BLOCK_T */

  (piece_structure_t){
    0, 1,
    0, 2,
    1, 0,
    1, 1,
  }, /* BLOCK_S */

  (piece_structure_t){
    0, 0,
    0, 1,
    1, 1,
    1, 2,
  }, /* BLOCK_Z */

  (piece_structure_t){
    2, 1,
    2, 2,
    3, 2,
    4, 2,
  }, /* BLOCK_J */

  (piece_structure_t){
    2, 2,
    2, 1,
    1, 2,
    0, 2,
  }, /* BLOCK_L */
};

piece_structure_t pieces_r1[7] = {
  (piece_structure_t){
    1, 0,
    1, 1,
    1, 2,
    1, 3,
  }, /* BLOCK_I */

  (piece_structure_t){
    0, 0,
    1, 0,
    0, 1,
    1, 1,
  }, /* BLOCK_O */

  (piece_structure_t){
    1, 0,
    1, 1,
    2, 1,
    1, 2,
  }, /* BLOCK_T */

  (piece_structure_t){
    0, 1,
    1, 1,
    1, 2,
    2, 2,
  }, /* BLOCK_S */

  (piece_structure_t){
    0, 2,
    1, 2,
    1, 1,
    2, 1,
  }, /* BLOCK_Z */

  (piece_structure_t){
    2, 2,
    3, 2,
    2, 3,
    2, 4,
  }, /* BLOCK_J */

  (piece_structure_t){
    2, 2,
    2, 0,
    2, 1,
    3, 2,
  }, /* BLOCK_L */
};

piece_structure_t pieces_r2[7] = {
  (piece_structure_t){
    0, 1,
    1, 1,
    2, 1,
    3, 1,
  }, /* BLOCK_I */

  (piece_structure_t){
    0, 0,
    1, 0,
    0, 1,
    1, 1,
  }, /* BLOCK_O */

  (piece_structure_t){
    0, 1,
    1, 1,
    2, 1,
    1, 2,
  }, /* BLOCK_T */

  (piece_structure_t){
    0, 1,
    0, 2,
    1, 0,
    1, 1,
  }, /* BLOCK_S */

  (piece_structure_t){
    0, 0,
    0, 1,
    1, 1,
    1, 2,
  }, /* BLOCK_Z */

  (piece_structure_t){
    0, 2,
    1, 2,
    2, 2,
    2, 3,
  }, /* BLOCK_J */

  (piece_structure_t){
    2, 2,
    2, 3,
    3, 2,
    4, 2,
  }, /* BLOCK_L */
};

piece_structure_t pieces_r3[7] = {
  (piece_structure_t){
    1, 0,
    1, 1,
    1, 2,
    1, 3,
  }, /* BLOCK_I */

  (piece_structure_t){
    0, 0,
    1, 0,
    0, 1,
    1, 1,
  }, /* BLOCK_O */

  (piece_structure_t){
    1, 0,
    0, 1,
    1, 1,
    1, 2,
  }, /* BLOCK_T */

  (piece_structure_t){
    0, 1,
    1, 1,
    1, 2,
    2, 2,
  }, /* BLOCK_S */

  (piece_structure_t){
    0, 2,
    1, 2,
    1, 1,
    2, 1,
  }, /* BLOCK_Z */

  (piece_structure_t){
    2, 0,
    2, 1,
    2, 2,
    1, 2,
  }, /* BLOCK_J */

  (piece_structure_t){
    2, 2,
    1, 2,
    2, 3,
    2, 4,
  }, /* BLOCK_L */
};

#endif
