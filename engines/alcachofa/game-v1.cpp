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

#include "common/config-manager.h"

using namespace Common;

namespace Alcachofa {

static constexpr const ScriptOp kScriptOpMap[] = {
	ScriptOp::Nop,
	ScriptOp::Dup,
	ScriptOp::PushAddr,
	ScriptOp::PushValue,
	ScriptOp::Deref,
	ScriptOp::Nop,
	ScriptOp::Pop1,
	ScriptOp::Store,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::PushDynAddr,
	ScriptOp::Nop,
	ScriptOp::ScriptCall,
	ScriptOp::KernelCall,
	ScriptOp::JumpIfFalse,
	ScriptOp::JumpIfTrue,
	ScriptOp::Jump,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Add,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Equals,
	ScriptOp::NotEquals,
	ScriptOp::BitAnd,
	ScriptOp::BitOr,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::ReturnVoid
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
	ScriptKernelTask::Animate,
	ScriptKernelTask::HadNoMousePressFor,
	ScriptKernelTask::ChangeCharacter,
	ScriptKernelTask::LerpOrSetCam,
	ScriptKernelTask::Drop,
	ScriptKernelTask::ChangeDoor,
	ScriptKernelTask::Disguise,
	ScriptKernelTask::ToggleRoomFloor,
	ScriptKernelTask::SetDialogLineReturn,
	ScriptKernelTask::DialogMenu,
	ScriptKernelTask::ChangeCharacterRoom,
	ScriptKernelTask::PlayMusic,
	ScriptKernelTask::StopMusic,
	ScriptKernelTask::WaitForMusicToEnd
};

static constexpr const int kKernelTaskArgCounts[] = {
	0,
	2,
	3,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	1,
	2,
	2,
	0,
	1,
	2,
	1,
	1,
	1,
	0,
	1,
	0,
	2,
	1,
	0,
	0
};

static constexpr const char *kMapFilesMovieAdventure[] = {
	"oeste.emc",
	"terror.emc",
	"global.emc",
	nullptr
};

static constexpr const char *kMapFilesTerror[] = {
	"terror.emc",
	"global.emc",
	nullptr
};

static constexpr const char *kMapFilesVaqueros[] = {
	"oeste.emc",
	"global.emc",
	nullptr
};

class GameWithVersion1 : public Game {
public:
	Point getResolution() override {
		return Point(800, 600);
	}

	Point getThumbnailResolution() override {
		return Point(266, 200);
	}

	GameFileReference getScriptFileRef() override {
		// V1 embeds the script into global.emc, it is overridden during world load
		return {};
	}

	const char *getDialogFileName() override {
		return "TEXTOS.TXT";
	}

	const char *getObjectFileName() override {
		return "OBJETOS.TXT";
	}

	char getTextFileKey() override {
		return kNoXORKey;
	}

	Point getSubtitlePos() override {
		return Point(g_system->getWidth() / 2, 150);
	}

	const char *getMenuRoom() override {
		return "MENU";
	}

	const char *getInitScriptName() override {
		return "INICIALIZAR_MUNDO";
	}

	Span<const ScriptOp> getScriptOpMap() override {
		return { kScriptOpMap, ARRAYSIZE(kScriptOpMap) };
	}

	Span<const ScriptKernelTask> getScriptKernelTaskMap() override {
		return { kScriptKernelTaskMap, ARRAYSIZE(kScriptKernelTaskMap) };
	}

	int32 getKernelTaskArgCount(int32 taskI) override {
		assert(taskI >= 0 && taskI < ARRAYSIZE(kKernelTaskArgCounts));
		return kKernelTaskArgCounts[taskI];
	}

	void updateScriptVariables() override {
		g_engine->script().variable("EstanAmbos") =
			g_engine->world().mortadelo().room() == g_engine->world().filemon().room();
	}

	String getSoundPath(const char *filename) override {
		return filename;
	}

	int32 getCharacterJingle(MainCharacterKind kind) override {
		return kind == MainCharacterKind::Mortadelo ? 15 : 16;
	}

	bool shouldFilterTexturesByDefault() override {
		return false;
	}

	bool shouldClipCamera() override {
		return true;
	}

	bool isAllowedToOpenMenu() override {
		return g_engine->player().semaphore().isReleased() &&
			dynamic_cast<RoomWithFloor *>(g_engine->player().currentRoom()) != nullptr;
	}

	bool isAllowedToInteract() override {
		return true; // original would be checking an unused script variable "Ocupados"
	}

	bool shouldScriptLockInteraction() override {
		return true;
	}

	bool shouldChangeCharacterUseGameLock() override {
		return false;
	}

	bool shouldAvoidCollisions() override {
		return false;
	}

	Point getMainCharacterSize() override {
		return { 40, 220 };
	}

	Character *unknownSayTextCharacter(const char *name, int32 dialogId) override {
		// an original bug in room POBLADO_INDIO, a dialog line would be skipped
		if (!scumm_stricmp(name, "JEFE_INDIO_HABLA_POSTE"))
			return dynamic_cast<Character *>(g_engine->player().currentRoom()->getObjectByName("JEFE_HABLA_POSTE"));
		return Game::unknownSayTextCharacter(name, dialogId);
	}

	void missingAnimation(const String &fileName) override {
		static const char *exemptions[] = {
			nullptr
		};

		const auto isInExemptions = [&] (const char *const *const list) {
			for (const char *const *exemption = list; *exemption != nullptr; exemption++) {
				if (fileName.equalsIgnoreCase(*exemption))
					return true;
			}
			return false;
		};

		if (isInExemptions(exemptions))
			debugC(1, kDebugGraphics, "Animation exemption triggered: %s", fileName.c_str());
		else
			Game::missingAnimation(fileName);
	}

	void onUserChangedCharacter() override {
		// An original bug in room POBLADO_INDIO if filemon is bound and mortadelo enters the room
		// the door A_PUENTE which was disabled is reenabled to allow mortadelo leaving
		// However if the user now changes character, the door is still enabled and filemon can
		// enter a ghost state walking through a couple rooms and softlocking.
		if (g_engine->player().currentRoom()->name().equalsIgnoreCase("POBLADO_INDIO"))
			g_engine->script().createProcess(g_engine->player().activeCharacterKind(), "ENTRAR_POBLADO_INDIO");
	}

	PointObject *getPointFromRoom(const char *roomName, const char *objectName, const char *action) {
		const auto *room = g_engine->world().getRoomByName(roomName);
		if (room == nullptr) {
			_message("Could not find room %s to get %s for %s");
			return nullptr;
		}
		auto *object = room->getObjectByName(objectName);
		if (object == nullptr) {
			_message("Could not find object %s in room %s for %s");
			return nullptr;
		}
		auto *point = dynamic_cast<PointObject *>(object);
		if (point == nullptr) {
			_message("Object %s in room %s for %s is not a point object");
			return nullptr;
		}
		return point;
	}

	PointObject *unknownGoPutTarget(const Process &process, const char *action, const char *name) override {
		if (!*name) // this can happen when a variable address is misused as string
			return nullptr;

		if (scumm_stricmp(action, "put"))
			return Game::unknownGoPutTarget(process, action, name);

		if (!scumm_stricmp("A_Poblado_Indio", name)) {
			// A_Poblado_Indio is a Door but is originally cast into a PointObject
			// a pointer and the draw order is then interpreted as position and the character snapped onto the floor shape.
			// Instead I just use the A_Poblado_Indio1 object which exists as counter-part for A_Poblado_Indio2 which should have been used
			auto target = dynamic_cast<PointObject *>(
				g_engine->world().getObjectByName(process.character(), "A_Poblado_Indio1"));
			if (target == nullptr)
				_message("Unknown put target A_Poblado_Indio1 during exemption for A_Poblado_Indio");
			return target;
		}

		if (!scumm_stricmp("Puerta_Casa_Freddy_Intermedia", name)) {
			// Another case of a door being cast into a PointObject
			return nullptr;
		}

		// an original bug, Pos_Final_Morta/File is defined in room ENTRADA_PUEBLO or PANTANO_EXT
		// but the current room is ENTRADA_PUEBLO_INTRO
		if (!scumm_stricmp(name, "Pos_Final_Morta") || !scumm_stricmp(name, "Pos_Final_File")) {
			// for terror there is no ENTRADA_PUEBLO either, nor Pos_Final_*
			if (g_engine->world().getRoomByName("ENTRADA_PUEBLO") == nullptr) {
				return !scumm_stricmp(name, "Pos_Final_Morta")
					? getPointFromRoom("PANTANO_EXT", "PANTANO_MORTA", action)
					: getPointFromRoom("PANTANO_EXT", "PANTANO_FILE", action);
			} else
				return getPointFromRoom("ENTRADA_PUEBLO", name, action);
		}

		return Game::unknownGoPutTarget(process, action, name);
	}

	PointObject *unknownCamLerpTarget(const char *action, const char *name) override {
		// the same bug as in unknownGoPutTarget
		if (!scumm_stricmp(name, "Pos_Final_Morta"))
			return getPointFromRoom("ENTRADA_PUEBLO", "Pos_Final_Morta", action);

		return Game::unknownCamLerpTarget(action, name);
	}

	void missingSound(const Common::String &fileName) override {
		if (fileName == "CHAS" ||
			fileName == "0563" ||
			fileName == "M2137" ||
			fileName == "1413" || // are stored in OESTE.EMC but played during terror outro
			fileName == "M1414")
			return;
		return Game::missingSound(fileName);
	}
};

class GameMovieAdventureOriginal : public GameWithVersion1 {
public:
	GameMovieAdventureOriginal() {
		const auto &desc = g_engine->gameDescription();
		if (desc.desc.flags & ADGF_CD) {
			const Path gameDir = ConfMan.getPath("path");
			SearchMan.addDirectory(gameDir.append("disk1/Install"));
			SearchMan.addDirectory(gameDir.append("disk2/Install"));
		}
	}

	const char *const *getMapFiles() override {
		return kMapFilesMovieAdventure;
	}

	Path getVideoPath(int32 videoId) override {
		return Path(String::format("disk1/Install/bin/data%02d.bin", videoId));
	}

	String getMusicPath(int32 trackId) override {
		const Room *room = g_engine->player().currentRoom();
		const int diskId = room != nullptr && room->mapIndex() == 1 ? 2 : 1;
		return String::format("disk%d/track%02d", diskId, trackId);
	}

	void onLoadedGameFiles() override {
		g_engine->script().variable("EsJuegoCompleto") = 0;
	}
};

class GameHalfMovieAdventure : public GameWithVersion1 {
public:
	Path getVideoPath(int32 videoId) override {
		return Path(String::format("bin/data%02d.bin", videoId));
	}

	String getMusicPath(int32 trackId) override {
		return String::format("track%02d", trackId);
	}

	// probably the original CDs have music, the Steam release has no music...
	void missingSound(const Common::String &fileName) override {
		if (fileName.contains("track")) {
			if (!_warnedAboutMusic) {
				_warnedAboutMusic = true;
				warning("This release does not contain music or the music was not extracted.");
			}
		} else
			GameWithVersion1::missingSound(fileName);
	}

private:
	bool _warnedAboutMusic = false;
};

class GameVaqueros : public GameHalfMovieAdventure {
public:
	const char *const *getMapFiles() override {
		return kMapFilesVaqueros;
	}

	void onLoadedGameFiles() override {
		g_engine->script().variable("EsJuegoCompleto") = 1;
	}
};


class GameTerror : public GameHalfMovieAdventure {
public:
	const char *const *getMapFiles() override {
		return kMapFilesTerror;
	}

	void onLoadedGameFiles() override {
		g_engine->script().variable("EsJuegoCompleto") = 2;
	}
};

Game *Game::createForMovieAdventureOriginal() {
	return new GameMovieAdventureOriginal();
}

Game *Game::createForVaqueros() {
	return new GameVaqueros();
}

Game *Game::createForTerror() {
	return new GameTerror();
}

}
