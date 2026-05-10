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

#include "gui/message.h"

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
	ScriptKernelTask::Nop,
	ScriptKernelTask::Disguise,
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
		return Point(g_system->getWidth() / 2, g_system->getHeight() - 200);
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

	int32 getCharacterJingle(MainCharacterKind kind) override {
		return g_engine->script().variable(
			kind == MainCharacterKind::Mortadelo ? "PistaMorta" : "PistaFile");
	}

	bool hasMortadeloVoice(const Character *character) override {
		return Game::hasMortadeloVoice(character) ||
			character->name().equalsIgnoreCase("MORTA_ATADO");
	}

	bool shouldFilterTexturesByDefault() override {
		return false;
	}

	bool shouldClipCamera() override {
		return true;
	}

	bool isAllowedToOpenMenu() override {
		return g_engine->sounds().musicSemaphore().isReleased();
	}

	bool isAllowedToInteract() override {
		return g_engine->player().semaphore().isReleased();
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

class GameWithVersion2_0 : public GameWithVersion2 {
public:
	void onLoadedGameFiles() override {
		GameWithVersion2::onLoadedGameFiles();

		auto &script = g_engine->script();
		script.fixNestedMenuPop(5921); // Mortadelo talking to ARQUEOLOGOS in CARRETERA
		script.fixNestedMenuPop(20898); // Filemon talking to MANOLO in FILE_PIRAMIDE
	}

	char getTextFileKey() override {
		return static_cast<char>(0xA3);
	}

	PointObject *unknownCamLerpTarget(const char *action, const char *name) override {
		// Original bug: a main character being reinterpret_cast to a PointObject, undefined behavior ensues
		if (scumm_stricmp(name, "FILEMON"))
			return Game::unknownCamLerpTarget(action, name);
		return nullptr;
	}
};

static constexpr const char *kMapFilesSecta[] = {
	"Mapas/mapa1.emc",
	"Mapas/mapa2.emc",
	"Mapas/global.emc",
	nullptr
};

static constexpr const char *kMapFilesMoscu[] = {
	"Mapas/mapa1.emc",
	"Mapas/global.emc",
	nullptr
};

static constexpr const char *kMapFilesEscarabajo[] = {
	"Mapas/mapa2.emc",
	"Mapas/global.emc",
	nullptr
};

class GameSecta : public GameWithVersion2_0 {
public:
	GameSecta() {
		// only the Steam Release has only the Videos in an ISO...
		if (!SearchMan.hasFile(getVideoPath(0)) && SearchMan.hasFile("VIDS.iso")) {
			_videosAreExtracted = false;
			GUI::MessageDialog dialog("Please extract VIDS.iso in order to play videos.");
			dialog.runModal();
		}
	}

	bool isKnownBadVideo(int32 videoId) override {
		// all videos are known bad if they are not extracted
		return !_videosAreExtracted;
	}

	void onLoadedGameFiles() override {
		g_engine->script().variable("EsJuegoCompleto") = 0;
	}

	const char *const *getMapFiles() override {
		return kMapFilesSecta;
	}

	String getMusicPath(int32 trackId) override {
		const Room *room = g_engine->player().currentRoom();
		const char *dirName = room != nullptr && room->mapIndex() == 1 ? "Music_Cleopatra" : "Music";
		return String::format("%s/Track%02d", dirName, trackId);
	}

private:
	bool _videosAreExtracted = true;
};

class GameMoscu : public GameWithVersion2_0 {
public:
	void onLoadedGameFiles() override {
		g_engine->script().variable("EsJuegoCompleto") = 1;
	}

	const char *const *getMapFiles() override {
		return kMapFilesMoscu;
	}

	String getMusicPath(int32 trackId) override {
		return String::format("track%d", trackId);
	}

	bool isKnownBadVideo(int32 videoId) override {
		return videoId == 0; // MPEG-4 codec is unsupported
	}
};

class GameEscarabajo : public GameWithVersion2_0 {
public:
	GameEscarabajo() {
		_hasMessedUpEncoding = !SearchMan.hasFile(Path(reencode("Animaciones/M\xC1SCARA MUSEO_RECEPCI\xD3N.ANI")));
	}

	void onLoadedGameFiles() override {
		g_engine->script().variable("EsJuegoCompleto") = 2;
	}

	const char *const *getMapFiles() override {
		return kMapFilesEscarabajo;
	}

	String getMusicPath(int32 trackId) override {
		return String::format("track%d", trackId);
	}

	bool isKnownBadVideo(int32 videoId) override {
		return videoId == 0; // MPEG-4 codec is unsupported
	}

	String reencodePath(const String &path) override {
		if (!_hasMessedUpEncoding)
			return Game::reencodePath(path);

		// The Steam release has wrong characters due to some messed up UTF8 conversion
		U32String u32String = path.decode(Common::CodePage::kISO8859_1);
		for (uint i = 0; i < u32String.size(); i++) {
			const auto ch = u32String[i];
			if (ch == 0xC1) // Á -> ╡
				u32String[i] = 0x2561;
			else if (ch == 0xD3) // Ó -> α
				u32String[i] = 0x03B1;
			else if (ch == 0xCD) // Í -> ╓
				u32String[i] = 0x2553;
		}
		return u32String.encode();
	}

private:
	bool _hasMessedUpEncoding = false;
};

Game *Game::createForSecta() {
	return new GameSecta();
}

Game *Game::createForMoscu() {
	return new GameMoscu();
}

Game *Game::createForEscarabajo() {
	return new GameEscarabajo();
}

}
