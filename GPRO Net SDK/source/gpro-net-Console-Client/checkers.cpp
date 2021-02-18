#include "gpro-net-Console-Client/checkers.h"
#include <math.h>

bool dirty = true;
int highlightX = 4; int highlightY = 4;
int timer = 0;
int maxTime = 200000;

int selectionX = -1, selectionY = -1;
int moveToX = -1, moveToY = -1;

int currentPlayer = 2;

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

		if ((tile & 3) == 1) //player 1
		{
			//check for standard, then jumps, then king standard, then king jumps
		}
		else if ((tile & 3) == 2) //player 2
		{
			//check for standard, then jumps, then king standard, then king jumps
		}
	}
}

void handleSelection(gpro_checkers* chk)
{
	if (highlightX % 2 != highlightY % 2) //this isn't a valid click
		return;

	char highlightTile = (*chk)[highlightY][highlightX / 2]; // divide by two because board x is between [0,4) and selection is between [0,8)
	char highlightPlayer = highlightTile & 3;
	if (selectionX == highlightX && selectionY == highlightY) //we clicked the same spot twice
	{
		selectionX = -1;
		selectionY = -1;
		return;
	}
	else if (highlightPlayer == currentPlayer)
	{
		selectionX = highlightX;
		selectionY = highlightY;
		return;
	}
	else if (highlightPlayer == 0) //there is no player where we're highlighting
	{
		//there _is_ a player here, we don't know whether it's player 1 or 2
		if (selectionX != -1 && selectionY != -1)
		{
			char selectedTile = (*chk)[selectionY][selectionX / 2];
			char selectedPlayer = (selectedTile & 3);
			bool king = (selectedTile & 4) != 0;


			//conditions that apply regardless of player

			//a single move can increase/decrease by 1 at most
			if (abs(selectionX - highlightX) > 2)
				return;
			//we're moving more than two spaces or not changing Y at all
			if (abs(selectionY - highlightY) > 2 || selectionY == highlightY)
				return;

			if (selectedPlayer == 1) //we're player 1
			{
				if (!king) //if we're not a king, we can't look above us
				{
					if (highlightY < selectionY)
						return;
				}
				//check if any jumps exist (and any king jumps, if this is a king)
				//if true, compare with those
				//else, check for standard movements (and king movements).
				//compare with those
			}
			else if (selectedPlayer == 2) //we're player 2
			{
				if (!king) //if we're not a king, we can't look below us
				{
					if (highlightY > selectionY)
						return;
				}
				bool jumpExists = false;
				if (selectionY % 2 == 0) //0, 2, 4, or 6
				{
					if (king)	//checking for king jumps to exist. If you CAN jump, you MUST jump.
					{
						//checking the squares diagonally down from the player to see if they have tiles
						char diagDownLeft = *(chk)[selectionY + 1][(selectionX / 2) - 1];
						jumpExists |= ((diagDownLeft & 3) == 1);
						char diagDownRight = *(chk)[selectionY + 1][(selectionX / 2)];
						jumpExists |= ((diagDownRight & 3) == 1);
					}
					if (selectionY > 0)
					{
						char diagUpLeft = *(chk)[selectionY - 1][(selectionX / 2) - 1];
						jumpExists |= ((diagUpLeft & 3) == 1);
						char diagUpRight = *(chk)[selectionY - 1][(selectionX / 2)];
						jumpExists |= ((diagUpRight & 3) == 1);
					}

					if (jumpExists)
					{
						/*
						 * at this point we are looking at exactly two y coordinates.
						 * It can only be an error if we're looking down without being a king, which we already escaped.
						 */
						bool xError = abs(selectionX - highlightX) != 1; //for a jump, we're moving by 1 every time
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
						
					}
					else
					{
						bool xError = selectionX - highlightX != 1 && selectionX != highlightX;
						bool yError = abs(selectionY - highlightY) > 1; //we already block jumps of > 2, now we block > 1
						if (xError || yError)
						{
							return;
						}
						char tmp = selectedTile;
						(*chk)[selectionY][selectionX / 2] = 0;
						(*chk)[highlightY][highlightX / 2] = tmp;
						return;
					}
				}
				

				//check if any jumps exist (and any king jumps, if this is a king)
				//if true, compare with those
				//else, check for standard movements (and king movements).
				//compare with those
			}
			//see if we can select a move!

			return;
		}
	}
}
