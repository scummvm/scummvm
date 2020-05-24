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

#ifndef DIRECTOR_SCORE_H
#define DIRECTOR_SCORE_H

#include "common/hash-str.h"

namespace Graphics {
	class ManagedSurface;
	class Font;
	class MacWindow;
}

namespace Common {
	class ReadStreamEndian;
	class SeekableSubReadStreamEndian;
}

namespace Director {

class Archive;
struct CastInfo;
class DirectorEngine;
class DirectorSound;
class Frame;
struct Label;
class Lingo;
struct Resource;
class Sprite;
class Stxt;
class BitmapCast;
class ButtonCast;
class ScriptCast;
class ShapeCast;
class TextCast;

struct ZoomBox {
	Common::Rect start;
	Common::Rect end;
	int delay;
	int step;
	uint32 startTime;
	uint32 nextTime;
};

class Score {
public:
	Score(DirectorEngine *vm);
	~Score();

	static Common::Rect readRect(Common::ReadStreamEndian &stream);
	static int compareLabels(const void *a, const void *b);
	bool loadArchive();
	void setStartToLabel(Common::String label);
	void gotoLoop();
	void gotoNext();
	void gotoPrevious();
	void startLoop();
	void setArchive(Archive *archive);
	Archive *getArchive() const { return _movieArchive; };
	void loadConfig(Common::SeekableSubReadStreamEndian &stream);
	void loadCastDataVWCR(Common::SeekableSubReadStreamEndian &stream);
	void loadCastData(Common::SeekableSubReadStreamEndian &stream, uint16 id, Resource *res);
	void loadCastInfo(Common::SeekableSubReadStreamEndian &stream, uint16 id);
	void loadLingoNames(Common::SeekableSubReadStreamEndian &stream);
	void loadLingoContext(Common::SeekableSubReadStreamEndian &stream);
	void setCurrentFrame(uint16 frameId) { _nextFrame = frameId; }
	uint16 getCurrentFrame() { return _currentFrame; }
	Common::String getMacName() const { return _macName; }
	Sprite *getSpriteById(uint16 id);
	void setSpriteCasts();
	void setSpriteBboxes();
	void loadSpriteImages(bool isSharedCast);
	void loadSpriteSounds(bool isSharedCast);
	void copyCastStxts();
	Graphics::ManagedSurface *getSurface() { return _surface; }

	Common::Rect getCastMemberInitialRect(int castId);
	void setCastMemberModified(int castId);

	int getPreviousLabelNumber(int referenceFrame);
	int getCurrentLabelNumber();
	int getNextLabelNumber(int referenceFrame);

	uint16 getSpriteIDFromPos(Common::Point pos);
	bool checkSpriteIntersection(uint16 spriteId, Common::Point pos);
	Common::Rect *getSpriteRect(uint16 spriteId);

	void addZoomBox(ZoomBox *box);
	void renderZoomBox(bool redraw = false);
	bool haveZoomBox() { return !_zoomBoxes.empty(); }

	int32 getStageColor() { return _stageColor; }

	Cast *getCastMember(int castId);
	void renderFrame(uint16 frameId, bool forceUpdate = false, bool updateStageOnly = false);
	void renderSprite(uint16 id);
	void unrenderSprite(uint16 spriteId);

private:
	void update();
	void renderText(uint16 spriteId, Common::Rect *textSize);
	void renderShape(uint16 spriteId);
	void renderButton(uint16 spriteId);
	void renderBitmap(uint16 spriteId);

	// ink.cpp
	void inkBasedBlit(Graphics::ManagedSurface *maskSurface, const Graphics::Surface &spriteSurface, InkType ink, Common::Rect drawRect, uint spriteId);
	void drawBackgndTransSprite(const Graphics::Surface &sprite, Common::Rect &drawRect, int spriteId);
	void drawMatteSprite(const Graphics::Surface &sprite, Common::Rect &drawRect);
	void drawGhostSprite(const Graphics::Surface &sprite, Common::Rect &drawRect);
	void drawReverseSprite(const Graphics::Surface &sprite, Common::Rect &drawRect, uint16 spriteId);

	// score.cpp
	void playSoundChannel(uint16 frameId);

	void readVersion(uint32 rid);
	void loadPalette(Common::SeekableSubReadStreamEndian &stream);
	void loadFrames(Common::SeekableSubReadStreamEndian &stream);
	void loadLabels(Common::SeekableSubReadStreamEndian &stream);
	void loadActions(Common::SeekableSubReadStreamEndian &stream);
	void loadScriptText(Common::SeekableSubReadStreamEndian &stream);
	void loadFileInfo(Common::SeekableSubReadStreamEndian &stream);
	void loadFontMap(Common::SeekableSubReadStreamEndian &stream);
	void dumpScript(const char *script, ScriptType type, uint16 id);
	Common::String getString(Common::String str);
	Common::Array<Common::String> loadStrings(Common::SeekableSubReadStreamEndian &stream, uint32 &entryType, bool hasHeader = true);

	bool processImmediateFrameScript(Common::String s, int id);

public:
	Common::Array<Frame *> _frames;
	Common::Array<Sprite *> _sprites;
	Common::HashMap<uint16, CastInfo *> _castsInfo;
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _castsNames;
	Common::SortedArray<Label *> *_labels;
	Common::HashMap<uint16, Common::String> _actions;
	Common::HashMap<uint16, bool> _immediateActions;
	Common::HashMap<uint16, Common::String> _fontMap;
	Common::Array<uint16> _castScriptIds;
	Graphics::ManagedSurface *_surface;
	Graphics::ManagedSurface *_maskSurface;
	Graphics::ManagedSurface *_backSurface;
	Graphics::ManagedSurface *_backSurface2;
	Graphics::Font *_font;
	Archive *_movieArchive;
	Common::Rect _movieRect;
	uint16 _currentMouseDownSpriteId;
	uint16 _currentClickOnSpriteId;
	bool _mouseIsDown;
	uint32 _lastEventTime;
	uint32 _lastRollTime;
	uint32 _lastClickTime;
	uint32 _lastKeyTime;
	uint32 _lastTimerReset;

	bool _stopPlay;
	uint32 _nextFrameTime;

	Common::HashMap<int, Cast *> *_loadedCast;

	Common::HashMap<int, const Stxt *> *_loadedStxts;

	uint16 _castIDoffset;

	int _numChannelsDisplayed;

	Graphics::MacWindow *_window;

	uint16 _framesRan; // used by kDebugFewFramesOnly

private:
	uint16 _versionMinor;
	uint16 _versionMajor;
	Common::String _macName;
	Common::String _createdBy;
	Common::String _changedBy;
	Common::String _script;
	Common::String _directory;
	byte _currentFrameRate;
	uint16 _castArrayStart;
	uint16 _currentFrame;
	uint16 _nextFrame;
	int _currentLabel;
	uint32 _flags;
	uint16 _castArrayEnd;
	uint16 _movieScriptCount;
	uint16 _stageColor;
	Lingo *_lingo;
	DirectorSound *_soundManager;
	DirectorEngine *_vm;

	Common::Array<ZoomBox *> _zoomBoxes;
};

} // End of namespace Director

#endif
