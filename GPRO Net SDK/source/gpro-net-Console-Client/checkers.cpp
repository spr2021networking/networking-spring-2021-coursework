#include "gpro-net-Console-Client/checkers.h"
#include <math.h>

bool dirty = true;
int highlightX = 4; int highlightY = 4;
int timer = 0;
int maxTime = 200000;

int selectionX = -1, selectionY = -1;
int moveToX = -1, moveToY = -1;

int currentPlayer = 1;

bool hasJumped; //used once a jump has occurred to prevent normal movement

void checkerLoop(gpro_checkers* chk)
{
	timer--;
	if (dirty)
	{
		drawBoard();
		drawPieces(chk);
		drawSelection(chk);
		drawHighlight(chk, highlightX, highlightY);
		gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
		dirty = false;
	}
	short keyState = GetKeyState(VK_UP);
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
			handleSelection(chk);
			dirty = true;
			timer = maxTime;
			//check if we're selecting or moving
		}
	}
}

void drawBoard()
{
	gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
	gpro_consoleClear();
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i % 2 == 0)
			{
				gpro_consoleSetColor(gpro_consoleColor_grey_d, gpro_consoleColor_grey_d);
				printf("  ");
				gpro_consoleSetColor(gpro_consoleColor_red, gpro_consoleColor_red);
				printf("  ");
			}
			else
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

void drawPieces(gpro_checkers* chk)
{
	gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_grey_d);
	int iTotal = 0;
	int jTotal = 0;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			char tile = (*chk)[i][j];
			char player = tile & 3;
			gpro_consoleColor color = (player == 1) ? gpro_consoleColor_white : gpro_consoleColor_blue;
			if (i % 2 == 0)
			{
				gpro_consoleSetCursor(4 * (j), i);

			}
			else
			{
				gpro_consoleSetCursor(4 * j + 2, i);
			}
			if (tile != 0)
			{
				gpro_consoleSetColor(color, gpro_consoleColor_grey_d);
				printf(((tile & 4) > 0) ? "[]" : "()");
			}
		}
	}
}

typedef struct Action Action;
struct Action
{
	char playerIndex;
	char startX, startY;
	char endX, endY;
	bool hasCaptured = false;
	char capturedCoords[2];
};


void drawHighlight(gpro_checkers* chk, int x, int y)
{
	gpro_consoleSetCursor(2 * (x), (y));
	gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_cyan);
	if (x % 2 == y % 2)
	{
		char tile = (*chk)[y][x / 2]; // divide by two because board x is between [0,4) and selection is between [0,8)
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

void drawSelection(gpro_checkers* chk)
{
	gpro_consoleSetCursor(2 * selectionX, selectionY);
	gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_green);
	char tile = (*chk)[selectionY][selectionX / 2]; // divide by two because board x is between [0,4) and selection is between [0,8)
	char player = tile & 3;
	gpro_consoleColor color = (player == 1) ? gpro_consoleColor_white : gpro_consoleColor_blue;
	if (tile != 0)
	{
		gpro_consoleSetColor(color, gpro_consoleColor_green);
		printf(((tile & 4) > 0) ? "[]" : "()");
	}
}

bool hasJump(gpro_checkers* chk)
{
	if (selectionX == -1 || selectionY == -1)
	{
		return false;
	}

	char selectedTile = (*chk)[selectionY][selectionX / 2];
	bool isSelectedKing = (selectedTile & 4) != 0;
	bool jumpExists = false;

	if (currentPlayer == 2)
	{
		int xOffset = selectionY % 2 == 1; //this is needed for shifting around when checking diagonals to handle the lack of alignment between rows
		if (isSelectedKing && selectionY < 6)
		{
			if (selectionX / 2 > 0)
			{
				char diagDownLeft = (*chk)[selectionY + 1][(selectionX / 2) - 1 + xOffset];
				jumpExists |= ((diagDownLeft & 3) == 1);
			}
			if (selectionX / 2 < 3)
			{
				char diagDownRight = (*chk)[selectionY + 1][(selectionX / 2) + xOffset];
				jumpExists |= ((diagDownRight & 3) == 1);
			}
		}
		//this is pretty complex, but it's checking to make sure that selectionY is not 0 IF selectionY is even, and that selectionY is not 7 IF selectionY is odd
		if (selectionY != (7 * (selectionY % 2 == 1)))
		{
			if (selectionX / 2 > 0)
			{
				char diagUpLeft = (*chk)[selectionY - 1][(selectionX / 2) - 1 + xOffset];
				jumpExists |= ((diagUpLeft & 3) == 1);
			}
			if (selectionX / 2 < 3)
			{
				char diagUpRight = (*chk)[selectionY - 1][(selectionX / 2) + xOffset];
				jumpExists |= ((diagUpRight & 3) == 1);
			}
		}
		return jumpExists;
	}
	return false;
}

void handleSelection(gpro_checkers* chk)
{
	if (highlightX % 2 != highlightY % 2) //this isn't a valid click as we're on a red square
	{
		return;
	}

	char highlightTile = (*chk)[highlightY][highlightX / 2]; // divide by two because board x is between [0,4) and selection is between [0,8)
	char highlightPlayer = highlightTile & 3;

	if (selectionX == highlightX && selectionY == highlightY) //we clicked the same spot twice, so we deselect
	{
		selectionX = -1;
		selectionY = -1;
		return;
	}
	else if (highlightPlayer == 3 - currentPlayer) //we're selecting a player that isn't us, this isn't allowed!
	{
		return;
	}
	else if (highlightPlayer == currentPlayer) //this player belongs to us but isn't the one we selected
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

		char selectedTile = (*chk)[selectionY][selectionX / 2];
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

		if (currentPlayer == 1) //we're player 1
		{
			//check if any jumps exist (and any king jumps, if this is a king)
			//if true, compare with those
			//else, check for standard movements (and king movements).
			//compare with those
			bool jumpExists = false;
			if (selectionY % 2 == 0) //0, 2, 4, or 6
			{
				if (selectionY < 7)	//checking for king jumps to exist. If you CAN jump, you MUST jump.
				{
					//checking the squares diagonally down from the player to see if they have tiles
					char diagDownLeft = (*chk)[selectionY + 1][(selectionX / 2) - 1];
					jumpExists |= ((diagDownLeft & 3) == 1);
					char diagDownRight = (*chk)[selectionY + 1][(selectionX / 2)];
					jumpExists |= ((diagDownRight & 3) == 1);
				}
				if (isSelectedKing && selectionY > 1)
				{
					char diagUpLeft = (*chk)[selectionY - 1][(selectionX / 2) - 1];
					jumpExists |= ((diagUpLeft & 3) == 1);
					char diagUpRight = (*chk)[selectionY - 1][(selectionX / 2)];
					jumpExists |= ((diagUpRight & 3) == 1);
				}

				if (jumpExists)
				{
					/*
					 * at this point we are looking at exactly two y coordinates.
					 * It can only be an error if we're looking down without being a king, which we already escaped.
					 */
					bool xError = abs(selectionX - highlightX) != 2;
					if (xError)
					{
						return;
					}

					//move piece and remove captured
					char tmp = selectedTile;
					(*chk)[selectionY][selectionX / 2] = 0;
					(*chk)[highlightY][highlightX / 2] = tmp;

					int yAvg = (selectionY + highlightY) / 2;
					if (highlightX > selectionX) //moving to the right
					{
						(*chk)[yAvg][highlightX / 2] = 0;
						return;
					}
					else //moving to the left
					{
						(*chk)[yAvg][selectionX / 2] = 0;
					}

					hasJumped = true;

					selectionX = highlightX;
					selectionY = highlightY;


					jumpExists = false;

					//rescan jumps, then possibly end turn
					if (!jumpExists)
					{
						//currentPlayer = 3 - currentPlayer;
						selectionX = -1;
						selectionY = -1;
					}

				}
				else if (!hasJumped) //can't scan here if you've already jumped
				{
					bool xError = abs(highlightX - selectionX) != 1;
					bool yError = abs(selectionY - highlightY) > 1; //we already block jumps of > 2, now we block > 1
					if (xError || yError)
					{
						return;
					}
					char tmp = selectedTile;
					(*chk)[selectionY][selectionX / 2] = 0;
					(*chk)[highlightY][highlightX / 2] = tmp;

					//currentPlayer = 3 - currentPlayer;
					return;
				}
			}
			else //1, 3, 5, 7
			{
				if (selectionY > 0)	//checking for king jumps to exist. If you CAN jump, you MUST jump.
				{
					//checking the squares diagonally down from the player to see if they have tiles
					char diagDownLeft = (*chk)[selectionY + 1][(selectionX / 2)];
					jumpExists |= ((diagDownLeft & 3) == 1);
					char diagDownRight = (*chk)[selectionY + 1][(selectionX / 2) + 1];
					jumpExists |= ((diagDownRight & 3) == 1);
				}
				if (isSelectedKing && selectionY > 1)
				{
					char diagUpLeft = (*chk)[selectionY - 1][(selectionX / 2)];
					jumpExists |= ((diagUpLeft & 3) == 1);
					char diagUpRight = (*chk)[selectionY - 1][(selectionX / 2) + 1];
					jumpExists |= ((diagUpRight & 3) == 1);
				}

				if (jumpExists)
				{
					/*
					 * at this point we are looking at exactly two y coordinates.
					 * It can only be an error if we're looking down without being a king, which we already escaped.
					 */
					bool xError = abs(selectionX - highlightX) != 2;
					if (xError)
					{
						return;
					}

					//move piece and remove captured
					char tmp = selectedTile;
					(*chk)[selectionY][selectionX / 2] = 0;
					(*chk)[highlightY][highlightX / 2] = tmp;

					int yAvg = (selectionY + highlightY) / 2;
					if (highlightX > selectionX) //moving to the right
					{
						(*chk)[yAvg][(highlightX / 2) + 1] = 0;
						return;
					}
					else //moving to the left
					{
						(*chk)[yAvg][highlightX / 2] = 0;
					}

					hasJumped = true;

					selectionX = highlightX;
					selectionY = highlightY;


					jumpExists = false;

					//rescan jumps, then possibly end turn
					if (!jumpExists)
					{
						//currentPlayer = 3 - currentPlayer;
						selectionX = -1;
						selectionY = -1;
					}

				}
				else if (!hasJumped)
				{
					bool xError = abs(highlightX - selectionX) != 1;
					bool yError = abs(selectionY - highlightY) > 1; //we already block jumps of > 2, now we block > 1
					if (xError || yError)
					{
						return;
					}
					char tmp = selectedTile;
					(*chk)[selectionY][selectionX / 2] = 0;
					(*chk)[highlightY][highlightX / 2] = tmp;

					//currentPlayer = 3 - currentPlayer;
					return;
				}
			}
		}
		else if (currentPlayer == 2) //we're player 2
		{
			bool jumpExists = hasJump(chk);
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

				//move piece and delete its old spot
				(*chk)[selectionY][selectionX / 2] = 0; //set old position to 0
				(*chk)[highlightY][highlightX / 2] = selectedTile; //set new position to the tile

				int yAvg = (selectionY + highlightY) / 2; //the space between ours
				if (highlightX > selectionX) //moving to the right
				{
					(*chk)[yAvg][selectionX / 2 + xOffset] = 0;
				}
				else //moving to the left
				{
					(*chk)[yAvg][selectionX / 2 - 1 + xOffset] = 0;
				}
				hasJumped = true;

				selectionX = highlightX;
				selectionY = highlightY;

				jumpExists = hasJump(chk);
				if (!jumpExists)
				{
					currentPlayer = 3 - currentPlayer;
					selectionX = -1;
					selectionY = -1;
					hasJumped = false;
				}
				return;
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
				(*chk)[selectionY][selectionX / 2] = 0;
				(*chk)[highlightY][highlightX / 2] = tmp;

				selectionX = -1;
				selectionY = -1;
				currentPlayer = 3 - currentPlayer;
				return;
			}
		}
		return;
	}
}
