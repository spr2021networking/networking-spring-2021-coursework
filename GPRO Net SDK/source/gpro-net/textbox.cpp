#include "gpro-net/textbox.h"

TextBox::TextBox(int lines)
{
	numLines = lines;
}

void TextBox::addMessage(std::string message)
{
	messages.push_back(message);
	if ((int)messages.size() < numLines)
	{
		messages.erase(messages.begin());
	}
}

void TextBox::draw(short xCursor, short yCursor)
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
}

void TextBox::setColor(gpro_consoleColor text, gpro_consoleColor bg)
{
	gpro_consoleSetColor(text, bg);
}

int TextBox::getInputY(int startY)
{
	return startY+numLines;
}
