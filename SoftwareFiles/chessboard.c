#include <system.h>
#include <alt_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "altera_avalon_spi.h"
#include "altera_avalon_spi_regs.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "usb_kb/GenericMacros.h"
#include "usb_kb/GenericTypeDefs.h"
#include "usb_kb/HID.h"
#include "usb_kb/MAX3421E.h"
#include "usb_kb/transfer.h"
#include "usb_kb/usb_ch9.h"
#include "usb_kb/USB.h"

#include "chessboard.h"

//bit 0123 = chess_code
//bit 4    = parity_code
//bit 5    = glow code
//bit 6    = pawn_special code
//bit 7	   = pawn_special code 2 (to signify they just moved two tiles up)
static const unsigned char GLOW			= 0b00100000;
static const unsigned char GLOWOFF		= 0b11011111;
static const unsigned char PAWNFIRST	= 0b01000000;
//static const unsigned char PAWNSECOND	= 0b10000000;
static const unsigned char EMPTYCODE	= 0b00001100;

void loop(int i){
	for(int a = 0; a<i ;a++){
		//do nothing
	}
	return;
}

signed short int min(signed short int a, signed short int b){
	if (a < b) return a;
	return b;
}

signed short int max(signed short int a, signed short int b){
	if (a < b) return b;
	return a;
}

int maxint(int a, int b){
	if (a < b) return b;
	return a;
}

int minint(int a, int b){
	if (a < b) return a;
	return b;
}
void print_click(int tile_index){
	printf("Clicked on tile %d! Piece is %d \n", tile_index, vga_ctrl->VRAM[tile_index]);

}

void clear_whitepawn_sp(){
	for(int i=32; i<40; i++){
		vga_ctrl->VRAM[i] &= 0b01111111;
	}
}

void clear_blackpawn_sp(){
	for(int i=24; i<32; i++){
		vga_ctrl->VRAM[i] &= 0b01111111;
	}
}

void populatechessboard(){
	printf("Populating Chess board!\n");

	vga_ctrl->VRAM[0] = 0x10;	//black rook
	vga_ctrl->VRAM[1] = 0x01;	//black knight
	vga_ctrl->VRAM[2] = 0x12;	//black bishop
	vga_ctrl->VRAM[3] = 0x03;	//black queen
	vga_ctrl->VRAM[4] = 0x14;	//black king

	//populating black other side of bishop, knight, rook
	vga_ctrl->VRAM[5] = 0x02;
	vga_ctrl->VRAM[6] = 0x11;
	vga_ctrl->VRAM[7] = 0x00;

	//populating black pawns
	for (alt_u8 i = 8; i<16; i++){
		if (i%2 == 0)	vga_ctrl->VRAM[i] = 0x05;
		else			vga_ctrl->VRAM[i] = 0x15;
	}

	//setting black pawn special bit for pawn move
	for (alt_u8 i = 8; i<16; i++){
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | PAWNFIRST;
	}

	//populating the 4 rows of empty tiles alternately
	for (alt_u8 i = 16; i < 24; i++){
		if (i%2 == 0) vga_ctrl->VRAM[i] = 0x1C;
		else		  vga_ctrl->VRAM[i] = 0x0C;
	}

	for (alt_u8 i = 24; i < 32; i++){
		if (i%2 == 0) vga_ctrl->VRAM[i] = 0x0C;
		else		  vga_ctrl->VRAM[i] = 0x1C;
	}

	for (alt_u8 i = 32; i < 40; i++){
		if (i%2 == 0) vga_ctrl->VRAM[i] = 0x1C;
		else		  vga_ctrl->VRAM[i] = 0x0C;
	}

	for (alt_u8 i = 40; i < 48; i++){
		if (i%2 == 0) vga_ctrl->VRAM[i] = 0x0C;
		else		  vga_ctrl->VRAM[i] = 0x1C;
	}
	//end of populating 4 rows of empty cells

	//populating white pawn
	for (alt_u8 i = 48; i < 56; i++){
		if (i%2 == 0)vga_ctrl->VRAM[i] = 0x16;
		else vga_ctrl->VRAM[i] = 0x06;
	}

	for (alt_u8 i = 48; i < 56; i++){
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | PAWNFIRST;
	}

	//white side pieces
	vga_ctrl->VRAM[56] = 0x07;	//rook
	vga_ctrl->VRAM[57] = 0x18;	//knight
	vga_ctrl->VRAM[58] = 0x09;	//bishop
	vga_ctrl->VRAM[59] = 0x1A;	//queen
	vga_ctrl->VRAM[60] = 0x0B;	//king
	vga_ctrl->VRAM[61] = 0x19;
	vga_ctrl->VRAM[62] = 0x08;
	vga_ctrl->VRAM[63] = 0x17;

	//Setting special bits for King and Rooks for castling
	vga_ctrl->VRAM[56] = vga_ctrl->VRAM[56] | PAWNFIRST;	//white left rook
	vga_ctrl->VRAM[63] = vga_ctrl->VRAM[63] | PAWNFIRST;	//white right rook
	vga_ctrl->VRAM[60] = vga_ctrl->VRAM[60] | PAWNFIRST;	//white king

	vga_ctrl->VRAM[0] = vga_ctrl->VRAM[0] | PAWNFIRST;	//black left rook
	vga_ctrl->VRAM[7] = vga_ctrl->VRAM[7] | PAWNFIRST;	//black right rook
	vga_ctrl->VRAM[4] = vga_ctrl->VRAM[4] | PAWNFIRST;	//black king
	printf("Done populating chess board! \n");
}

/*
 * Given any (drawX, drawY) that is within the chessboard, generates the corresponding index to access in VRAM
 * Should return a value 0 to 63
 */
alt_u8 chessVramIndex(unsigned long dx, unsigned long dy){
	alt_u8 divx = (dx - 140) / 45;		//column
	alt_u8 divy = (dy - 60) / 45;		//row
	alt_u8 ans = (divy*8) + divx;
	if (ans < 0 || ans >= 64) return 64;
	return ans;
}

/*
 * IMPORTANT: function is only called when the mouse left button is clicked!
 *
 * Given the coordinate position of mouse, determine whether we land on a chess piece
 * Return the code of the chess_piece (values 0 to 12) if we INDEED land on a chess piece
 * Return a SPECIALCODE if we did not land on a chess piece
 *
 */
alt_u8 tile_clicked(unsigned long mx, unsigned long my){
	int inside_board = 0;
	int x_offset, y_offset, row, column;
	if ((mx >= 140) && (mx <= 499) && (my >=60) && (my<=419)){
		inside_board = 1;
	}

	//we are not inside any tile
	if (inside_board == 0){
		return SPECIALCODE;		//13 because this value is impossible to be in VRAM[0..63],
								//because we have only 13 chess tiles (so technically 0 to 12 values are possible)
	}

	x_offset = mx - 140;
	y_offset = my - 60;
	row = y_offset / 45;
	column = x_offset / 45;
	return vga_ctrl->VRAM[row * 8 + column];
}

int within_board(int tile_index){
	return (tile_index >= 0 && tile_index <64);
}

int empty_tile(int tile_index){
	//assume index always within bounds
	if ((vga_ctrl->VRAM[tile_index] & EMPTYCODE) == EMPTYCODE) return 1;
	return 0;
}

int same_color(int curr_index, int dest_index){
	alt_u8 curr_color = vga_ctrl->VRAM[curr_index] & CODECHECK;
	alt_u8 dest_color = vga_ctrl->VRAM[dest_index] & CODECHECK;
	int s;

	if (curr_color >= 0 && curr_color <= 5){
		curr_color = 1;		//black
	}
	else if (curr_color >= 6 && curr_color <= 11){
		curr_color = 0;		//white
	}else{
		curr_color = 2;
	}

	if (dest_color >= 0 && dest_color <= 5){
		dest_color = 1;		//black
	}
	else if (dest_color >= 6 && dest_color <= 11){
		dest_color = 0;		//white
	}else{
		dest_color = 2;
	}

	s = (dest_color == curr_color);
	return s;
}

void turn_off_glow(){
	for(int i=0; i<64; i++){
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] & GLOWOFF;
	}
}

//check for within bounds and check for same color
void glow(int tile_index, int source){
	if (within_board(tile_index) && !same_color(source, tile_index))vga_ctrl->VRAM[tile_index] = vga_ctrl->VRAM[tile_index] | GLOW;
	return;
}

int valid_turn(alt_u8 chess_code, int white_turn, int black_turn){
	if (chess_code >= 0 && chess_code <= 5 && black_turn) return 1;
	if (chess_code >= 6 && chess_code <= 11 && white_turn) return 1;
	return 0;
}

void generate_black_pawn_move(int tile_index, int special_state){
	//black pawn case --> moving forward in terms of VRAM index
	int one_step, two_step;
	one_step = tile_index + 8;
	black_pawn_capture();

	int left = 1, right = 1;
	if (tile_index % 8 == 0)left = 0;
	if (tile_index % 8 == 7)right = 0;
	alt_u8 enpassant1, enpassant2;

	if (special_state){
		two_step = tile_index + 16;
		if (within_board(one_step) && empty_tile(one_step))
			vga_ctrl->VRAM[one_step] = vga_ctrl->VRAM[one_step] | GLOW; //the 6th bit is preserved for lighting up
		if (within_board(two_step) && empty_tile(two_step))
			vga_ctrl->VRAM[two_step] = vga_ctrl->VRAM[two_step] | GLOW;	//the 6th bit is preserved for lighting up
	}else{
		//not the first move
		//either go straight one up or eat diagonal
		if (within_board(one_step) && empty_tile(one_step))
			vga_ctrl->VRAM[one_step] = vga_ctrl->VRAM[one_step] | GLOW; //the 6th bit is preserved for lighting up

		//check if side tiles are en passant pawns
		if (left)enpassant1 = vga_ctrl->VRAM[tile_index - 1] & PAWNSECOND;
		if (right)enpassant2 = vga_ctrl->VRAM[tile_index + 1] & PAWNSECOND;
		//if one of them is, glow the eatable diagonal tile!
		if (enpassant1)vga_ctrl->VRAM[tile_index + 8 - 1] |= GLOW;
		if (enpassant2)vga_ctrl->VRAM[tile_index + 8 + 1] |= GLOW;
	}
}

void generate_white_pawn_move(int tile_index, int special_state){
	//white pawn case --> moving negative direction in terms of VRAM index

	int one_step, two_step;
	one_step = tile_index - 8;
	white_pawn_capture();
	int left = 1, right = 1;
	if (tile_index % 8 == 0)left = 0;
	if (tile_index % 8 == 7)right = 0;
	alt_u8 enpassant1, enpassant2;

	if (special_state){
		two_step = tile_index - 16;
		if (within_board(one_step)&& empty_tile(one_step)){
			vga_ctrl->VRAM[one_step] = vga_ctrl->VRAM[one_step] | GLOW; //the 6th bit is preserved for lighting up
		}
		if (within_board(two_step)&& empty_tile(two_step)){
			vga_ctrl->VRAM[two_step] = vga_ctrl->VRAM[two_step] | GLOW;	//the 6th bit is preserved for lighting up
		}
	}else{
		//not the first move
		//either go straight one up or eat diagonal
		if (within_board(one_step)&& empty_tile(one_step))
			vga_ctrl->VRAM[one_step] = vga_ctrl->VRAM[one_step] | GLOW; //the 6th bit is preserved for lighting up
		//check if side tiles are en passant pawns
		if (left)enpassant1 = vga_ctrl->VRAM[tile_index - 1] & PAWNSECOND;
		if (right)enpassant2 = vga_ctrl->VRAM[tile_index + 1] & PAWNSECOND;
		//if one of them is, glow the eatable diagonal tile!
		if (enpassant1)vga_ctrl->VRAM[tile_index - 8 - 1] |= GLOW;
		if (enpassant2)vga_ctrl->VRAM[tile_index - 8 + 1] |= GLOW;

	}
}

void generate_king_move(int tile_index, int king_color){
	//king can move 1 square in all directions
	//variable naming is consider the position of a white king (upright) but should be usable for black king too
	int up = tile_index - 8;
	int down = tile_index + 8;
	int left = tile_index - 1;
	int right = tile_index + 1;
	int upright_corner = up + 1, upleft_corner = up - 1, botleft_corner = down - 1, botright_corner = down + 1;

	int left_check = (tile_index % 8 == 0);	//if this is 1, that means cannot move left
	int right_check = (tile_index % 8 == 7);	//if this is 1 that means cannot move right
	//do not light up tile if it they are the same color
	if (within_board(up) && !same_color(tile_index, up)) vga_ctrl->VRAM[up] = vga_ctrl->VRAM[up] | GLOW;
	if (within_board(down)&& !same_color(tile_index, down)) vga_ctrl->VRAM[down] = vga_ctrl->VRAM[down] | GLOW;
	if (within_board(left)&& !same_color(tile_index, left) && !left_check) vga_ctrl->VRAM[left] = vga_ctrl->VRAM[left] | GLOW;
	if (within_board(right)&& !same_color(tile_index, right) && !right_check) vga_ctrl->VRAM[right] = vga_ctrl->VRAM[right] | GLOW;
	if (within_board(upright_corner)&& !same_color(tile_index, upright_corner) && !right_check) vga_ctrl->VRAM[upright_corner] = vga_ctrl->VRAM[upright_corner] | GLOW;
	if (within_board(upleft_corner)&& !same_color(tile_index, upleft_corner) && !left_check) vga_ctrl->VRAM[upleft_corner] = vga_ctrl->VRAM[upleft_corner] | GLOW;
	if (within_board(botleft_corner)&& !same_color(tile_index, botleft_corner) && !left_check) vga_ctrl->VRAM[botleft_corner] = vga_ctrl->VRAM[botleft_corner] | GLOW;
	if (within_board(botright_corner)&& !same_color(tile_index, botright_corner) &&!right_check) vga_ctrl->VRAM[botright_corner] = vga_ctrl->VRAM[botright_corner] | GLOW;

	//castling
	if(king_color == 0) black_castling();
	else white_castling();
}

int tile_attacked(int attacker, int piece_type){
	return (within_board(attacker) && ((vga_ctrl->VRAM[attacker] & CODECHECK) == piece_type));
}

int black_check_attacked(int tile_index){
	int attacker, attacked = 0;
	int move1block = 0, move2block = 0;
	//-----------------------------------PAWN---------------------------------------
	int left_pawn, right_pawn;
	if (tile_index % 8 == 0){
		left_pawn = 0;
	}else{
		left_pawn = 1;
	}
	if (tile_index % 8 == 7){
		right_pawn = 0;
	}else{
		right_pawn = 1;
	}

	if (left_pawn){
		attacker = tile_index + 8 - 1;
		if(tile_attacked(attacker, WHITEPAWN)) return 1;
	}
	if (right_pawn){
		attacker = tile_index + 8 + 1;
		if(tile_attacked(attacker, WHITEPAWN)) return 1;
	}
	printf("wpawn\n");
	//----------------------------KING-----------------------------------------
	int up, down, left, right;
	up = tile_index / 8;
	down = ((tile_index / 8) != 7);		//not equal 7 meaning not at the last row
	left = tile_index % 8;
	right = ((tile_index % 8) != 7);	//not equal 7 meaning not at the last column

	if (up){
		attacker = tile_index - 8;
		if(tile_attacked(attacker, WHITEKING)) return 1;
		if (left){
			//up and left
			attacker = tile_index - 8 - 1;
			if(tile_attacked(attacker, WHITEKING)) return 1;
		}
		if (right){
			//up and right
			attacker = tile_index - 8 + 1;
			if(tile_attacked(attacker, WHITEKING)) return 1;
		}
	}

	if (down){
		if (left){
			//up and left
			attacker = tile_index + 8 - 1;
			if(tile_attacked(attacker, WHITEKING)) return 1;
		}
		if (right){
			//up and left
			attacker = tile_index + 8 + 1;
			if(tile_attacked(attacker, WHITEKING)) return 1;
		}
	}

	if (left){
		attacker = tile_index - 1;
		if(tile_attacked(attacker, WHITEKING)) return 1;
	}

	if (right){
		attacker = tile_index + 1;
		if(tile_attacked(attacker, WHITEKING)) return 1;
	}
	printf("wking\n");
	//-----------------------ROOK QUEEN----------------------------
	for(int i=tile_index - 8; i>=0; i-=8){
		if (!within_board(i)) break;
		if (same_color(tile_index, i)) break;
		//empty tile will just loop through
		if (empty_tile(i)) continue;
		if (((vga_ctrl->VRAM[i]&CODECHECK) != WHITEQUEEN) && ((vga_ctrl->VRAM[i]&CODECHECK) != JUSTEMPTY) && (vga_ctrl->VRAM[i]&CODECHECK) != WHITEROOK) break;
		//first time seeing a piece that is not a white queen or a white rook
		if (tile_attacked(i, WHITEQUEEN) || tile_attacked(i, WHITEROOK)) {
			attacked = 1;
			break;
		}
	}
	if (attacked) return attacked;

	//generating move straight down
	for(int i=tile_index + 8; i<64; i+=8){
		if (!within_board(i)) break;
		if (same_color(tile_index, i)) break;
		//empty tile will just loop through
		if (empty_tile(i)) continue;
		if (((vga_ctrl->VRAM[i]&CODECHECK) != WHITEQUEEN)&& ((vga_ctrl->VRAM[i]&CODECHECK) != JUSTEMPTY) && (vga_ctrl->VRAM[i]&CODECHECK) != WHITEROOK) break;
		//first time seeing a piece that is not a white queen or a white rook
		if (tile_attacked(i, WHITEQUEEN) || tile_attacked(i, WHITEROOK)) {
			attacked = 1;
			break;
		}
	}
	if (attacked) return attacked;

	int end = (tile_index/8) * 8 + 8;
	int start = (tile_index/8) * 8;

	//going right in the same row
	for(int j=tile_index+1; j<end; j++){
		if (same_color(tile_index, j))break;
		if (empty_tile(j))continue;
		if (((vga_ctrl->VRAM[j]&CODECHECK) != WHITEQUEEN) && ((vga_ctrl->VRAM[j]&CODECHECK) != JUSTEMPTY) && ((vga_ctrl->VRAM[j]&CODECHECK) != WHITEROOK))break;
		if (tile_attacked(j, WHITEQUEEN) || tile_attacked(j, WHITEROOK)) {
			attacked = 1;
			break;
		}
	}
	if (attacked) return attacked;

	for (int j = tile_index-1; j>= start; j--){
		if (same_color(tile_index,j))break;
		if (empty_tile(j))continue;
		if (((vga_ctrl->VRAM[j]&CODECHECK) != WHITEQUEEN) && ((vga_ctrl->VRAM[j]&CODECHECK) != WHITEROOK)&& ((vga_ctrl->VRAM[j]&CODECHECK) != JUSTEMPTY))break;
		if (tile_attacked(j, WHITEQUEEN) || tile_attacked(j, WHITEROOK)) {
			attacked = 1;
			break;
		}
	}
	if (attacked) return attacked;
	printf("wrookqueen\n");
	//-------------------------------BISHOP QUEEN--------------------
	int move1, move2;
	move1block = 0;
	move2block = 0;
	int mod8 = tile_index % 8;

	int left_bq, right_bq;
	left_bq = mod8;
	right_bq = 7 - mod8;

	for (int i=0; i<left_bq; i++){
		if (attacked == 1) break;
		move1 = tile_index - ((i+1)*8) - (i+1);
		move2 = tile_index + ((i+1)*8) - (i+1);
		if (((vga_ctrl->VRAM[move1]&CODECHECK) != WHITEQUEEN) && ((vga_ctrl->VRAM[move1]&CODECHECK) != JUSTEMPTY) && ((vga_ctrl->VRAM[move1]&CODECHECK) != WHITEBISHOP)) move1block = 1;
		if (((vga_ctrl->VRAM[move2]&CODECHECK) != WHITEQUEEN) && ((vga_ctrl->VRAM[move2]&CODECHECK) != JUSTEMPTY) && ((vga_ctrl->VRAM[move2]&CODECHECK) != WHITEBISHOP)) move2block = 1;
		if(!move1block && (tile_attacked(move1, WHITEQUEEN) || tile_attacked(move1, WHITEBISHOP))){
			attacked = 1;
			break;
		}

		if(!move2block && (tile_attacked(move2, WHITEQUEEN) || tile_attacked(move2, WHITEBISHOP))){
			attacked = 1;
			break;
		}

	}
	if (attacked) return attacked;

	move1block = 0;
	move2block = 0;
	for(int i=0; i<right_bq; i++){
		move1 = tile_index - ((i+1)*8) + (i+1);
		move2 = tile_index + ((i+1)*8) + (i+1);

		if (((vga_ctrl->VRAM[move1]&CODECHECK) != WHITEQUEEN) && ((vga_ctrl->VRAM[move1]&CODECHECK) != WHITEBISHOP)&& ((vga_ctrl->VRAM[move1]&CODECHECK) != JUSTEMPTY)) move1block = 1;
		if (((vga_ctrl->VRAM[move2]&CODECHECK) != WHITEQUEEN) && ((vga_ctrl->VRAM[move2]&CODECHECK) != WHITEBISHOP)&& ((vga_ctrl->VRAM[move2]&CODECHECK) != JUSTEMPTY)) move2block = 1;
		if(!move1block&& (tile_attacked(move1, WHITEQUEEN) || tile_attacked(move1, WHITEBISHOP))){
			attacked = 1;
			break;
		}


		if(!move2block && (tile_attacked(move2, WHITEQUEEN) || tile_attacked(move2, WHITEBISHOP))){
			attacked = 1;
			break;
		}


	}
	if (attacked) return attacked;
	printf("wbishopqueen\n");
	//--------------------------------KNIGHT-------------------------
	int check, left_bool, left_bool2, right_bool, right_bool2;
	left_bool = 1;		//for 1 move left
	right_bool = 1;
	left_bool2 = 1;		//for 2 moves left
	right_bool2 = 1;
	check = tile_index % 8;
	switch (check){
	case 0:
		left_bool = 0;
		left_bool2 = 0;
		break;
	case 1:
		left_bool2 = 0;
		break;
	case 6:
		right_bool2 = 0;
		break;
	case 7:
		right_bool = 0;
		right_bool2 = 0;
		break;
	default:
		left_bool = 1;
		right_bool = 1;
		left_bool2 = 1;
		right_bool2 = 1;
		break;
	}
	//knight can have eight moves
	int upright = tile_index - 16 + 1;
	int upleft = tile_index - 16 - 1;
	int downright = tile_index + 16 + 1;
	int downleft = tile_index + 16 - 1;
	int rightup = tile_index + 2 - 8;
	int rightdown = tile_index + 2 + 8;
	int leftup = tile_index - 2 - 8;
	int leftdown = tile_index - 2 + 8;

	if (left_bool)attacked = tile_attacked(upleft, WHITEKNIGHT) || tile_attacked(downleft, WHITEKNIGHT);
	if (attacked) return attacked;
	if (left_bool2)attacked = tile_attacked(leftup, WHITEKNIGHT) || tile_attacked(leftdown, WHITEKNIGHT);
	if (attacked) return attacked;
	if (right_bool)attacked = tile_attacked(upright, WHITEKNIGHT) || tile_attacked(downright, WHITEKNIGHT);
	if (attacked) return attacked;
	if (right_bool2)attacked = tile_attacked(rightup, WHITEKNIGHT) || tile_attacked(rightdown, WHITEKNIGHT);
	printf("wknight %d  \n", attacked);
	return attacked;
}

int white_check_attacked(int tile_index){
	int attacker, attacked = 0;
	int move1block = 0, move2block = 0;
	//-----------------------------------PAWN---------------------------------------
	int left_pawn, right_pawn;
	if (tile_index % 8 == 0){
		left_pawn = 0;
	}else{
		left_pawn = 1;
	}
	if (tile_index % 8 == 7){
		right_pawn = 0;
	}else{
		right_pawn = 1;
	}

	if (left_pawn){
		attacker = tile_index - 8 - 1;
		if(tile_attacked(attacker, BLACKPAWN)) return 1;
	}
	if (right_pawn){
		attacker = tile_index - 8 + 1;
		if(tile_attacked(attacker, BLACKPAWN)) return 1;
	}
	printf("bpawn\n");
	//----------------------------KING-----------------------------------------
	int up, down, left, right;
	up = tile_index / 8;
	down = ((tile_index / 8) != 7);		//not equal 7 meaning not at the last row
	left = tile_index % 8;
	right = ((tile_index % 8) != 7);	//not equal 7 meaning not at the last column

	if (up){
		attacker = tile_index - 8;
		if(tile_attacked(attacker, BLACKKING)) return 1;
		if (left){
			//up and left
			attacker = tile_index - 8 - 1;
			if(tile_attacked(attacker, BLACKKING)) return 1;
		}
		if (right){
			//up and right
			attacker = tile_index - 8 + 1;
			if(tile_attacked(attacker, BLACKKING)) return 1;
		}
	}

	if (down){
		if (left){
			//up and left
			attacker = tile_index + 8 - 1;
			if(tile_attacked(attacker, BLACKKING)) return 1;
		}
		if (right){
			//up and left
			attacker = tile_index + 8 + 1;
			if(tile_attacked(attacker, BLACKKING)) return 1;
		}
	}

	if (left){
		attacker = tile_index - 1;
		if(tile_attacked(attacker, BLACKKING)) return 1;
	}

	if (right){
		attacker = tile_index + 1;
		if(tile_attacked(attacker, BLACKKING)) return 1;
	}
	printf("bking\n");
	//-----------------------ROOK QUEEN----------------------------
	for(int i=tile_index - 8; i>=0; i-=8){
		if (!within_board(i)) break;
		if (same_color(tile_index, i)) break;
		//empty tile will just loop through
		if (empty_tile(i)) continue;
		if (((vga_ctrl->VRAM[i]&CODECHECK) != BLACKQUEEN) && ((vga_ctrl->VRAM[i]&CODECHECK) != JUSTEMPTY) && (vga_ctrl->VRAM[i]&CODECHECK) != BLACKROOK) break;
		//first time seeing a piece that is not a white queen or a white rook
		if (tile_attacked(i, BLACKQUEEN) || tile_attacked(i, BLACKROOK)) {
			attacked = 1;
			break;
		}
	}
	if (attacked) return attacked;

	//generating move straight down
	for(int i=tile_index + 8; i<64; i+=8){
		if (!within_board(i)) break;
		if (same_color(tile_index, i)) break;
		//empty tile will just loop through
		if (empty_tile(i)) continue;
		if (((vga_ctrl->VRAM[i]&CODECHECK) != BLACKQUEEN)&& ((vga_ctrl->VRAM[i]&CODECHECK) != JUSTEMPTY) && (vga_ctrl->VRAM[i]&CODECHECK) != BLACKROOK) break;
		//first time seeing a piece that is not a white queen or a white rook
		if (tile_attacked(i, BLACKQUEEN) || tile_attacked(i, BLACKROOK)) {
			attacked = 1;
			break;
		}
	}
	if (attacked) return attacked;

	int end = (tile_index/8) * 8 + 8;
	int start = (tile_index/8) * 8;

	//going right in the same row
	for(int j=tile_index+1; j<end; j++){
		if (same_color(tile_index, j))break;
		if (empty_tile(j))continue;
		if (((vga_ctrl->VRAM[j]&CODECHECK) != BLACKQUEEN) && ((vga_ctrl->VRAM[j]&CODECHECK) != JUSTEMPTY)&& ((vga_ctrl->VRAM[j]&CODECHECK) != BLACKROOK))break;
		if (tile_attacked(j, BLACKQUEEN) || tile_attacked(j, BLACKROOK)) {
			attacked = 1;
			break;
		}
	}
	if (attacked) return attacked;

	for (int j = tile_index-1; j>= start; j--){
		if (same_color(tile_index,j))break;
		if (empty_tile(j))continue;
		if (((vga_ctrl->VRAM[j]&CODECHECK) != BLACKQUEEN)&& ((vga_ctrl->VRAM[j]&CODECHECK) != JUSTEMPTY) && ((vga_ctrl->VRAM[j]&CODECHECK) != BLACKROOK))break;
		if (tile_attacked(j, BLACKQUEEN) || tile_attacked(j, BLACKROOK)) {
			attacked = 1;
			break;
		}
	}
	if (attacked) return attacked;
	printf("brookqueen\n");
	//-------------------------------BISHOP QUEEN--------------------
	int move1, move2;
	move1block = 0;
	move2block = 0;
	int mod8 = tile_index % 8;

	int left_bq, right_bq;
	left_bq = mod8;
	right_bq = 7 - mod8;

	for (int i=0; i<left_bq; i++){
		if (attacked == 1) break;
		move1 = tile_index - ((i+1)*8) - (i+1);
		move2 = tile_index + ((i+1)*8) - (i+1);
		if (((vga_ctrl->VRAM[move1]&CODECHECK) != BLACKQUEEN)&& ((vga_ctrl->VRAM[move1]&CODECHECK) != JUSTEMPTY) && ((vga_ctrl->VRAM[move1]&CODECHECK) != BLACKBISHOP)) move1block = 1;
		if (((vga_ctrl->VRAM[move2]&CODECHECK) != BLACKQUEEN)&& ((vga_ctrl->VRAM[move2]&CODECHECK) != JUSTEMPTY) && ((vga_ctrl->VRAM[move2]&CODECHECK) != BLACKBISHOP)) move2block = 1;
		if(!move1block && (tile_attacked(move1, BLACKQUEEN) || tile_attacked(move1, BLACKBISHOP))){
			attacked = 1;
			break;
		}

		if(!move2block && (tile_attacked(move2, BLACKQUEEN) || tile_attacked(move2, BLACKBISHOP))){
			attacked = 1;
			break;
		}

	}
	if (attacked) return attacked;

	move1block = 0;
	move2block = 0;
	for(int i=0; i<right_bq; i++){
		move1 = tile_index - ((i+1)*8) + (i+1);
		move2 = tile_index + ((i+1)*8) + (i+1);
		if (((vga_ctrl->VRAM[move1]&CODECHECK) != BLACKQUEEN)&& ((vga_ctrl->VRAM[move1]&CODECHECK) != JUSTEMPTY) && ((vga_ctrl->VRAM[move1]&CODECHECK) != BLACKBISHOP)) move1block = 1;
		if (((vga_ctrl->VRAM[move2]&CODECHECK) != BLACKQUEEN)&& ((vga_ctrl->VRAM[move2]&CODECHECK) != JUSTEMPTY) && ((vga_ctrl->VRAM[move2]&CODECHECK) != BLACKBISHOP)) move2block = 1;
		if(!move1block&& (tile_attacked(move1, BLACKQUEEN) || tile_attacked(move1, BLACKBISHOP))){
			attacked = 1;
			break;
		}


		if(!move2block && (tile_attacked(move2, BLACKQUEEN) || tile_attacked(move2, BLACKBISHOP))){
			attacked = 1;
			break;
		}


	}
	if (attacked) return attacked;
	printf("bbishopqueen\n");
	//--------------------------------KNIGHT-------------------------
	int check, left_bool, left_bool2, right_bool, right_bool2;
	left_bool = 1;		//for 1 move left
	right_bool = 1;
	left_bool2 = 1;		//for 2 moves left
	right_bool2 = 1;
	check = tile_index % 8;
	switch (check){
	case 0:
		left_bool = 0;
		left_bool2 = 0;
		break;
	case 1:
		left_bool2 = 0;
		break;
	case 6:
		right_bool2 = 0;
		break;
	case 7:
		right_bool = 0;
		right_bool2 = 0;
		break;
	default:
		left_bool = 1;
		right_bool = 1;
		left_bool2 = 1;
		right_bool2 = 1;
		break;
	}
	//knight can have eight moves
	int upright = tile_index - 16 + 1;
	int upleft = tile_index - 16 - 1;
	int downright = tile_index + 16 + 1;
	int downleft = tile_index + 16 - 1;
	int rightup = tile_index + 2 - 8;
	int rightdown = tile_index + 2 + 8;
	int leftup = tile_index - 2 - 8;
	int leftdown = tile_index - 2 + 8;

	if (left_bool)attacked = tile_attacked(upleft, BLACKKNIGHT) || tile_attacked(downleft, BLACKKNIGHT);
	if (attacked) return attacked;
	if (left_bool2)attacked = tile_attacked(leftup, BLACKKNIGHT) || tile_attacked(leftdown, BLACKKNIGHT);
	if (attacked) return attacked;
	if (right_bool)attacked = tile_attacked(upright, BLACKKNIGHT) || tile_attacked(downright, BLACKKNIGHT);
	if (attacked) return attacked;
	if (right_bool2)attacked = tile_attacked(rightup, BLACKKNIGHT) || tile_attacked(rightdown, BLACKKNIGHT);
	printf("bknight %d  \n", attacked);
	return attacked;
}


//black needs to check tile Rook 1 2 3 King 5 6 Rook
//white needs to check tile Rook 57 58 59 King 61 62 Rook
void white_castling(){
	int king_moved		=	vga_ctrl->VRAM[60] & PAWNFIRST;
	int rookleft_moved 	=	vga_ctrl->VRAM[56] & PAWNFIRST;
	int rookright_moved =	vga_ctrl->VRAM[63] & PAWNFIRST;

	printf("100\n");
	printf("Initial  K: %d  RL: %d   RR: %d  \n", king_moved, rookleft_moved, rookright_moved);
	//checking for long castle
	if ((king_moved & rookleft_moved) == PAWNFIRST){
		//both king and left rook have not been moved
		printf("200\n");
		if (empty_tile(57) && empty_tile(58) && empty_tile(59)){
			//all three tiles are not attacked
			printf("300\n");
			if (!(white_check_attacked(57)) && (!white_check_attacked(58)) && !(white_check_attacked(59))){
				printf("400\n");
				glow(58, 60);
			}
		}
	}

	//checking for short castle
	if ((king_moved & rookright_moved) == PAWNFIRST){
		//both king and right rook have not been moved
		printf("500\n");
		if (empty_tile(61) && empty_tile(62)){
			//all three tiles are not attacked
			printf("600\n");
			if (!(white_check_attacked(61)) && !(white_check_attacked(62))){
				printf("700\n");
				glow(62, 60);
			}
		}
	}
}

void black_castling(){
	int king_moved		=	vga_ctrl->VRAM[4] & PAWNFIRST;
	int rookleft_moved 	=	vga_ctrl->VRAM[0] & PAWNFIRST;
	int rookright_moved =	vga_ctrl->VRAM[7] & PAWNFIRST;
	printf("1\n");
	printf("Initial  K: %d  RL: %d   RR: %d  \n", king_moved, rookleft_moved, rookright_moved);
	//checking for long castle
	if ((king_moved & rookleft_moved) == PAWNFIRST){
		printf("2\n");
		//both king and left rook have not been moved
		if (empty_tile(1) && empty_tile(2) && empty_tile(3)){
			//all three tiles are empty
			printf("3\n");
			if (!(black_check_attacked(1)) && (!black_check_attacked(2)) && !(black_check_attacked(3))){
				printf("%d    %d    %d   ----\n", black_check_attacked(1), black_check_attacked(2), black_check_attacked(3));
				glow(2, 4);
			}
		}
	}

	//checking for short castle
	if ((king_moved & rookright_moved) == PAWNFIRST){
		//both king and right rook have not been moved
		printf("4\n");
		if (empty_tile(5) && empty_tile(6)){
			//all three tiles are not attacked
			printf("5\n");
			if (!(black_check_attacked(5)) && (!black_check_attacked(6))){
				printf("%d    %d    ----\n", black_check_attacked(5), black_check_attacked(6));
				glow(6, 4);
			}
		}
	}
}

void generate_queen_move(int tile_index){
	generate_rook_move(tile_index);
	generate_bishop_move(tile_index);
}

void generate_rook_move(int tile_index){
	//generates all straight up and straight back until encounter blocking

	//generate ahead
	int curr = tile_index;
	int firstblock = 0;

	//generating move straight up
	for(int i=tile_index - 8; i>=0; i-=8){
		if (!within_board(i)) break;
		if (same_color(curr, i)) break;
		if (firstblock) break;
		if (within_board(i) && !same_color(curr, i)){
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | GLOW;
			if(!empty_tile(i))firstblock = 1;
		}
	}

	firstblock = 0;
	//generating move straight down
	for(int i=tile_index + 8; i<64; i+=8){
		if (!within_board(i)) break;
		if (same_color(curr, i)) break;
		if (firstblock) break;
		if (within_board(i) && !same_color(curr, i)){
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | GLOW;
			if (!empty_tile(i))firstblock = 1;
		}
	}

	int end = (tile_index/8) * 8 + 8;
	int start = (tile_index/8) * 8;

	//going right in the same row
	for(int j=curr+1; j<end; j++){
		if (same_color(curr, j)){
			break;
		}
		if (empty_tile(j)){
			glow(j, curr);
			continue;
		}
		//glowing a tile that can be attacked by a rook
		glow(j, curr);
		break;
	}

	for (int j = curr-1; j>= start; j--){
		if (same_color(curr,j))break;
		if (empty_tile(j)){
			glow(j, curr);
			continue;
		}
		//glowing a tile that can be attacked
		glow(j, curr);
		break;
	}

}

void generate_knight_move(int tile_index){
	int check, left_bool, left_bool2, right_bool, right_bool2;
	left_bool = 1;		//for 1 move left
	right_bool = 1;
	left_bool2 = 1;		//for 2 moves left
	right_bool2 = 1;
	check = tile_index % 8;
	switch (check){
	case 0:
		left_bool = 0;
		left_bool2 = 0;
		break;
	case 1:
		left_bool2 = 0;
		break;
	case 6:
		right_bool2 = 0;
		break;
	case 7:
		right_bool = 0;
		right_bool2 = 0;
		break;
	default:
		left_bool = 1;
		right_bool = 1;
		left_bool2 = 1;
		right_bool2 = 1;
		break;
	}
	//knight can have eight moves
	int upright = tile_index - 16 + 1;
	int upleft = tile_index - 16 - 1;
	int downright = tile_index + 16 + 1;
	int downleft = tile_index + 16 - 1;
	int rightup = tile_index + 2 - 8;
	int rightdown = tile_index + 2 + 8;
	int leftup = tile_index - 2 - 8;
	int leftdown = tile_index - 2 + 8;

	if (left_bool){
		glow(upleft, tile_index);
		glow(downleft, tile_index);
	}
	if (left_bool2){
		glow(leftup, tile_index);
		glow(leftdown, tile_index);
	}
	if (right_bool){
		glow(upright, tile_index);
		glow(downright, tile_index);
	}
	if(right_bool2){
		glow(rightup, tile_index);
		glow(rightdown, tile_index);
	}
}

void generate_bishop_move(int tile_index){
	//move up and right
	int move1, move2, move1_cont, move2_cont;
	move1_cont = 0;
	move2_cont = 0;
	int mod8 = tile_index % 8;

	int left, right;
	left = mod8;
	right = 7 - mod8;

	for (int i=0; i<left; i++){
		move1 = tile_index - ((i+1)*8) - (i+1);
		move2 = tile_index + ((i+1)*8) - (i+1);
		if (!(move1_cont)&&within_board(move1)){
			if (empty_tile(move1)){
				glow(move1, tile_index);
			}else{
				move1_cont = 1;
				glow(move1, tile_index);
			}
		}
		if (!(move2_cont) && within_board(move2)){
			if(empty_tile(move2)){
				glow(move2, tile_index);
			}else{
				move2_cont = 1;
				glow(move2, tile_index);
			}
		}
	}

	move1_cont = 0;
	move2_cont = 0;
	for(int i=0; i<right; i++){
		move1 = tile_index - ((i+1)*8) + (i+1);
		move2 = tile_index + ((i+1)*8) + (i+1);
		if (!(move1_cont)&&within_board(move1)){
			if (empty_tile(move1)){
				glow(move1, tile_index);
			}else{
				move1_cont = 1;
				glow(move1, tile_index);
			}
		}
		if (!(move2_cont) && within_board(move2)){
			if(empty_tile(move2)){
				glow(move2, tile_index);
			}else{
				move2_cont = 1;
				glow(move2, tile_index);
			}
		}
	}
}

void black_pawn_capture(int tile_index){
	//index looks in positive direction
	int diag1, diag2, right, left;
	right = 1;
	left = 1;
	switch (tile_index%8){
		case 7:
			right = 0;
			break;
		case 0:
			left = 0;
			break;
		default:
			right = 1;
			left = 1;
	}
	diag1 = tile_index + 8 + 1;	//right
	diag2 = tile_index + 8 - 1;	//left

	if (within_board(diag1) && !empty_tile(diag1) && right) glow(diag1, tile_index);
	if (within_board(diag2) && !empty_tile(diag2) && left) glow(diag2, tile_index);

}

void white_pawn_capture(int tile_index){
	//index looks in positive direction
	int diag1, diag2, right, left;
	right = 1;
	left = 1;
	switch (tile_index%8){
		case 7:
			right = 0;
			break;
		case 0:
			left = 0;
			break;
		default:
			right = 1;
			left = 1;
	}

	diag1 = tile_index - 8 + 1;
	diag2 = tile_index - 8 - 1;
	if (within_board(diag1) && !empty_tile(diag1) && right) glow(diag1, tile_index);
	if (within_board(diag2) && !empty_tile(diag2) && left) glow(diag2, tile_index);
}


/*
 * Given a chess_piece code, perform a left shift on it to filter out the special bits
 * Lighten up the glow code of corresponding VRAM index based on allowed moves.
 */
void foo(alt_u8 vram_code, alt_u8 tile_index){
	if (vram_code == SPECIALCODE){
		return;
	}

	alt_u8 chess_type = vram_code << 4; //only getting the code that represents chess type
	alt_u8 chess_piece;
	int special_code;
	switch (chess_type){
		case 0:			//rook
			generate_rook_move(tile_index);
			break;
		case 16:		//knight
			generate_knight_move(tile_index);
			break;
		case 32:		//bishop
			generate_bishop_move(tile_index);
			break;
		case 48:		//queen
			generate_queen_move(tile_index);
			break;
		case 64:		//king
			generate_king_move(tile_index, 0);
			break;
		case 80:		//black pawn
			special_code = vga_ctrl->VRAM[tile_index] & PAWNFIRST;
			generate_black_pawn_move(tile_index, special_code);
			break;
		case 96:		//white pawn
			special_code = vga_ctrl->VRAM[tile_index] & PAWNFIRST;
			generate_white_pawn_move(tile_index, special_code);
			break;
		case 112:		//rook
			generate_rook_move(tile_index);
			break;
		case 128:		//knight
			generate_knight_move(tile_index);
			break;
		case 144:		//bishop
			generate_bishop_move(tile_index);
			break;
		case 160:		//queen
			generate_queen_move(tile_index);
			break;
		case 176:		//king
			generate_king_move(tile_index, 1);
			break;
		case 192:		//empty

			break;
	}
}
