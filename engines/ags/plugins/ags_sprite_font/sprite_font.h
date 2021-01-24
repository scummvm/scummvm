#pragma once
class SpriteFont {
public:
	SpriteFont(void);
	~SpriteFont(void);
	int SpriteNumber;
	int MinChar;
	int MaxChar;
	int Rows;
	int Columns;
	int FontReplaced;
	int CharHeight;
	int CharWidth;
	bool Use32bit;
};

