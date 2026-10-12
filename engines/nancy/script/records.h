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

#ifndef NANCY_SCRIPT_RECORDS_H
#define NANCY_SCRIPT_RECORDS_H

#include "common/array.h"
#include "common/rect.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/script/record.h"

#include "engines/nancy/ui/ndui/element.h"

namespace Nancy {

namespace UI {
class Viewport;
}

namespace Script {

// The kinds of records a script can make, in the order of the names below
enum RecordKind {
	kRecordNavLogic		= 0,
	kRecordSummary		= 1,
	kRecordOverlay		= 2,
	kRecordMovie		= 3,
	kRecordSound		= 4,
	kRecordVideoConvo	= 5,
	kRecordSoundConvo	= 6,
	kRecordText			= 7,
	kRecordOverride		= 8,
	kRecordParticle		= 9,
	kRecordAnimOverlay	= 10,

	kNumRecordKinds		= 11
};

// What scripts call the kind ("NavLogic"), and the type of its objects ("ND.NavLogic")
const char *getRecordKindName(RecordKind kind);
const char *getRecordTypeName(RecordKind kind);

// Returns null for the kinds that are not supported
Record *makeRecord(RecordKind kind, SceneContext &context);

// What the "Draw" object of the scripts draws on: the record whose "Render"
// function is running. Null at any other time.
Graphics::ManagedSurface *getDrawTarget();

// What a record draws. Records are placed in the coordinates of the viewport.
class RecordGraphic : public RenderObject {
public:
	RecordGraphic() : RenderObject(0) {}
	virtual ~RecordGraphic() {}

	// Scripts order what they draw with numbers of their own
	void setScriptZ(double z, int32 streamZOffset);

protected:
	bool isViewportRelative() const override { return true; }
};

// The first record of every scene, which names the area the scene belongs to
// and its background. A background with more than one frame is a node the
// player can turn around in.
// The area has a set of music themes. One of them starts with the scene when
// no theme is playing, and a scene can ask for another one once it is over.
// TODO: Handle the modColor field.
class SummaryRecord : public Record {
public:
	SummaryRecord(SceneContext &context) : Record(context, "ND.Summary") {}

	const Common::String &getEnvironment() const { return _environment; }
	const Common::String &getBackgroundName() const { return _backgroundName; }

	// The viewport that shows the background, once it has been loaded
	void setBackground(UI::Viewport *background);

protected:
	void readTable(int tableIndex) override;
	void runOnce() override;
	void run(double sceneTime, double elapsedTime) override;
	bool hasTransform() const override { return true; }
	bool getField(const Common::String &key) override;

	void onMouseMove(const ScriptPoint &point, bool inViewport) override;
	bool getCursor(const ScriptPoint &point, bool inViewport, uint &cursorType, bool &isHot) override;
	bool onMouseEvent(RecordEvent event, const ScriptPoint &point, bool inViewport) override;

private:
	enum Turn { kNoTurn, kTurnLeft, kTurnRight, kTurnAround };

	// The side of the node the mouse is on
	Turn getTurnAt(const ScriptPoint &point, bool inViewport) const;
	void setFrame(uint frame);
	void playTheme();
	void updateTheme(double sceneTime);

	Common::String _environment;
	Common::String _backgroundName;

	bool _loopsTheme = false;
	double _themePause = 0.0;		// the seconds of silence between two themes
	double _themeEndTime = 0.0;		// when the theme was found to be over; 0 while one plays
	double _lastThemeCheck = 0.0;

	UI::Viewport *_background = nullptr;
	uint _numFrames = 0;
	uint _frame = 0;

	Turn _turn = kNoTurn;
	bool _isTurningFast = false;
	double _lastTurnTime = 0.0;
	double _sceneTime = 0.0;
};

// Changes the scene and sets flags, either right away or when its hotspot gets clicked
class NavLogicRecord : public Record {
public:
	NavLogicRecord(SceneContext &context) : Record(context, "ND.NavLogic") {}

protected:
	void readTable(int tableIndex) override;
	void run(double sceneTime, double elapsedTime) override;
	bool prepareReset() override { return true; }
	bool hasTransform() const override { return true; }
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

	bool getCursor(const ScriptPoint &point, bool inViewport, uint &cursorType, bool &isHot) override;
	bool onMouseEvent(RecordEvent event, const ScriptPoint &point, bool inViewport) override;

private:
	struct Flag {
		Common::String name;
		bool value = false;
	};

	void readScene(int valueIndex);
	void readCursor(int valueIndex);
	bool isOver(const ScriptPoint &point) const;
	void trigger();

	Common::String _sceneName;
	Common::String _streamName;
	int _frame = 0;

	ScriptRect _hotspot;
	bool _hasCursor = false;
	uint _cursorType = 0;
	bool _cursorIsHot = false;

	Common::Array<Flag> _flags;
};

// A record that only does what the functions a script gave it do
// TODO: Call RenderAttachment and OnLoad, and handle Send.
class OverrideRecord : public Record {
public:
	OverrideRecord(SceneContext &context) : Record(context, "ND.OverrideAR") {}
	virtual ~OverrideRecord();

	void updateGraphics() override;
	void onSave() override { callFunction("OnSave"); }
	bool hasMessagingName(const Common::String &name) override;

protected:
	void readTable(int tableIndex) override;
	bool prepareReset() override { return true; }
	bool isScripted() const override { return true; }
	void onActivation() override { callFunction("OnActivation"); }
	void onDeactivation() override { callFunction("OnDeactivation"); }
	void onZChanged() override;
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	// What the "Render" function draws on, for the records that have one
	RecordGraphic *_canvas = nullptr;
};

// Shows a part of an image
class OverlayRecord : public Record {
public:
	OverlayRecord(SceneContext &context) : Record(context, "ND.Overlay") {}

	void updateGraphics() override;

protected:
	void readTable(int tableIndex) override;
	void onZChanged() override;
	bool hasTransform() const override { return true; }
	void onTransformChanged() override { applyRects(); }
	void renderNow() override;
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	void applyRects();

	Graphics::ManagedSurface _image;
	ScriptRect _source;
	ScriptRect _dest;
	uint32 _modColor = 0xFFFFFFFF;	// what the colors of the picture get multiplied with
	bool _hasPicture = false;
	RecordGraphic _graphic;

	// The picture as a script had it drawn by itself, which lasts for a frame
	RecordGraphic _attachment;
	bool _hasAttachment = false;
	bool _isAttachmentDrawn = false;
};

// Shows the parts of an image one after the other
class AnimOverlayRecord : public Record {
public:
	AnimOverlayRecord(SceneContext &context) : Record(context, "ND.AnimOvlAR") {}

	void updateGraphics() override;

	void setPaused(bool isPaused) { _isPaused = isPaused; }

protected:
	void readTable(int tableIndex) override;
	void runOnce() override;
	void run(double sceneTime, double elapsedTime) override;
	void onZChanged() override;
	bool prepareReset() override;
	bool hasTransform() const override { return true; }
	void onTransformChanged() override { applyFrame(); }
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	// Frames are counted from 1
	int getFirstFrame() const { return _firstFrame > 0 ? _firstFrame : 1; }
	int getLastFrame() const { return _lastFrame > 0 ? _lastFrame : (int)_sources.size(); }
	void setFrame(int frame);
	void applyFrame();
	void beginLoop();

	Graphics::ManagedSurface _image;
	Common::Array<ScriptRect> _sources;
	ScriptRect _dest;
	uint32 _modColor = 0xFFFFFFFF;
	bool _hasPicture = false;
	RecordGraphic _graphic;

	int _firstFrame = -1;
	int _lastFrame = -1;
	int _frame = 1;
	double _framesPerSecond = 15.0;
	double _progress = 0.0;	// through the frames, from 0 to 1
	double _endTime = 0.0;

	int _numLoops = 1;		// 0 loops forever
	int _numLoopsDone = 0;
	bool _pausesOnLastFrame = false;
	bool _isPaused = false;
	bool _isVisible = true;
};

// Plays a sound, picked at random when it has several
class SoundRecord : public Record {
public:
	SoundRecord(SceneContext &context) : Record(context, "ND.SoundAR") {}

	void onSkip() override;

protected:
	void readTable(int tableIndex) override;
	void runOnce() override;
	void run(double sceneTime, double elapsedTime) override;
	bool prepareReset() override;
	bool getField(const Common::String &key) override;

private:
	Common::Array<Common::String> _names;
	uint16 _channel = 14;
	double _volume = 1.0;
	bool _isLooping = false;
	bool _usesCaption = true;

	SoundDescription _sound;
	Common::String _caption;	// what the sound says, while it is the one on screen
	bool _isPlaying = false;
	bool _wasPreempted = false;
	double _endTime = 0.0;
};

// Shows a text in the viewport, in one of the fonts and font colors of the game.
// The text is the key of one of the game's texts, unless the record says
// otherwise. It can be shown a letter at a time, and can move up or down.
// TODO: Handle the useBackground, bg and frame fields.
class TextRecord : public Record {
public:
	TextRecord(SceneContext &context) : Record(context, "ND.TextAR") {}

	void updateGraphics() override;

	// Shows the next letter of a text that isn't shown at once, along with the
	// markup in front of it. Returns false when there is nothing left to show.
	bool revealNext();

protected:
	void readTable(int tableIndex) override;
	void run(double sceneTime, double elapsedTime) override;
	void onZChanged() override;
	bool prepareReset() override;
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	void setText(const Common::String &text);
	void applyDest();

	UI::NDUI::Element _element;
	RecordGraphic _graphic;
	bool _isDirty = true;

	ScriptRect _dest;
	double _yOffset = 0.0;
	double _yVelocity = 0.0;	// per second

	// What the record goes back to when it starts over
	ScriptRect _initialDest;
	double _initialYOffset = 0.0;
	double _initialYVelocity = 0.0;

	bool _usesAutotext = true;
	bool _revealsAll = true;
	Common::String _text;
	Common::String _shownText;
	uint _numRevealed = 0;
};

// Plays a movie in the viewport
// TODO: Handle the source, volume, loop and modColor fields.
class MovieRecord : public Record {
public:
	MovieRecord(SceneContext &context) : Record(context, "ND.Movie") {}

	void updateGraphics() override;
	bool isSkippable() const override;

	void setPaused(bool isPaused) { _isPaused = isPaused; }

protected:
	void readTable(int tableIndex) override;
	void runOnce() override;
	void run(double sceneTime, double elapsedTime) override;
	void onZChanged() override;
	bool prepareReset() override;
	bool hasTransform() const override { return true; }
	void onTransformChanged() override { applyPosition(); }
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	void drawFrame();
	void applyPosition();

	Common::String _name;
	MoviePlayer _player;
	RecordGraphic _graphic;
	ScriptRect _dest;

	int _firstFrame = 0;
	int _lastFrame = 0;
	bool _pausesOnLastFrame = false;
	bool _isPaused = false;
	bool _isVisible = true;
	double _endTime = 0.0;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_RECORDS_H
