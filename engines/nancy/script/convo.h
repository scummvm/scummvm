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

#ifndef NANCY_SCRIPT_CONVO_H
#define NANCY_SCRIPT_CONVO_H

#include "common/array.h"
#include "common/str.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"

#include "engines/nancy/script/records.h"

#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace Script {

// Something the player can say in a conversation: the sound of it, whose name is
// also the key of its text, and the record that runs once it has been said.
// A response can come with a function that says whether it is on offer.
struct ConvoResponse {
	Common::String soundName;
	Record *onFinished = nullptr;
	int activeFunction = -1;	// in the registry of the scene's scripts
};

// A line of a conversation: a character says something, the player picks one of
// the responses on offer, says it, and the record that goes with the response
// runs, which as a rule moves on to the scene with the next line. A line without
// responses runs its default record when it is over.
//
// A video conversation shows the character with movies, one for each part of it
// (body, head). An XSheet with the name of the line's sound lists the parts, and
// the frame of each part's movie to show over time; a part without a frame number
// plays through by itself. A sound conversation plays its sounds one after another.
//
// Only one conversation runs in a scene: the ones that turn active afterwards are
// done at once. Its responses stay up until the scene goes away.
class ConvoRecord : public Record, public UI::NDUI::EngineTarget {
public:
	ConvoRecord(SceneContext &context, bool isVideo);
	virtual ~ConvoRecord();

	void updateGraphics() override;

	// Responses can be added while the scripts named by the record are run
	void addResponse(ConvoResponse *response) { _responses.push_back(response); }

	// The UI reports the response the player picked
	void onEngineAction(const UI::NDUI::Action &action) override;

protected:
	void readTable(int tableIndex) override;
	void runOnce() override;
	void run(double sceneTime, double elapsedTime) override;
	void onZChanged() override;

private:
	enum State {
		kStartLine,
		kWaitForLine,		// until the sound has started
		kPlayLine,
		kWaitForLineEnd,
		kStartResponse,
		kWaitForResponse,	// until the sound has started
		kPlayResponse
	};

	// One part of the character
	struct Tree {
		MoviePlayer player;
		RecordGraphic graphic;
		Common::Rect dest;
		Common::Array<int32> frames;	// the movie frame to show at each frame of the XSheet
		int32 shownFrame = -1;
	};

	static const int32 kFreeRunning = -1;

	bool loadXSheet(const Common::String &name);
	void showFrame(uint frame);
	void showMovieFrame(Tree &tree, int32 movieFrame);
	void drawTree(Tree &tree, const Graphics::Surface &frame);
	bool isTreeFreeRunning(const Tree &tree) const;
	bool isVideoDone() const;

	void playSound(const Common::String &name, uint16 channel);
	bool isResponseOnOffer(const ConvoResponse &response);
	void conclude();

	bool _isVideo;
	Common::Array<Common::String> _soundNames;
	Common::Array<Common::String> _responseFiles;
	Common::Array<ConvoResponse *> _responses;
	Record *_default = nullptr;

	Common::Array<Tree *> _trees;
	uint _numFrames = 0;
	uint32 _framesPerSecond = 0;
	uint _frame = 0;

	State _state = kStartLine;
	uint _curSound = 0;
	SoundDescription _sound;
	bool _hasSoundStarted = false;
	double _lineStartTime = 0.0;

	Common::Array<ConvoResponse *> _offeredResponses;
	int _pickedResponse = -1;
	bool _hasDialog = false;
};

void registerConvoAPI(lua_State *L);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_CONVO_H
