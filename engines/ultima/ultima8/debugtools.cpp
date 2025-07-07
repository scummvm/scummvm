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

#include "ultima/ultima8/debugtools.h"
#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_utils.h"
#include "ultima/ultima.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/palette.h"
#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/gumps/item_relative_gump.h"
#include "ultima/ultima8/gumps/target_gump.h"
#include "ultima/ultima8/usecode/usecode.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/actors/quick_avatar_mover_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/misc/debugger.h"

namespace Ultima {
namespace Ultima8 {

typedef struct ImGuiState {
	bool _itemStatsWindow = false;
	bool _paletteWindow = false;
	uint32 _targetItemId = kMainActorId;
	ObjId _targetGumpId = 0;
} ImGuiState;

ImGuiState *_state = nullptr;

void showItemStats() {
	if (!_state->_itemStatsWindow)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 550), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Item Stats", &_state->_itemStatsWindow)) {
		if (_state->_targetGumpId) {
			// Check if gump still exists and has a result
			Gump *gump = getGump(_state->_targetGumpId);
			if (gump) {
				if (gump->GetResult()) {
					_state->_targetItemId = gump->GetResult();
					_state->_targetGumpId = 0;
				}
			} else {
				_state->_targetGumpId = 0;
			}
		}

		ImGui::BeginChild("##scrolling", ImVec2(0, -30), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav); 
		if (_state->_targetItemId) {
			Item *item = getItem(_state->_targetItemId);
			if (item) {
				const ShapeInfo *si = item->getShapeInfo();

				if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("Properties", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
						// ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
						// ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
						// ImGui::TableHeadersRow();

						ImGui::TableNextColumn();
						ImGui::Text("Id");
						ImGui::TableNextColumn();
						ImGui::Text("%u", item->getObjId());
						ImGui::TableNextColumn();
						ImGui::Text("Class");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->GetClassType()._className);
						ImGui::TableNextColumn();
						ImGui::Text("Shape");
						ImGui::TableNextColumn();
						ImGui::Text("%u", item->getShape());
						ImGui::TableNextColumn();
						ImGui::Text("Frame");
						ImGui::TableNextColumn();
						ImGui::Text("%u", item->getFrame());
						ImGui::TableNextColumn();
						ImGui::Text("Map");
						ImGui::TableNextColumn();
						ImGui::Text("%u", item->getMapNum());
						ImGui::TableNextColumn();
						ImGui::Text("Location");
						ImGui::TableNextColumn();
						if (item->getParent()) {
							int32 gx, gy;
							item->getGumpLocation(gx, gy);
							ImGui::Text("(%d, %d)", gx, gy);
						} else {
							Point3 p = item->getLocation();
							ImGui::Text("(%d, %d, %d)", p.x, p.y, p.z);
						}
						ImGui::TableNextColumn();

						ImGui::Text("Footpad");
						ImGui::TableNextColumn();
						int32 xd, yd, zd;
						item->getFootpadData(xd, yd, zd);
						ImGui::Text("%d, %d, %d", xd, yd, zd);
						ImGui::TableNextColumn();
						// Original weights appear to be different
						ImGui::Text("Weight");
						ImGui::TableNextColumn();
						ImGui::Text("%d", item->getWeight());
						ImGui::TableNextColumn();
						ImGui::Text("Volume");
						ImGui::TableNextColumn();
						ImGui::Text("%d", item->getVolume());
						ImGui::TableNextColumn();
						// Original menu had "Anim" here - None, Unk, Normal, Fast
						// Original menu had "Fr" and "Sp" here
						ImGui::Text("Family");
						ImGui::TableNextColumn();
						ImGui::Text("%d", item->getFamily());
						ImGui::TableNextColumn();
						ImGui::Text("Usecode");
						ImGui::TableNextColumn();
						const char *ucname = GameData::get_instance()->getMainUsecode()->get_class_name(item->getShape());
						ImGui::Text("%s", ucname != nullptr ? ucname : "");
						ImGui::TableNextColumn();
						ImGui::Text("Quality");
						ImGui::TableNextColumn();
						ImGui::Text("%u", item->getQuality());
						ImGui::TableNextColumn();
						ImGui::Text("NPC Number");
						ImGui::TableNextColumn();
						ImGui::Text("%u", item->getNpcNum());
						ImGui::TableNextColumn();
						ImGui::Text("Equipment Type");
						ImGui::TableNextColumn();
						ImGui::Text("0x%x", si ? si->_equipType : 0);

						ImGui::EndTable();
					}
				}

				// ShapeInfo "Type Flags"
				Common::String shapeFlagsLabel = Common::String::format("Shape Flags: 0x%04x###shapeflags", si->_flags);
				if (ImGui::CollapsingHeader(shapeFlagsLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
					if (si) {
						if (ImGui::BeginTable("Shape Flags", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
							ImGui::TableNextColumn();
							ImGui::Text("Fixed");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_fixed() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Solid");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_solid() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Sea");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_sea() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Land");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_land() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Occlude");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_occl() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Bag");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_bag() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Damaging");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_damaging() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Noisy");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_noisy() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Draw");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_draw() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Ignore");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_ignore() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Roof");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_roof() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Translucent");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_translucent() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Editor");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_editor() ? "true" : "false");
							ImGui::TableNextColumn();
							ImGui::Text("Explode");
							ImGui::TableNextColumn();
							ImGui::Text("%s", si->is_u8_explode() ? "true" : "false");
							// Original menu had "InDlist" flag here

							ImGui::EndTable();
						}
					}
				}

				Common::String itemFlagsLabel = Common::String::format("Item Flags: 0x%04x###itemflags", item->getFlags());
				if (ImGui::CollapsingHeader(itemFlagsLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("Item Flags", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
						ImGui::TableNextColumn();
						// Original menu had "InDlist" flag here - 0x0001?
						ImGui::Text("Disposible");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_DISPOSABLE) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Owned"); // "Virtl"? Appears on items created by usecode
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_OWNED) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Contained");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_CONTAINED) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Invisible");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_INVISIBLE) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Flipped");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_FLIPPED) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("NPC");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_IN_NPC_LIST) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Fast Only"); // "GlobCr"
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_FAST_ONLY) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Gump Open"); // "GumpUp"
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_GUMP_OPEN) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Equipped");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_EQUIPPED) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Bounce");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_BOUNCING) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Ethereal");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_ETHEREAL) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Hanging");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_HANGING) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("In Fast Area");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_FASTAREA) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Low Friction");
						ImGui::TableNextColumn();
						ImGui::Text("%s", item->hasFlags(Item::FLG_LOW_FRICTION) ? "true" : "false");
						ImGui::TableNextColumn();
						ImGui::Text("Item Relative Gump"); // "IRGump" - 0x8000?
						ImGui::TableNextColumn();
						bool isRelative = false;
						if (item->getGump()) {
							Gump *g = getGump(item->getGump());
							if (g && dynamic_cast<ItemRelativeGump *>(g)) {
								isRelative = true;
							}
						}
						ImGui::Text("%s", isRelative ? "true" : "false");

						ImGui::EndTable();
					}
				}
			} else {
				ImGui::Text("Item not found: %d", _state->_targetItemId);
			}
		}
		ImGui::EndChild();

		if (ImGui::Button("Set Target")) {
			// If not targetting start a new target gump and wait
			if (!_state->_targetGumpId) {
				TargetGump *targetGump = new TargetGump(0, 0);
				targetGump->InitGump(0);
				_state->_targetGumpId = targetGump->getObjId(); 
			}
		}
	}
	ImGui::End();
}

static void showPalette() {
	if (!_state->_paletteWindow) {
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(320, 550), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Palettes", &_state->_paletteWindow)) {
		PaletteManager  *pm = PaletteManager::get_instance();
		Palette *p = pm->getPalette(PaletteManager::Pal_Game);
		if (p) {
			ImGui::SeparatorText("Game palette");
			ImGui::PushID("palette_0");
			ImGuiEx::Palette(*p);
			ImGui::PopID();
			ImGui::NewLine();
		}

		for (uint i = 1; i < pm->getNumPalettes(); i++) {
			p = pm->getPalette(static_cast<PaletteManager::PalIndex>(i));
			if (p) {
				Common::String text = Common::String::format("Palette %d", i);
				Common::String id = Common::String::format("palette_%d", i);
				ImGui::SeparatorText(text.c_str());
				ImGui::PushID(id.c_str());
				ImGuiEx::Palette(*p);
				ImGui::PopID();
				ImGui::NewLine();
			}
		}
	}
	ImGui::End();
}

void onImGuiInit() {
	_state = new ImGuiState();
}

void onImGuiRender() {
	ImGuiIO& io = ImGui::GetIO();
	if (!debugChannelSet(-1, kDebugImGui)) {
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}

	if (!_state)
		return;

	io.ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	Ultima8Engine *engine = Ultima8Engine::get_instance();

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Toggles")) {
			if (ImGui::MenuItem("Cheats", NULL, engine->areCheatsEnabled())) {
				bool flag = engine->areCheatsEnabled();
				engine->setCheatMode(!flag);
			}
			if (ImGui::MenuItem("Editor Items", NULL, engine->isShowEditorItems())) {
				bool flag = engine->isShowEditorItems();
				engine->setShowEditorItems(!flag);
			}
			if (ImGui::MenuItem("Footpads", NULL, GameMapGump::getShowFootpads())) {
				bool flag = GameMapGump::getShowFootpads();
				GameMapGump::setShowFootpads(!flag);
			}
			if (ImGui::BeginMenu("Gridlines")) {
				int gridlines = GameMapGump::getGridlines();
				if (ImGui::MenuItem("Auto", NULL, gridlines == -1)) {
					GameMapGump::setGridlines(gridlines == -1 ? 0 : -1);
				}
				if (ImGui::MenuItem("128 x 128", NULL, gridlines == 128)) {
					GameMapGump::setGridlines(gridlines == 128 ? 0 : 128);
				}
				if (ImGui::MenuItem("256 x 256", NULL, gridlines == 256)) {
					GameMapGump::setGridlines(gridlines == 256 ? 0 : 256);
				}
				if (ImGui::MenuItem("512 x 512", NULL, gridlines == 512)) {
					GameMapGump::setGridlines(gridlines == 512 ? 0 : 512);
				}
				if (ImGui::MenuItem("1024 x 1024", NULL, gridlines == 1024)) {
					GameMapGump::setGridlines(gridlines == 1024 ? 0 : 1024);
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Hack Mover", NULL, engine->isHackMoverEnabled())) {
				bool flag = engine->isHackMoverEnabled();
				engine->setHackMoverEnabled(!flag);
			}
			if (ImGui::MenuItem("Quick Movement", NULL, QuickAvatarMoverProcess::isEnabled())) {
				bool flag = QuickAvatarMoverProcess::isEnabled();
				QuickAvatarMoverProcess::setEnabled(!flag);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Item Stats", NULL, &_state->_itemStatsWindow);
			ImGui::MenuItem("Palette", NULL, &_state->_paletteWindow);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	showItemStats();
	showPalette();
}

void onImGuiCleanup() {
	delete _state;
	_state = nullptr;
}

} // End of namespace Ultima8
} // End of namespace Ultima

