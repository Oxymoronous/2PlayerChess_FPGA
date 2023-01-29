#ifndef CHESSBOARD_H_
#define CHESSBOARD_H_
#include <system.h>
#include <alt_types.h>

#define CHESSPIECES	64
#define SPECIALCODE 13
#define STATUSBARCHARACTERS 510

//#define ROOK		1
//#define KNIGHT		2
//#define BISHOP		3
//#define QUEEN		4
//#define KING		5
//#define PAWN		6
//#define EMPTY		7
//typedef unsigned char alt_u8;

#define BLACKROOK	0b00000000
#define BLACKKNIGHT 0b00000001
#define BLACKBISHOP	0b00000010
#define BLACKQUEEN	0b00000011
#define BLACKKING	0b00000100
#define BLACKPAWN	0b00000101

#define WHITEPAWN	0b00000110
#define WHITEROOK	0b00000111
#define WHITEKNIGHT 0b00001000
#define WHITEBISHOP	0b00001001
#define WHITEQUEEN	0b00001010
#define WHITEKING	0b00001011

#define JUSTEMPTY	0b00001100
#define PAWNSECOND  0b10000000
#define CODECHECK	0b00001111

struct TEXT_VGA_STRUCT {
	char VRAM [CHESSPIECES+STATUSBARCHARACTERS];				//index 0 to 63
	unsigned short int MOUSE_POSITION_X;
	unsigned short int MOUSE_POSITION_Y;
};

//you may have to change this line depending on your platform designer
static volatile struct TEXT_VGA_STRUCT* vga_ctrl = VGA_TEXT_MODE_CONTROLLER_0_BASE;

void running();
//void set_x_displacement(alt_u8 x_displacement);
//void set_y_displacement(alt_u8 y_displacement);
//void set_button_status(alt_u8 button_status);

void set_x_displacement(unsigned short int x_displacement);
void set_y_displacement(unsigned short int y_displacement);
void set_button_status(unsigned short int button_status);

void populatechessboard();

//void textVGAColorClr();
//void textVGADrawColorText(char* str, int x, int y, alt_u8 background, alt_u8 foreground);
//void setColorPalette (alt_u8 color, alt_u8 red, alt_u8 green, alt_u8 blue); //Fill in this code
//void textVGAColorScreenSaver(); //Call this for your demo


#endif /* TEXT_MODE_VGA_COLOR_H_ */
