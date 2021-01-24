#pragma once
#include "plugin/agsplugin.h"
#include "SpriteFont.h"
#include <vector>
class SpriteFontRenderer :
	public IAGSFontRenderer {
public:
	SpriteFontRenderer(IAGSEngine *engine);
	~SpriteFontRenderer(void);
	bool LoadFromDisk(int fontNumber, int fontSize) override {
		return true;
	}
	void FreeMemory(int fontNumber) override { }
	bool SupportsExtendedCharacters(int fontNumber) override;
	int GetTextWidth(const char *text, int fontNumber) override;
	int GetTextHeight(const char *text, int fontNumber) override;
	void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) override;
	void AdjustYCoordinateForFont(int *ycoord, int fontNumber) override { }
	void EnsureTextValidForFont(char *text, int fontNumber) override;
	void SetSpriteFont(int fontNum, int sprite, int rows, int columns, int charWidth, int charHeight, int charMin, int charMax, bool use32bit);


private:
	SpriteFont *getFontFor(int fontNum);
	void Draw(BITMAP *src, BITMAP *dest, int destx, int desty, int srcx, int srcy, int width, int height);
	std::vector<SpriteFont * > _fonts;
	IAGSEngine *_engine;
};

