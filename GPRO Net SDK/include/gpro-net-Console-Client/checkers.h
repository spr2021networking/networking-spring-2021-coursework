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
	Action action;
	gpro_checkers chk;
	bool dirty = true;
	int highlightX = 4; int highlightY = 4;
	int timer = 0;
	int maxTime = 5000;

	int selectionX = -1, selectionY = -1;
	int moveToX = -1, moveToY = -1;

	int currentPlayer = 2;

	bool hasJumped = false; //used once a jump has occurred to prevent normal movement

	CheckersInstance();

	void checkerLoop(int* outWinner);

	void drawCheckers();
	void drawBoard();
	void drawPieces();
	void drawHighlight();
	void drawSelection();

	void handleSelection();
	void checkInput();

	bool checkWin(int* outWinner);
	bool hasJump();
	bool tryKing();
	void processAction(Action* action);

	void reset();
	
};
#endif
