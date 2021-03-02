#include "gpro-net/textbox.h"

TextBox::TextBox(int lines)
{
	numLines = lines;
	dirty = true;
}

void TextBox::addMessage(std::string message)
{
	messages.push_back(message);
	if ((int)messages.size() >= numLines)
	{
		messages.erase(messages.begin());
	}
	dirty = true;
}

void TextBox::draw(short xCursor, short yCursor)
{
	if (dirty)
	{
		gpro_consoleSetCursor(xCursor, yCursor);
		for (int i = 0; i < (int)messages.size(); i++)
		{
			printf("%s", messages[i].c_str());
			if (i < (int)messages.size() - 1)
			{
				printf("\n");
			}
		}
		gpro_consoleSetCursor(0, getInputY(yCursor));
		dirty = false;
	}
}

void TextBox::setColor(gpro_consoleColor text, gpro_consoleColor bg)
{
	gpro_consoleSetColor(text, bg);
}

int TextBox::getInputY(int startY)
{
	return startY + (int)messages.size();
}

void TextBox::setLineCount(int lines)
{
	if (lines < (int)messages.size())
	{
		int diff = (int)messages.size() - lines;
		messages.erase(messages.begin(), messages.begin() + diff);
	}
	numLines = lines;
}

void TextBox::blankLine(short yCoord)
{
	gpro_consoleClear();
	dirty = true;
	draw(0, yCoord);
}

void TextBox::clear()
{
	messages.erase(messages.begin(), messages.end());
	dirty = true;
}
