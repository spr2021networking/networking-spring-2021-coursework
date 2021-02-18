#ifndef CHECKERS_H
#define CHECKERS_H

#include "gpro-net/gpro-net-common/gpro-net-gamestate.h"
#include "gpro-net/gpro-net-common/gpro-net-console.h"
#include <stdio.h>
#include <Windows.h>

void checkerLoop(gpro_checkers* chk);
void drawBoard();
void drawPieces(gpro_checkers* chk);
void drawHighlight(gpro_checkers* chk, int x, int y);
void drawSelection(gpro_checkers* chk);
void handleSelection(gpro_checkers* chk);
#endif
