#include "gpro-net-Console-Client/checkers.h"

bool dirty = true;
int x = 4; int y = 4;
int timer = 0;
int maxTime = 200000;
void checkerLoop(gpro_checkers* chk)
{
	timer--;
	if (dirty)
	{
		drawBoard();
		drawPieces(chk);
		drawHighlight(chk, x, y);
		gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
		dirty = false;
	}
	short keyState = GetKeyState(VK_UP);
	//printf("%i\n", keyState >> 15);
	if (timer <= 0)
	{
		if (GetKeyState(VK_UP) >> 15 != 0)
		{
			y = max(0, y - 1);
			dirty = true;
			timer = maxTime;
		}
		if (GetKeyState(VK_DOWN) >> 15 != 0)
		{
			y = min(7, y + 1);
			dirty = true;
			timer = maxTime;
		}
		if (GetKeyState(VK_LEFT) >> 15 != 0)
		{
			x = max(0, x - 1);
			dirty = true;
			timer = maxTime;
		}
		if (GetKeyState(VK_RIGHT) >> 15 != 0)
		{
			x = min(7, x + 1);
			dirty = true;
			timer = maxTime;
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

void drawHighlight(gpro_checkers* chk, int x, int y)
{
	gpro_consoleSetCursor(2 * (x), (y));
	gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_cyan);
	if (x % 2 == y % 2)
	{
		char tile = (*chk)[y][x/2]; // divide by two because board x is between [0,4) and selection is between [0,8)
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
