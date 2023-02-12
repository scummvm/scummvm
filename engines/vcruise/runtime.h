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

#ifndef VCRUISE_RUNTIME_H
#define VCRUISE_RUNTIME_H

#include "common/hashmap.h"

#include "vcruise/detection.h"

class OSystem;

namespace Graphics {

struct PixelFormat;
struct WinCursorGroup;
class ManagedSurface;

} // End of namespace Graphics

namespace Video {

class AVIDecoder;

} // End of namespace Video

namespace VCruise {

class AudioPlayer;
class TextParser;
struct ScriptSet;
struct Script;
struct Instruction;

enum GameState {
	kGameStateBoot,					// Booting the game
	kGameStateWaitingForAnimation,	// Waiting for a blocking animation to complete, then resuming script
	kGameStateQuit,					// Quitting
	kGameStateIdle,					// Waiting for input events
	kGameStateScript,				// Running a script
};

struct AnimationDef {
	AnimationDef();

	int animNum;	// May be negative if reversed
	uint firstFrame;
	uint lastFrame;	// Inclusive
};

struct RoomDef {
	Common::HashMap<Common::String, AnimationDef> animations;
	Common::HashMap<Common::String, Common::Rect> rects;
	Common::HashMap<Common::String, uint> vars;
	Common::HashMap<Common::String, int> values;
	Common::HashMap<Common::String, Common::String> texts;
	Common::HashMap<Common::String, int> consts;
	Common::HashMap<Common::String, int> sounds;

	Common::String name;
};

struct InteractionDef {
	InteractionDef();

	Common::Rect rect;
	uint16 interactionID;
	uint16 objectType;
};

struct MapScreenDirectionDef {
	Common::Array<InteractionDef> interactions;
};

struct MapDef {
	static const uint kNumScreens = 96;
	static const uint kNumDirections = 8;
	static const uint kFirstScreen = 0xa0;

	Common::SharedPtr<MapScreenDirectionDef> screenDirections[kNumScreens][kNumDirections];

	void clear();
	const MapScreenDirectionDef *getScreenDirection(uint screen, uint direction);
};

class Runtime {
public:
	Runtime(OSystem *system, Audio::Mixer *mixer, const Common::FSNode &rootFSNode, VCruiseGameID gameID);
	virtual ~Runtime();

	void initSections(Common::Rect gameRect, Common::Rect menuRect, Common::Rect trayRect, const Graphics::PixelFormat &pixFmt);

	void loadCursors(const char *exeName);

	bool runFrame();
	void drawFrame();

private:
	enum IndexParseType {
		kIndexParseTypeNone,
		kIndexParseTypeRoom,
		kIndexParseTypeRRoom,	// Rectangle room (constrains animation areas)
		kIndexParseTypeYRoom,	// Yes room (variable/ID mappings)
		kIndexParseTypeVRoom,	// Value room (value/ID mappings?)
		kIndexParseTypeTRoom,	// Text
		kIndexParseTypeCRoom,	// Const
		kIndexParseTypeSRoom,	// Sound
		kIndexParseTypeNameRoom,
	};

	enum AnimDecoderState {
		kAnimDecoderStateStopped,
		kAnimDecoderStatePlaying,
		kAnimDecoderStatePaused,
	};

	struct IndexPrefixTypePair {
		const char *prefix;
		IndexParseType parseType;
	};

	struct RenderSection {
		Common::SharedPtr<Graphics::ManagedSurface> surf;
		Common::Rect rect;

		void init(const Common::Rect &paramRect, const Graphics::PixelFormat &fmt);
	};

	typedef int32 ScriptArg_t;
	typedef int32 StackValue_t;

	bool bootGame();
	bool runIdle();
	bool runScript();
	bool runWaitForAnimation();
	void terminateScript();

	void loadIndex();
	void changeToScreen(uint roomNumber, uint screenNumber);
	void loadMap(Common::SeekableReadStream *stream);

	void changeMusicTrack(int musicID);
	void changeAnimation(const AnimationDef &animDef);

	AnimationDef stackArgsToAnimDef(const StackValue_t *args) const;

	void activateScript(const Common::SharedPtr<Script> &script);

	bool parseIndexDef(TextParser &parser, IndexParseType parseType, uint roomNumber, const Common::String &blamePath);
	void allocateRoomsUpTo(uint roomNumber);

	void scriptOpNumber(ScriptArg_t arg);
	void scriptOpRotate(ScriptArg_t arg);
	void scriptOpAngle(ScriptArg_t arg);
	void scriptOpAngleGGet(ScriptArg_t arg);
	void scriptOpSpeed(ScriptArg_t arg);
	void scriptOpSAnimL(ScriptArg_t arg);
	void scriptOpChangeL(ScriptArg_t arg);

	void scriptOpAnimR(ScriptArg_t arg);
	void scriptOpAnimF(ScriptArg_t arg);
	void scriptOpAnimN(ScriptArg_t arg);
	void scriptOpAnimG(ScriptArg_t arg);
	void scriptOpAnimS(ScriptArg_t arg);
	void scriptOpAnim(ScriptArg_t arg);

	void scriptOpStatic(ScriptArg_t arg);
	void scriptOpVarLoad(ScriptArg_t arg);
	void scriptOpVarStore(ScriptArg_t arg);
	void scriptOpSetCursor(ScriptArg_t arg);
	void scriptOpSetRoom(ScriptArg_t arg);
	void scriptOpLMB(ScriptArg_t arg);
	void scriptOpLMB1(ScriptArg_t arg);
	void scriptOpSoundS1(ScriptArg_t arg);
	void scriptOpSoundL2(ScriptArg_t arg);

	void scriptOpMusic(ScriptArg_t arg);
	void scriptOpMusicUp(ScriptArg_t arg);
	void scriptOpParm1(ScriptArg_t arg);
	void scriptOpParm2(ScriptArg_t arg);
	void scriptOpParm3(ScriptArg_t arg);
	void scriptOpParmG(ScriptArg_t arg);

	void scriptOpVolumeDn4(ScriptArg_t arg);
	void scriptOpVolumeUp3(ScriptArg_t arg);
	void scriptOpRandom(ScriptArg_t arg);
	void scriptOpDrop(ScriptArg_t arg);
	void scriptOpDup(ScriptArg_t arg);
	void scriptOpSay3(ScriptArg_t arg);
	void scriptOpSetTimer(ScriptArg_t arg);
	void scriptOpLoSet(ScriptArg_t arg);
	void scriptOpLoGet(ScriptArg_t arg);
	void scriptOpHiSet(ScriptArg_t arg);
	void scriptOpHiGet(ScriptArg_t arg);

	void scriptOpNot(ScriptArg_t arg);
	void scriptOpAnd(ScriptArg_t arg);
	void scriptOpOr(ScriptArg_t arg);
	void scriptOpCmpEq(ScriptArg_t arg);

	void scriptOpBitLoad(ScriptArg_t arg);
	void scriptOpBitSet0(ScriptArg_t arg);
	void scriptOpBitSet1(ScriptArg_t arg);

	void scriptOpDisc1(ScriptArg_t arg);
	void scriptOpDisc2(ScriptArg_t arg);
	void scriptOpDisc3(ScriptArg_t arg);

	void scriptOpEscOn(ScriptArg_t arg);
	void scriptOpEscOff(ScriptArg_t arg);
	void scriptOpEscGet(ScriptArg_t arg);
	void scriptOpBackStart(ScriptArg_t arg);

	void scriptOpAnimName(ScriptArg_t arg);
	void scriptOpValueName(ScriptArg_t arg);
	void scriptOpVarName(ScriptArg_t arg);
	void scriptOpSoundName(ScriptArg_t arg);
	void scriptOpCursorName(ScriptArg_t arg);

	void scriptOpCheckValue(ScriptArg_t arg);
	void scriptOpJump(ScriptArg_t arg);

	Common::Array<Common::SharedPtr<Graphics::WinCursorGroup> > _cursors;		// Cursors indexed as CURSOR_CUR_##
	Common::Array<Common::SharedPtr<Graphics::WinCursorGroup> > _cursorsShort;	// Cursors indexed as CURSOR_#

	OSystem *_system;
	uint _roomNumber;	// Room number can be changed independently of the loaded room, the screen doesn't change until a command changes it
	uint _screenNumber;
	uint _direction;

	uint _loadedRoomNumber;
	uint _activeScreenNumber;
	bool _havePendingScreenChange;
	GameState _gameState;

	bool _escOn;

	VCruiseGameID _gameID;

	Common::FSNode _rootFSNode;
	Common::FSNode _logDir;
	Common::FSNode _mapDir;
	Common::FSNode _sfxDir;
	Common::FSNode _animsDir;

	Common::Array<Common::SharedPtr<RoomDef> > _roomDefs;
	Common::SharedPtr<ScriptSet> _scriptSet;

	Common::SharedPtr<Script> _activeScript;
	uint _scriptNextInstruction;
	Common::Array<StackValue_t> _scriptStack;

	Common::SharedPtr<AudioPlayer> _musicPlayer;

	Common::SharedPtr<Video::AVIDecoder> _animDecoder;
	AnimDecoderState _animDecoderState;
	uint _animFrameNumber;
	uint _animLastFrame;
	uint _loadedAnimation;

	Audio::Mixer *_mixer;

	MapDef _map;

	RenderSection _gameSection;
	RenderSection _menuSection;
	RenderSection _traySection;

	static const uint kAnimDefStackArgs = 3;
};

} // End of namespace VCruise

#endif
