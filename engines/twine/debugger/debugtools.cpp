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

#include "twine/debugger/debugtools.h"
#include "backends/imgui/components/imgui_logger.h"
#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_fonts.h"
#include "backends/imgui/imgui_utils.h"
#include "common/log.h"
#include "common/scummsys.h"
#include "common/str-enc.h"
#include "common/str.h"
#include "common/util.h"
#include "graphics/palette.h"
#include "twine/debugger/debug_state.h"
#include "twine/debugger/dt-internal.h"
#include "twine/holomap.h"
#include "twine/parser/entity.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/scene.h"

#include "twine/shared.h"
#include "twine/twine.h"

namespace ImGuiEx {

bool InputIVec3(const char *label, TwinE::IVec3 &v, ImGuiInputTextFlags flags = 0) {
	int tmp[3] = {v.x, v.y, v.z};
	if (ImGui::InputInt3(label, tmp, flags)) {
		v.x = tmp[0];
		v.y = tmp[1];
		v.z = tmp[2];
		return true;
	}
	return false;
}

bool InputAngle(const char *label, int32 *v, int step = 1, int step_fast = 100, const char *format = "%.2f", ImGuiInputTextFlags flags = 0) {
	double tmp = TwinE::AngleToDegree(*v);
	if (ImGui::InputDouble(label, &tmp, step, step_fast, format, flags)) {
		*v = TwinE::DegreeToAngle(tmp);
		return true;
	}
	ImGui::SetItemTooltip("Angle: %i", (int)*v);
	return false;
}

bool InputBoundingBox(ImGuiID id, const char *prefixLabel, TwinE::BoundingBox &bbox) {
	TwinE::BoundingBox copy = bbox;
	Common::String idStr = Common::String::format("%s mins##mins%u", prefixLabel, id);
	if (ImGuiEx::InputIVec3(idStr.c_str(), copy.mins, ImGuiInputTextFlags_EnterReturnsTrue)) {
		if (copy.isValid()) {
			bbox.mins = copy.mins;
		}
		return true;
	}
	idStr = Common::String::format("%s maxs##maxs%u", prefixLabel, id);
	if (ImGuiEx::InputIVec3(idStr.c_str(), copy.maxs, ImGuiInputTextFlags_EnterReturnsTrue)) {
		if (copy.isValid()) {
			bbox.maxs = copy.maxs;
		}
		return true;
	}
	return false;
}

} // namespace ImGuiEx

namespace TwinE {

#define HOLOMAP_FLAGS_TITLE "Holomap flags"
#define GAME_FLAGS_TITLE "Game flags"
#define ACTOR_DETAILS_TITLE "Actor"
#define MENU_TEXT_TITLE "Menu texts"

static const char *toString(ShapeType type) {
	switch (type) {
	case ShapeType::kNone:
		return "None";
	case ShapeType::kSolid:
		return "Solid";
	case ShapeType::kStairsTopLeft:
		return "StairsTopLeft";
	case ShapeType::kStairsTopRight:
		return "StairsTopRight";
	case ShapeType::kStairsBottomLeft:
		return "StairsBottomLeft";
	case ShapeType::kStairsBottomRight:
		return "StairsBottomRight";
	case ShapeType::kDoubleSideStairsTop1:
		return "DoubleSideStairsTop1";
	case ShapeType::kDoubleSideStairsBottom1:
		return "DoubleSideStairsBottom1";
	case ShapeType::kDoubleSideStairsLeft1:
		return "DoubleSideStairsLeft1";
	case ShapeType::kDoubleSideStairsRight1:
		return "DoubleSideStairsRight1";
	case ShapeType::kDoubleSideStairsTop2:
		return "DoubleSideStairsTop2";
	case ShapeType::kDoubleSideStairsBottom2:
		return "DoubleSideStairsBottom2";
	case ShapeType::kDoubleSideStairsLeft2:
		return "DoubleSideStairsLeft2";
	case ShapeType::kDoubleSideStairsRight2:
		return "DoubleSideStairsRight2";
	case ShapeType::kFlatBottom1:
		return "FlatBottom1";
	case ShapeType::kFlatBottom2:
		return "FlatBottom2";
	default:
		return "Unknown";
	}
}

static void onLog(LogMessageType::Type type, int level, uint32 debugChannels, const char *message) {
	switch (type) {
	case LogMessageType::kError:
		_logger->addLog("[error]%s", message);
		break;
	case LogMessageType::kWarning:
		_logger->addLog("[warn]%s", message);
		break;
	case LogMessageType::kInfo:
		_logger->addLog("%s", message);
		break;
	case LogMessageType::kDebug:
		_logger->addLog("[debug]%s", message);
		break;
	}
}

void onImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = false;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;
	icons_config.GlyphOffset = {0, 4};

	static const ImWchar icons_ranges[] = {ICON_MIN_MS, ICON_MAX_MS, 0};
	ImGui::addTTFFontFromArchive("MaterialSymbolsSharp.ttf", 16.f, &icons_config, icons_ranges);

	_tinyFont = ImGui::addTTFFontFromArchive("FreeSans.ttf", 10.0f, nullptr, nullptr);

	_logger = new ImGuiEx::ImGuiLogger;

	Common::setLogWatcher(onLog);
}

static void holomapFlagsWindow(TwinEEngine *engine) {
	if (!engine->_debugState->_holomapFlagsWindow) {
		return;
	}
	if (ImGui::Begin(HOLOMAP_FLAGS_TITLE, &engine->_debugState->_holomapFlagsWindow)) {
		if (ImGui::BeginTable("###holomapflags", 8)) {
			for (int i = 0; i < engine->numHoloPos(); ++i) {
				ImGui::TableNextColumn();
				Common::String id = Common::String::format("[%03d]", i);
				ImGuiEx::InputInt(id.c_str(), &engine->_gameState->_holomapFlags[i]);
				ImGui::SetItemTooltip("%s", engine->_holomap->getLocationName(i));
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

static void paletteWindow(TwinEEngine *engine) {
	if (!engine->_debugState->_paletteWindow) {
		return;
	}

	const ImVec2 available = ImGui::GetContentRegionAvail();
	const float contentRegionHeight = available.y + ImGui::GetCursorPosY();
	const ImVec2 windowSize(10.0f * ImGui::GetFrameHeight(), contentRegionHeight);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Palettes", &engine->_debugState->_paletteWindow)) {
		if (engine->_screens->_flagPalettePcx) {
			ImGui::Text("palettepcx is active");
		} else {
			ImGui::Text("ptrpal is active");
		}

		ImGui::SeparatorText("Front buffer palette");
		const Graphics::Palette &frontBufferPalette = engine->_frontVideoBuffer.getPalette();
		ImGui::PushID("frontBufferPalette");
		ImGuiEx::Palette(frontBufferPalette);
		ImGui::PopID();

		ImGui::SeparatorText("PalettePCX");
		ImGui::PushID("palettePcx");
		ImGuiEx::Palette(engine->_screens->_palettePcx);
		ImGui::PopID();

		ImGui::SeparatorText("Palette");
		ImGui::PushID("ptrPal");
		ImGuiEx::Palette(engine->_screens->_ptrPal);
		ImGui::PopID();
	}
	ImGui::End();
}

static float WaitTime(void *data, int i) {
	TwinE::DebugState::FrameDataBuffer &buffer = *(TwinE::DebugState::FrameDataBuffer *)data;
	return (float)buffer[i].waitMillis;
}

static float FrameTime(void *data, int i) {
	TwinE::DebugState::FrameDataBuffer &buffer = *(TwinE::DebugState::FrameDataBuffer *)data;
	return (float)buffer[i].frameTime;
}

static void frameTimeWindow(TwinEEngine *engine) {
	if (!engine->_debugState->_frameTimeWindow) {
		return;
	}

	if (ImGui::Begin("Frame time", &engine->_debugState->_frameTimeWindow)) {
		ImGui::Checkbox("Record", &engine->_debugState->_frameDataRecording);
		ImGui::PlotHistogram("Wait time", WaitTime, &engine->_debugState->_frameData, (int)engine->_debugState->_frameData.size(), 0, "Wait time in millis", -100.0f, 100.0f, ImVec2(0, 80));
		ImGui::PlotHistogram("Frame time", FrameTime, &engine->_debugState->_frameData, (int)engine->_debugState->_frameData.size(), 0, "Frame time in millis", -100.0f, 100.0f, ImVec2(0, 80));
	}
	ImGui::End();
}

static void sceneFlagsWindow(TwinEEngine *engine) {
	if (!engine->_debugState->_sceneFlagsWindow) {
		return;
	}

	if (ImGui::Begin("Scene flags", &engine->_debugState->_sceneFlagsWindow)) {
		if (ImGui::BeginTable("###sceneflags", 8)) {
			for (int i = 0; i < NUM_SCENES_FLAGS; ++i) {
				ImGui::TableNextColumn();
				Common::String id = Common::String::format("[%03d]", i);
				ImGuiEx::InputInt(id.c_str(), &engine->_scene->_listFlagCube[i]);
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

static void gameFlagsWindow(TwinEEngine *engine) {
	if (!engine->_debugState->_gameFlagsWindow) {
		return;
	}
	if (ImGui::Begin(GAME_FLAGS_TITLE, &engine->_debugState->_gameFlagsWindow)) {
		ImGui::Text("Chapter %i", engine->_gameState->getChapter());
		if (ImGui::BeginTable("###gameflags", 8)) {
			for (int i = 0; i < NUM_GAME_FLAGS; ++i) {
				ImGui::TableNextColumn();
				Common::String id = Common::String::format("[%03d]", i);
				int16 val = engine->_gameState->hasGameFlag(i);
				if (ImGuiEx::InputInt(id.c_str(), &val)) {
					engine->_gameState->setGameFlag(i, val);
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

static void menuTextsWindow(TwinEEngine *engine) {
	if (!engine->_debugState->_menuTextWindow) {
		return;
	}
	if (ImGui::Begin(MENU_TEXT_TITLE, &engine->_debugState->_menuTextWindow)) {
		int id = (int)engine->_debugState->_textBankId;
		if (ImGui::InputInt("Text bank", &id)) {
			engine->_debugState->_textBankId = (TextBankId)id;
		}
		const TextBankId oldTextBankId = engine->_text->textBank();
		engine->_text->initDial(engine->_debugState->_textBankId);
		for (int32 i = 0; i < 1000; ++i) {
			char buf[256];
			if (engine->_text->getMenuText((TextId)i, buf, sizeof(buf))) {
				ImGui::Text("%4i: %s\n", i, buf);
			}
		}
		engine->_text->initDial(oldTextBankId);
	}
	ImGui::End();
}

static void sceneSelectionCombo(TwinEEngine *engine) {
	Scene *scene = engine->_scene;
	GameState *gameState = engine->_gameState;
	Common::U32String originalSceneName(gameState->_sceneName, Common::kDos850);
	const Common::String sceneName = originalSceneName.encode(Common::kUtf8);
	if (ImGui::BeginCombo("Scene", sceneName.c_str())) {
		for (int i = 0; i < engine->numHoloPos(); ++i) {
			Common::U32String originalLocationName(engine->_holomap->getLocationName(i), Common::kDos850);
			const Common::String locationName = originalLocationName.encode(Common::kUtf8);
			Common::String name = Common::String::format("[%03d] %s", i, locationName.c_str());
			if (ImGui::Selectable(name.c_str(), i == engine->_scene->_numCube)) {
				scene->_numCube = i;
				scene->_newCube = scene->_numCube;
				engine->_redraw->_firstTime = true;
			}
		}
		ImGui::EndCombo();
	}
}

static const struct ZonesDesc {
	const char *name;
	ZoneType type;
	const char *desc;
} ZoneDescriptions[] = {
	{"Cube", ZoneType::kCube, "Change to another scene"},
	{"Camera", ZoneType::kCamera, "Binds camera view"},
	{"Sceneric", ZoneType::kSceneric, "For use in Life Script"},
	{"Grid", ZoneType::kGrid, "Set disappearing Grid fragment"},
	{"Object", ZoneType::kObject, "Give bonus"},
	{"Text", ZoneType::kText, "Displays text message"},
	{"Ladder", ZoneType::kLadder, "Hero can climb on it"},
	{"Escalator", ZoneType::kEscalator, nullptr},
	{"Hit", ZoneType::kHit, nullptr},
	{"Rail", ZoneType::kRail, nullptr}};

static void sceneDetailsWindows(TwinEEngine *engine) {
	if (!engine->_debugState->_sceneDetailsWindow) {
		return;
	}
	if (ImGui::Begin("Scene", &engine->_debugState->_sceneDetailsWindow)) {
		Scene *scene = engine->_scene;
		GameState *gameState = engine->_gameState;
		ImGui::Text("Scene: %i", scene->_numCube);
		ImGui::Text("Scene name: %s", gameState->_sceneName);
		sceneSelectionCombo(engine);

		if (ImGui::Checkbox("Bounding boxes", &engine->_debugState->_showingActors)) {
			engine->_redraw->_firstTime = true;
		}
		if (ImGui::Checkbox("Clipping", &engine->_debugState->_showingClips)) {
			engine->_redraw->_firstTime = true;
		}
		if (ImGui::Checkbox("Zones", &engine->_debugState->_showingZones)) {
			engine->_redraw->_firstTime = true;
		}
		// if (ImGui::Checkbox("Tracks", &engine->_debugState->_showingTracks)) {
		// 	engine->_redraw->_firstTime = true;
		// }
		if (engine->_debugState->_showingZones) {
			if (ImGui::CollapsingHeader("Show zone types")) {
				for (int i = 0; i < ARRAYSIZE(ZoneDescriptions); ++i) {
					if (ImGui::CheckboxFlags(ZoneDescriptions[i].name, &engine->_debugState->_typeZones, (1u << (uint32)ZoneDescriptions[i].type))) {
						engine->_redraw->_firstTime = true;
					}
					if (ZoneDescriptions[i].desc) {
						ImGui::SetItemTooltip(ZoneDescriptions[i].desc);
					}
				}
			}
		}

		if (ImGui::CollapsingHeader("Zones##zonesheader")) {
			for (int i = 0; i < scene->_sceneNumZones; ++i) {
				ZoneStruct *zone = &scene->_sceneZones[i];
				ImGui::Text("Zone idx: %i", i);
				ImGui::Indent();
				const ZonesDesc &zoneDesc = ZoneDescriptions[(int)zone->type];
				ImGui::Text("Type: %s", zoneDesc.name);
				if (zoneDesc.desc != nullptr) {
					ImGui::SameLine();
					ImGui::Text("%s", zoneDesc.desc);
				}
				ImGui::PushID(i);
				ImGuiEx::InputIVec3("Mins", zone->mins);
				ImGuiEx::InputIVec3("Maxs", zone->maxs);
				ImGui::PopID();

				ImGui::Text("Num: %i", zone->num);
				ImGui::Text("Info0: %i", zone->infoData.generic.info0);
				ImGui::Text("Info1: %i", zone->infoData.generic.info1);
				ImGui::Text("Info2: %i", zone->infoData.generic.info2);
				ImGui::Text("Info3: %i", zone->infoData.generic.info3);
				ImGui::Text("Info4: %i", zone->infoData.generic.info4);
				ImGui::Text("Info5: %i", zone->infoData.generic.info5);
				ImGui::Text("Info6: %i", zone->infoData.generic.info6);
				ImGui::Text("Info7: %i", zone->infoData.generic.info7);
				ImGui::Unindent();
			}
		}

		if (ImGui::CollapsingHeader("Tracks##tracksheader")) {
			for (int i = 0; i < scene->_sceneNumTracks; ++i) {
				ImGui::Text("Track %i: %i %i %i", i, scene->_sceneTracks[i].x, scene->_sceneTracks[i].y, scene->_sceneTracks[i].z);
			}
		}

		if (ImGui::CollapsingHeader("Trajectories##trajectoriesheader")) {
			const TrajectoryData &trajectories = engine->_resources->getTrajectories();
			for (int i = 0; i < (int)trajectories.getTrajectories().size(); ++i) {
				const Trajectory *trajectory = trajectories.getTrajectory(i);
				ImGui::Text("Trajectory %i", i);
				ImGui::SameLine();
				Common::String buttonId = Common::String::format("Activate##activateTrajectory%i", i);
				if (ImGui::Button(buttonId.c_str())) {
					scene->_numHolomapTraj = i;
					scene->reloadCurrentScene();
				}
				ImGui::Indent();
				ImGui::Text("location: %i", trajectory->locationIdx);
				ImGui::Text("trajectory location: %i", trajectory->trajLocationIdx);
				ImGui::Text("vehicle: %i", trajectory->vehicleIdx);
				ImGui::Text("pos: %i %i %i", trajectory->angle.x, trajectory->angle.y, trajectory->angle.z);
				ImGui::Text("num anim frames: %i", trajectory->numAnimFrames);
				ImGui::Unindent();
			}
		}
		ImGuiEx::InputInt("Previous scene index", &scene->_oldcube);
		ImGuiEx::InputInt("Need change scene index", &scene->_newCube);

		ImGui::Text("Climbing flag");
		ImGui::SameLine();
		ImGuiEx::Boolean(scene->_flagClimbing);

		ImGuiEx::InputInt("Currently followed actor", &scene->_numObjFollow);

		ImGui::Checkbox("Enable enhancements", &scene->_enableEnhancements);
		ImGui::Checkbox("Render grid tiles", &scene->_enableGridTileRendering);
		ImGuiEx::InputInt("Current script value", &scene->_currentScriptValue);
		ImGuiEx::InputInt("Talking actor", &scene->_talkingActor);
		ImGuiEx::InputInt("Cube jingle", &scene->_cubeJingle);
		ImGuiEx::InputIVec3("New hero pos", scene->_newHeroPos);
		ImGuiEx::InputInt("Alpha light", &scene->_alphaLight);
		ImGuiEx::InputInt("Beta light", &scene->_betaLight);
		ImGuiEx::InputInt("Fall Y position", &scene->_startYFalling);
		ImGui::Text("Hero position type: %i", (int)scene->_heroPositionType);
	}
	ImGui::End();
}

static void actorDetailsWindow(int &actorIdx, TwinEEngine *engine) {
	if (!engine->_debugState->_actorDetailsWindow) {
		return;
	}
	ActorStruct *actor = engine->_scene->getActor(actorIdx);
	if (actor == nullptr) {
		return;
	}

	if (ImGui::Begin(ACTOR_DETAILS_TITLE, &engine->_debugState->_actorDetailsWindow)) {
		if (actorIdx < 0 || actorIdx > engine->_scene->_nbObjets) {
			actorIdx = 0;
		}
		Common::String currentActorLabel = Common::String::format("Actor %i", actorIdx);
		if (ImGui::BeginCombo("Actor", currentActorLabel.c_str())) {
			for (int i = 0; i < engine->_scene->_nbObjets; ++i) {
				Common::String label = Common::String::format("Actor %i", i);
				if (engine->_scene->_mecaPenguinIdx == i) {
					label += " (Penguin)";
				}
				const bool selected = i == actorIdx;
				if (ImGui::Selectable(label.c_str(), selected)) {
					actorIdx = i;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		ImGuiEx::InputIVec3("Pos", actor->_posObj);
		ImGuiEx::InputAngle("Rotation", &actor->_beta);
		ImGuiEx::InputInt("Rotation speed", &actor->_srot);
		ImGuiEx::InputInt("Life", &actor->_lifePoint);
		ImGuiEx::InputInt("Armor", &actor->_armor);
		ImGuiEx::InputBoundingBox(actorIdx, "Bounding box", actor->_boundingBox);

		if (ImGui::CollapsingHeader("Properties")) {
			if (ImGui::BeginTable("Properties", 2)) {
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableHeadersRow();

				ImGui::TableNextColumn();
				ImGui::Text("Followed");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_followedActor);
				ImGui::TableNextColumn();
				ImGui::Text("Control mode");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_controlMode);
				ImGui::TableNextColumn();
				ImGui::Text("Delay");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_delayInMillis);
				ImGui::TableNextColumn();
				ImGui::Text("Strength");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_strengthOfHit);
				ImGui::TableNextColumn();
				ImGui::Text("Hit by");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_hitBy);
				ImGui::TableNextColumn();
				ImGui::Text("Bonus");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_bonusParameter);
				ImGui::TableNextColumn();
				ImGui::Text("ZoneSce");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_zoneSce);
				ImGui::TableNextColumn();
				ImGui::Text("Brick shape");
				ImGui::TableNextColumn();
				ImGui::Text("%s", toString(actor->brickShape()));
				ImGui::TableNextColumn();
				ImGui::Text("Brick causes damage");
				ImGui::TableNextColumn();
				ImGuiEx::Boolean(actor->brickCausesDamage());
				ImGui::TableNextColumn();
				ImGui::Text("Collision");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_objCol);
				ImGui::TableNextColumn();
				ImGui::Text("Carried by");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_carryBy);
				ImGui::TableNextColumn();
				ImGui::Text("Talk color");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_talkColor);
				ImGui::TableNextColumn();
				ImGui::Text("Body");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_body); // TODO: link to resources
				ImGui::TableNextColumn();
				ImGui::Text("Gen body");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_genBody);
				ImGui::TableNextColumn();
				ImGui::Text("Save gen body");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_saveGenBody);
				ImGui::TableNextColumn();
				ImGui::Text("Gen anim");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_genAnim);
				ImGui::TableNextColumn();
				ImGui::Text("Next gen anim");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_nextGenAnim);
				ImGui::TableNextColumn();
				ImGui::Text("Ptr anim action");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_ptrAnimAction);
				ImGui::TableNextColumn();
				ImGui::Text("Sprite");
				ImGui::TableNextColumn();
				ImGui::Text("%i", actor->_sprite);
				ImGui::TableNextColumn();
				ImGui::Text("A3DS");
				ImGui::TableNextColumn();
				ImGui::Text("%i %i %i", actor->A3DS.Num, actor->A3DS.Deb, actor->A3DS.Fin);

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Work Flags")) {
			static const char *Names[] = {
				"WAIT_HIT_FRAME",
				"OK_HIT",
				"ANIM_END",
				"NEW_FRAME",
				"WAS_DRAWN",
				"OBJ_DEAD",
				"AUTO_STOP_DOOR",
				"ANIM_MASTER_ROT",
				"FALLING",
				"IS_TARGETABLE",
				"IS_BLINKING",
				"DRAW_SHADOW",
				"ANIM_MASTER_GRAVITY",
				"SKATING",
				"OK_RENVOIE",
				"LEFT_JUMP",
				"RIGHT_JUMP",
				"WAIT_SUPER_HIT",
				"TRACK_MASTER_ROT",
				"FLY_JETPACK",
				"DONT_PICK_CODE_JEU",
				"MANUAL_INTER_FRAME",
				"WAIT_COORD",
				"CHECK_FALLING"};
			if (ImGui::BeginTable("##workflags", 6)) {
				for (int i = 0; i < ARRAYSIZE(Names); ++i) {
					ImGui::TableNextColumn();
					ImGui::CheckboxFlags(Names[i], (uint32_t *)&actor->_workFlags, (1 << i));
				}
				ImGui::EndTable();
			}
		}
		if (ImGui::CollapsingHeader("Flags")) {
			static const char *Names[] = {
				"CHECK_OBJ_COL",
				"CHECK_BRICK_COL",
				"CHECK_ZONE",
				"SPRITE_CLIP",
				"PUSHABLE",
				"COL_BASSE",
				"CHECK_CODE_JEU",
				"CHECK_WATER_COL",
				"0x000100",
				"INVISIBLE",
				"SPRITE_3D",
				"OBJ_FALLABLE",
				"NO_SHADOW",
				"OBJ_BACKGROUND",
				"OBJ_CARRIER",
				"MINI_ZV",
				"POS_INVALIDE",
				"NO_CHOC",
				"ANIM_3DS",
				"NO_PRE_CLIP",
				"OBJ_ZBUFFER",
				"OBJ_IN_WATER",
			};
			if (ImGui::BeginTable("##staticflags", 6)) {
				for (int i = 0; i < ARRAYSIZE(Names); ++i) {
					ImGui::TableNextColumn();
					ImGui::CheckboxFlags(Names[i], (uint32_t *)&actor->_flags, (1 << i));
				}
				ImGui::EndTable();
			}
		}

		if (actor->_body != -1) {
			ImGui::SeparatorText("Body");
			if (actor->_entityDataPtr != nullptr) {
				BodyData &bodyData = actor->_entityDataPtr->getBody(actor->_body);
				ImGuiEx::InputBoundingBox((int)(uintptr)&bodyData, "Bounding box", bodyData.bbox);
			} else {
				ImGui::Text("No entity data");
			}
		}

		ImGui::SeparatorText("Entity");
		EntityData &entityData = actor->_entityData;
		Common::Array<EntityBody> &entityBodies = entityData.getBodies();
		ImGui::Text("Bodies: %i", (int)entityBodies.size());
		for (EntityBody &entityBody : entityBodies) {
			ImGui::Text("%s index: %i", Resources::HQR_FILE3D_FILE, entityBody.index);
			ImGui::Indent();
			ImGui::Text("%s index: %i", Resources::HQR_BODY_FILE, entityBody.hqrBodyIndex);
			Common::String id = Common::String::format("Has bounding box##%i", entityBody.index);
			ImGui::Checkbox(id.c_str(), &entityBody.actorBoundingBox.hasBoundingBox);
			ImGuiEx::InputBoundingBox((int)(uintptr)&entityBody, "Bounding box", entityBody.actorBoundingBox.bbox);
			ImGui::Unindent();
		}
		Common::Array<EntityAnim> &animations = entityData.getAnimations();
		ImGui::Text("Animations: %i", (int)animations.size());
		for (EntityAnim &animation : animations) {
			ImGui::Text("Animation type: %i", (int)animation.animation);
			ImGui::Indent();
			ImGui::Text("Body animation index: %i", (int)animation.animIndex);
			ImGui::Text("actions: %i", (int)animation._actions.size());
			for (EntityAnim::Action &action : animation._actions) {
				ImGui::BulletText("%i", (int)action.type);
			}
			ImGui::Unindent();
		}
	}
	ImGui::End();
}

static void gameStateMenu(TwinEEngine *engine) {
	if (ImGui::BeginMenu("Game State")) {
		int keys = engine->_gameState->_inventoryNumKeys;
		if (ImGui::InputInt("Keys", &keys)) {
			engine->_gameState->setKeys(keys);
		}
		int kashes = engine->_gameState->_goldPieces;
		if (ImGui::InputInt("Cash", &kashes)) {
			engine->_gameState->setKashes(kashes);
		}
		int zlitos = engine->_gameState->_zlitosPieces;
		if (ImGui::InputInt("Zlitos", &zlitos)) {
			engine->_gameState->setZlitos(zlitos);
		}
		int magicPoints = engine->_gameState->_magicPoint;
		if (ImGui::InputInt("Magic points", &magicPoints)) {
			engine->_gameState->setMagicPoints(magicPoints);
		}
		int magicLevel = engine->_gameState->_magicLevelIdx;
		if (ImGui::InputInt("Magic level", &magicLevel)) {
			engine->_gameState->_magicLevelIdx = CLIP<int16>(magicLevel, 0, 4);
		}
		int leafs = engine->_gameState->_inventoryNumLeafs;
		if (ImGui::InputInt("Leafs", &leafs)) {
			engine->_gameState->setLeafs(leafs);
		}
		int leafBoxes = engine->_gameState->_inventoryNumLeafsBox;
		if (ImGui::InputInt("Leaf boxes", &leafBoxes)) {
			engine->_gameState->setLeafBoxes(leafBoxes);
		}
		int gas = engine->_gameState->_inventoryNumGas;
		if (ImGui::InputInt("Gas", &gas)) {
			engine->_gameState->setGas(gas);
		}
		const TextBankId oldTextBankId = engine->_text->textBank();
		engine->_text->initDial(TextBankId::Inventory_Intro_and_Holomap);

		for (int i = 0; i < NUM_INVENTORY_ITEMS; ++i) {
			Common::String label;
			if (engine->_text->getText((TextId)(100 + i))) {
				Common::U32String original(engine->_text->_currDialTextEntry->string, Common::kDos850);
				label = original.encode(Common::kUtf8).substr(0, 30);
			} else {
				label = Common::String::format("Item %i", i);
			}
			uint8 &value = engine->_gameState->_inventoryFlags[i];
			bool hasItem = value != 0;
			if (ImGui::Checkbox(label.c_str(), &hasItem)) {
				value = hasItem == 0 ? 0 : 1;
			}
		}
		engine->_text->initDial(oldTextBankId);
		ImGui::EndMenu();
	}
}

static void gridMenu(TwinEEngine *engine) {
	if (ImGui::BeginMenu("Grid")) {
		ImGui::Text("World cube %i %i %i", engine->_grid->_worldCube.x, engine->_grid->_worldCube.y, engine->_grid->_worldCube.z);
#if 0
		Grid *grid = engine->_grid;

		if (ImGui::Button(ICON_MS_ADD)) {
			grid->_cellingGridIdx++;
			if (grid->_cellingGridIdx > 133) {
				grid->_cellingGridIdx = 133;
			}
		}
		if (ImGui::Button(ICON_MS_REMOVE)) {
			grid->_cellingGridIdx--;
			if (grid->_cellingGridIdx < 0) {
				grid->_cellingGridIdx = 0;
			}
		}
		// Enable/disable celling grid
		if (ImGui::Button("Apply ceiling grid")) {
			if (grid->_useCellingGrid == -1) {
				grid->_useCellingGrid = 1;
				// grid->createGridMap();
				grid->initCellingGrid(grid->_cellingGridIdx);
				debug("Enable Celling Grid index: %d", grid->_cellingGridIdx);
				engine->_scene->_needChangeScene = SCENE_CEILING_GRID_FADE_2; // tricky to make the fade
			} else if (grid->_useCellingGrid == 1) {
				grid->_useCellingGrid = -1;
				grid->copyMapToCube();
				engine->_redraw->_firstTime = true;
				debug("Disable Celling Grid index: %d", grid->_cellingGridIdx);
				engine->_scene->_needChangeScene = SCENE_CEILING_GRID_FADE_2; // tricky to make the fade
			}
		}
#endif
		ImGui::EndMenu();
	}
}

static void debuggerMenu(TwinEEngine *engine) {
	if (ImGui::BeginMenu("Debugger")) {
		ImGui::Text("Timer: %i", (int)engine->timerRef);
		if (ImGui::MenuItem("Logs")) {
			engine->_debugState->_loggerWindow = true;
		}
		if (ImGui::MenuItem("Texts")) {
			engine->_debugState->_menuTextWindow = true;
		}
		if (ImGui::MenuItem("Holomap flags")) {
			engine->_debugState->_holomapFlagsWindow = true;
		}
		if (ImGui::MenuItem("Game flags")) {
			engine->_debugState->_gameFlagsWindow = true;
		}
		if (ImGui::MenuItem("Scene details")) {
			engine->_debugState->_sceneDetailsWindow = true;
		}
		if (ImGui::MenuItem("Scene flags")) {
			engine->_debugState->_sceneFlagsWindow = true;
		}
		if (ImGui::MenuItem("Actor details")) {
			engine->_debugState->_actorDetailsWindow = true;
		}
		if (ImGui::MenuItem("Frame time")) {
			engine->_debugState->_frameTimeWindow = true;
		}

		ImGui::SeparatorText("Actions");

		if (ImGui::MenuItem("Center actor")) {
			ActorStruct *actor = engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX);
			actor->_posObj = engine->_grid->_worldCube;
			actor->_posObj.y += 1000;
		}
		if (ImGui::BeginMenu("Palettes")) {
			LifeScriptContext fakeCtx(0, engine->_scene->_sceneHero);
			if (ImGui::MenuItem("Show palette")) {
				engine->_debugState->_paletteWindow = true;
			}
			if (ImGui::MenuItem("Dark palette")) {
				engine->_scriptLife->lSET_DARK_PAL(engine, fakeCtx);
			}
			if (ImGui::MenuItem("Normal palette")) {
				engine->_scriptLife->lSET_NORMAL_PAL(engine, fakeCtx);
			}
#if 0
			// TODO: the fade functions are blocking and break the imgui begin/end cycle
			if (ImGui::MenuItem("lFADE_PAL_RED")) {
				engine->_scriptLife->lFADE_PAL_RED(engine, fakeCtx);
			}
			if (ImGui::MenuItem("lFADE_ALARM_RED")) {
				engine->_scriptLife->lFADE_ALARM_RED(engine, fakeCtx);
			}
			if (ImGui::MenuItem("lFADE_ALARM_PAL")) {
				engine->_scriptLife->lFADE_ALARM_PAL(engine, fakeCtx);
			}
			if (ImGui::MenuItem("lFADE_RED_PAL")) {
				engine->_scriptLife->lFADE_RED_PAL(engine, fakeCtx);
			}
			if (ImGui::MenuItem("lFADE_RED_ALARM")) {
				engine->_scriptLife->lFADE_RED_ALARM(engine, fakeCtx);
			}
			if (ImGui::MenuItem("lFADE_PAL_ALARM")) {
				engine->_scriptLife->lFADE_PAL_ALARM(engine, fakeCtx);
			}
#endif
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Force Redraw")) {
			engine->_redraw->_firstTime = true;
		}

		ImGui::SeparatorText("Options");

		ImGui::Checkbox("Free camera", &engine->_debugState->_useFreeCamera);
		ImGui::Checkbox("God mode", &engine->_debugState->_godMode);

		sceneSelectionCombo(engine);

		ImGui::EndMenu();
	}
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugImGui)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}
	static int currentActor = 0;

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	TwinEEngine *engine = (TwinEEngine *)g_engine;

	if (ImGui::BeginMainMenuBar()) {
		debuggerMenu(engine);
		gameStateMenu(engine);
		gridMenu(engine);
		ImGui::EndMainMenuBar();
	}

	actorDetailsWindow(currentActor, engine);
	sceneDetailsWindows(engine);
	menuTextsWindow(engine);
	holomapFlagsWindow(engine);
	gameFlagsWindow(engine);
	paletteWindow(engine);
	sceneFlagsWindow(engine);
	frameTimeWindow(engine);
	_logger->draw("Logger", &engine->_debugState->_loggerWindow);

	if (engine->_debugState->_openPopup) {
		ImGui::OpenPopup(engine->_debugState->_openPopup);
		engine->_debugState->_openPopup = nullptr;
	}
}

void onImGuiCleanup() {
	delete _logger;
	_logger = nullptr;
}

} // namespace TwinE
