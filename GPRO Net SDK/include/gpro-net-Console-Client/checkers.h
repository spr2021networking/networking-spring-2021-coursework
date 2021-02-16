#ifndef CHECKERS_H
#define CHECKERS_H

#include "gpro-net/gpro-net-common/gpro-net-gamestate.h"
#include "gpro-net/gpro-net-common/gpro-net-console.h"
#include <stdio.h>
#include <Windows.h>

void checkerLoop(gpro_checkers* chk);
void drawCheckers(gpro_checkers* chk);
void drawHighlight(int x, int y);
#endif
