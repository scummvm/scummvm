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

#ifndef NANCY_SCRIPT_TRANSFORMER_H
#define NANCY_SCRIPT_TRANSFORMER_H

#include "common/array.h"
#include "common/str.h"

#include "engines/nancy/script/record.h"

namespace Nancy {
namespace Script {

class GraphNode;

// The named stages a transform is made of, in the order they were pushed:
//
//   piece:PushTranslation("t", Vector:New(100, 50))
//   piece:PushRotation("r", 0)
//   piece:Blend("r", 0.5, math.pi)    -- turns it half around in half a second
//
// Setting the field of a stage's name puts it at a value right away; Blend() takes
// it there over a time, slowly at the end unless the record said sinLerp = false.
// While a stage blends, a function "<stage>AffectLerp" of the record can bend the
// progress from 0 to 1, and "<stage>Done" gets called once it is there.
// Nancy22 has them in Transformer records only; in Nancy26 every record has them,
// and a stage can also make the record see-through.
class BlendStages {
public:
	enum StageKind {
		kTranslation,
		kRotation,
		kScale,
		kAlpha		// Nancy26
	};

	BlendStages(Record &owner) : _owner(owner) {}

	bool isEmpty() const { return _stages.empty(); }

	// The product of the stages, and where they are going
	const ScriptMatrix &getTransform() const { return _transform; }
	const ScriptMatrix &getBlendedTransform() const { return _blendedTransform; }

	// The product of the alpha stages; 1 without any
	double getAlpha() const { return _alpha; }
	bool hasAlpha() const { return _hasAlpha; }

	void setSineLerp(bool isSineLerp) { _isSineLerp = isSineLerp; }
	bool isSineLerp() const { return _isSineLerp; }

	double getTimeMultiplier() const { return _timeMultiplier; }
	void setTimeMultiplier(double multiplier) { _timeMultiplier = multiplier; }

	// Moves the blends on. Returns whether anything changed.
	bool advance(double seconds);

	// The functions of the scripts, which return their number of results. The
	// record is at stack index 1.
	int push(lua_State *L, StageKind kind);
	int blend(lua_State *L);
	int finishBlend(lua_State *L);
	int isBlending(lua_State *L);
	int removeAll(lua_State *L);

	// The value of a stage, by its name
	bool getField(lua_State *L, const Common::String &key);
	bool setField(lua_State *L, const Common::String &key);

	// The functions above, for the records that have them
	static bool pushFunction(lua_State *L, const Common::String &key);

private:
	struct Stage {
		Common::String name;
		StageKind kind = kTranslation;

		// x and y; a rotation and an alpha have their value in x
		double start[2] = { 0.0, 0.0 };
		double current[2] = { 0.0, 0.0 };
		double target[2] = { 0.0, 0.0 };

		double time = 0.0;
		double duration = 0.0;

		ScriptMatrix matrix;	// of the current value
		ScriptMatrix blended;	// of the target

		bool isBlending() const { return duration > time; }
	};

	Stage *findStage(const Common::String &name);
	Stage &checkStage(lua_State *L, int index);

	static void readValue(lua_State *L, int index, StageKind kind, double value[2]);
	static ScriptMatrix makeMatrix(StageKind kind, const double value[2]);
	void pushValue(lua_State *L, const Stage &stage);

	// Puts a stage at a value at once
	void setStage(Stage &stage, const double value[2]);
	void applyProgress(Stage &stage, double progress);

	// Moves a stage on in its blend. Returns whether it was blending.
	bool advanceStage(Stage &stage, double seconds);

	// Works out the product of the stages, and tells the record
	void update();

	Record &_owner;
	Common::Array<Stage> _stages;
	bool _isSineLerp = true;
	double _timeMultiplier = 1.0;	// Nancy24: how much faster the blends go
	ScriptMatrix _transform;
	ScriptMatrix _blendedTransform;
	double _alpha = 1.0;
	bool _hasAlpha = false;
};

// Nancy22 moves, turns and resizes what a scene shows with Transformer records.
// The records a Transformer gets with Attach() are moved by its transform on top
// of their own, and so are other Transformers attached to it.
class TransformerRecord : public Record {
public:
	TransformerRecord(SceneContext &context, const char *typeName = "ND.TransformerAR");

	RecordType getType() const override { return kRecordTransformer; }

	GraphNode *getNode() const { return _node; }

	int attach(lua_State *L);

	void onStagesChanged() override;

protected:
	void readTable(int tableIndex) override;
	void run(double sceneTime, double elapsedTime) override;
	void onZChanged() override;
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	GraphNode *_node;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_TRANSFORMER_H
