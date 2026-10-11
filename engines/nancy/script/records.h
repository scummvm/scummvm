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
#include "engines/nancy/renderobject.h"

#include "engines/nancy/script/record.h"

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
class SummaryRecord : public Record {
public:
	SummaryRecord(SceneContext &context) : Record(context, "ND.Summary") {}

	const Common::String &getEnvironment() const { return _environment; }
	const Common::String &getBackgroundName() const { return _backgroundName; }

	// The viewport that shows the background, once it has been loaded
	void setBackground(UI::Viewport *background);

protected:
	void readTable(int tableIndex) override;
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

	Common::String _environment;
	Common::String _backgroundName;

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
class OverrideRecord : public Record {
public:
	OverrideRecord(SceneContext &context) : Record(context, "ND.OverrideAR") {}
	virtual ~OverrideRecord();

	void updateGraphics() override;

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

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_RECORDS_H
