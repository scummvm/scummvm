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

#include "common/textconsole.h"

#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/transformer.h"

namespace Nancy {
namespace Script {

static const double kMinBlendTime = 0.001;

TransformerRecord::TransformerRecord(SceneContext &context) :
		Record(context, "ND.TransformerAR"),
		_node(context.createGraphNode()) {}

void TransformerRecord::readTable(int tableIndex) {
	readField(getLua(), tableIndex, "sinLerp", _isSineLerp);
	_node->setZ(getLocalZ());
}

// The node goes wherever the record gets attached to
void TransformerRecord::onZChanged() {
	if (_node->getParent() != getParentNode()) {
		_node->setParent(getParentNode());
	}

	_node->setZ(getLocalZ());
}

TransformerRecord::Stage *TransformerRecord::findStage(const Common::String &name) {
	for (Stage &stage : _stages) {
		if (stage.name == name) {
			return &stage;
		}
	}

	return nullptr;
}

TransformerRecord::Stage &TransformerRecord::checkStage(lua_State *L, int index) {
	const char *name = luaL_checkstring(L, index);
	Stage *stage = findStage(name);
	if (!stage) {
		luaL_argerror(L, index, Common::String::format("Transformer AR does not have a stage by the name '%s'", name).c_str());
	}

	return *stage;
}

// An angle, or x and y: a vector, a point, or two numbers
void TransformerRecord::readValue(lua_State *L, int index, StageKind kind, double value[2]) {
	if (kind == kRotation) {
		value[0] = luaL_checknumber(L, index);
		return;
	}

	if (!lua_istable(L, index) && !lua_isuserdata(L, index)) {
		value[0] = luaL_checknumber(L, index);
		value[1] = luaL_checknumber(L, index + 1);
		return;
	}

	lua_getfield(L, index, "x");
	lua_getfield(L, index, "y");
	if (!lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
		luaL_argerror(L, index, "Table must have 'x' and 'y' fields");
	}

	value[0] = lua_tonumber(L, -2);
	value[1] = lua_tonumber(L, -1);
	lua_pop(L, 2);
}

ScriptMatrix TransformerRecord::makeMatrix(StageKind kind, const double value[2]) {
	ScriptMatrix matrix;

	switch (kind) {
	case kTranslation:
		matrix.m[3][0] = value[0];
		matrix.m[3][1] = value[1];
		break;
	case kRotation:
		// D3DXMatrixRotationZ
		matrix.m[0][0] = cos(value[0]);
		matrix.m[0][1] = sin(value[0]);
		matrix.m[1][0] = -sin(value[0]);
		matrix.m[1][1] = cos(value[0]);
		break;
	case kScale:
		matrix.m[0][0] = value[0];
		matrix.m[1][1] = value[1];
		break;
	}

	return matrix;
}

void TransformerRecord::pushValue(const Stage &stage) {
	lua_State *L = getLua();

	if (stage.kind == kRotation) {
		lua_pushnumber(L, stage.current[0]);
		return;
	}

	// A translation goes nowhere in depth, and a scale keeps it
	ScriptVector vector(stage.current[0], stage.current[1], stage.kind == kScale ? 1.0 : 0.0);
	Script::pushVector(L, vector);
}

void TransformerRecord::setStage(Stage &stage, const double value[2]) {
	for (int i = 0; i < 2; ++i) {
		stage.start[i] = stage.current[i] = stage.target[i] = value[i];
	}

	stage.matrix = stage.blended = makeMatrix(stage.kind, value);
}

void TransformerRecord::applyProgress(Stage &stage, double progress) {
	for (int i = 0; i < 2; ++i) {
		stage.current[i] = stage.start[i] + (stage.target[i] - stage.start[i]) * progress;
	}

	stage.matrix = makeMatrix(stage.kind, stage.current);
}

bool TransformerRecord::advanceStage(Stage &stage, double seconds) {
	if (!stage.isBlending()) {
		return false;
	}

	lua_State *L = getLua();
	stage.time += seconds;
	double progress = CLIP(stage.time / stage.duration, 0.0, 1.0);

	// The scripts can bend the progress; otherwise it slows down towards the end
	lua_pushnumber(L, progress);
	if (callFunction((stage.name + "AffectLerp").c_str(), 1, 1)) {
		if (!lua_isnumber(L, -1)) {
			warning("TransformerAR::AffectLerp: Value returned was not a number");
		}

		progress = lua_tonumber(L, -1);
		lua_pop(L, 1);
	} else if (_isSineLerp) {
		progress = sin(progress * M_PI * 0.5);
	}

	applyProgress(stage, CLIP(progress, 0.0, 1.0));

	if (!stage.isBlending()) {
		for (int i = 0; i < 2; ++i) {
			stage.start[i] = stage.current[i] = stage.target[i];
		}

		stage.matrix = stage.blended;

		// The stage may go away along with the record in there
		const Common::String doneName = stage.name + "Done";
		callFunction(doneName.c_str());
	}

	return true;
}

// The stages make the transform in the order they were pushed in
void TransformerRecord::updateTransform() {
	ScriptMatrix transform;
	ScriptMatrix blendedTransform;
	for (const Stage &stage : _stages) {
		transform = transform * stage.matrix;
		blendedTransform = blendedTransform * stage.blended;
	}

	_blendedTransform = blendedTransform;
	_node->setTransform(transform);
}

void TransformerRecord::run(double sceneTime, double elapsedTime) {
	bool hasChanged = false;
	for (uint i = 0; i < _stages.size(); ++i) {
		hasChanged |= advanceStage(_stages[i], elapsedTime);
	}

	if (hasChanged) {
		updateTransform();
	}
}

// transformer:Push<Translation|Rotation|Scale>(name, value)
int TransformerRecord::pushStage(lua_State *L, StageKind kind) {
	const char *name = luaL_checkstring(L, 2);
	if (findStage(name)) {
		return luaL_argerror(L, 2, Common::String::format("Transformer AR already has a stage by the name '%s'", name).c_str());
	}

	Stage stage;
	stage.name = name;
	stage.kind = kind;

	double value[2];
	readValue(L, 3, kind, value);
	setStage(stage, value);

	_stages.push_back(stage);
	updateTransform();
	return 0;
}

// transformer:Blend(name, seconds, value)
int TransformerRecord::blend(lua_State *L) {
	Stage &stage = checkStage(L, 2);
	const double duration = luaL_checknumber(L, 3);

	readValue(L, 4, stage.kind, stage.target);
	stage.blended = makeMatrix(stage.kind, stage.target);
	for (int i = 0; i < 2; ++i) {
		stage.start[i] = stage.current[i];
	}

	stage.duration = MAX(kMinBlendTime, duration);
	stage.time = 0.0;

	updateTransform();
	return 0;
}

// transformer:FinishBlend([name]) puts a stage where it is going right away, or
// all of them
int TransformerRecord::finishBlend(lua_State *L) {
	Common::String name;
	if (lua_gettop(L) > 1) {
		name = checkStage(L, 2).name;
	}

	for (uint i = 0; i < _stages.size(); ++i) {
		if (name.empty() || _stages[i].name == name) {
			advanceStage(_stages[i], _stages[i].duration - _stages[i].time);
		}
	}

	updateTransform();
	return 0;
}

// transformer:IsBlending(name)
int TransformerRecord::isBlending(lua_State *L) {
	lua_pushboolean(L, checkStage(L, 2).isBlending());
	return 1;
}

// transformer:Attach(record, ...)
int TransformerRecord::attach(lua_State *L) {
	for (int i = 2; i <= lua_gettop(L); ++i) {
		GraphNode *node = testGraphNode(L, i);
		if (node) {
			node->setParent(_node);
		} else {
			checkRecord(L, i)->setParentNode(_node);
		}
	}

	return 0;
}

static TransformerRecord *checkTransformer(lua_State *L) {
	TransformerRecord *transformer = dynamic_cast<TransformerRecord *>(checkRecord(L, 1));
	if (!transformer) {
		luaL_typerror(L, 1, "ND.TransformerAR");
	}

	return transformer;
}

static int transformerPushTranslation(lua_State *L) {
	return checkTransformer(L)->pushStage(L, TransformerRecord::kTranslation);
}

static int transformerPushRotation(lua_State *L) {
	return checkTransformer(L)->pushStage(L, TransformerRecord::kRotation);
}

static int transformerPushScale(lua_State *L) {
	return checkTransformer(L)->pushStage(L, TransformerRecord::kScale);
}

static int transformerBlend(lua_State *L) {
	return checkTransformer(L)->blend(L);
}

static int transformerFinishBlend(lua_State *L) {
	return checkTransformer(L)->finishBlend(L);
}

static int transformerIsBlending(lua_State *L) {
	return checkTransformer(L)->isBlending(L);
}

static int transformerAttach(lua_State *L) {
	return checkTransformer(L)->attach(L);
}

static const luaL_Reg kTransformerFunctions[] = {
	{ "Attach", transformerAttach },
	{ "Blend", transformerBlend },
	{ "FinishBlend", transformerFinishBlend },
	{ "IsBlending", transformerIsBlending },
	{ "PushTranslation", transformerPushTranslation },
	{ "PushRotation", transformerPushRotation },
	{ "PushScale", transformerPushScale },
	{ nullptr, nullptr }
};

bool TransformerRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "transform") {
		pushMatrix(L, _node->getTransform());
		return true;
	} else if (key == "blendedTransform") {
		pushMatrix(L, _blendedTransform);
		return true;
	}

	for (const luaL_Reg *function = kTransformerFunctions; function->name; ++function) {
		if (key == function->name) {
			lua_pushcfunction(L, function->func);
			return true;
		}
	}

	const Stage *stage = findStage(key);
	if (stage) {
		pushValue(*stage);
		return true;
	}

	return false;
}

// Setting a stage puts it at the value at once
bool TransformerRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	Stage *stage = findStage(key);
	if (stage) {
		if (stage->isBlending()) {
			luaL_error(L, "Stage %s is in the midst of blending. It cannot be overridden.", key.c_str());
		}

		double value[2];
		readValue(L, 3, stage->kind, value);
		setStage(*stage, value);
		updateTransform();
		return true;
	}

	if (key == "transform" || key == "blendedTransform") {
		luaL_error(L, "You cannot set the transform or blendedTransform fields of a transformer AR directly");
	}

	return false;
}

} // End of namespace Script
} // End of namespace Nancy
