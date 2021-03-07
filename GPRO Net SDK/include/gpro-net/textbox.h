/*
* textbox.h
* Contributors: Ben Cooper and Scott Dagen
* Contributions: text rendering
*/
#pragma once

#include <vector>
#include <string>

#include "gpro-net-common/gpro-net-console.h"
struct TextBox
{
	TextBox(int lines = 20);
	int numLines;
	std::vector<std::string> messages;
	//add text to storage and clear out any messages that need to be deleted
	void addMessage(std::string message);
	//render all text
	void draw(short xCursor, short yCursor);
	//wrapper for gpro_consoleSetColor, assigns the text and background color.
	void setColor(gpro_consoleColor text, gpro_consoleColor bg);
	//find user input coordinate when given the starting y for rendering
	int getInputY(int startY);
	//Resize the textbox
	void setLineCount(int lines);
	//delete all text
	void clear();
	//tell the textbox to draw again
	bool dirty;
};