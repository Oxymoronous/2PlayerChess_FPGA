#ifndef STATUSBAR_H_
#define STATUSBAR_H_

#include "chessboard.h"

#define ROWCHARCOUNT	17
#define FORFEITROW		22
#define DRAWROW			24
#define RESTARTROW		26

#define P1PIECES		1
#define P2PIECES		7
#define WHITE			0
#define BLACK			1

#define PWN				4
#define RK				3
#define KN				3
#define BI				3

#define TURNOFFINVI		0b01111111
#define TURNONINVI		0b10000000
#define YESNOLINE		568	//dx position for vertical line
#define WHITESCOREINDEX	64 + (29 * 17)
#define BLACKSCOREINDEX WHITESCOREINDEX + 10

#define EVALBARSTART	64 + (20*17)
#define WHITEBARMID		EVALBARSTART + 7
#define BLACKBARMID		WHITEBARMID + 1
#define WHITEEVALSCORE	64 + (21 * 17)
#define BLACKEVALSCORE	WHITEEVALSCORE + 9
#define EVALBARWEIGHT	300

extern int blackbar_index;
extern int whitebar_index;

void populatestatusbar();
void populateFRAM(const char* string, int* start);

#endif
