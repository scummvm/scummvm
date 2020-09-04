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

//#include "graphics/macgui/macwindowmanager.h"

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

enum RenderMode {
	kRenderModeNormal,
	kRenderForceUpdate,
	kRenderUpdateStageOnly,
	kRenderNoUnrender,
	kRenderNoWindowRender
};

class Score {
public:
	Score(Movie *movie);
	~Score();

	Movie *getMovie() const { return _movie; }

	void loadFrames(Common::SeekableReadStreamEndian &stream);
	void loadLabels(Common::SeekableReadStreamEndian &stream);
	void loadActions(Common::SeekableReadStreamEndian &stream);

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

	void setSpriteCasts();

	int getPreviousLabelNumber(int referenceFrame);
	int getCurrentLabelNumber();
	int getNextLabelNumber(int referenceFrame);

	uint16 getSpriteIDFromPos(Common::Point pos);
	uint16 getMouseSpriteIDFromPos(Common::Point pos);
	uint16 getActiveSpriteIDFromPos(Common::Point pos);
	bool checkSpriteIntersection(uint16 spriteId, Common::Point pos);
	Common::List<Channel *> getSpriteIntersections(const Common::Rect &r);

	bool renderTransition(uint16 frameId);
	void renderFrame(uint16 frameId, RenderMode mode = kRenderModeNormal);
	void renderSprites(uint16 frameId, RenderMode mode = kRenderModeNormal);
	void renderCursor(Common::Point pos);

private:
	void update();

	void playSoundChannel(uint16 frameId);

	void screenShot();

	bool processImmediateFrameScript(Common::String s, int id);

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

	PlayState _playState;
	uint32 _nextFrameTime;
	int _waitForChannel;
	bool _waitForClick;
	bool _waitForClickCursor;
	bool _cursorDirty;
	int _activeFade;
	Cursor *_currentCursor;

	int _numChannelsDisplayed;

	uint16 _framesRan; // used by kDebugFewFramesOnly

private:
	DirectorEngine *_vm;
	Lingo *_lingo;
	Movie *_movie;
	Window *_window;

	uint16 _currentFrame;
	uint16 _nextFrame;
	int _currentLabel;
	DirectorSound *_soundManager;
};

} // End of namespace Director

#endif
