/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SLUDGE_GRAPHICS_H
#define SLUDGE_GRAPHICS_H

#include "common/stream.h"

#include "graphics/surface.h"
#include "graphics/transparent_surface.h"

#include "sludge/sprbanks.h"

namespace Sludge {

class Parallax;
class SludgeEngine;
class SpritePalette;

struct StackHandler;
struct FrozenStuffStruct;
struct LoadedSpriteBank;
struct OnScreenPerson;
struct SpriteBank;
struct Sprite;
struct SpriteLayers;
struct ZBufferData;

enum ELightMapMode {
	LIGHTMAPMODE_NONE = -1,
	LIGHTMAPMODE_HOTSPOT,
	LIGHTMAPMODE_PIXEL,
	LIGHTMAPMODE_NUM
};

class GraphicsManager {
public:
	GraphicsManager(SludgeEngine *vm);
	virtual ~GraphicsManager();

	void init();
	void kill();

	// graphics
	void setWindowSize(uint winWidth, uint winHeight) { _winWidth = winWidth; _winHeight = winHeight; }
	bool initGfx();
	void display();
	void clear();

	// Parallax
	bool loadParallax(uint16 v, uint16 fracX, uint16 fracY);
	void killParallax();
	void saveParallax(Common::WriteStream *fp);
	void drawParallax();

	// Backdrop
	void killAllBackDrop();
	bool resizeBackdrop(int x, int y);
	bool killResizeBackdrop(int x, int y);
	void killBackDrop();
	void loadBackDrop(int fileNum, int x, int y);
	void mixBackDrop(int fileNum, int x, int y);
	void drawBackDrop();
	void blankScreen(int x1, int y1, int x2, int y2);
	void blankAllScreen();
	void darkScreen();
	void saveHSI(Common::WriteStream *stream);
	bool loadHSI(Common::SeekableReadStream *stream, int, int, bool);
	bool mixHSI(Common::SeekableReadStream *stream, int x = 0, int y = 0);
	void drawLine(uint, uint, uint, uint);
	void drawHorizontalLine(uint, uint, uint);
	void drawVerticalLine(uint, uint, uint);
	void hardScroll(int distance);
	bool getRGBIntoStack(uint x, uint y, StackHandler *sH);
	void saveBackdrop(Common::WriteStream *stream); // To game save
	void loadBackdrop(int ssgVersion, Common::SeekableReadStream *streamn); // From game save

	// Lightmap
	int _lightMapMode;
	void killLightMap();
	bool loadLightMap(int v);
	void saveLightMap(Common::WriteStream *stream);
	bool loadLightMap(int ssgVersion, Common::SeekableReadStream *stream);

	// Snapshot
	void nosnapshot();
	bool snapshot();
	void saveSnapshot(Common::WriteStream *stream);
	bool restoreSnapshot(Common::SeekableReadStream *stream);

	// Camera
	int getCamX() { return _cameraX; }
	int getCamY() { return _cameraY; }
	float getCamZoom() { return _cameraZoom; }
	void aimCamera(int cameraX, int cameraY);
	void zoomCamera(int z);

	// Screen
	int getCenterX(int width) { return (_winWidth - width) >> 1; }
	int checkSizeValide(int width, int height) { return ((width >= 0) && (height >= 0) && (width < (int)_winWidth) && (height < (int)_winHeight)); }

	// Freeze
	bool freeze();
	void unfreeze(bool killImage = true);
	int howFrozen();
	bool isFrozen() { return (_frozenStuff != nullptr); }

	// Sprites
	static void forgetSpriteBank(SpriteBank &forgetme);
	bool loadSpriteBank(char *filename, SpriteBank &loadhere);
	bool loadSpriteBank(int fileNum, SpriteBank &loadhere, bool isFont);

	void fontSprite(int x1, int y1, Sprite &single, const SpritePalette &fontPal);
	void flipFontSprite(int x1, int y1, Sprite &single, const SpritePalette &fontPal);

	bool scaleSprite(Sprite &single, const SpritePalette &fontPal, OnScreenPerson *thisPerson, bool mirror);
	void fixScaleSprite(int x1, int y1, Sprite &single, const SpritePalette &fontPal, OnScreenPerson *thisPerson, const int camX, const int camY, bool);

	void pasteSpriteToBackDrop(int x1, int y1, Sprite &single, const SpritePalette &fontPal);
	bool reserveSpritePal(SpritePalette &sP, int n);
	void burnSpriteToBackDrop(int x1, int y1, Sprite &single, const SpritePalette &fontPal);

	void resetSpriteLayers(ZBufferData *ptrZBuffer, int x, int y, bool upsidedown);
	void addSpriteDepth(Graphics::Surface *ptr, int depth, int x, int y, Graphics::FLIP_FLAGS flip, int width = -1, int height = -1, bool disposeAfterUse = false);
	void displaySpriteLayers();
	void killSpriteLayers();

	// Sprite Bank
	LoadedSpriteBank *loadBankForAnim(int ID);

	// ZBuffer
	bool setZBuffer(int y);
	void killZBuffer();
	void drawZBuffer(int x, int y, bool upsidedown);
	void saveZBuffer(Common::WriteStream *stream);
	bool loadZBuffer(Common::SeekableReadStream *stream);

	// Colors
	void setBlankColor(int r, int g, int b) { _currentBlankColour = _renderSurface.format.RGBToColor(r & 255, g & 255, b & 255);};
	void setBurnColor(int r, int g, int b) {
		_currentBurnG = r;
		_currentBurnG = g;
		_currentBurnB = b;
	}
	void saveColors(Common::WriteStream *stream);
	void loadColors(Common::SeekableReadStream *stream);

	// Thumbnail
	bool setThumbnailSize(int thumbWidth, int thumbHeight);
	bool saveThumbnail(Common::WriteStream *stream);
	bool skipThumbnail(Common::SeekableReadStream *stream);
	void showThumbnail(const Common::String &filename, int x, int y);

	// Transition
	void setBrightnessLevel(int brightnessLevel);
	void setFadeMode(int fadeMode) { _fadeMode = fadeMode; };
	void fixBrightness();
	void resetRandW();

private:
	SludgeEngine *_vm;

	uint _winWidth, _winHeight, _sceneWidth, _sceneHeight;

	// renderSurface
	Graphics::Surface _renderSurface;

	// Snapshot
	Graphics::Surface _snapshotSurface;

	// LightMap
	int _lightMapNumber;
	Graphics::Surface _lightMap;

	// Parallax
	Parallax *_parallaxStuff;

	// Camera
	float _cameraZoom;
	int _cameraX, _cameraY;

	// Freeze
	FrozenStuffStruct *_frozenStuff;
	Graphics::Surface _freezeSurface;
	void freezeGraphics();

	// Back drop
	Graphics::Surface _backdropSurface;
	Graphics::Surface _origBackdropSurface;
	bool _backdropExists;
	bool reserveBackdrop();

	// Sprites
	SpriteLayers *_spriteLayers;
	void fontSprite(bool flip, int x, int y, Sprite &single, const SpritePalette &fontPal);
	Graphics::Surface *duplicateSurface(Graphics::Surface *surface);
	void blendColor(Graphics::Surface * surface, uint32 color, Graphics::TSpriteBlendMode mode);
	Graphics::Surface *applyLightmapToSprite(Graphics::Surface *&blitted, OnScreenPerson *thisPerson, bool mirror, int x, int y, int x1, int y1, int diffX, int diffY);

	// Sprite banks
	LoadedSpriteBanks _allLoadedBanks;

	// ZBuffer
	ZBufferData *_zBuffer;
	void sortZPal(int *oldpal, int *newpal, int size);

	// Colors
	uint _currentBlankColour;
	byte _currentBurnR, _currentBurnG, _currentBurnB;

	// Thumbnail
	int _thumbWidth;
	int _thumbHeight;

	// Transition
	byte _brightnessLevel;
	byte _fadeMode;
};

} // End of namespace Sludge


#endif // SLUDGE_GRAPHICS_H
