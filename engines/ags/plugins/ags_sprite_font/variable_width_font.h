#pragma once
#include <map>
#include "CharacterEntry.h"
#include "plugin/agsplugin.h"
class VariableWidthFont {
public:
	VariableWidthFont(void);
	~VariableWidthFont(void);
	void SetGlyph(int character, int x, int y, int width, int height);
	int SpriteNumber;
	int FontReplaced;
	int Spacing;
	std::map<char, CharacterEntry> characters;

private:

};

