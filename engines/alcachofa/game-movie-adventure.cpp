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

static constexpr const ScriptKernelTask kScriptKernelTaskMapV30[] = {
	ScriptKernelTask::Nop,
	ScriptKernelTask::PlayVideo,
	ScriptKernelTask::PlaySound,
	ScriptKernelTask::PlayMusic,
	ScriptKernelTask::StopMusic,
	ScriptKernelTask::WaitForMusicToEnd,
	ScriptKernelTask::ShowCenterBottomText,
	ScriptKernelTask::StopAndTurn,
	ScriptKernelTask::StopAndTurnMe,
	ScriptKernelTask::ChangeCharacter,
	ScriptKernelTask::SayText,
	ScriptKernelTask::Nop,
	ScriptKernelTask::Go,
	ScriptKernelTask::Put,
	ScriptKernelTask::ChangeCharacterRoom,
	ScriptKernelTask::KillProcesses,
	ScriptKernelTask::On,
	ScriptKernelTask::Off,
	ScriptKernelTask::Pickup,
	ScriptKernelTask::CharacterPickup,
	ScriptKernelTask::Drop,
	ScriptKernelTask::CharacterDrop,
	ScriptKernelTask::Delay,
	ScriptKernelTask::HadNoMousePressFor,
	ScriptKernelTask::Nop,
	ScriptKernelTask::Fork,
	ScriptKernelTask::Animate,
	ScriptKernelTask::AnimateCharacter,
	ScriptKernelTask::AnimateTalking,
	ScriptKernelTask::ChangeRoom,
	ScriptKernelTask::ToggleRoomFloor,
	ScriptKernelTask::SetDialogLineReturn,
	ScriptKernelTask::DialogMenu,
	ScriptKernelTask::ClearInventory,
	ScriptKernelTask::Nop,
	ScriptKernelTask::FadeType0,
	ScriptKernelTask::FadeType1,
	ScriptKernelTask::LerpWorldLodBias,
	ScriptKernelTask::FadeType2,
	ScriptKernelTask::SetActiveTextureSet,
	ScriptKernelTask::SetMaxCamSpeedFactor,
	ScriptKernelTask::WaitCamStopping,
	ScriptKernelTask::CamFollow,
	ScriptKernelTask::CamShake,
	ScriptKernelTask::LerpCamXY,
	ScriptKernelTask::LerpCamZ,
	ScriptKernelTask::LerpCamScale,
	ScriptKernelTask::LerpCamToObjectWithScale,
	ScriptKernelTask::LerpCamToObjectResettingZ,
	ScriptKernelTask::LerpCamRotation,
	ScriptKernelTask::FadeIn,
	ScriptKernelTask::FadeOut,
	ScriptKernelTask::FadeIn2,
	ScriptKernelTask::FadeOut2,
	ScriptKernelTask::LerpCamToObjectKeepingZ
};

// in V3.1 there is the LerpCharacterLodBias and LerpCamXYZ tasks, no other differences

static constexpr const ScriptKernelTask kScriptKernelTaskMapV31[] = {
	ScriptKernelTask::Nop,
	ScriptKernelTask::PlayVideo,
	ScriptKernelTask::PlaySound,
	ScriptKernelTask::PlayMusic,
	ScriptKernelTask::StopMusic,
	ScriptKernelTask::WaitForMusicToEnd,
	ScriptKernelTask::ShowCenterBottomText,
	ScriptKernelTask::StopAndTurn,
	ScriptKernelTask::StopAndTurnMe,
	ScriptKernelTask::ChangeCharacter,
	ScriptKernelTask::SayText,
	ScriptKernelTask::Nop,
	ScriptKernelTask::Go,
	ScriptKernelTask::Put,
	ScriptKernelTask::ChangeCharacterRoom,
	ScriptKernelTask::KillProcesses,
	ScriptKernelTask::LerpCharacterLodBias,
	ScriptKernelTask::On,
	ScriptKernelTask::Off,
	ScriptKernelTask::Pickup,
	ScriptKernelTask::CharacterPickup,
	ScriptKernelTask::Drop,
	ScriptKernelTask::CharacterDrop,
	ScriptKernelTask::Delay,
	ScriptKernelTask::HadNoMousePressFor,
	ScriptKernelTask::Nop,
	ScriptKernelTask::Fork,
	ScriptKernelTask::Animate,
	ScriptKernelTask::AnimateCharacter,
	ScriptKernelTask::AnimateTalking,
	ScriptKernelTask::ChangeRoom,
	ScriptKernelTask::ToggleRoomFloor,
	ScriptKernelTask::SetDialogLineReturn,
	ScriptKernelTask::DialogMenu,
	ScriptKernelTask::ClearInventory,
	ScriptKernelTask::Nop,
	ScriptKernelTask::FadeType0,
	ScriptKernelTask::FadeType1,
	ScriptKernelTask::LerpWorldLodBias,
	ScriptKernelTask::FadeType2,
	ScriptKernelTask::SetActiveTextureSet,
	ScriptKernelTask::SetMaxCamSpeedFactor,
	ScriptKernelTask::WaitCamStopping,
	ScriptKernelTask::CamFollow,
	ScriptKernelTask::CamShake,
	ScriptKernelTask::LerpCamXY,
	ScriptKernelTask::LerpCamZ,
	ScriptKernelTask::LerpCamScale,
	ScriptKernelTask::LerpCamToObjectWithScale,
	ScriptKernelTask::LerpCamToObjectResettingZ,
	ScriptKernelTask::LerpCamRotation,
	ScriptKernelTask::FadeIn,
	ScriptKernelTask::FadeOut,
	ScriptKernelTask::FadeIn2,
	ScriptKernelTask::FadeOut2,
	ScriptKernelTask::LerpCamXYZ,
	ScriptKernelTask::LerpCamToObjectKeepingZ
};

static constexpr const char *kMapFiles[] = { // not really inherent to V3 but holds true for all V3 games
	"MAPAS/MAPA5.EMC",
	"MAPAS/MAPA4.EMC",
	"MAPAS/MAPA3.EMC",
	"MAPAS/MAPA2.EMC",
	"MAPAS/MAPA1.EMC",
	"MAPAS/GLOBAL.EMC",
	nullptr
};

class GameWithVersion3 : public Game {
public:
	Point getResolution() override {
		return Point(1024, 768);
	}

	const char *const *getMapFiles() override {
		return kMapFiles;
	}

	Span<const ScriptOp> getScriptOpMap() override {
		return { kScriptOpMap, ARRAYSIZE(kScriptOpMap) };
	}

	void updateScriptVariables() override {
		Script &script = g_engine->script();
		if (g_engine->input().wasAnyMousePressed()) // yes, this variable is never reset by the engine (only by script)
			script.variable("SeHaPulsadoRaton") = 1;

		script.variable("EstanAmbos") = g_engine->world().mortadelo().room() == g_engine->world().filemon().room();
		script.variable("textoson") = g_engine->config().subtitles() ? 1 : 0;
	}

	void onLoadedGameFiles() override {
		// this notifies the script whether we are a demo
		if (g_engine->world().loadedMapCount() == 2)
			g_engine->script().variable("EsJuegoCompleto") = 2;
		else if (g_engine->world().loadedMapCount() == 3) // I don't know this demo
			g_engine->script().variable("EsJuegoCompleto") = 1;
	}

	bool doesRoomHaveBackground(const Room *room) override {
		return !room->name().equalsIgnoreCase("Global") &&
			!room->name().equalsIgnoreCase("HABITACION_NEGRA");
	}

	bool shouldCharacterTrigger(const Character *character, const char *action) override {
		// An original hack to check that bed sheet is used on the other main character only in the correct room
		// There *is* another script variable (es_casa_freddy) that should check this
		// but, I guess, Alcachofa Soft found a corner case where this does not work?
		if (scumm_stricmp(action, "iSABANA") == 0 &&
			dynamic_cast<const MainCharacter *>(character) != nullptr &&
			!character->room()->name().equalsIgnoreCase("CASA_FREDDY_ARRIBA")) {
			return false;
		}

		return Game::shouldCharacterTrigger(character, action);
	}

	void onUserChangedCharacter() override {
		// An original bug in room POBLADO_INDIO if filemon is bound and mortadelo enters the room
		// the door A_PUENTE which was disabled is reenabled to allow mortadelo leaving
		// However if the user now changes character, the door is still enabled and filemon can
		// enter a ghost state walking through a couple rooms and softlocking.
		if (g_engine->player().currentRoom()->name().equalsIgnoreCase("POBLADO_INDIO"))
			g_engine->script().createProcess(g_engine->player().activeCharacterKind(), "ENTRAR_POBLADO_INDIO");
	}

	bool hasMortadeloVoice(const Character *character) override {
		return Game::hasMortadeloVoice(character) ||
			character->name().equalsIgnoreCase("MORTADELO_TREN"); // an original hard-coded special case
	}

	PointObject *unknownGoPutTarget(const Process &process, const char *action, const char *name) override {
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

		if (!scumm_stricmp("PUNTO_VENTANA", name)) {
			// The object is in the previous, now inactive room.
			// Luckily Mortadelo already is at that point so not further action required
			return nullptr;
		}

		if (!scumm_stricmp("Puerta_Casa_Freddy_Intermedia", name)) {
			// Another case of a door being cast into a PointObject
			return nullptr;
		}

		return Game::unknownGoPutTarget(process, action, name);
	}

	void unknownAnimateCharacterObject(const char *name) override {
		if (!scumm_stricmp(name, "COGE F DCH") || // original bug in MOTEL_ENTRADA
			!scumm_stricmp(name, "CHIQUITO_IZQ"))
			return;
		Game::unknownAnimateCharacterObject(name);
	}

	void missingSound(const String &fileName) override {
		if (fileName == "CHAS" || fileName == "517")
			return;
		Game::missingSound(fileName);
	}
};

class GameMovieAdventureSpecialV30 : public GameWithVersion3 {
public:
	Span<const ScriptKernelTask> getScriptKernelTaskMap() override {
		return { kScriptKernelTaskMapV30, ARRAYSIZE(kScriptKernelTaskMapV30) };
	}

	void updateScriptVariables() override {
		GameWithVersion3::updateScriptVariables();

		// in V3.0 there is no CalcularTiempoSinPulsarRaton variable to reset the timer
		g_engine->script().setScriptTimer(g_engine->input().wasAnyMousePressed());
	}

	bool shouldClipCamera() override {
		return true;
	}

	void missingAnimation(const String &fileName) override {
		static const char *exemptions[] = {
			"ANIMACION.AN0",
			"PP_MORTA.AN0",
			"ESTOMAGO.AN0",
			"CREDITOS.AN0",
			"HABITACION NEGRA.AN0",
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
};

class GameMovieAdventureSpecialV31 : public GameWithVersion3 {
public:
	Span<const ScriptKernelTask> getScriptKernelTaskMap() override {
		return { kScriptKernelTaskMapV31, ARRAYSIZE(kScriptKernelTaskMapV31) };
	}

	void updateScriptVariables() override {
		GameWithVersion3::updateScriptVariables();

		Script &script = g_engine->script();
		script.setScriptTimer(!script.variable("CalcularTiempoSinPulsarRaton"));
		script.variable("modored") = 0; // this is signalling whether a network connection is established
	}

	bool shouldClipCamera() override {
		return g_engine->script().variable("EncuadrarCamara") != 0;
	}

	void drawScreenStates() override {
		if (int32 borderWidth = g_engine->script().variable("BordesNegros")) {
			int16 width = g_system->getWidth();
			int16 height = g_system->getHeight();
			g_engine->drawQueue().add<BorderDrawRequest>(Rect(0, 0, width, borderWidth), kBlack);
			g_engine->drawQueue().add<BorderDrawRequest>(Rect(0, height - borderWidth, width, height), kBlack);
		}
	}

	bool shouldTriggerDoor(const Door *door) override {
		// An invalid door target, the character will go to the door and then ignore it (also in original engine)
		// this is a bug introduced in V3.1
		if (door->targetRoom() == "LABERINTO" && door->targetObject() == "a_LABERINTO_desde_LABERINTO_2")
			return false;
		return Game::shouldTriggerDoor(door);
	}

	void missingAnimation(const String &fileName) override {
		static const char *exemptions[] = {
			"ANIMACION.AN0",
			"DESPACHO_SUPER2_OL_SOMBRAS2.AN0",
			"PP_MORTA.AN0",
			"DESPACHO_SUPER2___FONDO_PP_SUPER.AN0",
			"ESTOMAGO.AN0",
			"CREDITOS.AN0",
			"MONITOR___OL_EFECTO_FONDO.AN0",
			nullptr
		};

		// these only happen in the german demo
		static const char *demoExemptions[] = {
			"TROZO_1.AN0",
			"TROZO_2.AN0",
			"TROZO_3.AN0",
			"TROZO_4.AN0",
			"TROZO_5.AN0",
			"TROZO_6.AN0",
			"NOTA_CINE_NEGRO.AN0",
			"PP_JOHN_WAYNE_2.AN0",
			"ARQUEOLOGO_ESTATICO_TIA.AN0",
			"ARQUEOLOGO_HABLANDO_TIA.AN0",
			nullptr
		};

		const auto isInExemptions = [&] (const char *const *const list) {
			for (const char *const *exemption = list; *exemption != nullptr; exemption++) {
				if (fileName.equalsIgnoreCase(*exemption))
					return true;
			}
			return false;
		};

		if (isInExemptions(exemptions) ||
			((g_engine->gameDescription().desc.flags & ADGF_DEMO) && isInExemptions(demoExemptions)))
			debugC(1, kDebugGraphics, "Animation exemption triggered: %s", fileName.c_str());
		else
			Game::missingAnimation(fileName);
	}

	void unknownAnimateObject(const char *name) override {
		if (!scumm_stricmp("EXPLOSION DISFRAZ", name))
			return;
		Game::unknownAnimateObject(name);
	}

	void unknownSayTextCharacter(const char *name, int32 dialogId) override {
		if (!scumm_stricmp(name, "OFELIA") && dialogId == 3737)
			return;
		Game::unknownSayTextCharacter(name, dialogId);
	}

	void missingSound(const String &fileName) override {
		if ((g_engine->gameDescription().desc.flags & ADGF_DEMO) && (
			fileName == "M4996" ||
			fileName == "T40"))
			return;
		GameWithVersion3::missingSound(fileName);
	}

	bool isKnownBadVideo(int32 videoId) override {
		return
			(videoId == 3 && (g_engine->gameDescription().desc.flags & ADGF_DEMO)) || // The german trailer is WMV-encoded
			Game::isKnownBadVideo(videoId);
	}

	void invalidVideo(int32 videoId, const char *context) override {
		// the second intro-video is DV-encoded in the spanish steam version
		if (videoId == 1 && g_engine->gameDescription().desc.language != DE_DEU)
			warning("Could not play video %d (%s) (WMV not supported)", videoId, context);
		else
			Game::invalidVideo(videoId, context);
	}
};

Game *Game::createForMovieAdventure() {
	if (g_engine->version() == EngineVersion::V3_0)
		return new GameMovieAdventureSpecialV30();
	else
		return new GameMovieAdventureSpecialV31();
}

}
