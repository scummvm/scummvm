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

#include "comfy/comfy.h"

#include "common/endian.h"

#ifdef USE_IMGUI
#include "backends/imgui/imgui.h"
#endif

namespace Comfy {

#ifdef USE_IMGUI

static void debugIncludeMapPoint(int16 &mapLeft, int16 &mapTop, int16 &mapRight, int16 &mapBottom, int16 x, int16 y) {
	mapLeft = MIN(mapLeft, x);
	mapTop = MIN(mapTop, y);
	mapRight = MAX(mapRight, x);
	mapBottom = MAX(mapBottom, y);
}

static void debugIncludeMapRect(int16 &mapLeft, int16 &mapTop, int16 &mapRight, int16 &mapBottom,
		int16 left, int16 top, int16 right, int16 bottom) {
	debugIncludeMapPoint(mapLeft, mapTop, mapRight, mapBottom, left, top);
	debugIncludeMapPoint(mapLeft, mapTop, mapRight, mapBottom, right, bottom);
}

static ImVec2 debugMapPointToCanvas(const ImVec2 &canvasPos, int16 mapLeft, int16 mapTop,
		float scale, float margin, int16 x, int16 y) {
	return ImVec2(canvasPos.x + ((float)x - mapLeft) * scale + margin,
		canvasPos.y + ((float)y - mapTop) * scale + margin);
}

bool ComfyEngine::debugScriptHasRange(uint32 pc, uint32 width) {
	uint32 tileOffset = pc % COMFY_TILE_SIZE;
	uint32 tileBase = pc - tileOffset;
	uint32 tileSize = tileBase < _picDataSize ? MIN<uint32>(COMFY_TILE_SIZE, _picDataSize - tileBase) : 0;

	return tileBase <= _comfyObjData.size() && tileSize <= _comfyObjData.size() - tileBase &&
		tileOffset <= tileSize && width <= tileSize - tileOffset;
}

uint16 ComfyEngine::debugScriptReadWord(uint32 pc) {
	return debugScriptHasRange(pc, 2) ? READ_LE_UINT16(&_comfyObjData[pc]) : (uint16)0;
}

void ComfyEngine::drawActorDebugUi(bool *visible) {
	if (!visible || !*visible)
		return;

	ImGui::SetNextWindowPos(ImVec2(760, 8), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(900, 760), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Comfy Actors", visible)) {
		ImGui::End();
		return;
	}

	ImGui::Text("Game: %s  version: %u  actors: %u", getGameId().c_str(), _engineVersion, (uint)_actors.size());
	ImGui::TextDisabled("Tip: disable Visible/Active to isolate a problematic actor.");
	bool centerFramebuffer = ImGui::Button("Center framebuffer");
	ImGui::SameLine();
	ImGui::Checkbox("Tree view", &_state->_actorDebugTreeView);
	if (_state->_actorDebugLastScene != _currentScene ||
			_state->_actorDebugLastSceneGeneration != _debugSceneGeneration) {
		centerFramebuffer = true;
		_state->_actorDebugLastScene = _currentScene;
		_state->_actorDebugLastSceneGeneration = _debugSceneGeneration;
	}
	ImGui::Separator();

	ImGui::BeginChild("comfy_actor_map", ImVec2(0, 320), true, ImGuiWindowFlags_HorizontalScrollbar);
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	int16 mapLeft = 0;
	int16 mapTop = 0;
	int16 mapRight = _logicalScreenWidth;
	int16 mapBottom = _logicalScreenHeight;

	for (uint i = 0; i < _actors.size(); i++) {
		Actor &actor = _actors[i];
		uint16 sceneHandle = actorReadU16(actor, kActorSceneHandle);
		if (!sceneHandle && i != 0)
			continue;

		int32 xFixed = (int32)actorReadU32(actor, kActorXFixed);
		int32 yFixed = (int32)actorReadU32(actor, kActorYFixed);
		uint16 parent = actorReadU16(actor, kActorParent);
		uint guard = 0;
		while (parent && parent < _actors.size() && guard++ < COMFY_ACTOR_COUNT) {
			Actor *parentActor = actorGetPtr(parent);
			if (!parentActor)
				break;

			xFixed += (int32)actorReadU32(*parentActor, kActorXFixed);
			yFixed += (int32)actorReadU32(*parentActor, kActorYFixed);
			parent = actorReadU16(*parentActor, kActorParent);
		}

		int16 x = (int16)(xFixed >> 12);
		int16 y = (int16)(yFixed >> 12);
		debugIncludeMapPoint(mapLeft, mapTop, mapRight, mapBottom, x, y);

		uint32 selector = actorReadU32(actor, kActorSpriteSelector);
		if (selector && !(selector & 0xFF000000) && selector != 0x00FFFFFF &&
				selector < _spriteResources.size() && _spriteResources[selector].loaded) {
			SpriteResource &sprite = _spriteResources[selector];
			debugIncludeMapRect(mapLeft, mapTop, mapRight, mapBottom,
				x - sprite.header.hotspotX, y - sprite.header.hotspotY,
				x - sprite.header.hotspotX + sprite.header.width,
				y - sprite.header.hotspotY + sprite.header.height);
		} else if (selector & 0xFF000000) {
			uint32 pc = selector & 0x00FFFFFF;
			int16 wordsLeft = (int16)(debugScriptReadWord(pc) - 1);
			pc += 2;
			while (wordsLeft > 0 && debugScriptHasRange(pc, 6)) {
				uint16 rawSpriteId = debugScriptReadWord(pc);
				int16 dx = debugScriptReadWord(pc + 2);
				int16 dy = debugScriptReadWord(pc + 4);
				pc += 6;
				wordsLeft -= 3;
				uint16 spriteId = rawSpriteId;
				if (spriteId & 0x8000) {
					if (!debugScriptHasRange(pc, 4))
						break;

					wordsLeft -= 2;
					uint16 key = debugScriptReadWord(pc);
					pc += 2;
					if (keyBitTest(key))
						spriteId &= 0x7FFF;
					else
						spriteId = debugScriptReadWord(pc);

					pc += 2;
				}

				int16 limbX = x + dx;
				int16 limbY = y + dy;
				debugIncludeMapPoint(mapLeft, mapTop, mapRight, mapBottom, limbX, limbY);
				if (spriteId && spriteId < _spriteResources.size() && _spriteResources[spriteId].loaded) {
					SpriteResource &sprite = _spriteResources[spriteId];
					debugIncludeMapRect(mapLeft, mapTop, mapRight, mapBottom,
						limbX - sprite.header.hotspotX, limbY - sprite.header.hotspotY,
						limbX - sprite.header.hotspotX + sprite.header.width,
						limbY - sprite.header.hotspotY + sprite.header.height);
				}
			}
		}
	}

	const float margin = 32.0F;
	float scale = 1.0F;
	ImVec2 canvasSize((float)(mapRight - mapLeft) * scale + margin * 2.0F,
		(float)(mapBottom - mapTop) * scale + margin * 2.0F);

	ImDrawList *drawList = ImGui::GetWindowDrawList();
	drawList->PushClipRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);
	drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(18, 20, 24, 230));
	drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(180, 180, 180, 180));
	ImVec2 screenTopLeft = debugMapPointToCanvas(canvasPos, mapLeft, mapTop, scale, margin, 0, 0);
	ImVec2 screenBottomRight = debugMapPointToCanvas(canvasPos, mapLeft, mapTop, scale, margin,
		_logicalScreenWidth, _logicalScreenHeight);
	drawList->AddRectFilled(screenTopLeft, screenBottomRight, IM_COL32(38, 43, 54, 220));
	drawList->AddRect(screenTopLeft, screenBottomRight, IM_COL32(110, 170, 240, 230));
	for (uint i = 0; i < _actors.size(); i++) {
		Actor &actor = _actors[i];
		uint16 sceneHandle = actorReadU16(actor, kActorSceneHandle);
		if (!sceneHandle && i != 0)
			continue;

		int32 xFixed = (int32)actorReadU32(actor, kActorXFixed);
		int32 yFixed = (int32)actorReadU32(actor, kActorYFixed);
		uint16 parent = actorReadU16(actor, kActorParent);
		uint guard = 0;
		while (parent && parent < _actors.size() && guard++ < COMFY_ACTOR_COUNT) {
			Actor *parentActor = actorGetPtr(parent);
			if (!parentActor)
				break;

			xFixed += (int32)actorReadU32(*parentActor, kActorXFixed);
			yFixed += (int32)actorReadU32(*parentActor, kActorYFixed);
			parent = actorReadU16(*parentActor, kActorParent);
		}

		int16 x = (int16)(xFixed >> 12);
		int16 y = (int16)(yFixed >> 12);
		bool visibleActor = actorReadU8(actor, kActorVisible) != 0;
		bool activeActor = actorReadU8(actor, kActorActive) != 0;
		ImU32 color = visibleActor ? (activeActor ? IM_COL32(82, 220, 120, 230) : IM_COL32(238, 185, 74, 230)) :
			IM_COL32(120, 120, 120, 170);

		ImVec2 point = debugMapPointToCanvas(canvasPos, mapLeft, mapTop, scale, margin, x, y);
		drawList->AddCircleFilled(point, 3.0F, color);

		char actorLabel[16];
		snprintf(actorLabel, sizeof(actorLabel), "%u", i);
		drawList->AddText(ImVec2(point.x + 4.0F, point.y - 4.0F), color, actorLabel);

		uint32 selector = actorReadU32(actor, kActorSpriteSelector);
		if (visibleActor && selector && !(selector & 0xFF000000) && selector != 0x00FFFFFF &&
				selector < _spriteResources.size() && _spriteResources[selector].loaded) {
			SpriteResource &sprite = _spriteResources[selector];
			ImVec2 topLeft = debugMapPointToCanvas(canvasPos, mapLeft, mapTop, scale, margin,
				x - sprite.header.hotspotX, y - sprite.header.hotspotY);
			ImVec2 bottomRight = debugMapPointToCanvas(canvasPos, mapLeft, mapTop, scale, margin,
				x - sprite.header.hotspotX + sprite.header.width,
				y - sprite.header.hotspotY + sprite.header.height);
			drawList->AddRect(topLeft, bottomRight, color);
		} else if (selector & 0xFF000000) {
			uint32 pc = selector & 0x00FFFFFF;
			int16 wordsLeft = (int16)(debugScriptReadWord(pc) - 1);
			pc += 2;
			while (wordsLeft > 0 && debugScriptHasRange(pc, 6)) {
				uint16 rawSpriteId = debugScriptReadWord(pc);
				int16 dx = debugScriptReadWord(pc + 2);
				int16 dy = debugScriptReadWord(pc + 4);
				pc += 6;
				wordsLeft -= 3;
				uint16 spriteId = rawSpriteId;
				if (spriteId & 0x8000) {
					if (!debugScriptHasRange(pc, 4))
						break;

					wordsLeft -= 2;
					uint16 key = debugScriptReadWord(pc);
					pc += 2;
					if (keyBitTest(key))
						spriteId &= 0x7FFF;
					else
						spriteId = debugScriptReadWord(pc);

					pc += 2;
				}

				int16 limbX = x + dx;
				int16 limbY = y + dy;
				ImVec2 limbPoint = debugMapPointToCanvas(canvasPos, mapLeft, mapTop, scale, margin, limbX, limbY);
				drawList->AddCircleFilled(limbPoint, 2.0F, IM_COL32(120, 205, 255, 220));
				if (spriteId && spriteId < _spriteResources.size() && _spriteResources[spriteId].loaded) {
					SpriteResource &sprite = _spriteResources[spriteId];
					ImVec2 topLeft = debugMapPointToCanvas(canvasPos, mapLeft, mapTop, scale, margin,
						limbX - sprite.header.hotspotX, limbY - sprite.header.hotspotY);
					ImVec2 bottomRight = debugMapPointToCanvas(canvasPos, mapLeft, mapTop, scale, margin,
						limbX - sprite.header.hotspotX + sprite.header.width,
						limbY - sprite.header.hotspotY + sprite.header.height);
					drawList->AddRect(topLeft, bottomRight, IM_COL32(120, 205, 255, 180));
				}
			}
		}
	}

	ImGui::Dummy(canvasSize);
	if (centerFramebuffer) {
		ImVec2 childSize = ImGui::GetWindowSize();
		float targetX = ((float)-mapLeft + (float)_logicalScreenWidth / 2.0F) * scale + margin - childSize.x / 2.0F;
		float targetY = ((float)-mapTop + (float)_logicalScreenHeight / 2.0F) * scale + margin - childSize.y / 2.0F;
		ImGui::SetScrollX(MAX(0.0F, targetX));
		ImGui::SetScrollY(MAX(0.0F, targetY));
	}
	drawList->PopClipRect();
	ImGui::EndChild();
	ImGui::Separator();

	ImGui::BeginChild("comfy_actor_table_child", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
	if (_state->_actorDebugTreeView) {
		bool visited[COMFY_ACTOR_COUNT];
		memset(visited, 0, sizeof(visited));
		uint16 actorStack[COMFY_ACTOR_COUNT * 2];
		uint stackCount = 0;

		if (_actors.size()) {
			actorStack[stackCount] = 0;
			stackCount++;
			visited[0] = true;
		}

		while (stackCount) {
			stackCount--;
			uint16 actorIndex = actorStack[stackCount];
			if (actorIndex == 0xFFFF) {
				ImGui::TreePop();
				continue;
			}

			if (actorIndex >= _actors.size())
				continue;

			Actor &actor = _actors[actorIndex];
			uint16 sceneHandle = actorReadU16(actor, kActorSceneHandle);
			if (!sceneHandle && actorIndex != 0)
				continue;

			uint16 children[COMFY_ACTOR_COUNT];
			uint childCount = 0;
			uint16 child = actorReadU16(actor, kActorChildHead);
			uint guard = 0;
			while (child && child < _actors.size() && child < COMFY_ACTOR_COUNT &&
					childCount < COMFY_ACTOR_COUNT && guard++ < COMFY_ACTOR_COUNT) {
				if (!visited[child]) {
					children[childCount] = child;
					childCount++;
					visited[child] = true;
				}

				Actor *childActor = actorGetPtr(child);
				if (!childActor)
					break;

				child = actorReadU16(*childActor, kActorSiblingHead);
			}

			uint32 selector = actorReadU32(actor, kActorSpriteSelector);
			int32 xFixed = (int32)actorReadU32(actor, kActorXFixed);
			int32 yFixed = (int32)actorReadU32(actor, kActorYFixed);
			uint16 parent = actorReadU16(actor, kActorParent);
			guard = 0;
			while (parent && parent < _actors.size() && guard++ < COMFY_ACTOR_COUNT) {
				Actor *parentActor = actorGetPtr(parent);
				if (!parentActor)
					break;

				xFixed += (int32)actorReadU32(*parentActor, kActorXFixed);
				yFixed += (int32)actorReadU32(*parentActor, kActorYFixed);
				parent = actorReadU16(*parentActor, kActorParent);
			}

			int16 x = (int16)(xFixed >> 12);
			int16 y = (int16)(yFixed >> 12);
			bool visibleActor = actorReadU8(actor, kActorVisible) != 0;
			bool activeActor = actorReadU8(actor, kActorActive) != 0;
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (!childCount)
				flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			ImGui::PushID((int)actorIndex);
			bool open = ImGui::TreeNodeEx("actor", flags, "%u", actorIndex);

			ImGui::SameLine(120.0F);
			ImGui::Text("scene=%u pos=%d,%d selector=0x%08X move=%d",
				sceneHandle, x, y, selector, (int16)actorReadU16(actor, kActorMoveTicks));

			ImGui::SameLine(520.0F);
			if (ImGui::Checkbox("Visible", &visibleActor)) {
				actorWriteU8(actor, kActorVisible, visibleActor ? 1 : 0);
				renderSetDirty();
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Active", &activeActor))
				actorWriteU8(actor, kActorActive, activeActor ? 1 : 0);
			ImGui::PopID();

			if (open && childCount) {
				if (stackCount < ARRAYSIZE(actorStack)) {
					actorStack[stackCount] = 0xFFFF;
					stackCount++;
				}

				while (childCount && stackCount < ARRAYSIZE(actorStack)) {
					childCount--;
					actorStack[stackCount] = children[childCount];
					stackCount++;
				}
			}

			if (!stackCount) {
				for (uint i = 0; i < _actors.size() && i < COMFY_ACTOR_COUNT &&
						stackCount < ARRAYSIZE(actorStack); i++) {
					Actor &orphanActor = _actors[i];
					if (visited[i] || (!actorReadU16(orphanActor, kActorSceneHandle) && i != 0))
						continue;

					actorStack[stackCount] = i;
					stackCount++;
					visited[i] = true;
					break;
				}
			}
		}
	} else if (ImGui::BeginTable("comfy_actor_table", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX, ImVec2(0, 0))) {
		ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 32.0F);
		ImGui::TableSetupColumn("Scene", ImGuiTableColumnFlags_WidthFixed, 48.0F);
		ImGui::TableSetupColumn("Parent", ImGuiTableColumnFlags_WidthFixed, 52.0F);
		ImGui::TableSetupColumn("Abs X", ImGuiTableColumnFlags_WidthFixed, 48.0F);
		ImGui::TableSetupColumn("Abs Y", ImGuiTableColumnFlags_WidthFixed, 48.0F);
		ImGui::TableSetupColumn("Selector", ImGuiTableColumnFlags_WidthFixed, 88.0F);
		ImGui::TableSetupColumn("PC", ImGuiTableColumnFlags_WidthFixed, 80.0F);
		ImGui::TableSetupColumn("Visible", ImGuiTableColumnFlags_WidthFixed, 58.0F);
		ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 52.0F);
		ImGui::TableSetupColumn("Move", ImGuiTableColumnFlags_WidthFixed, 44.0F);
		ImGui::TableHeadersRow();

		for (uint i = 0; i < _actors.size(); i++) {
			Actor &actor = _actors[i];
			uint16 sceneHandle = actorReadU16(actor, kActorSceneHandle);
			if (!sceneHandle && i != 0)
				continue;

			uint32 selector = actorReadU32(actor, kActorSpriteSelector);
			int32 xFixed = (int32)actorReadU32(actor, kActorXFixed);
			int32 yFixed = (int32)actorReadU32(actor, kActorYFixed);
			uint16 parent = actorReadU16(actor, kActorParent);
			uint guard = 0;
			while (parent && parent < _actors.size() && guard++ < COMFY_ACTOR_COUNT) {
				Actor *parentActor = actorGetPtr(parent);
				if (!parentActor)
					break;

				xFixed += (int32)actorReadU32(*parentActor, kActorXFixed);
				yFixed += (int32)actorReadU32(*parentActor, kActorYFixed);
				parent = actorReadU16(*parentActor, kActorParent);
			}

			int16 x = (int16)(xFixed >> 12);
			int16 y = (int16)(yFixed >> 12);
			bool visibleActor = actorReadU8(actor, kActorVisible) != 0;
			bool activeActor = actorReadU8(actor, kActorActive) != 0;

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			bool open = ImGui::TreeNodeEx((void *)(uintptr)i, ImGuiTreeNodeFlags_SpanFullWidth, "%u", i);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%u", sceneHandle);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%u", actorReadU16(actor, kActorParent));
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%d", x);
			ImGui::TableSetColumnIndex(4);
			ImGui::Text("%d", y);
			ImGui::TableSetColumnIndex(5);
			ImGui::Text("0x%08X", selector);
			ImGui::TableSetColumnIndex(6);
			ImGui::Text("0x%08X", actorReadU32(actor, kActorCurrentPc));
			ImGui::TableSetColumnIndex(7);
			ImGui::PushID((int)i * 2);
			if (ImGui::Checkbox("", &visibleActor)) {
				actorWriteU8(actor, kActorVisible, visibleActor ? 1 : 0);
				renderSetDirty();
			}
			ImGui::PopID();
			ImGui::TableSetColumnIndex(8);
			ImGui::PushID((int)i * 2 + 1);
			if (ImGui::Checkbox("", &activeActor))
				actorWriteU8(actor, kActorActive, activeActor ? 1 : 0);
			ImGui::PopID();
			ImGui::TableSetColumnIndex(9);
			ImGui::Text("%d", (int16)actorReadU16(actor, kActorMoveTicks));

			if (open) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("links prev=%u next=%u child=%u sibling=%u tail=%u",
					actorReadU16(actor, kActorPrevLink), actorReadU16(actor, kActorNextLink),
					actorReadU16(actor, kActorChildHead), actorReadU16(actor, kActorSiblingHead),
					actorReadU16(actor, kActorChildTail));
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("local fixed x=0x%08X y=0x%08X moveDx=0x%08X moveDy=0x%08X",
					actorReadU32(actor, kActorXFixed), actorReadU32(actor, kActorYFixed),
					actorReadU32(actor, kActorMoveDx), actorReadU32(actor, kActorMoveDy));
				if (selector & 0xFF000000) {
					uint32 pc = selector & 0x00FFFFFF;
					int16 wordsLeft = (int16)(debugScriptReadWord(pc) - 1);
					pc += 2;
					uint limb = 0;
					while (wordsLeft > 0 && limb < 16 && debugScriptHasRange(pc, 6)) {
						uint16 rawSpriteId = debugScriptReadWord(pc);
						int16 dx = debugScriptReadWord(pc + 2);
						int16 dy = debugScriptReadWord(pc + 4);
						pc += 6;
						wordsLeft -= 3;
						uint16 spriteId = rawSpriteId;
						uint16 key = 0;
						if (spriteId & 0x8000) {
							if (!debugScriptHasRange(pc, 4))
								break;

							wordsLeft -= 2;
							key = debugScriptReadWord(pc);
							pc += 2;
							if (keyBitTest(key))
								spriteId &= 0x7FFF;
							else
								spriteId = debugScriptReadWord(pc);

							pc += 2;
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("limb %u raw=0x%04X sprite=%u key=%u dx=%d dy=%d final=%d,%d",
							limb, rawSpriteId, spriteId, key, dx, dy, x + dx, y + dy);
						limb++;
					}
				}

				ImGui::TreePop();
			}
		}

		ImGui::EndTable();
	}
	ImGui::EndChild();

	ImGui::End();
}


#endif

} // End of namespace Comfy
