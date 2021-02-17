#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include "gpro-net/gpro-net-common/gpro-net-gamestate.h"
#include "gpro-net/gpro-net-common/gpro-net-console.h"
#include <stdio.h>
#include <Windows.h>

void battleshipLoop(gpro_battleship* shipBrd, gpro_battleship* atkBrd);
void drawShipBoard();
void drawAttackBoard();
void placeShips(int x, int y, gpro_battleship* shipBrd);
void drawSelectedTile(int x, int y);
void fireShot(int x, int y);
#endif
