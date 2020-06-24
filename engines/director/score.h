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
#include "graphics/macgui/macwindowmanager.h"

namespace Graphics {
	class ManagedSurface;
	class Font;
	class MacWindow;
	struct ZoomBox;
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
struct Channel;
class Sprite;
class Stxt;
class BitmapCast;
class ScriptCast;
class ShapeCast;
class TextCast;

struct TransParams {
	TransitionType type;
	uint duration;
	uint chunkSize;
	uint area;

	int steps;
	int stepDuration;

	int xStepSize;
	int yStepSize;

	int xpos, ypos;

	int stripSize;

	TransParams() {
		type = kTransNone;
		duration = 250;
		chunkSize = 1;
		area = 0;
		steps = 0;
		stepDuration = 0;
		stripSize = 0;

		xStepSize = yStepSize = 0;
		xpos = ypos = 0;
	}
};

struct Channel {
	Sprite *_sprite;

	bool _visible;
	uint _constraint;
	Common::Point _currentPoint;
	Common::Point _delta;

	Channel(Sprite *sp);
	Common::Rect getBbox();
	Common::Point getPosition();
	void updateLocation();
	void addDelta(Common::Point pos);
	void resetPosition();
};

class Score {
public:
	Score(DirectorEngine *vm);
	~Score();

	static Common::Rect readRect(Common::ReadStreamEndian &stream);
	static int compareLabels(const void *a, const void *b);
	bool loadArchive(bool isSharedCast);
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
	Channel *getChannelById(uint16 id);
	Sprite *getSpriteById(uint16 id);
	void setSpriteCasts();
	void loadSpriteImages(bool isSharedCast);
	void loadSpriteSounds(bool isSharedCast);
	void copyCastStxts();
	Graphics::ManagedSurface *getSurface() { return _surface; }

	Common::Rect getCastMemberInitialRect(int castId);
	void setCastMemberModified(int castId);

	int getPreviousLabelNumber(int referenceFrame);
	int getCurrentLabelNumber();
	int getNextLabelNumber(int referenceFrame);

	uint16 getSpriteIDFromPos(Common::Point pos, bool onlyActive = false);
	bool checkSpriteIntersection(uint16 spriteId, Common::Point pos);

	Cast *getCastMember(int castId);
	const Stxt *getStxt(int castId);
	void renderFrame(uint16 frameId, bool forceUpdate = false, bool updateStageOnly = false);
	void markDirtyRect(Common::Rect dirty);

	// transition.cpp
	void playTransition(uint16 transDuration, uint8 transArea, uint8 transChunkSize, TransitionType transType);

private:
	void update();
	void renderShape(uint16 spriteId);

	// ink.cpp
	void inkBasedBlit(Graphics::ManagedSurface *maskSurface, const Graphics::Surface &spriteSurface, InkType ink, Common::Rect drawRect, uint spriteId);
	void drawMatteSprite(const Graphics::Surface &sprite, Common::Rect &drawRect);
	void drawReverseSprite(const Graphics::Surface &sprite, Common::Rect &drawRect, uint16 spriteId);

	// transitions.cpp
	void initTransParams(TransParams &t, Common::Rect &clipRect);
	void dissolveTrans(TransParams &t, Common::Rect &clipRect);
	void dissolvePatternsTrans(TransParams &t, Common::Rect &clipRect);
	void transMultiPass(TransParams &t, Common::Rect &clipRect);
	void transZoom(TransParams &t, Common::Rect &clipRect);

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
	void screenShot();
	Common::String getString(Common::String str);
	Common::Array<Common::String> loadStrings(Common::SeekableSubReadStreamEndian &stream, uint32 &entryType, bool hasHeader = true);

	bool processImmediateFrameScript(Common::String s, int id);

public:
	Common::Array<Channel *> _channels;
	Common::Array<Frame *> _frames;
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
	uint32 _lastEventTime;
	uint32 _lastRollTime;
	uint32 _lastClickTime;
	uint32 _lastKeyTime;
	uint32 _lastTimerReset;
	uint16 _stageColor;

	byte _puppetTempo;
	bool _stopPlay;
	uint32 _nextFrameTime;

	Common::HashMap<int, Cast *> *_loadedCast;

	Common::HashMap<int, const Stxt *> *_loadedStxts;

	uint16 _castIDoffset;

	int _numChannelsDisplayed;

	Graphics::MacWindow *_window;

	uint16 _framesRan; // used by kDebugFewFramesOnly

	int _lingoArchive;

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
	Lingo *_lingo;
	DirectorSound *_soundManager;
	DirectorEngine *_vm;
};

} // End of namespace Director

#endif
