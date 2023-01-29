#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usb_kb/GenericMacros.h"
#include "usb_kb/GenericTypeDefs.h"
#include "usb_kb/HID.h"
#include "usb_kb/MAX3421E.h"
#include "usb_kb/transfer.h"
#include "usb_kb/usb_ch9.h"
#include "usb_kb/USB.h"

#include "statusbar.h"

int blackbar_index = BLACKBARMID;
int whitebar_index = WHITEBARMID;

static const char p1[17]		=	"P1 (white)       ";
static const char blank[17]		=	"                 ";
static const char p2[17]		=	"P2 (black)       ";
static const char dashed[17]	=	"-----------------";
static const char evalstr[17]	=	"Evaluation       ";

								   //012345678
static const char evalbar[17]	=	"}}}}}}}}}}}}}}}} ";	//using special character to represent thick vertical bar
															//see modification in FONTROM.sv for curly close bracket

								   //0123456789
static const char evalscr[17]	=	"W:       B:      ";
static const char draw[17]		=	"Draw             ";
static const char forfeit[17]	=	"Forfeit          ";
static const char restart[17]	=	"Restart          ";
static const char yesno[17]		=	"  Yes?  |  No?   ";	//index 9 is the vertical line, words have 2 leading spaces
static const char score[17]		=	"W.Score | B.Score";
static const char pieces1[17]	=	"Pwn Pwn Pwn Pwn  ";
static const char pieces2[17]	=	"Pwn Pwn Pwn Pwn  ";
static const char pieces3[17]	=	"Rk Rk Kn Kn Bi Bi";
static const char pieces4[17]	=	"Queen King       ";

static const char m0[17]		=	"    Restart?     ";
//for player 1
static const char m1[17]		=	"  P1 forfeits?   ";
static const char m2[17] 		=	"  P1 forfeited!  ";
static const char m3[17] 		= 	" P1 asks to draw.";
static const char m4[17] 		= 	" P1 accepts draw.";
static const char m5[17] 		=	" P1 rejects draw.";

//for player 2
static const char m2_1[17]		=	"  P2 forfeits?   ";
static const char m2_2[17] 		=	"  P2 forfeited!  ";
static const char m2_3[17] 		=	" P2 asks to draw.";
static const char m2_4[17]		=	" P2 accepts draw.";
static const char m2_5[17] 		=	" P2 rejects draw.";

static const char p1w[17]		=	" P1 won! Restart.";
static const char p2w[17]		=	" P2 won! Restart.";

int absol(int num){
	if (num < 0) return num * (-1);
	return num;
}

void populateFRAM(const char* string, int* start){
	int s = *start;
	for (int i=0; i<ROWCHARCOUNT; i++){
		vga_ctrl->VRAM[s+i] = string[i];
	}
	*start = s +17;
}

void populatestatusbar(){
	int* start;
	*start = 64;
	populateFRAM(p1, start);
	//4 blank lines + 1 dashed line
	populateFRAM(pieces1, start);
	populateFRAM(pieces2, start);
	populateFRAM(pieces3, start);
	populateFRAM(pieces4, start);
	populateFRAM(dashed, start);
	//player 2
	populateFRAM(p2, start);
	//4 blank lines + 1 dashed
	populateFRAM(pieces1, start);
	populateFRAM(pieces2, start);
	populateFRAM(pieces3, start);
	populateFRAM(pieces4, start);
	populateFRAM(dashed, start);
	//-------------------------------------------12 lines

	populateFRAM(blank, start);		// this will be status message, index 12;
	populateFRAM(blank, start);
	populateFRAM(blank, start);
	populateFRAM(blank, start);
	populateFRAM(blank, start);
	populateFRAM(blank, start);
	populateFRAM(blank, start);
	populateFRAM(evalstr, start);	//index 19, evaluation bar
	populateFRAM(evalbar, start);	//index 20, evaluation graph
	populateFRAM(evalscr, start);		//index 21, evaluation score
									//" W.Eval | B.Eval ";
									//"-????1234-????567";
									// 0123456789
	//-------------------------------------------22 lines
	populateFRAM(forfeit, start);
	populateFRAM(yesno, start);
	populateFRAM(draw, start);
	populateFRAM(yesno, start);
	populateFRAM(restart, start);
	populateFRAM(yesno, start);
	populateFRAM(score, start);
	populateFRAM(blank, start);

	invi_row(FORFEITROW+1);
	invi_row(DRAWROW+1);
	invi_row(RESTARTROW+1);

	invi_row(P1PIECES);
	invi_row(P1PIECES+1);
	invi_row(P1PIECES+2);
	invi_row(P1PIECES+3);

	invi_row(P2PIECES);
	invi_row(P2PIECES+1);
	invi_row(P2PIECES+2);
	invi_row(P2PIECES+3);
	return;
}

void balance_evalbar(){
	//low invisible bit means white advantage bar
	blackbar_index = BLACKBARMID;
	whitebar_index = WHITEBARMID;
	vga_ctrl->VRAM[EVALBARSTART] &= TURNOFFINVI;
	vga_ctrl->VRAM[EVALBARSTART+1] &= TURNOFFINVI;
	vga_ctrl->VRAM[EVALBARSTART+2] &= TURNOFFINVI;
	vga_ctrl->VRAM[EVALBARSTART+3] &= TURNOFFINVI;
	vga_ctrl->VRAM[EVALBARSTART+4] &= TURNOFFINVI;
	vga_ctrl->VRAM[EVALBARSTART+5] &= TURNOFFINVI;
	vga_ctrl->VRAM[EVALBARSTART+6] &= TURNOFFINVI;
	vga_ctrl->VRAM[EVALBARSTART+7] &= TURNOFFINVI;

	int black_barstart = EVALBARSTART + 8;
	vga_ctrl->VRAM[black_barstart] |= TURNONINVI;
	vga_ctrl->VRAM[black_barstart+1] |= TURNONINVI;
	vga_ctrl->VRAM[black_barstart+2] |= TURNONINVI;
	vga_ctrl->VRAM[black_barstart+3] |= TURNONINVI;
	vga_ctrl->VRAM[black_barstart+4] |= TURNONINVI;
	vga_ctrl->VRAM[black_barstart+5] |= TURNONINVI;
	vga_ctrl->VRAM[black_barstart+6] |= TURNONINVI;
	vga_ctrl->VRAM[black_barstart+7] |= TURNONINVI;
}

void update_evalbar(int difference){
	loop(100);
	balance_evalbar();
	loop(100);

	int white_adv = 0, black_adv = 0;
	if (difference < 0){
		black_adv = 1;
	}else if (difference > 0){
		white_adv = 1;
	}else{
		return;
	}

	int abs_diff = floor(absol(difference) /(EVALBARWEIGHT*1.0));	//how many bars we need to flip
	abs_diff = min(7, abs_diff);	//always leave at least one bar for opponent


	int attack_index;
	if (white_adv){
		attack_index = blackbar_index;

		//updating the blackbar_index reference and whitebar_index reference
		blackbar_index += abs_diff;
		whitebar_index += abs_diff;
		while(abs_diff){
			vga_ctrl->VRAM[attack_index] &= TURNOFFINVI;
			attack_index++;
			abs_diff--;
		}
		return;
	}
	else if(black_adv){
		attack_index = whitebar_index;

		blackbar_index -= abs_diff;
		whitebar_index -= abs_diff;
		//going in the negative direction of bar
		while(abs_diff){
			vga_ctrl->VRAM[attack_index] |= TURNONINVI;
			attack_index--;
			abs_diff--;
		}
		return;
	}
}

//max score is 810 = 80 + 100 + 120 + 90 + 500 = 890
int get_piece_score(alt_u8 piece_index){
	int index=0;
	switch(piece_index){
	case 0:	//pawn
		index = 10;
		break;
	case 1:	//rook
		index = 50;
		break;
	case 2:		//knight and bishop
	case 3:
		index = 30;
		break;

	case 4:			//queen
		index = 90;
		break;
	case 5:			//king
		index = 500;
		break;
	default:
		index = 0;
		break;
	}
	return index;
}

//display the score by modifying VRAM
void update_eval_score(int white_score, int black_score){
	int wsign = 0, bsign = 0;	//positive

	if (white_score < 0) wsign = 1;
	if (black_score < 0) bsign = 1;

	white_score = absol(white_score);
	black_score = absol(black_score);

	int w0 = white_score / 1000;	w0 = absol(w0);
	int w1 = (white_score-(w0*1000)) / 100;	w1 = absol(w1);							//getting the hundredths digit
	int w2 = (white_score -(w0*1000) -(w1 * 100)) / 10; w2 = absol(w2);				//getting the tenths digit
	int w3 = white_score - (w0*1000) - (w1 * 100) - (w2 * 10); w3 = absol(w3);						//getting the unit digit

	int b0 = black_score / 1000;	b0 = absol(b0);
	int b1 = (black_score-(b0*1000)) / 100;   	b1 = absol(b1);
	int b2 = (black_score -(b0*1000) -(b1*100)) / 10;   b2 = absol(b2);
	int b3 = black_score - (b0*1000) - (b1 * 100) - (b2 * 10);	b3 = absol(b3);

	//revealing the sign bit on display
//	if (wsign){
//		vga_ctrl->VRAM[WHITEEVALSCORE] = vga_ctrl->VRAM[WHITEEVALSCORE] & TURNOFFINVI;
//	}else{
//		vga_ctrl->VRAM[WHITEEVALSCORE] = vga_ctrl->VRAM[WHITEEVALSCORE] | TURNONINVI;
//	}
//
//	if (bsign){
//		vga_ctrl->VRAM[BLACKEVALSCORE] = vga_ctrl->VRAM[BLACKEVALSCORE] & TURNOFFINVI;
//	}else{
//		vga_ctrl->VRAM[BLACKEVALSCORE] = vga_ctrl->VRAM[BLACKEVALSCORE] | TURNONINVI;
//	}

	vga_ctrl->VRAM[WHITEEVALSCORE+2] = (char)('0' + w0);
	vga_ctrl->VRAM[WHITEEVALSCORE+3] = (char)('0' + w1);
	vga_ctrl->VRAM[WHITEEVALSCORE+4] = (char)('0' + w2);
	vga_ctrl->VRAM[WHITEEVALSCORE+5] = (char)('0' + w3);

	vga_ctrl->VRAM[BLACKEVALSCORE+2] = (char)('0' + b0);
	vga_ctrl->VRAM[BLACKEVALSCORE+3] = (char)('0' + b1);
	vga_ctrl->VRAM[BLACKEVALSCORE+4] = (char)('0' + b2);
	vga_ctrl->VRAM[BLACKEVALSCORE+5] = (char)('0' + b3);
}

//modifying VRAM content that display player scores
void update_score(int white_score, int black_score){
	int w1 = white_score / 100;	//getting the hundredths digit
	int w2 = (white_score - (w1 * 100)) / 10; //getting the tenths digit
	int w3 = white_score - (w1 * 100) - (w2 * 10);	//getting the unit digit

	int b1 = black_score / 100;
	int b2 = (black_score - (b1*100)) / 10;
	int b3 = black_score - (b1 * 100) - (b2 * 10);

	vga_ctrl->VRAM[WHITESCOREINDEX] = (char)('0' + w1);
	vga_ctrl->VRAM[WHITESCOREINDEX+1] = (char)('0' + w2);
	vga_ctrl->VRAM[WHITESCOREINDEX+2] = (char)('0' + w3);

	vga_ctrl->VRAM[BLACKSCOREINDEX] = (char)('0' + b1);
	vga_ctrl->VRAM[BLACKSCOREINDEX+1] = (char)('0' + b2);
	vga_ctrl->VRAM[BLACKSCOREINDEX+2] = (char)('0' + b3);
}

void populatemessage(int player, int clear, int forfeit, int draw){
	//let row 12 to be row for game status message
	int message_row = 12;	//this is an index, index 0 - 11 has been taken by player pieces and dashes
	int* start;
	*start = 64 + (message_row * 17);
	printf("Entered populate message\n");
	if (clear & forfeit & draw){
		if (player == 1){
			populateFRAM(p1w, start);
			return;
		}
		populateFRAM(p2w, start);
		return;
	}


	if (clear == 1 || clear == -1){
		populateFRAM(blank, start);
		return;
	}else if(clear == -2){
		populateFRAM(m0, start);
		return;
	}


	//+1 means player accepted, -1 means player rejected
	//-2 means someone clicked on the settings button
	if (player == 1){
		printf("Player 1\n");
		switch(forfeit){
		case -2:
			populateFRAM(m1, start);
			return;
			break;
		case 1:
			populateFRAM(m2, start);
			return;
			break;
		case -1:
			populateFRAM(blank, start);		//player 1 decides to not forfeit
			return;
			break;
		default:
			break;
		}

		switch (draw){
		case -2:
			populateFRAM(m3, start);
			return;
			break;
		case 1:
			populateFRAM(m4, start);
			return;
			break;
		case -1:
			populateFRAM(m5, start);
			return;
			break;
		default:
			break;
		}
	}

	if (player == 2){
		printf("Player 2\n");
		switch(forfeit){
		case -2:
			populateFRAM(m2_1, start);
			return;
			break;
		case 1:
			populateFRAM(m2_2, start);
			return;
			break;
		case -1:
			populateFRAM(blank, start);		//player 1 decides to not forfeit
			return;
			break;
		default:
			break;
		}

		switch (draw){
		case -2:
			populateFRAM(m2_3, start);
			return;
			break;
		case 1:
			populateFRAM(m2_4, start);
			return;
			break;
		case -1:
			populateFRAM(m2_5, start);
			return;
			break;
		default:
			break;
		}

	}
	return;
}
//returns the settings row that has been clicked
//if we did not click a settings row, return 3
//settings 0 = forfeit
//settings 1 = draw
//settings 2 = restart
alt_u8 settings_clicked(unsigned long mx, unsigned long my){
	int inside_statusbar = 0;
	int y_offset;
	int index;
	if ((mx >= 504) && (mx <= 639) ){
		inside_statusbar = 1;
	}

	//we are not inside any tile
	if (inside_statusbar == 0){
		index = 3;		//13 because this value is impossible to be in VRAM[0..63],
						//because we have only 13 chess tiles (so technically 0 to 12 values are possible)
		return index;
	}

	y_offset = my / 16;
	switch (y_offset){
	case FORFEITROW:
		index = 0;
		break;
	case DRAWROW:
		index = 1;
		break;
	case RESTARTROW:
		index = 2;
		break;
	default:
		index = 3;
		break;
	}
	printf("Entered-exiting settingsclicked %d \n", index);
	return index;
}

int yesno_clicked(unsigned long mx, unsigned long my){
	int forfeit_yesno=0, draw_yesno=0, restart_yesno=0;
	int solution = 0;
	//not within status bar
	if (!(mx >= 504 && mx <= 639)){
		return 0;
	}

	forfeit_yesno = ((my >= 368) && (my<=383));
	draw_yesno = ((my>=400) && (my<=415));
	restart_yesno = ((my>=432) && (my<=447));

	if(forfeit_yesno){
		if (mx < YESNOLINE)solution= 1;
		else if (mx > YESNOLINE)solution= -1;
		else solution= 0;
	}
	else if(draw_yesno){
		if (mx < YESNOLINE)solution= 2;
		else if (mx > YESNOLINE)solution= -2;
		else solution= 0;
	}
	if(restart_yesno){
		if (mx < YESNOLINE)solution= 3;
		else if (mx > YESNOLINE)solution= -3;
		else solution= 0;
	}
	printf("Exiting yesno %d \n", solution);
	return solution;					//inside status bar but did not click yes no
}

//setting the invi bit to high
void invi_row(int row){
	int rowfoo =64 + (row * 17);
	for (int i=rowfoo; i<rowfoo+17; i++){
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | 0b10000000;
	}
}

//index 0 pawn
//index 1 rook
//index 2 knight
//index 3 bishop
//index 4 queen
//index 5 king

//player represents the victim
void reveal_dead_piece(int index, int count, int player){
	printf("Revealing a dead piece!\n");
	int start;
	int pawn_special = count / 5;	//if this is greater than or equal to 1, calculation needs to handle

	if (player == BLACK){
		start = 183;
	}else if (player == WHITE){
		start = 81;
	}

	switch(index){
	case 0:
		//pawn
		start = start + ((count-1) * PWN) + pawn_special;
		vga_ctrl->VRAM[start] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+1] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+2] &= TURNOFFINVI;

		break;
	case 1:
		//rook
		start += 34;
		start = start + ((count - 1) * RK);
		vga_ctrl->VRAM[start] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+1] &= TURNOFFINVI;
		//vga_ctrl[start+2] &= TURNOFFINVI;
		break;
	case 2:
		start = start + 34 + (2 * RK);
		start = start + ((count-1) * KN);
		vga_ctrl->VRAM[start] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+1] &= TURNOFFINVI;
		break;
	case 3:
		start = start + 34 + (2 * RK) + (2 * KN);
		start = start + ((count-1) * BI);
		vga_ctrl->VRAM[start] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+1] &= TURNOFFINVI;
		break;
	case 4:
		start += 51;
		vga_ctrl->VRAM[start] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+1] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+2] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+3] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+4] &= TURNOFFINVI;

		break;
	case 5:
		start = start + 51 + 6;		//moving to king
		vga_ctrl->VRAM[start] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+1] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+2] &= TURNOFFINVI;
		vga_ctrl->VRAM[start+3] &= TURNOFFINVI;
		break;
	default:
		break;
	}
	printf("Revealed a dead piece!\n");
}

int captured_piece_index(alt_u8 piece){
	int index=-1;
	switch(piece){
	case BLACKPAWN:
	case WHITEPAWN:
		index = 0;
		break;
	case BLACKROOK:
	case WHITEROOK:
		index = 1;
		break;
	case BLACKKNIGHT:
	case WHITEKNIGHT:
		index = 2;
		break;
	case BLACKBISHOP:
	case WHITEBISHOP:
		index = 3;
		break;
	case BLACKQUEEN:
	case WHITEQUEEN:
		index = 4;
		break;
	case BLACKKING:
	case WHITEKING:
		index = 5;
		break;
	default:
		index = -1;
		break;
	}
	return index;
}

//Toggle glow of settings bar after we click anywhere else that is not related to the settings
void glow_row(int row, int glow_boolean){
	int start;

	alt_u8 toggle_glow;
	if (glow_boolean){
		toggle_glow = 0b01111111;	//turning on glow = showing the pixels for yesno, change status bar to red
	}else{
		toggle_glow = 0b10000000;	//turn off glow = hiding yes no, change status bar to green
	}

	switch(row){
	case 0:
		start = 64 + (FORFEITROW * 17);
		break;
	case 1:
		start = 64 + (DRAWROW * 17);
		break;
	case 2:
		start = 64 + (RESTARTROW * 17);
		break;
	default:
		break;
	}

	for (int i=start; i<start+17;i++){
		int choice_start;
		if (glow_boolean){
			//turn off the MSB
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] & toggle_glow;	//setting glow
			vga_ctrl->VRAM[i+17] = vga_ctrl->VRAM[i+17] & toggle_glow;	//setting visible (YES NO)
		}
		else{
			//turn on the MSB
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | toggle_glow;
			vga_ctrl->VRAM[i+17] = vga_ctrl->VRAM[i+17] | toggle_glow;
		}
	}

}
