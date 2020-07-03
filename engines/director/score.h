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
	class SeekableSubReadStreamEndian;
}

namespace Director {

class Stage;
class Archive;
class DirectorEngine;
class DirectorSound;
class Frame;
struct Label;
class Movie;
struct Resource;
struct Channel;
class Sprite;
class CastMember;

enum RenderMode {
	kRenderModeNormal,
	kRenderForceUpdate,
	kRenderUpdateStageOnly,
	kRenderNoUnrender
};

struct TransParams {
	TransitionType type;
	uint frame;
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
		frame = 0;
		duration = 250;
		chunkSize = 1;
		area = 0;
		steps = 0;
		stepDuration = 0;
		stripSize = 0;

		xStepSize = yStepSize = 0;
		xpos = ypos = 0;
	}

	TransParams(uint16 d, uint16 a, uint16 c, TransitionType t) :
		duration(d), area(a), chunkSize(c), type(t) {}
};

struct MacShape {
	InkType ink;
	byte spriteType;
	byte foreColor;
	byte backColor;
	int lineSize;
};

struct Channel {
	Sprite *_sprite;

	bool _dirty;
	bool _visible;
	uint _constraint;
	Common::Point _currentPoint;
	Common::Point _delta;

	Channel(Sprite *sp);
	bool isDirty(Sprite *nextSprite = nullptr);

	Common::Rect getBbox();
	Common::Point getPosition();
	MacShape *getShape();
	Graphics::ManagedSurface *getSurface();
	const Graphics::Surface *getMask();

	void setClean(Sprite *nextSprite, int spriteId);
	void addDelta(Common::Point pos);
};

class Score {
public:
	Score(Movie *movie);
	~Score();

	void loadFrames(Common::SeekableSubReadStreamEndian &stream);
	void loadLabels(Common::SeekableSubReadStreamEndian &stream);
	void loadActions(Common::SeekableSubReadStreamEndian &stream);

	static int compareLabels(const void *a, const void *b);
	void setStartToLabel(Common::String label);
	void gotoLoop();
	void gotoNext();
	void gotoPrevious();
	void startLoop();
	void setCurrentFrame(uint16 frameId) { _nextFrame = frameId; }
	uint16 getCurrentFrame() { return _currentFrame; }
	Channel *getChannelById(uint16 id);
	Sprite *getSpriteById(uint16 id);

	void setSpriteCasts();

	int getPreviousLabelNumber(int referenceFrame);
	int getCurrentLabelNumber();
	int getNextLabelNumber(int referenceFrame);

	uint16 getSpriteIDFromPos(Common::Point pos, bool onlyActive = false);
	bool checkSpriteIntersection(uint16 spriteId, Common::Point pos);
	Common::List<Channel *> getSpriteIntersections(const Common::Rect &r);

	bool renderTransition(uint16 frameId);
	void renderFrame(uint16 frameId, RenderMode mode = kRenderModeNormal);
	void renderSprites(uint16 frameId, RenderMode mode = kRenderModeNormal);

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
	bool _stopPlay;
	uint32 _nextFrameTime;

	int _numChannelsDisplayed;

	uint16 _framesRan; // used by kDebugFewFramesOnly

private:
	DirectorEngine *_vm;
	Lingo *_lingo;
	Movie *_movie;
	Stage *_stage;

	uint16 _currentFrame;
	uint16 _nextFrame;
	int _currentLabel;
	DirectorSound *_soundManager;
};

} // End of namespace Director

#endif
