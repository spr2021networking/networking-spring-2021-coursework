#include "gpro-net-Console-Client/battleship.h"

void battleshipLoop(gpro_battleship* blt)
{
}

void drawShipBoard()
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

void drawAttackBoard()
{
}

void drawShips(int x, int y)
{
}
