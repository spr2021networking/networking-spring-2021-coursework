/*
* checkers.h
* Contributors: Ben Cooper and Scott Dagen
* Contributions: checkers implementation, including state and logic
*/

#ifndef CHECKERS_H
#define CHECKERS_H

#include "gpro-net/gpro-net-common/gpro-net-gamestate.h"
#include "gpro-net/gpro-net.h"
#include "gpro-net/gpro-net-common/gpro-net-console.h"
#include <stdio.h>
#include <Windows.h>

typedef struct CheckersInstance CheckersInstance;

struct CheckersInstance
{
	//whether we're player 1 or 2, or a spectator
	int playerNum = 0;
	//the action that we send and update, contains some other state info
	Action action;
	gpro_checkers chk;
	bool dirty = true; //whether the board needs to be redrawn
	int highlightX = 4; int highlightY = 4;

	//used to prevent the highlight from moving too quickly, updates when input is called.
	int timer = 0;
	int maxTime = 5000;

	//coordinates for selection 
	int selectionX = -1, selectionY = -1;

	//whose turn it currently is
	int currentPlayer = 2;

	bool hasJumped = false; //used once a jump has occurred to prevent normal movement

	CheckersInstance();

	//local implementation
	void checkerLoop(int* outWinner);

	//rendering
	void drawCheckers();
	void drawBoard();
	void drawPieces();
	void drawHighlight();
	void drawSelection();

	//selection processing
	void handleSelection();
	void checkInput();

	//utility functions, including processing input from other player
	bool checkWin(int* outWinner);
	bool hasJump();
	bool tryKing();
	void processAction(Action* action);

	void reset();
	
};
#endif
