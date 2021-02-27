#include "gpro-net-Console-Client/checkers.h"
#include <math.h>

/// <summary>
/// Return whether a winner has been determined, and outputs the result to outWinner. "No winner" = -1
/// </summary>
/// <param name="outWinner"></param>
/// <returns></returns>
bool CheckersInstance::checkWin(int* outWinner)
{
	bool hasP1 = false, hasP2 = false;

	//loop through all tiles to check whether they have a piece in them
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; i < 4; j++)
		{
			if (hasP1 && hasP2)
			{
				*outWinner = -1;
				return false;
			}
			hasP1 |= (chk[i][j] & 3) == 1;
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
		*outWinner = 0; //this should never happen, it means the board is empty
	}
	return true;
}

CheckersInstance::CheckersInstance()
{
	gpro_checkers_reset(chk);
}

/// <summary>
/// The equivalent of an update loop. Outputs the winner after each loop. Default is -1
/// </summary>
/// <param name="outWinner"></param>
void CheckersInstance::checkerLoop(int* outWinner)
{
	timer--;
	if (dirty)
	{
		drawCheckers();
		if (checkWin(outWinner))
		{
			return;
		}

		gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
		dirty = false;
	}
	checkInput();
}

void CheckersInstance::checkInput()
{
	timer--;
	//printf("%i\n", keyState >> 15);
	if (timer <= 0)
	{
		if (GetKeyState(VK_UP) >> 15 != 0)
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

void CheckersInstance::drawCheckers()
{
	drawBoard();
	drawPieces();
	drawSelection();
	drawHighlight();
	gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
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
			gpro_consoleColor color = (player == 1) ? gpro_consoleColor_white : gpro_consoleColor_blue;
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
		gpro_consoleColor color = (player == 1) ? gpro_consoleColor_white : gpro_consoleColor_blue;
		if (tile != 0)
		{
			gpro_consoleSetColor(color, gpro_consoleColor_cyan);
			printf(((tile & 4) > 0) ? "[]" : "()");
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
/// Draw the selected square
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
		if (selectionX / 2 > 0)
		{
			char diagDownLeft = chk[selectionY + 1][(selectionX / 2) - 1 + xOffset];
			char diagDownLeft2 = chk[selectionY + 2][(selectionX / 2) - 1];
			jumpExists |= ((diagDownLeft & 3) == otherPlayer && diagDownLeft2 == 0);
		}
		if (selectionX / 2 < 3)
		{
			char diagDownRight = chk[selectionY + 1][(selectionX / 2) + xOffset];
			char diagDownRight2 = chk[selectionY + 2][(selectionX / 2) + 1];
			jumpExists |= ((diagDownRight & 3) == otherPlayer && diagDownRight2 == 0);
		}
	}
	if (canCheckUp)
	{
		if (selectionX / 2 > 0)
		{
			char diagUpLeft = chk[selectionY - 1][(selectionX / 2) - 1 + xOffset];
			char diagUpLeft2 = chk[selectionY - 2][(selectionX / 2) - 1];
			jumpExists |= ((diagUpLeft & 3) == otherPlayer && diagUpLeft2 == 0);
		}
		if (selectionX / 2 < 3)
		{
			char diagUpRight = chk[selectionY - 1][(selectionX / 2) + xOffset];
			char diagUpRight2 = chk[selectionY - 2][(selectionX / 2) + 1];
			jumpExists |= ((diagUpRight & 3) == otherPlayer && diagUpRight2 == 0);
		}
	}
	return jumpExists;
}

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

void CheckersInstance::processAction(Action* action)
{
	if (action->playerIndex != playerNum)
	{
		chk[action->endX][action->endY] = chk[action->startX][action->startY];
		chk[action->startX][action->startY] = 0;
		if (action->hasCaptured)
		{
			chk[action->capturedX][action->capturedY] = 0;
		}
		if (action->endTurn)
		{
			currentPlayer = playerNum;
		}
	}
	drawCheckers();
	
}

void CheckersInstance::reset()
{
	gpro_checkers_reset(chk);
	action.setName("");
}

void CheckersInstance::handleSelection()
{
	action.playerIndex = 0;
	action.endTurn = true;
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

		//conditions that apply regardless of player

		//we're moving more than two spaces or not changing X at all
		if (abs(selectionX - highlightX) > 2 || selectionX == highlightX)
		{
			return;
		}

		//we're moving more than two spaces or not changing Y at all
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
				if (chk[yAvg][selectionX / 2 + xOffset] != (3 - currentPlayer)) //ensure that we're actually jumping a piece
				{
					return;
				}
				else
				{
					chk[yAvg][selectionX / 2 + xOffset] = 0;
				}
				 
			}
			else //moving to the left
			{
				if (chk[yAvg][selectionX / 2 - 1 + xOffset] != (3 - currentPlayer))
				{
					return;
				}
				else
				{
					chk[yAvg][selectionX / 2 - 1 + xOffset] = 0;

					action.playerIndex = currentPlayer;
					action.hasCaptured = true;
					action.capturedX = selectionX / 2 - 1 + xOffset;
					action.capturedY = yAvg;
				}
			}

			//move piece and delete its old spot
			chk[selectionY][selectionX / 2] = 0; //set old position to 0
			chk[highlightY][highlightX / 2] = selectedTile; //set new position to the tile

			action.startX = selectionX / 2;
			action.startY = selectionY;

			action.endX = highlightX / 2;
			action.endY = highlightY;

			hasJumped = true;

			if (tryKing())
			{
				return;
			}

			selectionX = highlightX;
			selectionY = highlightY;

			jumpExists = hasJump();
			if (!jumpExists)
			{
				currentPlayer = 3 - currentPlayer;
				selectionX = -1;
				selectionY = -1;
				hasJumped = false;
			}
			else
			{
				action.endTurn = false;
			}
		}
		else if (!hasJumped)
		{
			bool xError = abs(highlightX - selectionX) != 1;
			bool yError = abs(selectionY - highlightY) != 1;
			if (xError || yError)
			{
				return;
			}
			char tmp = selectedTile;
			chk[selectionY][selectionX / 2] = 0;
			chk[highlightY][highlightX / 2] = tmp;

			action.playerIndex = currentPlayer;
			action.hasCaptured = false;

			action.startX = selectionX / 2;
			action.startY = selectionY;

			action.endX = highlightX / 2;
			action.endY = highlightY;

			//the selection and current player are updated in this function,
			//so we can (and must) return early
			if (tryKing())
			{
				return;
			}

			selectionX = -1;
			selectionY = -1;
			currentPlayer = 3 - currentPlayer;
		}
	}
	return;
}
