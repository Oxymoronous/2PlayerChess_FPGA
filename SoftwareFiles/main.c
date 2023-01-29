#include <stdio.h>
#include <time.h>
#include "system.h"
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
#include "statusbar.h"
#include "evaluate.h"
#include "rand.h"

#define EMPTYTILE_CODE	12

static const unsigned char REMOVEPIECE 	=	0b00010000;	//clears all bits except parity
static const unsigned char GETPIECECODE =	0b00001111;
static int black_pieces[6] = {0,0,0,0,0,0};
static int white_pieces[6] = {0,0,0,0,0,0};
static int settings_flag[3] = {0,0,0};
static int endgame = 0;
static int wscore = 0;
static int bscore = 0;

int main(){
	printf("Entering populate chess board... \n");
	populatechessboard();
	populatestatusbar();

	//Alpha-beta
	//Config* c = init_config();

	init_tables();
	loop(100);
	vga_ctrl->MOUSE_POSITION_X = 320;
	vga_ctrl->MOUSE_POSITION_Y = 240;

	running();
	return 0;
}

void clear_settings_flag(){
	settings_flag[0] = 0;
	settings_flag[1] = 0;
	settings_flag[2] = 0;
	return;
}

void clear_all_pieces(){
	for(int i =0 ;i<6; i++){
		black_pieces[i] = 0;
		white_pieces[i] = 0;
	}
	return;
}
//ECE 385 USB Host Shield code
//based on Circuits-at-home USB Host code 1.x
//to be used for ECE 385 course materials
//Revised October 2020 - Zuofu Cheng
extern HID_DEVICE hid_device;

static BYTE addr = 1; 				//hard-wired USB address
const char* const devclasses[] = { " Uninitialized", " HID Keyboard", " HID Mouse", " Mass storage" };

BYTE GetDriverandReport() {
	BYTE i;
	BYTE rcode;
	BYTE device = 0xFF;
	BYTE tmpbyte;

	DEV_RECORD* tpl_ptr;
	printf("Reached USB_STATE_RUNNING (0x40)\n");
	for (i = 1; i < USB_NUMDEVICES; i++) {
		tpl_ptr = GetDevtable(i);
		if (tpl_ptr->epinfo != NULL) {
			printf("Device: %d", i);
			printf("%s \n", devclasses[tpl_ptr->devclass]);
			device = tpl_ptr->devclass;
		}
	}
	//Query rate and protocol
	rcode = XferGetIdle(addr, 0, hid_device.interface, 0, &tmpbyte);
	if (rcode) {   //error handling
		printf("GetIdle Error. Error code: ");
		printf("%x \n", rcode);
	} else {
		printf("Update rate: ");
		printf("%x \n", tmpbyte);
	}
	printf("Protocol: ");
	rcode = XferGetProto(addr, 0, hid_device.interface, &tmpbyte);
	if (rcode) {   //error handling
		printf("GetProto Error. Error code ");
		printf("%x \n", rcode);
	} else {
		printf("%d \n", tmpbyte);
	}
	return device;
}

void running(){
	BYTE rcode;
	BOOT_MOUSE_REPORT buf;		//USB mouse report

	BYTE runningdebugflag = 0;//flag to dump out a bunch of information when we first get to USB_STATE_RUNNING
	BYTE errorflag = 0; //flag once we get an error device so we don't keep dumping out state info
	BYTE device;

	signed short int tmp_mx, tmp_my;		//use signed long variable to perform subtraction from mouse data packet vector
	alt_u8 chess_piece_code, chess_vram_index, settings_row;
	alt_u8 chess_glow_code;

	int white_turn, black_turn, turn_check, previous_index;
	int player, captured_index, cmp;
	alt_u8 previous_piece;
	white_turn = 1;
	black_turn = 0;

	int wanttomove = 0;
	int moved = 0;

	int yesno_status = 0;
	settings_row = 3;

	int counter = 0;

	printf("initializing MAX3421E...\n");
	MAX3421E_init();
	printf("initializing USB...\n");
	USB_init();
	printf("Zeroth %d %d  \n", w_evalscore, b_evalscore);
	update_score(wscore, bscore);			//initializing both player scores to 0
	loop(100);
	side2move = 0;
	cmp = eval();
	update_evalbar(cmp);
	//Config c = init_config();
	loop(100);								//beginning of the game, both eval scores should be equal
	printf("Initial %d %d  \n", w_evalscore, b_evalscore);
	update_eval_score(w_evalscore, b_evalscore);		//displaying player evaluation scores
	//setting up USB polling for mouse action
	while (1) {
		counter+=1;
		MAX3421E_Task();
		USB_Task();
		if (GetUsbTaskState() == USB_STATE_RUNNING) {
			if (!runningdebugflag) {
				runningdebugflag = 1;
				device = GetDriverandReport();
			}
			else if (device == 2) {
				rcode = mousePoll(&buf);
				if (rcode == hrNAK) {
					//no mouse packet is sent (I think)
					set_x_displacement(vga_ctrl->MOUSE_POSITION_X);
					set_y_displacement(vga_ctrl->MOUSE_POSITION_Y);
					set_button_status(0);
					//printf(".");
					continue;
				} else if (rcode) {
					continue;
				}
				//rcode == 0 execute below
				//printf("Executing %d ...\n", counter);
				tmp_mx = (signed short int)vga_ctrl->MOUSE_POSITION_X;
				tmp_my = (signed short int)vga_ctrl->MOUSE_POSITION_Y;

				set_x_displacement((unsigned short int)max(min(tmp_mx + (signed char) buf.Xdispl, 624), 0));
				set_y_displacement((unsigned short int)max(min(tmp_my + (signed char) buf.Ydispl, 464), 0));
				set_button_status(buf.button);

				vga_ctrl->MOUSE_POSITION_X = (unsigned short int)max(min(tmp_mx + (signed char) buf.Xdispl, 624), 0);
				vga_ctrl->MOUSE_POSITION_Y = (unsigned short int)max(min(tmp_my + (signed char) buf.Ydispl, 464), 0);

				//List* moves = init_list();					//creating a new list for moves

				if (white_turn){
					player = 1;
				}
				else{
					player = 2;
				}


				//when white reaches its own turn again, the special bit for en passant is turned off
				if (white_turn) clear_whitepawn_sp();
				else clear_blackpawn_sp();

				eval();		//this is for restarting
							//we do not care about the score difference here, we just want to reupdate the scores
				update_eval_score(w_evalscore, b_evalscore);				//need this for restarting the board
				//initializing because yesno status might change the settings flag
				glow_row(0, settings_flag[0]);
				glow_row(1, settings_flag[1]);
				glow_row(2, settings_flag[2]);

				if (buf.button & 0x01 == 1){
					chess_piece_code = tile_clicked(vga_ctrl->MOUSE_POSITION_X, vga_ctrl->MOUSE_POSITION_Y);
					//returns 0 for ff, 1 for draw, 2 for restart
					settings_row = settings_clicked(vga_ctrl->MOUSE_POSITION_X, vga_ctrl->MOUSE_POSITION_Y);

                    //toggling the settings flag
					//if someone clicks on any of the settings bar, toggle that settings bar and set all other bars to low
					//what happens when we click on settings bar, and then click on chess tile?
					switch(settings_row){
					case 0:
						settings_flag[settings_row] = (settings_flag[settings_row] + 1)%2;
						settings_flag[1] = 0;
						settings_flag[2] = 0;
						break;
					case 1:
						settings_flag[settings_row] = (settings_flag[settings_row] + 1)%2;
						settings_flag[0] = 0;
						settings_flag[2] = 0;
						break;
					case 2:
						settings_flag[settings_row] = (settings_flag[settings_row] + 1)%2;
						settings_flag[0] = 0;
						settings_flag[1] = 0;
						break;

					default:
						//settings_flag[0] = 0;
						//settings_flag[1] = 0;
						//settings_flag[2] = 0;
						break;

					}

					glow_row(0, settings_flag[0]);
					glow_row(1, settings_flag[1]);
					glow_row(2, settings_flag[2]);
					printf("settingsflags : %d  %d  %d\n", settings_flag[0], settings_flag[1], settings_flag[2]);
                    if (settings_flag[0] == 1){
						//if one of the player consider forfeits, move to the next round with same player turns
						//no player is allowed to move until unclick the bar
                    	populatemessage(player, 0, -2, 0); //player clicked forfeit bar
					}

					if (settings_flag[1] == 1){
						//if one of the player suggested draw, switch turns
						populatemessage(player, 0, 0, -2); //player clicked forfeit bar
						white_turn = (white_turn + 1)%2;
						black_turn = (black_turn + 1)%2;
					}

					if (settings_flag[2] == 1){
						populatemessage(player, -2, 0, 0);
					}

					//if not a single settings bar is clicked
					if ((settings_flag[0] | settings_flag[1] | settings_flag[2]) == 0){
						populatemessage(player, 1, 0, 0);
						//continue;
					}

					//tells us whether we click on the status bar yes no region
					yesno_status = yesno_clicked(vga_ctrl->MOUSE_POSITION_X, vga_ctrl->MOUSE_POSITION_Y);

					if (yesno_status != 0){
						//yesno_mod +-1 = forfeit, +-2 = draw, restart = +-3
                        int yesno_mod = yesno_status;
						int f = 0, d = 0, r = 0;
						if (yesno_status < 0) yesno_mod = yesno_status * (-1);

						f = yesno_mod == 1;		//clicked on ff yesno
						d = yesno_mod == 2;
						r = yesno_mod == 3;

						//negative means accepted, positive means proposal
						//populate function(player, clear, forfeit, draw)
                        if (f && settings_flag[yesno_mod-1]){
							if (yesno_status > 0){
								//a player chose to forfeit
								printf("Forfeited\n");
								populatemessage(player, 0, 1, 0);
							}else{
								//player decide to not forfeit
								printf("Not forfeit\n");
								populatemessage(player, 0, -1, 0);
							}
						}

						if (d && settings_flag[yesno_mod-1]){
							if (yesno_status > 0){
								//opponent accepted draw
								printf("Draw\n");
								populatemessage(player, 0, 0, 1);
							}else{
								//opponent rejected draw
								printf("No draw\n");
								populatemessage(player, 0, 0, -1);
							}
						}

						if (r && settings_flag[yesno_mod-1]){
							if (yesno_status > 0){
								//chose to restart
								loop(1000);
								populatechessboard();		//clears and restarts chessboard here
								loop(1000);
								populatestatusbar();		//clears and restarts chessboard here
								loop(1000);
								white_turn = 1;	//restarts player turns
								black_turn = 0;
								clear_all_pieces();
								clear_settings_flag();

								wscore = 0;
								bscore = 0;

								//resetting
								reset_board();
								side2move = 0;
								blackbar_index = BLACKBARMID;
								whitebar_index = WHITEBARMID;

								endgame = 0;
								loop(100);
								update_score(wscore, bscore);
								cmp = eval();
								update_evalbar(cmp);
								update_eval_score(w_evalscore, b_evalscore);
								loop(100);
							}else{
								//chose to not restart
								populatemessage(player, 1, 0, 0);
							}
						}
                        settings_flag[yesno_mod - 1] = 0;
					}

					if (endgame) continue;	//a player previously won the game, waiting for user to restart

					if (chess_piece_code == SPECIALCODE){
						continue;		//do nothing because we did not click a chess tile
					}
					chess_vram_index = chessVramIndex(vga_ctrl->MOUSE_POSITION_X, vga_ctrl->MOUSE_POSITION_Y);
					if (chess_vram_index == 64){
						continue;
					}
					chess_glow_code = vga_ctrl->VRAM[chess_vram_index] & 0b00100000;

					turn_check = valid_turn(chess_piece_code & GETPIECECODE, white_turn, black_turn);

					if (!chess_glow_code && (chess_piece_code != EMPTYTILE_CODE) && turn_check){
						//clicked on a chess_piece tile that is not glowed --> want to move pieces
						turn_off_glow();							//everytime click on a chess_piece, turns off all glow
						previous_piece = chess_piece_code & GETPIECECODE;
						previous_index = chess_vram_index;			//need to remember so we can clear tile when user hits on glowed tile
						foo(chess_piece_code, chess_vram_index);	//turns on glow here, this does the left shifting
					}

					else if (chess_glow_code){
						//clicked on a glowed tile, anyone can move!
						//how do we castle
						turn_off_glow();
						int short_castle = 0;
						int long_castle = 0;
						int index_diff = chess_vram_index - previous_index;
						if (index_diff == 2)short_castle = 1;	//short castle
						if (index_diff == -2)long_castle = 1;	//long  castle

						alt_u8 captured_piece = vga_ctrl->VRAM[chess_vram_index] & CODECHECK;
						//int score = get_piece_score(captured_piece);
						//handling special cases for castling and en passant
						if (previous_piece == WHITEKING){
							if (short_castle){
								vga_ctrl->VRAM[61] &= REMOVEPIECE;	//clear first
								vga_ctrl->VRAM[61] |= WHITEROOK;
								vga_ctrl->VRAM[63] = (vga_ctrl->VRAM[63] & REMOVEPIECE) + 12;
							}
							if (long_castle){
								vga_ctrl->VRAM[59] &= REMOVEPIECE;
								vga_ctrl->VRAM[59] |= WHITEROOK;
								vga_ctrl->VRAM[56] = (vga_ctrl->VRAM[56] & REMOVEPIECE) + 12;
							}
						}
						if (previous_piece == BLACKKING){
							if (short_castle){
								vga_ctrl->VRAM[5] &= REMOVEPIECE;
								vga_ctrl->VRAM[5] |= BLACKROOK;
								vga_ctrl->VRAM[7] = (vga_ctrl->VRAM[7] & REMOVEPIECE) + 12;
							}
							if (long_castle){
								vga_ctrl->VRAM[3] &= REMOVEPIECE;
								vga_ctrl->VRAM[3] |= BLACKROOK;
								vga_ctrl->VRAM[0] = (vga_ctrl->VRAM[0] & REMOVEPIECE) + 12;
							}
						}


						vga_ctrl->VRAM[chess_vram_index] = (vga_ctrl->VRAM[chess_vram_index] & REMOVEPIECE) | previous_piece;
						vga_ctrl->VRAM[previous_index] = (vga_ctrl->VRAM[previous_index] & REMOVEPIECE) + 12;	//clearing the previous tile																												//into a blank tile (type 12)

						//setting special bit for pawn first time moving two tiles
						//handling en passant capturing tiles
						if (previous_piece == BLACKPAWN){
							if (index_diff == 16){
								vga_ctrl->VRAM[chess_vram_index] |= PAWNSECOND;
							}

							if ((index_diff!=8) && (index_diff!=16)&&(captured_piece==JUSTEMPTY)){
								//this means we are performing an enpassant capture
								captured_piece = WHITEPAWN;
								vga_ctrl->VRAM[chess_vram_index-8] = (vga_ctrl->VRAM[chess_vram_index-8] & REMOVEPIECE) + 12;	//capture the enemy enpassant pawn
							}
						}

						if (previous_piece == WHITEPAWN){
							//printf("Here\n");
							//printf("INDEXDIFF: %d \n ", index_diff);
							if (index_diff == -16){
								vga_ctrl->VRAM[chess_vram_index] |= PAWNSECOND;
							}

							if ((index_diff!=(-8)) && (index_diff != -16)&&(captured_piece==JUSTEMPTY)){
								//this means we are performing an enpassant capture
								//printf("here2\n");
								captured_piece = BLACKPAWN;
								vga_ctrl->VRAM[chess_vram_index+8] = (vga_ctrl->VRAM[chess_vram_index+8] & REMOVEPIECE) + 12;	//capture the enemy enpassant pawn
							}
						}
						//at this point, any pieces that have been captured, must already been replaced
						VGA_EvalBoard_Update(previous_index, chess_vram_index);	//this is to update the evaluation board representation
						loop(100);
						cmp = eval();
						update_evalbar(cmp);		//get difference after player makes a move, and update the evaluation bar
						loop(100);

						captured_index = captured_piece_index(captured_piece);
						//printf("CAPTURED! %d", captured_index);
						if (captured_index == -1){
							//did not capture any pieces
							white_turn = (white_turn+1)%2;
							black_turn = (black_turn+1)%2;		//only switch turns when we hit on a glow tile
							side2move = white_turn ^ 1;
							continue;
						}

						//capture a piece
						int a, score;
						score = get_piece_score(captured_index);
						if (white_turn){
							//white captures black
							black_pieces[captured_index] += 1;
							a = black_pieces[captured_index];
							wscore += score;
							loop(100);
							update_score(wscore, bscore);
							//that means we are revealing black pieces
							loop(100);
							reveal_dead_piece(captured_index, black_pieces[captured_index], BLACK);
							if (black_pieces[5]){
								printf("White Ends Game!\n");
								populatemessage(player, 1,1,1);
								endgame = 1;
								continue;
							}
						}else{
							//black captures white
							white_pieces[captured_index] += 1;
							a = white_pieces[captured_index];
							bscore += score;
							loop(100);
							update_score(wscore, bscore);
							loop(100);
							//revealing white tomb
							reveal_dead_piece(captured_index, white_pieces[captured_index], WHITE);
							if (white_pieces[5]){
								printf("Black Won!\n");
								populatemessage(player, 1, 1, 1);
								endgame = 1;
								continue;
							}
						}
						white_turn = (white_turn+1)%2;
						black_turn = (black_turn+1)%2;		//only switch turns when we hit on a glow tile
						side2move = white_turn ^ 1;
					}

				}


			}
		} else if (GetUsbTaskState() == USB_STATE_ERROR) {
			if (!errorflag) {
				errorflag = 1;
				printf("USB Error State\n");
				//print out string descriptor here
			}
		} else //not in USB running state
		{
			printf("USB task state: ");
			printf("%x\n", GetUsbTaskState());
			if (runningdebugflag) {	//previously running, reset USB hardware just to clear out any funky state, HS/FS etc
				runningdebugflag = 0;
				MAX3421E_init();
				USB_init();
			}
 			errorflag = 0;
		}
		//counter += 1;
	}
	return;
}

void set_x_displacement(unsigned short int mouse_x){
	IOWR_ALTERA_AVALON_PIO_DATA(0x8001540, mouse_x);
}

void set_y_displacement(unsigned short int mouse_y){
	IOWR_ALTERA_AVALON_PIO_DATA(0x8001530, mouse_y);
}

void set_button_status(unsigned short int button_status){
	IOWR_ALTERA_AVALON_PIO_DATA(0x8001550, button_status);
}
