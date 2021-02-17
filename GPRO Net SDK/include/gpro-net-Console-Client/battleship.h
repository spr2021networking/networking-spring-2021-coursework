#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include "gpro-net/gpro-net-common/gpro-net-gamestate.h"
#include "gpro-net/gpro-net-common/gpro-net-console.h"
#include <stdio.h>
#include <Windows.h>

void battleshipLoop(gpro_battleship* blt);
void drawShipBoard();
void drawAttackBoard();
//void drawPieces(gpro_checkers* chk);
void drawShips(int x, int y);
#endif
