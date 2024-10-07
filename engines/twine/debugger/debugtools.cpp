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

#include "backends/imgui/IconsMaterialSymbols.h"

#include "backends/imgui/imgui.h"
#include "common/util.h"
#include "twine/debugger/debug_state.h"
#include "twine/debugger/debugtools.h"
#include "twine/debugger/dt-internal.h"
#include "twine/holomap.h"
#include "twine/renderer/redraw.h"
#include "twine/scene/actor.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/scene.h"

#include "twine/shared.h"
#include "twine/twine.h"

namespace TwinE {

#define GAME_STATE_TITLE "Game State"
#define MAIN_WINDOW_TITLE "Debug window"
#define HOLOMAP_FLAGS_TITLE "Holomap flags"
#define GAME_FLAGS_TITLE "Game flags"
#define ACTOR_DETAILS_TITLE "Actor"
#define GRID_TITLE "Grid"
#define MENU_TEXT_TITLE "Menu texts"

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
}

static void mainWindow(int &currentActor, TwinEEngine *engine) {
	if (ImGui::Begin(MAIN_WINDOW_TITLE)) {
		Scene *scene = engine->_scene;
		GameState *gameState = engine->_gameState;
		ImGui::Text("Scene: %i", scene->_currentSceneIdx);
		ImGui::Text("Scene name: %s", gameState->_sceneName);

		if (ImGui::Checkbox("Bounding boxes", &engine->_debugState->_showingActors)) {
			engine->_redraw->_firstTime = true;
		}
		if (ImGui::Checkbox("Clipping", &engine->_debugState->_showingClips)) {
			engine->_redraw->_firstTime = true;
		}
		if (ImGui::Checkbox("Zones", &engine->_debugState->_showingZones)) {
			engine->_redraw->_firstTime = true;
		}

		if (engine->_debugState->_showingZones) {
			if (ImGui::CollapsingHeader("Zones")) {
				static const struct ZonesDesc {
					const char *name;
					ZoneType type;
					const char *desc;
				} d[] = {
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

				for (int i = 0; i < ARRAYSIZE(d); ++i) {
					ImGui::CheckboxFlags(d[i].name, &engine->_debugState->_typeZones, (1u << (uint32)d[i].type));
					if (d[i].desc) {
						ImGui::SetTooltip(d[i].desc);
					}
				}
			}
		}

		if (ImGui::BeginCombo("Scene", gameState->_sceneName)) {
			for (int i = 0; i < engine->numHoloPos(); ++i) {
				Common::String name = Common::String::format("[%03d] %s", i, engine->_holomap->getLocationName(i));
				if (ImGui::Selectable(name.c_str(), i == engine->_scene->_currentSceneIdx)) {
					scene->_currentSceneIdx = i;
					scene->_needChangeScene = scene->_currentSceneIdx;
					engine->_redraw->_firstTime = true;
				}
			}
			ImGui::EndCombo();
		}

		if (currentActor < 0 || currentActor > engine->_scene->_nbObjets) {
			currentActor = 0;
		}
		Common::String currentActorLabel = Common::String::format("Actor %i", currentActor);
		if (ImGui::BeginCombo("Actor", currentActorLabel.c_str())) {
			for (int i = 0; i < engine->_scene->_nbObjets; ++i) {
				Common::String label = Common::String::format("Actor %i", i);
				const bool selected = i == currentActor;
				if (ImGui::Selectable(label.c_str(), selected)) {
					currentActor = i;
				}
			}
			ImGui::EndCombo();
		}
	}
	ImGui::End();
}

static void gameState(TwinEEngine *engine) {
	if (ImGui::Begin(GAME_STATE_TITLE)) {
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
	}
	ImGui::End();
}

static void holomapFlags(TwinEEngine *engine) {
	if (ImGui::Begin(HOLOMAP_FLAGS_TITLE)) {
		if (ImGui::BeginTable("###holomapflags", 8)) {
			for (int i = 0; i < engine->numHoloPos(); ++i) {
				ImGui::TableNextColumn();
				ImGui::Text("[%03d] = %d", i, engine->_gameState->_holomapFlags[i]);
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

static void gameFlags(TwinEEngine *engine) {
	if (ImGui::Begin(GAME_FLAGS_TITLE)) {
		ImGui::Text("Chapter %i", engine->_gameState->getChapter());
		if (ImGui::BeginTable("###gameflags", 8)) {
			for (int i = 0; i < NUM_GAME_FLAGS; ++i) {
				ImGui::TableNextColumn();
				ImGui::Text("[%03d] = %d", i, engine->_gameState->hasGameFlag(i));
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

static void grid(TwinEEngine *engine) {
	if (ImGui::Begin(GRID_TITLE)) {
		ImGui::Text("World cube %i %i %i", engine->_grid->_worldCube.x, engine->_grid->_worldCube.y, engine->_grid->_worldCube.z);
		ImGui::Text("World cube %i %i %i", engine->_grid->_worldCube.x, engine->_grid->_worldCube.y, engine->_grid->_worldCube.z);

		Grid *grid = engine->_grid;
		// Increase celling grid index
		if (ImGui::Button(ICON_MS_PLUS_ONE)) {
			grid->_cellingGridIdx++;
			if (grid->_cellingGridIdx > 133) {
				grid->_cellingGridIdx = 133;
			}
		}
		// Decrease celling grid index
		if (ImGui::Button(ICON_MS_EV_SHADOW_MINUS)) {
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
	}
	ImGui::End();
}

static void menuTexts(TwinEEngine *engine) {
	if (ImGui::Begin(MENU_TEXT_TITLE)) {
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

static void actorDetails(int actorIdx, TwinEEngine *engine) {
	if (ActorStruct *actor = engine->_scene->getActor(actorIdx)) {
		if (ImGui::Begin(ACTOR_DETAILS_TITLE)) {
			ImGui::Text("Idx %i", actor->_actorIdx);
			ImGui::Text("Pos %i %i %i", actor->_posObj.x, actor->_posObj.y, actor->_posObj.z);
			ImGui::Text("Followed %i", actor->_followedActor);
			ImGui::Text("Rotation %i", actor->_beta);
			ImGui::Text("Speed %i", actor->_speed);
			ImGui::Text("Control mode %i", actor->_controlMode);
			ImGui::Text("Delay %i", actor->_delayInMillis);
			ImGui::Text("Crop %i %i %i %i", actor->_cropLeft, actor->_cropTop, actor->_cropRight, actor->_cropBottom);
			ImGui::Text("Strength %i", actor->_strengthOfHit);
			ImGui::Text("Hit by %i", actor->_hitBy);
			ImGui::Text("Bonus %i", actor->_bonusParameter);
			ImGui::Text("Life %i", actor->_lifePoint);
			ImGui::Text("Brick shape %i", actor->brickShape());
			ImGui::Text("Brick causes damage %i", actor->brickCausesDamage());
			ImGui::Text("Collision %i", actor->_objCol);
			ImGui::Text("Body %i", actor->_body);
			ImGui::Text("Gen body %i", actor->_genBody);
			ImGui::Text("Save gen body %i", actor->_saveGenBody);
			ImGui::Text("Gen anim %i", actor->_genAnim);
			ImGui::Text("Next gen anim %i", actor->_nextGenAnim);
			ImGui::Text("Ptr anim action %i", actor->_ptrAnimAction);
			ImGui::Text("Sprite %i", actor->_sprite);
			ImGui::Text("A3DS %i %i %i", actor->A3DS.Num, actor->A3DS.Deb, actor->A3DS.Fin);
			ImGui::Text("Hit by %i", actor->_hitBy);
		}
		ImGui::End();
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
		if (ImGui::BeginMenu("Debugger")) {
			ImGui::Checkbox("Free camera", &engine->_debugState->_useFreeCamera);
			ImGui::Checkbox("God mode", &engine->_debugState->_godMode);

			if (ImGui::MenuItem("Center actor")) {
				ActorStruct *actor = engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX);
				actor->_posObj = engine->_grid->_worldCube;
				actor->_posObj.y += 1000;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	mainWindow(currentActor, engine);
	gameState(engine);
	grid(engine);
	menuTexts(engine);

	// TODO: combine them
	holomapFlags(engine);
	gameFlags(engine);

	actorDetails(currentActor, engine);
}

void onImGuiCleanup() {
}

} // namespace TwinE
