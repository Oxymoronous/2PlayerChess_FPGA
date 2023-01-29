//#include <stdio.h>
//#include <stdlib.h>
//#include "rand.h"
//
//
//const int global_board2[64] = {
//		BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK,
//		BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN,
//		EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY,
//		EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY,
//		EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY,
//		EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY, EVAL_EMPTY,
//		WHITE_PAWN, WHITE_PAWN,  WHITE_PAWN, WHITE_PAWN,  WHITE_PAWN,  WHITE_PAWN,  WHITE_PAWN,  WHITE_PAWN,
//		WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK
//};
//
//Config* init_config(){
//	//Config c = {4, 1, WHITE, 0, WHITE, 1};	//depth, mode, user_color, endgame, turn, bestscore
//	Config* c = malloc(sizeof(Config));
//	c->DEPTH = 4;
//	c->MODE = 1;
//	c->USER_COLOR = WHITE;
//	c->END_GAME = 0;
//	c->TURN = 0;
//	c->BEST = 1;
//	for(int i=0; i<64; i++){
//		c->BOARD[i] = global_board2[i];
//	}
//	return c;
//}
//
//Config* create_new_config(PtrConfig c, int turn, int board[64], int depth){
//	Config* new = init_config();
//	new->TURN = turn;
//	new->DEPTH = depth;
//	new->BEST = c->BEST;
//
//	for (int i=0; i<64; i++){
//		new->BOARD[i] = board[i];
//	}
//
//	return new;
//}
//
//int copy_board(int board[64], int nextboard[64]){
//	int i;
//	for (i = 0; i < 64; i++){
//		nextboard[i] = board[i];
//	}
//	return 1;
//}
//
////remove the first node of the list
////make the next of removed first node to be the new first;
//void pop_data(List* list){
//	PtrNode next = list->first->next;
//	free(list->first);
//	list->first = next;
//	list->size --;
//}
//
//List* init_list(){
//	List* new = malloc(sizeof(List));
//	if (new == NULL){
//		return NULL;
//	}
//	new->first = NULL;
//	new->size = 0;
//	return new;
//}
//
//int free_list(List* list){
//	while(list->size > 0){
//		pop_data(list);
//	}
//	free(list);
//	return 1;
//}
//
////int king_is_taken(PtrConfig c, Move move){
////	Config new = create_new_config(c, c->TURN);
////	return 1;
////}
//
//Move create_move(int start, int end, int piece, Config* c, int threat){
//	Move move = {start, end, piece, -1, threat};
//	copy_board(c->BOARD, (&move)->board);
//	(&move)->board[start] = EVAL_EMPTY;
//	(&move)->board[end] = piece;
//	return move;
//}
//
//Node* create_node(Move move){
//	Node* newnode = malloc(sizeof(Node));
//	newnode->m = move;
//	newnode->next = NULL;
//	return newnode;
//}
//
//int insert_move(List* list, Move move){
//	Node* new = create_node(move);
//	if (new == NULL) return 0;
//	if (list->size == 0){
//		list->first = new;
//		list->size++;
//		return 1;
//	}
//	//insert new node as head
//	new->next = list->first;
//	list->first = new;
//	list->size++;
//	return 1;
//}
//
//int engine_empty_tile(int piece){
//	return piece == EVAL_EMPTY;
//}
//
//int engine_same_color(Config* c, int current, int next){
//	int curr_color = c->BOARD[current] % 2;
//	int dest_color = c->BOARD[next] % 2;
//		int s;
//
//	if (curr_color % 2 == 1){
//		curr_color = 1;	//black
//	}else{
//		curr_color = 0;
//	}
//
//	if (dest_color % 2 == 1){
//		dest_color = 1;	//black
//	}else{
//		dest_color = 0;
//	}
//
//	if (curr_color == 12){
//		curr_color = 2;
//	}
//
//	if (dest_color == 12){
//		dest_color = 2;
//	}
//
//	s = (dest_color == curr_color);
//	return s;
//}
//
//
////white pawn
//int engine_gwpm(Config* c, List* moves, int tile_index){
//	printf("WPM\n");
//	int special_state = 0, left = 1, right = 1;
//	if (tile_index >= 48 && tile_index <= 55){
//		//this means the current white pawn can advance 1 or two squares
//		special_state = 1;
//	}
//
//	if (tile_index % 8 == 0){
//		left = 0;
//	}
//
//	if (tile_index % 8 == 7){
//		right = 0;
//	}
//
//	int left_attack = tile_index - 8 - 1;
//	int right_attack = tile_index - 8 + 1;
//	int onestep = tile_index - 8;
//	int twostep = tile_index - 16;
//	if (left && within_board(left_attack) && c->BOARD[left_attack] != EVAL_EMPTY && (c->BOARD[left_attack] %2 == 1)){
//		Move move = create_move(tile_index, left_attack, WHITE_PAWN, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	if (right && within_board(right_attack) && c->BOARD[right_attack] != EVAL_EMPTY && (c->BOARD[left_attack] %2 == 1)){
//		Move move = create_move(tile_index, right_attack, WHITE_PAWN, c, 1);
//		if(!insert_move(moves, move)){
//					return 0;
//				}
//	}
//	if (onestep && within_board(onestep) && c->BOARD[onestep] == EVAL_EMPTY){
//		Move move = create_move(tile_index, onestep, WHITE_PAWN, c, 0);
//		if(!insert_move(moves, move)){
//					return 0;
//				}
//	}
//	if (special_state && twostep && within_board(twostep) && c->BOARD[twostep == EVAL_EMPTY]){
//		Move move = create_move(tile_index, twostep, WHITE_PAWN, c, 0);
//		if(!insert_move(moves, move)){
//					return 0;
//				}
//	}
//	return 1;
//}
////black pawn
//int engine_gbpm(Config* c, List* moves, int tile_index){
//	int special_state = 0, left = 1, right = 1;
//	if (tile_index >= 8 && tile_index <= 15){
//		//this means the current white pawn can advance 1 or two squares
//		special_state = 1;
//	}
//
//	if (tile_index % 8 == 0){
//		left = 0;
//	}
//
//	if (tile_index % 8 == 7){
//		right = 0;
//	}
//
//	int left_attack = tile_index + 8 - 1;
//	int right_attack = tile_index + 8 + 1;
//	int onestep = tile_index - 8;
//	int twostep = tile_index - 16;
//	if (left && within_board(left_attack) && c->BOARD[left_attack] != EVAL_EMPTY && (c->BOARD[left_attack] %2 == 0)){
//		Move move = create_move(tile_index, left_attack, BLACK_PAWN, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	if (right && within_board(right_attack) && c->BOARD[right_attack] != EVAL_EMPTY && (c->BOARD[left_attack] %2 == 0)){
//		Move move = create_move(tile_index, right_attack, BLACK_PAWN, c, 1);
//		if(!insert_move(moves, move)){
//					return 0;
//				}
//	}
//	if (onestep && within_board(onestep) && c->BOARD[onestep] == EVAL_EMPTY){
//		Move move = create_move(tile_index, onestep, BLACK_PAWN, c, 0);
//		if(!insert_move(moves, move)){
//					return 0;
//				}
//	}
//	if (special_state && twostep && within_board(twostep) && c->BOARD[twostep == EVAL_EMPTY]){
//		Move move = create_move(tile_index, twostep, BLACK_PAWN, c, 0);
//		if(!insert_move(moves, move)){
//					return 0;
//				}
//	}
//	return 1;
//}
////rook
//int engine_grm(Config* c, List* moves, int tile_index){
//	printf("rook %d\n", tile_index);
//	//generate ahead
//		int curr = tile_index;
//		int firstblock = 0;
//		int piece = c->BOARD[tile_index];
//		//generating move straight up
//		for(int i=tile_index - 8; i>=0; i-=8){
//			if (!within_board(i)) break;
//			if (engine_same_color(c, curr, i)) break;
//			if (firstblock) break;
//			if (within_board(i) && !engine_same_color(c, curr, i)){
//
//				//vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | GLOW;
//				Move move = create_move(tile_index, i, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//				//run into a non-empty cell
//				if(!(c->BOARD[i] == EVAL_EMPTY))firstblock = 1;
//			}
//		}
//
//		firstblock = 0;
//		//generating move straight down
//		for(int i=tile_index + 8; i<64; i+=8){
//			if (!within_board(i)) break;
//			if (engine_same_color(c, curr, i)) break;
//			if (firstblock) break;
//			if (within_board(i) && !engine_same_color(c, curr, i)){
//
//				//vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | GLOW;
//				Move move = create_move(tile_index, i, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//				//run into a non-empty cell
//				if(!(c->BOARD[i] == EVAL_EMPTY))firstblock = 1;
//			}
//		}
//
//		int end = (tile_index/8) * 8 + 8;
//		int start = (tile_index/8) * 8;
//
//		//going right in the same row
//		for(int j=curr+1; j<end; j++){
//			if (engine_same_color(c, curr, j)){
//				break;
//			}
//			if (c->BOARD[j] == EVAL_EMPTY){
//				Move move = create_move(tile_index, j, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//				continue;
//			}
//			//glowing a tile that can be attacked by a rook
//			Move move = create_move(tile_index, j, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//			break;
//		}
//
//		for (int j = curr-1; j>= start; j--){
//			if (engine_same_color(c, curr, j)){
//				break;
//			}
//			if (c->BOARD[j] == EVAL_EMPTY){
//				Move move = create_move(tile_index, j, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//				continue;
//			}
//			//glowing a tile that can be attacked by a rook
//			Move move = create_move(tile_index, j, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//			break;
//		}
//		return 1;
//}
////bishop
//int engine_gbm(Config* c, List* moves, int tile_index){
//	printf("bishop\n");
//	int move1, move2, move1_cont, move2_cont;
//	move1_cont = 0;
//	move2_cont = 0;
//	int mod8 = tile_index % 8;
//
//	int piece = c->BOARD[tile_index];
//
//	int left, right;
//	left = mod8;
//	right = 7 - mod8;
//
//	for (int i=0; i<left; i++){
//		move1 = tile_index - ((i+1)*8) - (i+1);
//		move2 = tile_index + ((i+1)*8) - (i+1);
//		if (!(move1_cont)&&within_board(move1)){
//			if (engine_empty_tile(c->BOARD[move1])){
//				Move move = create_move(tile_index, move1, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//			}else{
//				move1_cont = 1;
//				Move move = create_move(tile_index, move1, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//			}
//		}
//		if (!(move2_cont) && within_board(move2)){
//			if(engine_empty_tile(c->BOARD[move2])){
//				Move move = create_move(tile_index, move2, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//			}else{
//				move2_cont = 1;
//				Move move = create_move(tile_index, move2, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//			}
//		}
//	}
//
//	move1_cont = 0;
//	move2_cont = 0;
//	for(int i=0; i<right; i++){
//		move1 = tile_index - ((i+1)*8) + (i+1);
//		move2 = tile_index + ((i+1)*8) + (i+1);
//		if (!(move1_cont)&&within_board(move1)){
//			if (engine_empty_tile(c->BOARD[move1])){
//				Move move = create_move(tile_index, move1, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//			}else{
//				move1_cont = 1;
//				Move move = create_move(tile_index, move1, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//			}
//		}
//		if (!(move2_cont) && within_board(move2)){
//			if(engine_empty_tile(c->BOARD[move2])){
//				Move move = create_move(tile_index, move2, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//			}else{
//				move2_cont = 1;
//				Move move = create_move(tile_index, move2, piece, c, 1);
//				if(!insert_move(moves, move)){
//					return 0;
//				}
//			}
//		}
//	}
//	return 1;
//}
////knight
//int engine_gknm(Config* c, List* moves, int tile_index){
//	printf("knight %d\n", tile_index);
//	int check, left_bool, left_bool2, right_bool, right_bool2;
//	int piece = c->BOARD[tile_index];
//	left_bool = 1;		//for 1 move left
//	right_bool = 1;
//	left_bool2 = 1;		//for 2 moves left
//	right_bool2 = 1;
//	check = tile_index % 8;
//	switch (check){
//	case 0:
//		left_bool = 0;
//		left_bool2 = 0;
//		break;
//	case 1:
//		left_bool2 = 0;
//		break;
//	case 6:
//		right_bool2 = 0;
//		break;
//	case 7:
//		right_bool = 0;
//		right_bool2 = 0;
//		break;
//	default:
//		left_bool = 1;
//		right_bool = 1;
//		left_bool2 = 1;
//		right_bool2 = 1;
//		break;
//	}
//	//knight can have eight moves
//	int upright = tile_index - 16 + 1;
//	int upleft = tile_index - 16 - 1;
//	int downright = tile_index + 16 + 1;
//	int downleft = tile_index + 16 - 1;
//	int rightup = tile_index + 2 - 8;
//	int rightdown = tile_index + 2 + 8;
//	int leftup = tile_index - 2 - 8;
//	int leftdown = tile_index - 2 + 8;
//
//	if (left_bool){
//		//glow(upleft, tile_index);
//		if (within_board(upleft) && !engine_same_color(c, upleft, tile_index)){
//			Move move = create_move(tile_index, upleft, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//		}
//
//		//glow(downleft, tile_index);
//		if (within_board(downleft) && !engine_same_color(c, downleft, tile_index)){
//			Move move = create_move(tile_index, downleft, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//		}
//	}
//	if (left_bool2){
//		//glow(leftup, tile_index);
//		if (within_board(leftup) && !engine_same_color(c, leftup, tile_index)){
//			Move move = create_move(tile_index, leftup, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//		}
//		//glow(leftdown, tile_index);
//		if (within_board(leftdown) && !engine_same_color(c, leftdown, tile_index)){
//			Move move = create_move(tile_index, leftdown, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//		}
//	}
//	if (right_bool){
//		//glow(upright, tile_index);
//		//glow(downright, tile_index);
//
//		if (within_board(upright) && !engine_same_color(c, upright, tile_index)){
//			Move move = create_move(tile_index, upright, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//		}
//
//		if (within_board(downright) && !engine_same_color(c, downright, tile_index)){
//			Move move = create_move(tile_index, downright, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//		}
//
//	}
//	if(right_bool2){
//		//glow(rightup, tile_index);
//		//glow(rightdown, tile_index);
//		if (within_board(rightup) && !engine_same_color(c, rightup, tile_index)){
//			Move move = create_move(tile_index, rightup, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//		}
//		if (within_board(rightdown) && !engine_same_color(c, rightdown, tile_index)){
//			Move move = create_move(tile_index, rightdown, piece, c, 1);
//			if(!insert_move(moves, move)){
//				return 0;
//			}
//		}
//	}
//	return 1;
//}
////queen
//int engine_gqm(Config* c, List* moves, int tile_index){
//	int a = engine_grm(c, moves, tile_index);
//	int b = engine_gbm(c, moves, tile_index);
//	return a&b;
//}
////king
//int engine_gkm(Config* c, List* moves, int tile_index){
//	//printf("knight\n");
//	int up = tile_index - 8;
//	int down = tile_index + 8;
//	int left = tile_index - 1;
//	int right = tile_index + 1;
//	int upright_corner = up + 1, upleft_corner = up - 1, botleft_corner = down - 1, botright_corner = down + 1;
//
//	int piece = c->BOARD[tile_index];
//
//	int left_check = (tile_index % 8 == 0);	//if this is 1, that means cannot move left
//	int right_check = (tile_index % 8 == 7);	//if this is 1 that means cannot move right
//	//do not light up tile if it they are the same color
//	if (within_board(up) && !engine_same_color(c, tile_index, up)) {
//		Move move = create_move(tile_index, up, piece, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	if (within_board(down)&& !engine_same_color(c, tile_index, down)) {
//		Move move = create_move(tile_index, down, piece, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	if (within_board(left)&& !engine_same_color(c, tile_index, left) && !left_check) {
//		Move move = create_move(tile_index, left, piece, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	if (within_board(right)&& !engine_same_color(c, tile_index, right) && !right_check){
//		Move move = create_move(tile_index, right, piece, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	if (within_board(upright_corner)&& !engine_same_color(c, tile_index, upright_corner) && !right_check){
//		Move move = create_move(tile_index, upright_corner, piece, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	if (within_board(upleft_corner)&& !engine_same_color(c, tile_index, upleft_corner) && !left_check) {
//		Move move = create_move(tile_index, upleft_corner, piece, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	if (within_board(botleft_corner)&& !engine_same_color(c, tile_index, botleft_corner) && !left_check) {
//		Move move = create_move(tile_index, botleft_corner, piece, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	if (within_board(botright_corner)&& !engine_same_color(c, tile_index, botright_corner) &&!right_check) {
//		Move move = create_move(tile_index, botright_corner, piece, c, 1);
//		if(!insert_move(moves, move)){
//			return 0;
//		}
//	}
//	return 1;
//}
//
//
//int generate_moves_loc(Config* c, List* moves, int start_index){
//	int piece = c->BOARD[start_index];
//	switch (piece){
//	case EVAL_EMPTY:
//		return 1;
//	case WHITE_PAWN:
//		return engine_gwpm(c, moves, start_index);
//		break;
//	case BLACK_PAWN:
//		return engine_gbpm(c, moves, start_index);
//		break;
//	case WHITE_ROOK:
//	case BLACK_ROOK:
//		loop(100);
//		return engine_grm(c, moves, start_index);
//		break;
//	case WHITE_BISHOP:
//	case BLACK_BISHOP:
//		loop(100);
//		return engine_gbm(c, moves, start_index);
//		break;
//	case WHITE_KNIGHT:
//	case BLACK_KNIGHT:
//		loop(100);
//		return engine_gknm(c, moves, start_index);
//		break;
//	case WHITE_KING:
//	case BLACK_KING:
//		return engine_gkm(c, moves, start_index);
//		break;
//	case WHITE_QUEEN:
//	case BLACK_QUEEN:
//		loop(100);
//		return engine_gqm(c, moves, start_index);
//		break;
//	default:
//		return 1;
//		break;
//	}
//	return 1;
//}
//
//int generate_legal_moves(PtrConfig c, List* moves)
//{
//	//call generate moves
//	for(int i =0; i<64; i++){
//		if (c->BOARD[i] == EVAL_EMPTY){
//			//empty tile, just ignore
//			continue;
//		}
//		else if ((c->BOARD[i] % 2 == 0) && c->TURN == 0){
//			//white's turn to move
//			loop(100);
//			generate_moves_loc(c, moves, i);
//			loop(100);
//		}else if ((c->BOARD[i]%2 == 1) && (c->TURN == 1) && (c->BOARD[i]!=EVAL_EMPTY)){
//			//black's turn to move
//			generate_moves_loc(c, moves, i);
//		}
//	}
//	return 1;
//}
//
//int alphabeta(Config* c, List* moves){
//	//the first calling of rec_alpha will start with maximizing player
//
//	//Config c, List* moves, int depth, int alpha, int beta,
//	//max_player, int score_turn
//	if (rec_alphabeta(c, moves, c->DEPTH, NEGMAX, POSMAX, 1, c->TURN) == ALPHABETAFAIL) return 0;
//	return 1;
//}
//
//int rec_alphabeta(Config* c, List* moves, int depth, int alpha, int beta, int max_player, int score_turn){
//		if (depth != c->DEPTH){
//			if (!generate_legal_moves(c, moves)){
//				free_list(moves);
//				return ALPHABETAFAIL;
//			}
//		}
//		printf("Done generating moves! \n");
//		print_generated_moves(moves);
//		int v;
//		int i = 0;
//		Node* child_node = moves->first;
//		if (c->DEPTH == 0 || moves->size == 0){
//			//this means we reach a terminal node
//			if (moves->size == 0){
//				c->TURN = 1 ^ c->TURN;
//			}
//
//			if (c->DEPTH != depth){
//				free_list(moves);
//			}
//
//			int s = eval();
//			return s;
//		}
//		if (max_player) {//maximizing player
//				printf("Entered Max\n");
//				v = -1000001;
//				while (i < moves->size) {
//					Move child = child_node->m;
//					List * next_moves = init_list();
//					if (next_moves == NULL) {
//						free_list(moves);
//						return ALPHABETAFAIL;
//					}
//					Config* new = create_new_config(&c, 1 ^ c->TURN, child.board,
//							c->DEPTH - 1);
//					int new_score = rec_alphabeta(new, next_moves, depth, alpha, beta,
//							0, score_turn);
//					if (new_score == ALPHABETAFAIL) {
//						free_list(moves);
//						return ALPHABETAFAIL;
//					}
//					v = max(v, new_score);
//					alpha = max(alpha, v);
//					if (c->DEPTH == depth) {
//						child_node->m.score = new_score;
//					}
//					if (beta < alpha) {
//						break;
//					}
//					// pruning starts only in second level of the alphabeta tree
//					if ((beta == alpha) && (c->DEPTH < depth - 1)) {
//						break;
//					}
//
//					i++;
//					child_node = child_node->next;
//				}
//				if (c->DEPTH != depth) {
//					free_list(moves);
//				}
//				return v;
//			} else {
//				v = 1000001;
//				while (i < moves->size) {
//					Move child = child_node->m;
//					List * next_moves = init_list();
//					if (next_moves == NULL) {
//						free_list(moves);
//						return 20000;
//					}
//					Config* new = create_new_config(&c, 1 ^ c->TURN, child.board,
//							c->DEPTH - 1);
//					int new_score = rec_alphabeta(new, next_moves, depth, alpha, beta,
//							1, score_turn);
//					v = min(v, new_score);
//					beta = min(beta, v);
//					if (beta < alpha) {
//						break;
//					}
//					if ((beta == alpha) && (c->DEPTH < depth - 1)) {
//						break;
//					}
//					i++;
//					child_node = child_node->next;
//				}
//			}
//			if (c->DEPTH != depth) {
//				free_list(moves);
//			}
//			return v;
//
//}
//
//int get_best_moves(PtrConfig c, List* moves, int depth){
//	Config* new = create_new_config(c, c->TURN, c->BOARD, depth);
//	alphabeta(new, moves);		//generating the list of best moves within a certain depth
//	return 1;
//}
//
//int find_move_made(List* moves, int start, int end){
//	if (moves == NULL){
//		return ALPHABETAFAIL;
//	}
//	Node* s		= moves->first;
//	while(s!=NULL){
//		if (s->m.src == start && s->m.dst == end){
//			return s->m.score;
//		}
//		s = s->next;
//	}
//	return ALPHABETAFAIL;
//}
//
//int print_generated_moves(List* moves){
//	Node* start = moves->first;
//	while(start != NULL){
//		printf("Start:%d, End:%d \n", start->m.src, start->m.dst);
//		start = start->next;
//	}
//	return 1;
//}
//
////user_turn --> get_best_moves --> create_new_config + alpha_beta --> recursive_alphabeta --> generate_legal_moves
////generate_legal_moves->generate_moves->generate_moves_loc
