#include "gpro-net-Console-Client/battleship.h"

bool shipsPlaced = false;
bool attemptingShipPlacement = false;
bool battleShipDirty = true;
int bShipX = 4; int bShipY = 4;
void battleshipLoop(gpro_battleship* shipBrd, gpro_battleship* atkBrd)
{
	if (battleShipDirty)
	{

		drawShipBoard();
		drawAttackBoard();
		drawSelectedTile(bShipX, bShipY);
		gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
		placeShips(bShipX, bShipY, shipBrd);
		battleShipDirty = false;
	}
	keyState();
}

void drawShipBoard()
{
	gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
	gpro_consoleClear();
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (i % 2 == 0)
			{
				gpro_consoleSetColor(gpro_consoleColor_blue, gpro_consoleColor_blue);
				printf("  ");
				gpro_consoleSetColor(gpro_consoleColor_blue, gpro_consoleColor_blue);
				printf("  ");
			}
			else
			{
				gpro_consoleSetColor(gpro_consoleColor_blue, gpro_consoleColor_blue);
				printf("  ");
				gpro_consoleSetColor(gpro_consoleColor_blue, gpro_consoleColor_blue);
				printf("  ");
			}
		}
		printf("\n");
	}
}

void drawAttackBoard()
{
	gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
	//gpro_consoleClear();
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (i % 2 == 0)
			{
				gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_cyan);
				printf("  ");
				gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_cyan);
				printf("  ");
			}
			else
			{
				gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_cyan);
				printf("  ");
				gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_cyan);
				printf("  ");
			}
		}
		printf("\n");
	}
}

void placeShips(int x, int y, gpro_battleship* shipBrd)
{
	bool placingShips = true;
	int lastX = 0;
	int lastY = 0;
	/*get input on first location and end location, or maybe run a loop for each ship and check valid placements?
	* while(placing ships)
	* {
	*	keyState();
	*	if(attemptingShipPlacement)
	{
		for(int i = 0; i < 5, i++)
		{
			bool firstPartPlaced = false;
			valid position check
			if(firstPartPlaced && (bShipX == lastX - 1||bShipX == lastX + 1 || bShipX) && (bShipY == lastY - 1||bShipY == lastY + 1))
			{
				*(shipBrd)[bShipX][bShipY] += 3 << 4;
				char tile = (*shipBrd)[bShipX][bShipY];
				char player = tile & 3;
				gpro_consoleColor color = (player == 1) ? gpro_consoleColor_white : gpro_consoleColor_blue;
				if (tile != 0 && lastX != bShipX && lastY != bShipY) //check to make sure the same spot isn't being used
				{
					gpro_consoleSetColor(color, gpro_consoleColor_grey_d);
					printf("S");
					if(!firstPartPlaced)
					{
						firstPartPlaced = true;
					}
				}
				else
				{
					printf("  ");
				}
			* place an S on the ship board accordingly
			}
			
		}
	}
	repeat for each of the remaining ships, 4,3,3,2
	  }*/
	
}

void drawSelectedTile(int x, int y)
{
	gpro_consoleSetCursor(2 * (x), (y));
	gpro_consoleSetColor(gpro_consoleColor_red_d, gpro_consoleColor_red_d);
	printf("xx");

}

void fireShot(int x, int y)
{
	/*
	* get the current selected tile, and then compare the tile to the other player's Ship board
	* send a message to the server to the other player with the coordinates, return true or false, update accordingly
	* if hit
	* *(shipBrd)[bShipX][bSHipY] += 3 << 4;
	* gpro_consoleSetColor(color, gpro_consoleColor_grey_d);
	* printf("D");
	* else do nothing
	* update the attack board for the player
	* *(atkBrd)[bShipX][bSHipY] += 3 << 4;
	* gpro_consoleSetColor(color, gpro_consoleColor_grey_d);
	* printf("H");
	* else
	* {
	*	printf("M");
	* }
	* if hit passes, draw an H on the attack board, and have a D replace the S on the other player's Ship board
	* otherwise draw an M for miss on the attack board
	*/
}

void keyState()
{
	short keyState = GetKeyState(VK_UP);
	//printf("%i\n", keyState >> 15);
	if (GetKeyState(VK_UP) >> 15 != 0)
	{
		bShipY = max(0, bShipY - 1);
		battleShipDirty = true;
	}
	if (GetKeyState(VK_DOWN) >> 20 != 0)
	{
		bShipY = min(19, bShipY + 1);
		battleShipDirty = true;
	}
	if (GetKeyState(VK_LEFT) >> 15 != 0)
	{
		bShipX = max(0, bShipX - 1);
		battleShipDirty = true;
	}
	if (GetKeyState(VK_RIGHT) >> 15 != 0)
	{
		bShipX = min(9, bShipX + 1);
		battleShipDirty = true;
	}
	if (GetKeyState(VK_RETURN) >> 15 != 0)
	{
		attemptingShipPlacement = true;
		battleShipDirty = true;
	}
}
