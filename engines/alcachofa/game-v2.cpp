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
	ScriptKernelTask::SayTextOnlySound, // only used in secta/escarabajo with subtitles off (speaking with YAFAR)
	ScriptKernelTask::AnimateCharacter, // from here on only corvino
	ScriptKernelTask::AnimateTalking,
	ScriptKernelTask::ClearInventory,
	ScriptKernelTask::WaitForMouseClick,
	ScriptKernelTask::KillProcesses
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

	Point getObjectNameOffset() override {
		return Point(0, -50);
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

	String getSoundPath(const char *filename) override {
		return String("Sonidos/") + filename;
	}

	int32 getCharacterJingle(MainCharacterKind kind) override {
		return g_engine->script().variable(
			kind == MainCharacterKind::Mortadelo ? "PistaMorta" : "PistaFile");
	}

	bool shouldMusicLoop() override {
		// The credits cutscene is slightly longer than the
		// so we just disable looping and keep the last couple seconds quiet
		return !g_engine->player().currentRoom()->name().equalsIgnoreCase("CREDITOS");
	}

	bool hasMortadeloVoice(const Character *character) override {
		return Game::hasMortadeloVoice(character) ||
			character->name().equalsIgnoreCase("MORTA_ATADO");
	}

	bool shouldFilterTexturesByDefault() override {
		return true;
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

	String alternativeAnimationName(const String &path, int variant) override {
		U32String u32String = path.decode(Common::CodePage::kUtf8);
		bool hasChanges = false;
		if (variant == 0) {
			// SearchMan cannot find case-insensitive with non-ascii special characters
			// (e.g. "autógrafo" is not found by "AUTÓGRAFO")
			for (uint i = 0; i < u32String.size(); i++) {
				const auto ch = u32String[i];
				if (ch == 0xC1) {
					u32String[i] = 0xE1; // Á -> á
					hasChanges = true;
				} else if (ch == 0xD3) {
					u32String[i] = 0xF3; // Ó -> ó
					hasChanges = true;
				} else if (ch == 0xCD) {
					u32String[i] = 0xED; // Í -> í
					hasChanges = true;
				} else if (ch == 0xD1) {
					u32String[i] = 0xF1; // Ñ -> ñ
					hasChanges = true;
				} else if (ch == 0xC9) {
					u32String[i] = 0xE9; // É -> é
					hasChanges = true;
				}
			}
			return hasChanges ? u32String.encode() : String();
		} else if (variant == 1) {
			// Some of the Steam releases have wrong characters due to a messed up UTF8 conversion
			for (uint i = 0; i < u32String.size(); i++) {
				const auto ch = u32String[i];
				if (ch == 0xC1) { // Á -> ╡
					u32String[i] = 0x2561;
					hasChanges = true;
				} else if (ch == 0xD3) { // Ó -> α
					u32String[i] = 0x03B1;
					hasChanges = true;
				} else if (ch == 0xCD) { // Í -> ╓
					u32String[i] = 0x2553;
					hasChanges = true;
				}
			}
			return hasChanges ? u32String.encode() : String();
		} else
			return String();
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

	Path getVideoPath(int32 videoId) override {
		return Path(String::format("Bin/DATA%02d.BIN", videoId));
	}

	char getTextFileKey() override {
		return static_cast<char>(0xA3);
	}

	

	void missingAnimation(const String &filename) override {
		// missing background, not actually necessary as graphic objects cover the entire room
		if (filename.equalsIgnoreCase("HISTORIA_CLEOPATRA")) {
			// unrelated to the animation, if we query this we apparently entered the room
			// there is an original script bug where triggering this dialog twice
			// only the last graphic object is active blocking all others
			// we fix this by restoring the original state
			const Room *room = g_engine->player().currentRoom();
			ObjectBase *firstPicture = room->getObjectByName("OL Cleo1");
			ObjectBase *lastPicture = room->getObjectByName("OL Cleo10");
			if (firstPicture != nullptr && lastPicture != nullptr &&
				!firstPicture->isEnabled() && lastPicture->isEnabled()) {
				firstPicture->toggle(true);
				lastPicture->toggle(false);
			}
			return; // and we still ignore the missing background
		}
		GameWithVersion2::missingAnimation(filename);
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
static const char *const *const kMapFilesCorvino = kMapFilesSecta;

static constexpr const char *kMapFilesMoscu[] = {
	"Mapas/mapa1.emc",
	"Mapas/global.emc",
	nullptr
};
static const char *const *const kMapFilesBalones = kMapFilesMoscu;

static constexpr const char *kMapFilesEscarabajo[] = {
	"Mapas/mapa2.emc",
	"Mapas/global.emc",
	nullptr
};
static const char *const *const kMapFilesMamelucos = kMapFilesEscarabajo;

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
		const Room *room = g_engine->player().lastGameRoom();
		const char *dirName = room != nullptr && room->mapIndex() == 1 ? "Music_Cleopatra" : "Music";
		return String::format("%s/Track%02d", dirName, trackId); // 1-based indexing
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
		return String::format("track%d", trackId - 1); // 0-based indexing
	}

	bool isKnownBadVideo(int32 videoId) override {
		return videoId == 0; // MPEG-4 codec is unsupported
	}
};

class GameEscarabajo : public GameWithVersion2_0 {
public:
	void onLoadedGameFiles() override {
		g_engine->script().variable("EsJuegoCompleto") = 2;
	}

	const char *const *getMapFiles() override {
		return kMapFilesEscarabajo;
	}

	String getMusicPath(int32 trackId) override {
		return String::format("track%d", trackId - 1); // 0-based indexing
	}

	bool isKnownBadVideo(int32 videoId) override {
		return videoId == 0; // MPEG-4 codec is unsupported
	}
};

class GameWithVersion2_1 : public GameWithVersion2 {
public:
	void onLoadedGameFiles() override {
		GameWithVersion2::onLoadedGameFiles();

		auto &script = g_engine->script();
		script.fixNestedMenuPop(25096); // Filemon using phone in PRIMER_PLANO_TELEFONO
	}

	char getTextFileKey() override {
		return static_cast<char>(0x60);
	}

	String getMusicPath(int32 trackId) override {
		return String::format("Sonidos/T%d", trackId);
	}

	void missingAnimation(const Common::String &fileName) override {
		if (fileName == "VARITA.ANI") // this one seems bad, it is the inventory icon for I_RAMAS
			return;
		return GameWithVersion2::missingAnimation(fileName);
	}

	void missingSound(const Common::String &fileName) override {
		// seems like in the last level, they didnt test anymore? Many missing sounds in the same area
		if (fileName == "Sonidos/2147" ||
			fileName == "Sonidos/2148" ||
			fileName == "Sonidos/2149" ||
			fileName == "Sonidos/2150" ||
			fileName == "Sonidos/2151" ||
			fileName == "Sonidos/2159" ||
			fileName == "Sonidos/2160" ||
			fileName == "Sonidos/2161" ||
			fileName == "Sonidos/2162")
			return;
		GameWithVersion2::missingSound(fileName);
	}

	void unknownDropItem(const char *name) override {
		// an original bug, the item will just stay in the inventory unusable
		if (scumm_stricmp(name, "I_ELECTROCHOCK") == 0)
			return;
		return GameWithVersion2::unknownDropItem(name);
	}

	bool shouldTriggerDoor(const Door *door) override {
		// an original bug, a door left active to a non-existant target
		// probably the room layout was different during development
		if (door->name().equalsIgnoreCase("a_SALA_DE_TROFEOS_desde_ENFERMERIA"))
			return false;

		return GameWithVersion2::shouldTriggerDoor(door);
	}
};

class GameCorvino : public GameWithVersion2_1 {
public:
	void onLoadedGameFiles() override {
		GameWithVersion2_1::onLoadedGameFiles();

		g_engine->script().variable("EsJuegoCompleto") = 0;
	}

	const char *const *getMapFiles() override {
		return kMapFilesCorvino;
	}

	Path getVideoPath(int32 videoId) override {
		return Path(String::format("Data/DATA%02d.BIN", videoId));
	}

	bool isKnownBadVideo(int32 videoId) override {
		// These use DV codec in the steam release
		return videoId < 2;
	}
};

class GameBalones : public GameWithVersion2_1 {
public:
	void onLoadedGameFiles() override {
		GameWithVersion2_1::onLoadedGameFiles();

		g_engine->script().variable("EsJuegoCompleto") = 1;
	}

	const char *const *getMapFiles() override {
		return kMapFilesBalones;
	}

	Path getVideoPath(int32 videoId) override {
		return Path(String::format("Bin/DATA%02d.BIN", videoId));
	}

	void missingAnimation(const Common::String &filename) override {
		if (filename.equalsIgnoreCase("DOCE EUROS.ANI") ||
			filename.equalsIgnoreCase("ASPIRADORA_MANGUERA.ANI") ||
			filename.equalsIgnoreCase("ASPIRADORA.ANI") ||
			filename.equalsIgnoreCase("ASPIRADOR.ANI") ||
			filename.equalsIgnoreCase("CABEZA.ANI") ||
			filename.equalsIgnoreCase("CANICAS.ANI") ||
			filename.equalsIgnoreCase("HUEVO.ANI") ||
			filename.equalsIgnoreCase("NAVAJA.ANI") ||
			filename.equalsIgnoreCase("PALANCA.ANI") ||
			filename.equalsIgnoreCase("PELUCA MOJADA.ANI") ||
			filename.equalsIgnoreCase("PERA.ANI") ||
			filename.equalsIgnoreCase("PERA CON PITORRO.ANI") ||
			filename.equalsIgnoreCase("PILA.ANI") ||
			filename.equalsIgnoreCase("PINCEL.ANI") ||
			filename.equalsIgnoreCase("PINCEL CON COSA REPUGNANTE.ANI") ||
			filename.equalsIgnoreCase("PINCEL SIN CEL.ANI") ||
			filename.equalsIgnoreCase("PELO.ANI") ||
			filename.equalsIgnoreCase("PELO RESINA.ANI") ||
			filename.equalsIgnoreCase("PINTAU\xC3\x91""AS.ANI") ||
			filename.equalsIgnoreCase("TARJETA CRÉDITO.ANI") ||
			filename.equalsIgnoreCase("MANGUERA.ANI") ||
			filename.equalsIgnoreCase("VENTILADOR.ANI") ||
			filename.equalsIgnoreCase("BISO\xC3\x91\xC3\x89.ANI"))
			return;
		GameWithVersion2_1::missingAnimation(filename);
	}
};

class GameMamelucos : public GameWithVersion2_1 {
public:
	void onLoadedGameFiles() override {
		GameWithVersion2_1::onLoadedGameFiles();

		g_engine->script().variable("EsJuegoCompleto") = 2;
	}

	const char *const *getMapFiles() override {
		return kMapFilesMamelucos;
	}

	Path getVideoPath(int32 videoId) override {
		return Path(String::format("Bin/DATA%02d.BIN", videoId));
	}

	void missingAnimation(const String &filename) override {
		if (filename.equalsIgnoreCase("AUTÓGRAFO.ANI") ||
			filename.equalsIgnoreCase("GOMA3.ANI") ||
			filename.equalsIgnoreCase("IMAN.ANI") ||
			filename.equalsIgnoreCase("BOTELLA WHISKEY.ANI") ||
			filename.equalsIgnoreCase("VASO LLENO.ANI") ||
			filename.equalsIgnoreCase("VASO VACIO.ANI") ||
			filename.equalsIgnoreCase("PAÑUELO.ANI") ||
			filename.equalsIgnoreCase("PAÑUELO CON EXTRAÑAS MANCHAS.ANI") ||
			filename.equalsIgnoreCase("TABLÓN.ANI") ||
			filename.equalsIgnoreCase("TENAZAS.ANI") ||
			filename.equalsIgnoreCase("VASO QUITANERVIOS.ANI") ||
			filename.equalsIgnoreCase("PEGAMENTO.ANI") ||
			filename.equalsIgnoreCase("MEDIO EURO.ANI") ||
			filename.equalsIgnoreCase("BOTELLA LICOR.ANI") ||
			filename.equalsIgnoreCase("OSCAR TECNOLOGICO.ANI") ||
			filename.equalsIgnoreCase("COSA RARA CON BOMBILLA.ANI") ||
			filename.equalsIgnoreCase("CONTRATO SIN FIRMAR.ANI") ||
			filename.equalsIgnoreCase("CONTRATO FIRMADO.ANI") ||
			filename.equalsIgnoreCase("BOTELLA CHAMPAGNE.ANI") ||
			filename.equalsIgnoreCase("BOMBA.ANI") ||
			filename.equalsIgnoreCase("OTRA BOMBA.ANI") ||
			filename.equalsIgnoreCase("BOMBA CON RELOJ.ANI"))
			return;
		GameWithVersion2_1::missingAnimation(filename);
	}

	void missingSound(const String &filename) override {
		if (filename.equalsIgnoreCase("Sonidos/T2"))
			return;
		GameWithVersion2_1::missingSound(filename);
	}
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

Game *Game::createForCorvino() {
	return new GameCorvino();
}

Game *Game::createForBalones() {
	return new GameBalones();
}

Game *Game::createForMamelucos() {
	return new GameMamelucos();
}

}
