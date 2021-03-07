/*
* textbox.cpp
* Contributors: Ben Cooper and Scott Dagen
* Contributions: text rendering
*/

#include "gpro-net/textbox.h"

//create textbox with specific size
TextBox::TextBox(int lines)
{
	numLines = lines;
	dirty = true;
}

//add message and remove old if necessary
void TextBox::addMessage(std::string message)
{
	messages.push_back(message);
	if ((int)messages.size() >= numLines)
	{
		messages.erase(messages.begin());
	}
	dirty = true;
}

//draw text if the text box has been modified
void TextBox::draw(short xCursor, short yCursor)
{
	if (dirty)
	{
		gpro_consoleSetCursor(xCursor, yCursor);
		for (int i = 0; i < (int)messages.size(); i++)
		{
			printf("%s", messages[i].c_str());
			if (i > 0) //add whitespace to prevent a rendering error
			{
				int numSpaces = (int)messages[i - 1].size() - (int)messages[i].size();
				for (int j = 0; j < numSpaces; j++)
				{
					printf(" ");
				}
			}
			//if (i < (int)messages.size() - 1)
			{
				printf("\n");
			}
		}
		gpro_consoleSetCursor(0, getInputY(yCursor));
		dirty = false;
	}
}

//wrapper for gpro_consoleSetColor, assigns the text and background color.
void TextBox::setColor(gpro_consoleColor text, gpro_consoleColor bg)
{
	gpro_consoleSetColor(text, bg);
}

//find offset from the start coordinate
int TextBox::getInputY(int startY)
{
	return startY + (int)messages.size();
}

//resize textbox, erasing old messages as needed
void TextBox::setLineCount(int lines)
{
	if (lines < (int)messages.size())
	{
		int diff = (int)messages.size() - lines;
		messages.erase(messages.begin(), messages.begin() + diff);
	}
	numLines = lines;
}

//clear all messages
void TextBox::clear()
{
	messages.erase(messages.begin(), messages.end());
	dirty = true;
}
