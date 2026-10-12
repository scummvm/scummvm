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

#include "engines/nancy/nancy.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/actionrecord.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/scenestream.h"

#include "engines/nancy/ui/viewport.h"

#ifdef ENABLE_NANCY_V4
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"
#endif

namespace Nancy {
namespace State {

static const uint16 kNoSceneID = 0x7ffe;

void StreamFlags::read(Common::SeekableReadStream &stream) {
	lifetime = stream.readByte();
	capturesInput = stream.readByte();
	runsFirst = stream.readByte();
	stream.skip(1);	// Unknown
	zOffset = stream.readSint32LE();
}

SceneStream::SceneStream(const Common::String &name, const StreamFlags &flags, int32 zOffset) :
		_name(name),
		_flags(flags),
		_zOffset(zOffset) {
	_sceneInfo.sceneID = kNoSceneID;
	_genericFlags.resize(g_nancy->getStaticData().genericEventFlags.size());
	clearScene();
}

SceneStream::~SceneStream() {
	clearScene();
}

bool SceneStream::update() {
	if (_isEnded) {
		return false;
	}

	processRecords();

	if (_hasNextScene && !_isEnded) {
		_hasNextScene = false;

		if (loadScene()) {
			processRecords();
		} else if (_sceneName.empty()) {
			// There was nothing to run in the first place
			_isEnded = true;
		}
	}

	return !_isEnded;
}

void SceneStream::processRecords() {
#ifdef ENABLE_NANCY_V4
	if (_scriptScene) {
		_scriptScene->run(NancySceneState.getFrameSeconds());
		return;
	}
#endif

	_actionManager.processActionRecords();
}

void SceneStream::onSave() {
#ifdef ENABLE_NANCY_V4
	if (_scriptScene) {
		_scriptScene->onSave();
	}
#endif
}

bool SceneStream::receiveMessage(const Script::Message &message, bool &wasTaken) {
#ifdef ENABLE_NANCY_V4
	if (_scriptScene) {
		return _scriptScene->receiveMessage(message, wasTaken);
	}
#endif

	return false;
}

void SceneStream::handleInput(NancyInput &input) {
	if (_isEnded) {
		return;
	}

#ifdef ENABLE_NANCY_V4
	if (_scriptScene) {
		_scriptScene->handleInput(input);
		return;
	}
#endif

	_actionManager.handleInput(input);
}

void SceneStream::changeScene(const Common::String &sceneName, uint16 frameID) {
	// Only the first request made in a frame counts
	if (_hasNextScene || sceneName.equalsIgnoreCase("s32767")) {
		return;
	}

	_hasNextScene = true;
	_nextSceneName = sceneName;
	_nextFrameID = frameID;
}

void SceneStream::setEventFlag(int16 index, byte flag) {
	const Common::Array<uint16> &genericFlags = g_nancy->getStaticData().genericEventFlags;
	for (uint i = 0; i < genericFlags.size(); ++i) {
		if (genericFlags[i] == index) {
			_genericFlags[i] = flag;
		}
	}
}

byte SceneStream::getEventFlag(int16 index) const {
	const Common::Array<uint16> &genericFlags = g_nancy->getStaticData().genericEventFlags;
	for (uint i = 0; i < genericFlags.size(); ++i) {
		if (genericFlags[i] == index) {
			return _genericFlags[i];
		}
	}

	return g_nancy->_false;
}

#ifdef ENABLE_NANCY_V4
// Nancy19 scenes are Lua scripts, which make their own records
bool SceneStream::loadScriptedScene() {
	Script::SceneContext *scriptScene = new Script::SceneContext(_name, _nextSceneName, _nextFrameID, _zOffset);
	if (!scriptScene->load()) {
		warning("Stream %s: could not load scene %s", _name.c_str(), _nextSceneName.c_str());
		delete scriptScene;
		return false;
	}

	beginScene();
	_scriptScene = scriptScene;

	Script::SummaryRecord *summary = _scriptScene->getSummary();
	if (!summary->getBackgroundName().empty()) {
		loadBackground(Common::Path(summary->getBackgroundName()));
	}

	summary->setBackground(_background);
	return true;
}
#endif

// Takes the stream from the scene it was running to the one it asked for
void SceneStream::beginScene() {
	// The streams that were started for the outgoing scene end with it
	const bool hadScene = !_sceneName.empty();
	clearScene();

	if (hadScene) {
		for (uint i = 0; i < _children.size(); ) {
			if (_children[i].lifetime == StreamFlags::kEndsWithParentScene) {
				NancySceneState.getStreams().end(_children[i].name);
				_children.remove_at(i);
			} else {
				++i;
			}
		}
	}

	_sceneName = _nextSceneName;
	_sceneInfo.frameID = _nextFrameID;

	// Numbered scenes are named after their number
	_sceneInfo.sceneID = kNoSceneID;
	if (_sceneName.size() > 1 && tolower(_sceneName[0]) == 's' && Common::isDigit(_sceneName[1])) {
		_sceneInfo.sceneID = atoi(_sceneName.c_str() + 1);
		NancySceneState.addSceneVisit(_sceneInfo.sceneID);
	}

	debugC(0, kDebugScene, "Stream %s: loading scene %s, frame %u", _name.c_str(), _sceneName.c_str(), _nextFrameID);
}

void SceneStream::loadBackground(const Common::Path &videoFile) {
	_background = new UI::Viewport();
	_background->init();
	_background->setZOrder(_background->getZOrder() + _zOffset);
	_background->loadVideo(videoFile, _sceneInfo.frameID, 0, kPanNone, kLargeVideoFormat);
	_background->registerGraphics();
}

bool SceneStream::loadScene() {
#ifdef ENABLE_NANCY_V4
	if (g_nancy->getGameType() >= kGameTypeNancy19) {
		return loadScriptedScene();
	}
#endif

	IFF *sceneIFF = g_nancy->_resource->loadIFF(Common::Path(_nextSceneName));
	if (!sceneIFF) {
		warning("Stream %s: missing scene file %s", _name.c_str(), _nextSceneName.c_str());
		return false;
	}

	Common::SeekableReadStream *summaryChunk = sceneIFF->getChunkStream("TSUM");
	if (!summaryChunk) {
		warning("Stream %s: scene file %s has no summary", _name.c_str(), _nextSceneName.c_str());
		delete sceneIFF;
		return false;
	}

	Common::Path videoFile;
	summaryChunk->skip(0x32);	// Description
	readFilename(*summaryChunk, videoFile);
	delete summaryChunk;

	beginScene();

	if (!videoFile.empty() && !videoFile.equalsIgnoreCase("NO_BG")) {
		loadBackground(videoFile);
	}

	Common::SeekableReadStream *actionRecordChunk = nullptr;
	for (uint i = 0; actionRecordChunk = sceneIFF->getChunkStream("ACT", i), actionRecordChunk != nullptr; ++i) {
		_actionManager.addNewActionRecord(*actionRecordChunk, sceneIFF->getChunkSource("ACT", i));
		delete actionRecordChunk;

		// The records that draw something are stacked along with their stream
		RenderObject *renderObject = dynamic_cast<RenderObject *>(_actionManager._records.back());
		if (renderObject && _zOffset) {
			renderObject->setZOrder(renderObject->getZOrder() + _zOffset);
		}
	}

	delete sceneIFF;
	return true;
}

void SceneStream::clearScene() {
	// Nothing of a stream's scene carries over to the next one
	for (Action::ActionRecord *record : _actionManager._records) {
		delete record;
	}

	_actionManager._records.clear();
	_actionManager.clearActionRecords();

#ifdef ENABLE_NANCY_V4
	delete _scriptScene;
	_scriptScene = nullptr;
#endif

	delete _background;
	_background = nullptr;

	for (byte &flag : _genericFlags) {
		flag = g_nancy->_false;
	}
}

static const char *const kMainStreamName = "MainStream";

SceneStream *SceneStreamManager::find(const Common::String &name) const {
	for (SceneStream *stream : _streams) {
		if (stream && !stream->isEnded() && stream->getName().equalsIgnoreCase(name)) {
			return stream;
		}
	}

	return nullptr;
}

void SceneStreamManager::begin(const Common::String &name, const Common::String &sceneName, uint16 frameID, const StreamFlags &flags) {
	if (find(name)) {
		// Already running
		return;
	}

	int32 zOffset = flags.zOffset + (_running ? _running->getZOffset() : 0);
	SceneStream *stream = new SceneStream(name, flags, zOffset);
	stream->changeScene(sceneName, frameID);

	if (flags.runsFirst) {
		_streams.insert_at(0, stream);
	} else {
		_streams.push_back(stream);
	}

	// The stream that starts another one decides how long it lives
	SceneStream::Child child;
	child.name = name;
	child.lifetime = flags.lifetime;
	if (_running) {
		_running->_children.push_back(child);
	} else {
		_mainStreamChildren.push_back(child);
	}
}

void SceneStreamManager::end(const Common::String &name) {
	SceneStream *stream = name.empty() ? _running : find(name);
	if (!stream) {
		return;
	}

	stream->end();
	endChildren(stream->_children);
}

void SceneStreamManager::endChildren(const Common::Array<SceneStream::Child> &children) {
	for (const SceneStream::Child &child : children) {
		if (child.lifetime != StreamFlags::kIndependent) {
			end(child.name);
		}
	}
}

void SceneStreamManager::changeScene(const Common::String &name, const SceneChangeDescription &sceneDescription) {
	SceneStream *stream = name.empty() ? _running : find(name);
	if (stream) {
		stream->changeScene(Common::String::format("s%u", sceneDescription.sceneID), sceneDescription.frameID);
		return;
	}

	if (name.empty() || name.equalsIgnoreCase(kMainStreamName)) {
		SceneStream *running = _running;
		_running = nullptr;
		NancySceneState.changeScene(sceneDescription);
		_running = running;
	} else {
		warning("Tried to change the scene of stream %s, which is not running", name.c_str());
	}
}

void SceneStreamManager::changeScene(const Common::String &name, const Common::String &sceneName, uint16 frameID) {
	SceneStream *stream = name.empty() ? _running : find(name);
	if (stream) {
		stream->changeScene(sceneName, frameID);
	} else {
		warning("Tried to change the scene of stream %s, which is not running", name.c_str());
	}
}

Common::String SceneStreamManager::getSceneName(const Common::String &name) const {
	if (name.equalsIgnoreCase(kMainStreamName)) {
		return Common::String::format("s%u", NancySceneState.getMainSceneInfo().sceneID);
	}

	const SceneStream *stream = name.empty() ? _running : find(name);
	return stream ? stream->getSceneName() : Common::String();
}

void SceneStreamManager::process() {
	if (_streams.empty()) {
		NancySceneState.processMainStream();
		return;
	}

	// Streams begin and end while the records run
	const Common::Array<SceneStream *> streams = _streams;
	for (SceneStream *stream : streams) {
		if (!stream) {
			NancySceneState.processMainStream();
			continue;
		}

		_running = stream;
		if (!stream->update()) {
			// Takes the streams that depend on it along
			endChildren(stream->_children);
		}

		_running = nullptr;
	}

	for (uint i = 0; i < _streams.size(); ) {
		if (_streams[i] && _streams[i]->isEnded()) {
			delete _streams[i];
			_streams.remove_at(i);
		} else {
			++i;
		}
	}
}

// Returns whether a stream kept the input to itself
bool SceneStreamManager::handleInput(NancyInput &input, bool beforeMainStream) {
	const Common::Array<SceneStream *> streams = _streams;
	bool isBeforeMainStream = true;

	for (SceneStream *stream : streams) {
		if (!stream) {
			isBeforeMainStream = false;
			continue;
		}

		if (isBeforeMainStream != beforeMainStream) {
			continue;
		}

		_running = stream;
		stream->handleInput(input);
		_running = nullptr;

		if (stream->getFlags().capturesInput) {
			return true;
		}
	}

	return false;
}

void SceneStreamManager::clear() {
	for (SceneStream *stream : _streams) {
		delete stream;
	}

	_streams.clear();
	_mainStreamChildren.clear();
	_environment.clear();

	if (g_nancy->getGameType() >= kGameTypeNancy17) {
		// The main stream
		_streams.push_back(nullptr);
	}
}

// The streams that were started for the outgoing scene of the main stream end with it
void SceneStreamManager::onMainSceneChange() {
	for (uint i = 0; i < _mainStreamChildren.size(); ) {
		if (_mainStreamChildren[i].lifetime == StreamFlags::kEndsWithParentScene) {
			end(_mainStreamChildren[i].name);
			_mainStreamChildren.remove_at(i);
		} else {
			++i;
		}
	}
}

// Every environment has a script of its own, with its ambient sounds for example,
// which runs for as long as the main stream stays in it. It can have another one
// that is run once the main stream leaves it.
void SceneStreamManager::changeEnvironment(const Common::String &newEnvironment) {
	if (newEnvironment.equalsIgnoreCase(_environment)) {
		return;
	}

	const ENVS *envs = GetEngineData(ENVS);
	if (!envs) {
		return;
	}

	for (const ENVS::Environment &env : envs->environments) {
		if (env.code.equalsIgnoreCase(_environment)) {
			end(env.code + "_Enter_Stream");

			if (!env.leaveScriptName.empty()) {
				StreamFlags flags;
				flags.runsFirst = true;
				begin(env.code + "_Leave_Stream", env.leaveScriptName.toString(), 0, flags);
			}
		}
	}

	_environment = newEnvironment;

	for (const ENVS::Environment &env : envs->environments) {
		if (env.code.equalsIgnoreCase(newEnvironment) && !env.ambientScriptName.empty()) {
			StreamFlags flags;
			flags.runsFirst = true;
			begin(env.code + "_Enter_Stream", env.ambientScriptName.toString(), 0, flags);
		}
	}
}

bool SceneStreamManager::receiveMessage(const Script::Message &message, bool &wasTaken) {
	bool hasReceiver = false;
	wasTaken = false;

#ifdef ENABLE_NANCY_V4
	// A scene can end its stream when it gets a message
	const Common::Array<SceneStream *> streams = _streams;
	for (SceneStream *stream : streams) {
		if (wasTaken) {
			break;
		}

		if (!stream) {
			hasReceiver |= NancySceneState.receiveMainSceneMessage(message, wasTaken);
		} else if (!stream->isEnded()) {
			hasReceiver |= stream->receiveMessage(message, wasTaken);
		}
	}
#endif

	return hasReceiver;
}

// Only the scenes of the streams get saved; their records start over
void SceneStreamManager::store() {
	StreamData *data = (StreamData *)NancySceneState.getPuzzleData(StreamData::getTag());
	data->streams.clear();

	for (SceneStream *stream : _streams) {
		StreamData::Stream saved;

		if (stream) {
			if (stream->isEnded()) {
				continue;
			}

			stream->onSave();
			saved.name = stream->getName();
			saved.lifetime = stream->getFlags().lifetime;
			saved.capturesInput = stream->getFlags().capturesInput;
			saved.runsFirst = stream->getFlags().runsFirst;
			saved.zOffset = stream->getZOffset();
			saved.sceneName = stream->getSceneName();
			saved.frameID = stream->getSceneInfo().frameID;
			saved.children = stream->_children;
		} else {
			saved.isMainStream = true;
			saved.children = _mainStreamChildren;
		}

		data->streams.push_back(saved);
	}
}

void SceneStreamManager::restore() {
	const StreamData *data = (const StreamData *)NancySceneState.getPuzzleData(StreamData::getTag());

	for (const StreamData::Stream &saved : data->streams) {
		if (saved.isMainStream) {
			_mainStreamChildren = saved.children;
			continue;
		}

		// Loading the scene has already started some of them
		if (find(saved.name) || saved.sceneName.empty()) {
			continue;
		}

		StreamFlags flags;
		flags.lifetime = saved.lifetime;
		flags.capturesInput = saved.capturesInput;
		flags.runsFirst = saved.runsFirst;

		SceneStream *stream = new SceneStream(saved.name, flags, saved.zOffset);
		stream->changeScene(saved.sceneName, saved.frameID);
		stream->_children = saved.children;
		_streams.push_back(stream);
	}
}

} // End of namespace State
} // End of namespace Nancy
