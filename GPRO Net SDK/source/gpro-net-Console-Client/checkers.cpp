#include "gpro-net-Console-Client/checkers.h"

bool dirty = true;
int x = 4; int y = 4;
void checkerLoop(gpro_checkers* chk)
{
	if (dirty)
	{
		drawCheckers(chk);
		drawHighlight(x, y);
		gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
		dirty = false;
	}
	short keyState = GetKeyState(VK_UP);
	//printf("%i\n", keyState >> 15);
	if (GetKeyState(VK_UP) >> 15 != 0)
	{
		y = max(0, y-1);
		dirty = true;
	}
	if (GetKeyState(VK_DOWN) >> 15 != 0)
	{
		y = min(7, y + 1);
		dirty = true;
	}

}

void drawCheckers(gpro_checkers* chk)
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
				printf("xx");
				gpro_consoleSetColor(gpro_consoleColor_red, gpro_consoleColor_red);
				printf("xx");
			}
			else
			{
				gpro_consoleSetColor(gpro_consoleColor_red, gpro_consoleColor_red);
				printf("xx");
				gpro_consoleSetColor(gpro_consoleColor_grey_d, gpro_consoleColor_grey_d);
				printf("xx");
			}
		}
		printf("\n");
	}
}

void drawHighlight(int x, int y)
{
	gpro_consoleSetCursor(2 * (x), (y));
	gpro_consoleSetColor(gpro_consoleColor_cyan, gpro_consoleColor_cyan);
	printf("xx");
}
