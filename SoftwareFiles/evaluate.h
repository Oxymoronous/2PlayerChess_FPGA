#ifndef EVALUATE_H_
#define EVALUATE_H_

#include <system.h>
#include <alt_types.h>
#include "statusbar.h"

#define PAWN   0
#define KNIGHT 1
#define BISHOP 2
#define ROOK   3
#define QUEEN  4
#define KING   5

#define WHITE_PAWN      (2*PAWN   + WHITE)			//0
#define BLACK_PAWN      (2*PAWN   + BLACK)			//1
#define WHITE_KNIGHT    (2*KNIGHT + WHITE)			//2
#define BLACK_KNIGHT    (2*KNIGHT + BLACK)			//3
#define WHITE_BISHOP    (2*BISHOP + WHITE)
#define BLACK_BISHOP    (2*BISHOP + BLACK)
#define WHITE_ROOK      (2*ROOK   + WHITE)
#define BLACK_ROOK      (2*ROOK   + BLACK)
#define WHITE_QUEEN     (2*QUEEN  + WHITE)
#define BLACK_QUEEN     (2*QUEEN  + BLACK)
#define WHITE_KING      (2*KING   + WHITE)
#define BLACK_KING      (2*KING   + BLACK)			//11
#define EVAL_EMPTY           (BLACK_KING  +  1)		//12

extern int side2move;
extern int w_evalscore;
extern int b_evalscore;

int convert_piece_rep(alt_u8 VRAM_rep);
void VGA_EvalBoard_Update(int previous_index, int current_index);
void init_tables();
int eval();

#endif
