#pragma once
#include "plugin/agsplugin.h"
#include "VariableWidthFont.h"
#include <vector>
class VariableWidthSpriteFontRenderer :
	public IAGSFontRenderer {
public:
	VariableWidthSpriteFontRenderer(IAGSEngine *engine);
	~VariableWidthSpriteFontRenderer(void);
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
	void SetGlyph(int fontNum, int charNum, int x, int y, int width, int height);
	void SetSprite(int fontNum, int spriteNum);
	void SetSpacing(int fontNum, int spacing);

private:
	IAGSEngine *_engine;
	std::vector<VariableWidthFont * > _fonts;
	VariableWidthFont *getFontFor(int fontNum);
	void Draw(BITMAP *src, BITMAP *dest, int destx, int desty, int srcx, int srcy, int width, int height);
};

