/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DIRECTOR_SCORE_H
#define DIRECTOR_SCORE_H

//#include "graphics/macgui/macwindowmanager.h"

#include "director/cursor.h"

namespace Graphics {
	struct Surface;
	class ManagedSurface;
	class Font;
	class MacWindow;
	struct ZoomBox;
}

namespace Common {
	class ReadStreamEndian;
	class SeekableReadStreamEndian;
}

namespace Director {

class Window;
class Archive;
class DirectorEngine;
class DirectorSound;
class Frame;
struct Label;
class Movie;
struct Resource;
class Cursor;
class Channel;
class Sprite;
class CastMember;
class AudioDecoder;

enum RenderMode {
	kRenderModeNormal,
	kRenderForceUpdate
};

class Score {
public:
	Score(Movie *movie);
	~Score();

	Movie *getMovie() const { return _movie; }

	void loadFrames(Common::SeekableReadStreamEndian &stream, uint16 version);
	void loadLabels(Common::SeekableReadStreamEndian &stream);
	void loadActions(Common::SeekableReadStreamEndian &stream);
	void loadSampleSounds(uint type);

	static int compareLabels(const void *a, const void *b);
	uint16 getLabel(Common::String &label);
	Common::String *getLabelList();
	Common::String *getFrameLabel(uint id);
	void setStartToLabel(Common::String &label);
	void gotoLoop();
	void gotoNext();
	void gotoPrevious();
	void startPlay();
	void step();
	void stopPlay();

	void setCurrentFrame(uint16 frameId) { _nextFrame = frameId; }
	uint16 getCurrentFrame() { return _currentFrame; }
	int getNextFrame() { return _nextFrame; }

	int getCurrentPalette();
	int resolvePaletteId(int id);

	Channel *getChannelById(uint16 id);
	Sprite *getSpriteById(uint16 id);
	Sprite *getOriginalSpriteById(uint16 id);

	void setSpriteCasts();

	int getPreviousLabelNumber(int referenceFrame);
	int getCurrentLabelNumber();
	int getNextLabelNumber(int referenceFrame);

	uint16 getSpriteIDFromPos(Common::Point pos);
	uint16 getMouseSpriteIDFromPos(Common::Point pos);
	uint16 getActiveSpriteIDFromPos(Common::Point pos);
	bool checkSpriteIntersection(uint16 spriteId, Common::Point pos);
	Common::List<Channel *> getSpriteIntersections(const Common::Rect &r);
	uint16 getSpriteIdByMemberId(CastMemberID id);

	bool renderTransition(uint16 frameId);
	void renderFrame(uint16 frameId, RenderMode mode = kRenderModeNormal);
	void renderSprites(uint16 frameId, RenderMode mode = kRenderModeNormal);
	bool renderPrePaletteCycle(uint16 frameId, RenderMode mode = kRenderModeNormal);
	void setLastPalette(uint16 frameId);
	bool isPaletteColorCycling();
	void renderPaletteCycle(uint16 frameId, RenderMode mode = kRenderModeNormal);
	void renderCursor(Common::Point pos, bool forceUpdate = false);
	void updateWidgets(bool hasVideoPlayback);

	void invalidateRectsForMember(CastMember *member);

	void playSoundChannel(uint16 frameId, bool puppetOnly);

	Common::String formatChannelInfo();

private:
	void update();
	void playQueuedSound();

	void screenShot();

	bool processImmediateFrameScript(Common::String s, int id);
	bool processFrozenScripts();

public:
	Common::Array<Channel *> _channels;
	Common::Array<Frame *> _frames;
	Common::SortedArray<Label *> *_labels;
	Common::HashMap<uint16, Common::String> _actions;
	Common::HashMap<uint16, bool> _immediateActions;

	byte _currentFrameRate;

	byte _puppetTempo;
	bool _puppetPalette;
	int _lastPalette;
	int _paletteTransitionIndex;
	byte _paletteSnapshotBuffer[768];

	PlayState _playState;
	uint32 _nextFrameTime;
	int _lastTempo;
	int _waitForChannel;
	int _waitForVideoChannel;
	bool _waitForClick;
	bool _waitForClickCursor;
	bool _cursorDirty;
	int _activeFade;
	Cursor _defaultCursor;
	CursorRef _currentCursor;

	int _numChannelsDisplayed;

private:
	DirectorEngine *_vm;
	Lingo *_lingo;
	Movie *_movie;
	Window *_window;

	uint16 _currentFrame;
	uint16 _nextFrame;
	int _currentLabel;
	DirectorSound *_soundManager;
	int _currentPalette;

	int _previousBuildBotBuild = -1;
};

} // End of namespace Director

#endif
