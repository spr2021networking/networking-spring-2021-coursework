#pragma once

#include <vector>
#include <string>

#include "gpro-net-common/gpro-net-console.h"
struct TextBox
{
	TextBox(int lines);
	int numLines;
	std::vector<std::string> messages;
	void addMessage(std::string message);
	void draw(short xCursor, short yCursor);
	void setColor(gpro_consoleColor text, gpro_consoleColor bg);

	int getInputY(int startY);
};