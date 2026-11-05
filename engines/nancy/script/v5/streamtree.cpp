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

#include "common/algorithm.h"
#include "common/debug.h"

#include "engines/nancy/nancy.h"

#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"

#include "engines/nancy/script/v5/streamtree.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/scenestream.h"

namespace Nancy {
namespace Script {

// Each band is this many z orders, with the background at the bottom. The
// scripts use z from -100 to 10, and tell apart z values as close as 0.05.
static const int kBandSize = 5000;
static const int kBandBackgroundZ = 50;
static const double kLowestScriptZ = -100.0;
static const double kStepsPerScriptZ = 40.0;

static int getBandStart(int32 band) {
	return CLIP<int>(band, 0, RecordGraphic::kMaxScriptZOrder / kBandSize - 1) * kBandSize;
}

uint16 getBandZOrder(double z, int32 band) {
	const int steps = (int)floor((z - kLowestScriptZ) * kStepsPerScriptZ + 0.5);
	return (uint16)(getBandStart(band) + CLIP<int>(kBandBackgroundZ * 2 + steps, kBandBackgroundZ + 1, kBandSize - 1));
}

uint16 getBandBackgroundZOrder(int32 band) {
	return (uint16)(getBandStart(band) + kBandBackgroundZ);
}

double getRankZ(int rank) {
	return kLowestScriptZ + (rank + 1) / kStepsPerScriptZ;
}

// The records of a scene

void SceneContext::onBandChanged() {
	for (Record *record : _records) {
		record->onParentNodeChanged();
	}

	for (Record *record : _newRecords) {
		record->onParentNodeChanged();
	}
}

namespace {

// The z values from the topmost node down to a record, which order the records
// the way the tree does
struct DrawKey {
	Record *record;
	uint index;		// of creation, for records with the same place
	Common::Array<double> path;
};

bool isDrawnBefore(const DrawKey &a, const DrawKey &b) {
	for (uint i = 0; i < a.path.size() && i < b.path.size(); ++i) {
		if (a.path[i] != b.path[i]) {
			return a.path[i] < b.path[i];
		}
	}

	if (a.path.size() != b.path.size()) {
		return a.path.size() < b.path.size();
	}

	return a.index < b.index;
}

} // End of anonymous namespace

void SceneContext::updateDrawRanks() {
	Common::Array<DrawKey> keys;
	keys.resize(_records.size());

	for (uint i = 0; i < _records.size(); ++i) {
		DrawKey &key = keys[i];
		key.record = _records[i];
		key.index = i;
		key.path.push_back(_records[i]->getLocalZ());
		for (const GraphNode *node = _records[i]->getParentNode(); node; node = node->getParent()) {
			key.path.insert_at(0, node->getLocalZ());
		}
	}

	Common::sort(keys.begin(), keys.end(), isDrawnBefore);

	for (uint i = 0; i < keys.size(); ++i) {
		keys[i].record->setDrawRank(i);
	}
}

} // End of namespace Script

namespace State {

// The streams

static bool isDrawnBefore(const Script::StreamNodeState *a, const Script::StreamNodeState *b) {
	if (a->isInViewport != b->isInViewport) {
		return a->isInViewport;
	}

	if (a->z != b->z) {
		return a->z < b->z;
	}

	return a->sequence < b->sequence;
}

void SceneStreamManager::getDrawOrder(Common::Array<SceneStream *> &streams) const {
	streams.clear();
	for (SceneStream *stream : _streams) {
		if (stream && stream->isEnded()) {
			continue;
		}

		const Script::StreamNodeState *state = stream ? &stream->getNodeState() : &_mainNodeState;

		uint i = 0;
		while (i < streams.size()) {
			const Script::StreamNodeState *other = streams[i] ? &streams[i]->getNodeState() : &_mainNodeState;
			if (isDrawnBefore(state, other)) {
				break;
			}

			++i;
		}

		streams.insert_at(i, stream);
	}
}

void SceneStreamManager::updateBands() {
	Common::Array<SceneStream *> streams;
	getDrawOrder(streams);

	for (uint i = 0; i < streams.size(); ++i) {
		Script::StreamNodeState &state = streams[i] ? streams[i]->getNodeState() : _mainNodeState;
		if (state.band == (int32)i) {
			continue;
		}

		state.band = i;
		if (streams[i]) {
			streams[i]->onBandChanged();
		} else {
			NancySceneState.onMainStreamBandChanged();
		}
	}
}

double SceneStreamManager::getCapturingZ(double z, bool isInViewport) const {
	// The managers and the cheat tools stay in front
	for (SceneStream *stream : _streams) {
		const Script::StreamNodeState &state = stream ? stream->getNodeState() : _mainNodeState;
		if (stream && (stream->isEnded() || stream->getName().equalsIgnoreCase(getManagerStreamName()) || stream->getName().equalsIgnoreCase("CHEAT"))) {
			continue;
		}

		if (state.isInViewport == isInViewport && state.z >= z) {
			z = state.z + 1.0;
		}
	}

	return z;
}

Script::StreamNodeState *SceneStreamManager::getNodeState(const Common::String &name) {
	if (name.equalsIgnoreCase(getMainStreamName())) {
		return &_mainNodeState;
	}

	SceneStream *stream = find(name);
	return stream ? &stream->getNodeState() : nullptr;
}

int SceneStreamManager::receiveNancy26Message(lua_State *L, const Common::String &sender, const Common::String &name, const Common::String &action) {
	Script::StreamNodeState *state = getNodeState(name);
	if (!state) {
		return 0;
	}

	Common::String upperAction = action;
	upperAction.toUppercase();
	if (upperAction == "ENABLE") {
		state->isEnabled = true;
	} else if (upperAction == "DISABLE") {
		state->isEnabled = false;
	} else if (upperAction == "PLAY") {
		state->isPaused = false;
	} else if (upperAction == "PAUSE") {
		state->isPaused = true;
	} else if (upperAction == "LOCK") {
		state->isPaused = true;
		state->isEnabled = false;
	} else if (upperAction == "UNLOCK") {
		state->isPaused = false;
		state->isEnabled = true;
	} else if (upperAction == "ISPAUSED") {
		lua_pushboolean(L, state->isPaused);
		return 1;
	}

	debugC(1, kDebugScene, "%s asked stream %s to %s", sender.c_str(), name.c_str(), action.c_str());
	return 0;
}

void SceneStream::onBandChanged() {
	if (!_scriptScene) {
		return;
	}

	_scriptScene->onBandChanged();
	if (_background) {
		_background->setZOrder(Script::RecordGraphic::getBackgroundZOrder(_scriptScene->getZOffset()));
	}
}

// The main stream, and the streams the game begins along with it

void Scene::onMainStreamBandChanged() {
	if (!_scriptScene) {
		return;
	}

	_scriptScene->onBandChanged();
	_viewport.setZOrder(Script::RecordGraphic::getBackgroundZOrder(_scriptScene->getZOffset()));
}

// Nancy26 makes its UI of records like any other scene, which the exe puts
// over the whole screen. The stream of the cheat tools comes with it.
void Scene::initNancy26UI() {
	// The exe begins the managers first, which the scripts of the UI call on
	beginManagers();

	StreamFlags uiFlags;
	uiFlags.isInViewport = false;
	_streams.begin(SceneStreamManager::getUIStreamName(), "UI_MAIN_SC", 0, uiFlags);
	_streams.loadNow(SceneStreamManager::getUIStreamName());

	StreamFlags cheatFlags;
	cheatFlags.z = 10.0;
	cheatFlags.isInViewport = false;
	_streams.begin("CHEAT", "CHEAT_MAIN_SC", 0, cheatFlags);
}

} // End of namespace State
} // End of namespace Nancy
