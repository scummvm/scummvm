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

#include "alcachofa/alcachofa.h"
#include "alcachofa/game.h"
#include "alcachofa/script.h"

using namespace Common;

namespace Alcachofa {

static constexpr const ScriptOp kScriptOpMap[] = {
	ScriptOp::Nop,
	ScriptOp::Dup,
	ScriptOp::PushAddr,
	ScriptOp::PushValue,
	ScriptOp::Deref,
	ScriptOp::Crash, ///< would crash original engine by writing to read-only memory
	ScriptOp::PopN,
	ScriptOp::Store,
	ScriptOp::Crash,
	ScriptOp::Crash,
	ScriptOp::LoadString,
	ScriptOp::LoadString, ///< exactly the same as LoadString
	ScriptOp::Crash,
	ScriptOp::ScriptCall,
	ScriptOp::KernelCall,
	ScriptOp::JumpIfFalse,
	ScriptOp::JumpIfTrue,
	ScriptOp::Jump,
	ScriptOp::Negate,
	ScriptOp::BooleanNot,
	ScriptOp::Mul,
	ScriptOp::Crash,
	ScriptOp::Crash,
	ScriptOp::Add,
	ScriptOp::Sub,
	ScriptOp::Less,
	ScriptOp::Greater,
	ScriptOp::LessEquals,
	ScriptOp::GreaterEquals,
	ScriptOp::Equals,
	ScriptOp::NotEquals,
	ScriptOp::BitAnd,
	ScriptOp::BitOr,
	ScriptOp::Crash,
	ScriptOp::Crash,
	ScriptOp::Crash,
	ScriptOp::Crash,
	ScriptOp::ReturnValue
};

static constexpr const ScriptKernelTask kScriptKernelTaskMap[] = {
	ScriptKernelTask::Nop,
	ScriptKernelTask::SayText,
	ScriptKernelTask::Go,
	ScriptKernelTask::Delay,
	ScriptKernelTask::PlaySound,
	ScriptKernelTask::FadeIn,
	ScriptKernelTask::FadeOut,
	ScriptKernelTask::Put,
	ScriptKernelTask::ChangeRoom,
	ScriptKernelTask::PlayVideo,
	ScriptKernelTask::StopAndTurn,
	ScriptKernelTask::StopAndTurnMe,
	ScriptKernelTask::On,
	ScriptKernelTask::Off,
	ScriptKernelTask::Pickup,
	ScriptKernelTask::CharacterPickup,
	ScriptKernelTask::Animate,
	ScriptKernelTask::HadNoMousePressFor,
	ScriptKernelTask::ChangeCharacter,
	ScriptKernelTask::LerpOrSetCam,
	ScriptKernelTask::Drop,
	ScriptKernelTask::CharacterDrop,
	ScriptKernelTask::ChangeDoor,
	ScriptKernelTask::CamShake,
	ScriptKernelTask::ToggleRoomFloor,
	ScriptKernelTask::SetDialogLineReturn,
	ScriptKernelTask::DialogMenu,
	ScriptKernelTask::ChangeCharacterRoom,
	ScriptKernelTask::PlayMusic,
	ScriptKernelTask::StopMusic,
	ScriptKernelTask::WaitForMusicToEnd,
	ScriptKernelTask::SayTextV2
};

class GameWithVersion2 : public Game {
public:
	Point getResolution() override {
		return Point(800, 600);
	}

	Point getThumbnailResolution() override {
		return Point(266, 200); // TODO: Check this resolution value
	}

	GameFileReference getScriptFileRef() override {
		return GameFileReference("Script/MORTADELO.COD");
	}

	const char *getDialogFileName() override {
		return "Fondos/MUSEO_F.ANI";
	}

	const char *getObjectFileName() override {
		return "Fondos/MUSEO_O.ANI";
	}
	
	Point getSubtitlePos() override {
		return Point(g_system->getWidth() / 2, 150); // TODO: Check subtitle position
	}

	const char *getMenuRoom() override {
		return "MENUPRINCIPAL";
	}

	const char *getInitScriptName() override {
		return "main";
	}

	Span<const ScriptOp> getScriptOpMap() override {
		return { kScriptOpMap, ARRAYSIZE(kScriptOpMap) };
	}

	Span<const ScriptKernelTask> getScriptKernelTaskMap() override {
		return { kScriptKernelTaskMap, ARRAYSIZE(kScriptKernelTaskMap) };
	}

	void updateScriptVariables() override {
		Script &script = g_engine->script();
		script.variable("EstanAmbos") = g_engine->world().mortadelo().room() == g_engine->world().filemon().room();
		script.variable("textoson") = g_engine->config().subtitles() ? 1 : 0;
	}

	Path getVideoPath(int32 videoId) override {
		return Path(String::format("Bin/DATA%02d.BIN", videoId));
	}

	String getSoundPath(const char *filename) override {
		return String("Sonidos/") + filename;
	}

	String getMusicPath(int32 trackId) override {
		return String::format("Music/Track%02d", trackId);
	}

	int32 getCharacterJingle(MainCharacterKind kind) override {
		return g_engine->script().variable(
			kind == MainCharacterKind::Mortadelo ? "PistaMorta" : "PistaFile");
	}

	bool shouldFilterTexturesByDefault() override {
		return true; // TODO: Check this!
	}

	bool shouldClipCamera() override {
		return true;
	}

	bool isAllowedToOpenMenu() override {
		return g_engine->sounds().musicSemaphore().isReleased();
	}

	bool isAllowedToInteract() override {
		return true; // original would be checking an unused script variable "Ocupados"
	}

	bool shouldScriptLockInteraction() override {
		return false;
	}

	bool shouldChangeCharacterUseGameLock() override {
		return false;
	}

	bool shouldAvoidCollisions() override {
		return true;
	}

	Point getMainCharacterSize() override {
		return { 40, 220 };
	}

	bool doesRoomHaveBackground(const Room *room) override {
		return !room->name().equalsIgnoreCase("Global");
	}
};

static constexpr const char *kMapFilesSecta[] = {
	"Mapas/mapa1.emc",
	"Mapas/mapa2.emc",
	"Mapas/global.emc",
	nullptr
};

class GameSecta : public GameWithVersion2 {
public:
	const char *const *getMapFiles() override {
		return kMapFilesSecta;
	}

	char getTextFileKey() override {
		return static_cast<char>(0xA3);
	}
};

Game *Game::createForSecta() {
	return new GameSecta();
}

}
