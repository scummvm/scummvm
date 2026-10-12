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

#ifndef NANCY_STATE_SCENESTREAM_H
#define NANCY_STATE_SCENESTREAM_H

#include "common/array.h"
#include "common/str.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/action/actionmanager.h"

namespace Common {
class SeekableReadStream;
}

namespace Nancy {

struct NancyInput;

namespace UI {
class Viewport;
}

namespace Script {
class SceneContext;
struct Message;
}

namespace State {

// What a stream gets started with
struct StreamFlags {
	enum Lifetime {
		kIndependent			= 0,
		kEndsWithParentScene	= 1,	// ...and with the parent itself
		kEndsWithParent			= 2
	};

	void read(Common::SeekableReadStream &stream);

	byte lifetime = kIndependent;
	bool capturesInput = false;	// the streams after this one get no input
	bool runsFirst = false;		// put in front of the running streams
	int32 zOffset = 0;			// added to the z order of everything the stream draws
};

// Nancy17 runs several scenes side by side, each in a stream of its own. The scene
// the player walks around in is the main stream, and is still handled by the Scene
// state itself; this is one of the others. Those play the ambient sounds of an
// environment, fill the journal, show a close-up or a phone call on top of the main
// scene, and so on. A scene change made by a stream's records only changes the scene
// of that stream.
class SceneStream {
public:
	typedef StreamData::Child Child;

	// The z offset is the stream's own on top of the one of the stream that started it
	SceneStream(const Common::String &name, const StreamFlags &flags, int32 zOffset);
	~SceneStream();

	// Runs the records of the stream's scene, and loads the next scene when one got
	// requested. Returns false once the stream is over.
	bool update();
	void handleInput(NancyInput &input);

	// Nancy19: the scripts of the scene store what they keep in a saved game
	void onSave();

	// Nancy19: hands a message to the scene. Returns false when it isn't for it.
	bool receiveMessage(const Script::Message &message, bool &wasTaken);

	void changeScene(const Common::String &sceneName, uint16 frameID);
	void end() { _isEnded = true; }
	bool isEnded() const { return _isEnded; }

	void setEventFlag(int16 index, byte flag);
	byte getEventFlag(int16 index) const;

	const Common::String &getName() const { return _name; }
	const StreamFlags &getFlags() const { return _flags; }
	int32 getZOffset() const { return _zOffset; }
	const Common::String &getSceneName() const { return _sceneName; }
	SceneChangeDescription &getSceneInfo() { return _sceneInfo; }
	Action::ActionManager &getActionManager() { return _actionManager; }

	Common::Array<Child> _children;

private:
	bool loadScene();
#ifdef ENABLE_NANCY_V4
	bool loadScriptedScene();	// Nancy19+
#endif
	void beginScene();
	void loadBackground(const Common::Path &videoFile);
	void clearScene();
	void processRecords();

	Common::String _name;
	StreamFlags _flags;
	int32 _zOffset;

	Common::String _sceneName;
	SceneChangeDescription _sceneInfo;
	Action::ActionManager _actionManager;
	UI::Viewport *_background = nullptr;

#ifdef ENABLE_NANCY_V4
	// Nancy19: the script of the scene, which takes the place of the action records
	Script::SceneContext *_scriptScene = nullptr;
#endif

	// Every stream has generic event flags of its own
	Common::Array<byte> _genericFlags;

	bool _hasNextScene = false;
	Common::String _nextSceneName;
	uint16 _nextFrameID = 0;

	bool _isEnded = false;
};

// The streams running next to the main one. Where a stream gets named, no name
// stands for the stream whose records are running.
class SceneStreamManager {
public:
	void begin(const Common::String &name, const Common::String &sceneName, uint16 frameID, const StreamFlags &flags);
	void end(const Common::String &name);
	void changeScene(const Common::String &name, const SceneChangeDescription &sceneDescription);
	void changeScene(const Common::String &name, const Common::String &sceneName, uint16 frameID);
	Common::String getSceneName(const Common::String &name) const;
	bool isRunning(const Common::String &name) const { return find(name) != nullptr; }

	// The stream whose records are running or getting input; null for the main one
	SceneStream *getRunning() const { return _running; }

	void process();
	bool handleInput(NancyInput &input, bool beforeMainStream);

	void clear();
	bool isEmpty() const { return _streams.empty(); }

	void onMainSceneChange();
	void changeEnvironment(const Common::String &newEnvironment);

	// Nancy19: hands a message to the scenes of the streams it is for. Returns
	// false when there is none.
	bool receiveMessage(const Script::Message &message, bool &wasTaken);

	// Keeps the streams in the saved game, and starts them again after loading
	void store();
	void restore();

private:
	SceneStream *find(const Common::String &name) const;
	void endChildren(const Common::Array<SceneStream::Child> &children);

	// Every stream in the order they run and get input in. The main stream is
	// the scene itself, and takes part as a null entry.
	Common::Array<SceneStream *> _streams;
	Common::Array<SceneStream::Child> _mainStreamChildren;
	SceneStream *_running = nullptr;
	Common::String _environment;
};

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_SCENESTREAM_H
