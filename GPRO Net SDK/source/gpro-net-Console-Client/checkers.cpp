/*
* checkers.cpp
* Contributors: Ben Cooper and Scott Dagen
* Contributions: checkers implementation, including state and logic
*/

#include "gpro-net-Console-Client/checkers.h"
#include <math.h>

// Return whether a winner has been determined, and outputs the result to outWinner. "No winner" = 0
bool CheckersInstance::checkWin(int* outWinner)
{
	bool hasP1 = false, hasP2 = false;

	//loop through all tiles to check whether they have a piece in them
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (hasP1 && hasP2)
			{
				*outWinner = 0;
				return false;
			}
			hasP1 |= (chk[i][j] & 3) == 1; //check if the player flag is 1 or 2, and OR it with hasP1 or hasP2
			hasP2 |= (chk[i][j] & 3) == 2;
		}
	}
	if (hasP1 && !hasP2)
	{
		*outWinner = 1;
	}
	else if (!hasP1 && hasP2)
	{
		*outWinner = 2;
	}
	else
	{
		*outWinner = 4; //this should never happen, it means the board is empty.
	}
	return true;
}

/// <summary>
/// Initialize the gpro_checkers instance and the stored action
/// </summary>
CheckersInstance::CheckersInstance()
{
	gpro_checkers_reset(chk);
	action.reset(true);
}

//local version of checkers. See gpro-net.h for more information
void CheckersInstance::checkerLoop(int* outWinner)
{
	timer--;
	if (dirty)
	{
		drawCheckers(); //rendering
		checkWin(outWinner); //check win state

		gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
		dirty = false;
	}
	checkInput(); //check input
}

//update highlight or selection
void CheckersInstance::checkInput()
{
	timer--; //used to slow down input so the player can actually select things with any degree of precision
	//printf("%i\n", keyState >> 15);
	if (timer <= 0)
	{
		if (GetKeyState(VK_UP) >> 15 != 0) //check keypresses for up, down, left, right, and enter
		{
			highlightY = max(0, highlightY - 1);
			dirty = true;
			timer = maxTime;
		}
		if (GetKeyState(VK_DOWN) >> 15 != 0)
		{
			highlightY = min(7, highlightY + 1);
			dirty = true;
			timer = maxTime;
		}
		if (GetKeyState(VK_LEFT) >> 15 != 0)
		{
			highlightX = max(0, highlightX - 1);
			dirty = true;
			timer = maxTime;
		}
		if (GetKeyState(VK_RIGHT) >> 15 != 0)
		{
			highlightX = min(7, highlightX + 1);
			dirty = true;
			timer = maxTime;
		}
		if (GetKeyState(VK_RETURN) >> 15 != 0)
		{
			handleSelection();
			dirty = true;
			timer = maxTime;
		}
	}
}

/// <summary>
/// Draws the board, the pieces, and optionally the selection + highlight if it's your turn and the game is running
/// </summary>
void CheckersInstance::drawCheckers()
{
	drawBoard();
	drawPieces();
	if (currentPlayer == playerNum && playerNum != 0 && action.readyToPlay)
	{
		drawSelection();
		drawHighlight();
	}
	gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black); //reset colors post-rendering
	dirty = false;
}

/// <summary>
/// Render the actual board (just the background)
/// </summary>
void CheckersInstance::drawBoard()
{
	gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
	gpro_consoleClear();
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i % 2 == 0) //even-numbered rows start with grey
			{
				gpro_consoleSetColor(gpro_consoleColor_grey_d, gpro_consoleColor_grey_d);
				printf("  ");
				gpro_consoleSetColor(gpro_consoleColor_red, gpro_consoleColor_red);
				printf("  ");
			}
			else //odd-numbered rows start with red
			{
				gpro_consoleSetColor(gpro_consoleColor_red, gpro_consoleColor_red);
				printf("  ");
				gpro_consoleSetColor(gpro_consoleColor_grey_d, gpro_consoleColor_grey_d);
				printf("  ");
			}
		}
		printf("\n");
	}
}

/// <summary>
/// Draw all pieces
/// </summary>
void CheckersInstance::drawPieces()
{
	gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_grey_d);
	int iTotal = 0;
	int jTotal = 0;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			char tile = chk[i][j];
			char player = tile & 3;
			gpro_consoleColor color = (player == 1) ? gpro_consoleColor_white : gpro_consoleColor_blue; //blue for down, white for up
			if (i % 2 == 0)
			{
				gpro_consoleSetCursor(4 * (j), i); //even-numbered rows don't have an offset

			}
			else
			{
				gpro_consoleSetCursor(4 * j + 2, i); //odd-numbered rows have an offset because they start with red (2 characters)
			}
			if (tile != 0)
			{
				gpro_consoleSetColor(color, gpro_consoleColor_grey_d);
				printf(((tile & 4) > 0) ? "[]" : "()");
			}
		}
	}
}

/// <summary>
/// Draw the square you're 'hovering' over
/// </summary>
void CheckersInstance::drawHighlight()
{
	gpro_consoleSetCursor(2 * highlightX, highlightY);
	gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_cyan);
	if (highlightX % 2 == highlightY % 2)
	{
		char tile = chk[highlightY][highlightX / 2]; // divide by two because board x is between [0,4) and selection is between [0,8)
		char player = tile & 3;
		gpro_consoleColor color = (player == 1) ? gpro_consoleColor_white : gpro_consoleColor_blue; //blue for down, white for up
		if (tile != 0)
		{
			gpro_consoleSetColor(color, gpro_consoleColor_cyan);
			printf(((tile & 4) > 0) ? "[]" : "()"); //[] for king, () for pawn
		}
		else
		{
			printf("  ");
		}
	}
	else
	{
		printf("  ");
	}
}

/// <summary>
/// Draw the selected square. Very similar code to dawHighlight, but skips some of the logic for an empty piece
/// </summary>
void CheckersInstance::drawSelection()
{
	if (selectionX != -1 && selectionY != -1)
	{
		gpro_consoleSetCursor(2 * selectionX, selectionY);
		gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_green);
		char tile = chk[selectionY][selectionX / 2]; // divide by two because board x is between [0,4) and selection is between [0,8)
		char player = tile & 3;
		gpro_consoleColor color = (player == 1) ? gpro_consoleColor_white : gpro_consoleColor_blue;
		if (tile != 0)
		{
			gpro_consoleSetColor(color, gpro_consoleColor_green);
			printf(((tile & 4) > 0) ? "[]" : "()");
		}
	}
}

/// <summary>
/// Checks if a jump exists, scanning the tiles (up to 8) around the selected piece to look for the required pattern
/// </summary>
/// <returns></returns>
bool CheckersInstance::hasJump()
{
	if (selectionX == -1 || selectionY == -1)
	{
		return false;
	}

	char selectedTile = chk[selectionY][selectionX / 2];
	bool isSelectedKing = (selectedTile & 4) != 0;
	bool jumpExists = false;
	int xOffset = selectionY % 2 == 1; //this is needed for shifting around when checking diagonals to handle the lack of alignment between rows

	bool canCheckUp = selectionY > 1 && (currentPlayer == 2 || isSelectedKing); //height check plus a player-specific king check
	bool canCheckDown = selectionY < 6 && (currentPlayer == 1 || isSelectedKing); //height check plus a player-specific king check

	int otherPlayer = 3 - currentPlayer;
	if (canCheckDown)
	{
		if (selectionX / 2 > 0) //if we have space to jump left, check the down left two tiles
		{
			char diagDownLeft = chk[selectionY + 1][(selectionX / 2) - 1 + xOffset];
			char diagDownLeft2 = chk[selectionY + 2][(selectionX / 2) - 1];
			jumpExists |= ((diagDownLeft & 3) == otherPlayer && diagDownLeft2 == 0);
		}
		if (selectionX / 2 < 3) //if we have space to jump right, check the down right two tiles
		{
			char diagDownRight = chk[selectionY + 1][(selectionX / 2) + xOffset];
			char diagDownRight2 = chk[selectionY + 2][(selectionX / 2) + 1];
			jumpExists |= ((diagDownRight & 3) == otherPlayer && diagDownRight2 == 0);
		}
	}
	if (canCheckUp)
	{
		if (selectionX / 2 > 0) //if we have space to jump left, check the up left two tiles
		{
			char diagUpLeft = chk[selectionY - 1][(selectionX / 2) - 1 + xOffset];
			char diagUpLeft2 = chk[selectionY - 2][(selectionX / 2) - 1];
			jumpExists |= ((diagUpLeft & 3) == otherPlayer && diagUpLeft2 == 0);
		}
		if (selectionX / 2 < 3) //if we have space to jump right, check the up right two tiles
		{
			char diagUpRight = chk[selectionY - 1][(selectionX / 2) + xOffset];
			char diagUpRight2 = chk[selectionY - 2][(selectionX / 2) + 1];
			jumpExists |= ((diagUpRight & 3) == otherPlayer && diagUpRight2 == 0);
		}
	}
	return jumpExists;
}

/// <summary>
/// Attempt to king the selected piece.
/// </summary>
/// <returns></returns>
bool CheckersInstance::tryKing()
{
	int kingIndex = currentPlayer == 1 ? 7 : 0;
	bool isSelectedKing = (chk[selectionY][selectionX / 2] & 4) != 0;
	if (highlightY == kingIndex && !isSelectedKing) //we're getting kinged
	{
		chk[highlightY][highlightX / 2] = currentPlayer + 4; //set king
		currentPlayer = 3 - currentPlayer;
		selectionX = -1;
		selectionY = -1;
		hasJumped = false;
		return true;
	}
	return false;
}

/// <summary>
/// Process a received action, updating pieces and checking for whether the game state indicates a player winning
/// </summary>
/// <param name="action"></param>
void CheckersInstance::processAction(Action* action)
{
	if (action->playerIndex != playerNum)
	{
		chk[action->endY][action->endX] = chk[action->startY][action->startX]; //move piece
		chk[action->startY][action->startX] = 0;
		if (action->hasCaptured)
		{
			chk[action->capturedY][action->capturedX] = 0; //delete captured piece
		}
		if (action->endTurn)
		{
			currentPlayer = 3 - currentPlayer; //swap turn
		}
		if (action->becomeKing)
		{
			chk[action->endY][action->endX] = action->playerIndex + 4; //set the king flag
		}
	}
	dirty = true;
	this->action.readyToPlay = action->readyToPlay;
	checkWin(&this->action.winner);
	
}

/// <summary>
/// Reset the board and the action
/// </summary>
void CheckersInstance::reset()
{
	gpro_checkers_reset(chk);
	action.reset(true);
}

/// <summary>
/// Handle pretty much all game logic. When enter is pressed, check whether a valid move has been performed and update the stored action
/// Has a LOT of early exits
/// </summary>
void CheckersInstance::handleSelection()
{
	if (playerNum == 0) //spectators can't move, no action taken
	{
		return;
	}

	action.reset();

	if (highlightX % 2 != highlightY % 2) //this isn't a valid click as we're on a red square
	{
		return;
	}

	char highlightTile = chk[highlightY][highlightX / 2]; // divide by two because board x is between [0,4) and selection is between [0,8)
	char highlightPlayer = highlightTile & 3;

	if (selectionX == highlightX && selectionY == highlightY && !hasJumped) //we clicked the same spot twice, so we deselect
	{
		selectionX = -1;
		selectionY = -1;
		return;
	}
	else if (highlightPlayer == 3 - currentPlayer) //we're selecting a player that isn't us, this isn't allowed!
	{
		return;
	}
	else if (highlightPlayer == currentPlayer && !hasJumped) //this player belongs to us but isn't the one we selected
	{
		selectionX = highlightX;
		selectionY = highlightY;
		return;
	}
	else if (highlightPlayer == 0) //there is no player where we're highlighting, basically the default case
	{
		if (selectionX == -1 || selectionY == -1) //there is nothing selected
		{
			return;
		}

		char selectedTile = chk[selectionY][selectionX / 2];
		bool isSelectedKing = (selectedTile & 4) != 0;
		int xOffset = selectionY % 2 == 1;

		//we're moving more than two spaces or not changing X at all. Valid differences are 1 and 2
		if (abs(selectionX - highlightX) > 2 || selectionX == highlightX)
		{
			return;
		}

		//we're moving more than two spaces or not changing Y at all. Valid differences are 1 and 2
		if (abs(selectionY - highlightY) > 2 || selectionY == highlightY)
		{
			return;
		}

		//flip the signs of a lot of values if selectedPlayer == 1. Returns 1 if player == 2 and -1 if player == 1
		int dirMod = 2 * (currentPlayer != 1) - 1;

		//if player 2, highlightY > selectionY is invalid for pawns. If player 1, highlightY < selectionY is invalid for pawns.
		//This checks both cases thanks to dirMod.
		bool invalidPawnSelection = (highlightY - selectionY) * dirMod > 0;
		if (!isSelectedKing && invalidPawnSelection)
			return;

		//check if the requirements for a jump are present
		bool jumpExists = hasJump();
		if (jumpExists)
		{
			/*
			 at this point we are looking at exactly two y coordinates.
			 It can only be an error if we're looking down without being a king, which we already escaped.
			 */
			bool xError = abs(selectionX - highlightX) != 2;
			if (xError)
			{
				return;
			}

			int yAvg = (selectionY + highlightY) / 2; //the space between ours

			if (highlightX > selectionX) //moving to the right
			{
				if ((chk[yAvg][selectionX / 2 + xOffset] & 3) != (3 - currentPlayer)) //ensure that we're actually jumping a piece
				{
					return; //if we're not jumping, we return, and no actions are taken
				}
				else
				{
					chk[yAvg][selectionX / 2 + xOffset] = 0; //piece is captured

					//configure a jump action
					action.playerIndex = currentPlayer;
					action.hasCaptured = true;
					action.capturedX = selectionX / 2 + xOffset;
					action.capturedY = yAvg;
				}
				 
			}
			else //moving to the left
			{
				if ((chk[yAvg][selectionX / 2 - 1 + xOffset] & 3) != (3 - currentPlayer))
				{
					return;
				}
				else
				{
					chk[yAvg][selectionX / 2 - 1 + xOffset] = 0;

					//configure a jump action
					action.playerIndex = currentPlayer;
					action.hasCaptured = true;
					action.capturedX = selectionX / 2 - 1 + xOffset;
					action.capturedY = yAvg;
				}
			}

			//move piece and delete its old spot
			chk[selectionY][selectionX / 2] = 0; //set old position to 0
			chk[highlightY][highlightX / 2] = selectedTile; //set new position to the tile

			//update action
			action.startX = selectionX / 2;
			action.startY = selectionY;

			action.endX = highlightX / 2;
			action.endY = highlightY;

			hasJumped = true;

			if (tryKing())
			{
				action.becomeKing = true;
				action.endTurn = true;
				return;
			}

			//update selection index
			selectionX = highlightX;
			selectionY = highlightY;

			//check for a second jump
			jumpExists = hasJump();
			if (!jumpExists)
			{
				currentPlayer = 3 - currentPlayer;
				selectionX = -1;
				selectionY = -1;
				hasJumped = false;
				action.endTurn = true;
			}
			else
			{
				action.endTurn = false;
			}
		}
		else if (!hasJumped) //similar to !jumpExists, but also checks if the previous action was a jump, so people don't get a free extra move if they've already jumped
		{
			bool xError = abs(highlightX - selectionX) != 1;
			bool yError = abs(selectionY - highlightY) != 1;
			if (xError || yError)
			{
				return;
			}

			//move tile
			char tmp = selectedTile;
			chk[selectionY][selectionX / 2] = 0;
			chk[highlightY][highlightX / 2] = tmp;

			//set action data
			action.playerIndex = currentPlayer;
			action.hasCaptured = false;
			action.endTurn = true;

			action.startX = selectionX / 2;
			action.startY = selectionY;

			action.endX = highlightX / 2;
			action.endY = highlightY;

			//the selection and current player are updated in this function,
			//so we can (and must) return early
			if (tryKing())
			{
				action.becomeKing = true;
				action.endTurn = true;
				return;
			}

			selectionX = -1;
			selectionY = -1;
			currentPlayer = 3 - currentPlayer; //end our turn
		}
	}
	return;
}
