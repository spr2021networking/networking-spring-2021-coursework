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

	//constructor, same as reset()
	CheckersInstance();

	/// <summary>
	/// The equivalent of an update loop. Outputs the winner after each loop. Default is 0. Not used in multiplayer
	/// </summary>
	/// <param name="outWinner"></param>
	void checkerLoop(int* outWinner);

	//rendering functions. drawCheckers() draws the entire system, each of the other functions are a step in the process
	void drawCheckers();
	void drawBoard(); //step 1, background
	void drawPieces(); //step 2, pieces
	void drawHighlight(); //step 4, cursor (listed out of function order because they were written in a different order)
	void drawSelection(); //step 3, selection

	//selection processing (handle the enter key)
	void handleSelection();
	//update the highlight
	void checkInput();

	//utility functions, including processing input from other player

	// Return whether a winner has been determined, and outputs the result to outWinner. "No winner" = 0
	bool checkWin(int* outWinner);
	//check whether any jumps are possible from the selected position
	bool hasJump();
	//try to king the highlighted/selected piece
	bool tryKing();
	//process action from server (from other player)
	void processAction(Action* action);

	//same as constructor, resets gamestate and Action
	void reset();
	
};
#endif
