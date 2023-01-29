//#ifndef RAND_H_
//#define RAND_H_
//
//#include "evaluate.h"
//
//#define NEGMAX -100000
//#define POSMAX 100000
//#define ALPHABETAFAIL	20000
//
//typedef struct Locations{
//	int row;
//	int col;
//}Location;
//
//typedef struct Moves{
//	int src;
//	int dst;
//	int type;
//	int score;
//	int threat;
//	int board[64];
//}Move;
//
//typedef struct Node{
//	Move m;
//	struct Node* next;
//}Node;
//
//typedef struct List{
//	struct Node* first;
//	int size;
//}List;
//
//typedef struct Config{
//	int DEPTH;
//	int MODE;			//this is always set to 1, meaning always is 2 players game mode
//	int USER_COLOR;
//	int END_GAME;
//	int TURN;
//	int BEST;
//	int BOARD[64];
//}Config;
//
//typedef Config* PtrConfig;
//typedef struct Node* PtrNode;
//
//#define BOARD(i) = c->BOARD[i];
//
//#endif
