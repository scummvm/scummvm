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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "graphics/imgui/imgui.h"

#include "common/debug-channels.h"
#include "twp/twp.h"
#include "twp/debugtools.h"
#include "twp/thread.h"
#include "twp/lighting.h"
#include "twp/squtil.h"

namespace Twp {

static struct {
	bool _showThreads = false;
	bool _showObjects = false;
	bool _showStack = false;
	bool _showAudio = false;
	bool _showResources = false;
	bool _showScenegraph = false;
	bool _showActor = false;
	Node *_node = nullptr;
	ImGuiTextFilter _objFilter;
	ImGuiTextFilter _actorFilter;
	int _fadeEffect = 0;
	float _fadeDuration = 0.f;
	bool _fadeToSepia = false;
	Common::String _textureSelected;
	int _selectedActor = 0;
} state;

ImVec4 gray(0.6f, 0.6f, 0.6f, 1.f);

static void drawThreads() {
	if (!state._showThreads)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	const auto &threads = g_twp->_threads;
	if (ImGui::Begin("Threads", &state._showThreads)) {
		ImGui::Text("# threads: %u", threads.size());
		ImGui::Separator();

		if (ImGui::BeginTable("Threads", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("Id");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Type");
			ImGui::TableSetupColumn("Func");
			ImGui::TableSetupColumn("Src");
			ImGui::TableSetupColumn("Line");
			ImGui::TableHeadersRow();

			if (g_twp->_cutscene) {
				Common::SharedPtr<ThreadBase> thread(g_twp->_cutscene);
				SQStackInfos infos;
				sq_stackinfos(thread->getThread(), 0, &infos);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%5d", thread->getId());
				ImGui::TableNextColumn();
				ImGui::Text("%-56s", thread->getName().c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%-6s", "cutscene");
				ImGui::TableNextColumn();
				ImGui::Text("%-9s", infos.funcname);
				ImGui::TableNextColumn();
				ImGui::Text("%-9s", infos.source);
				ImGui::TableNextColumn();
				ImGui::Text("%5lld", infos.line);
			}

			for (const auto &thread : threads) {
				SQStackInfos infos;
				sq_stackinfos(thread->getThread(), 0, &infos);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%5d", thread->getId());
				ImGui::TableNextColumn();
				ImGui::Text("%-56s", thread->getName().c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%-6s", thread->isGlobal() ? "global" : "local");
				ImGui::TableNextColumn();
				ImGui::Text("%-9s", infos.funcname);
				ImGui::TableNextColumn();
				ImGui::Text("%-9s", infos.source);
				ImGui::TableNextColumn();
				ImGui::Text("%5lld", infos.line);
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

static void drawObjects() {
	if (!state._showObjects)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Objects", &state._showObjects);
	state._objFilter.Draw();

	// show object list
	for (auto &layer : g_twp->_room->_layers) {
		for (auto &obj : layer->_objects) {
			if (state._objFilter.PassFilter(obj->_key.c_str())) {
				ImGui::PushID(obj->getId());
				bool visible = obj->_node->isVisible();
				if (ImGui::Checkbox("", &visible)) {
					obj->_node->setVisible(visible);
				}
				ImGui::SameLine();
				Common::String name = obj->_key.empty() ? obj->getName() : Common::String::format("%s(%s) %d", obj->getName().c_str(), obj->_key.c_str(), obj->getId());
				bool selected = false;
				if (ImGui::Selectable(name.c_str(), &selected)) {
					// gObject = obj;
				}
				ImGui::PopID();
			}
		}
	}

	ImGui::End();
}

static Common::String toString(Audio::Mixer::SoundType type) {
	switch (type) {
	case Audio::Mixer::kPlainSoundType:
		return "sound";
	case Audio::Mixer::kMusicSoundType:
		return "music";
	case Audio::Mixer::kSFXSoundType:
		return "sfx";
	case Audio::Mixer::kSpeechSoundType:
		return "speech";
	}
	return "?";
}

static ImVec4 getCategoryColor(Audio::Mixer::SoundType type) {
	switch (type) {
	case Audio::Mixer::kPlainSoundType:
		return ImVec4(0.f, 1.f, 0.f, 1.f);
	case Audio::Mixer::kMusicSoundType:
		return ImVec4(1.f, 0.f, 0.f, 1.f);
	case Audio::Mixer::kSFXSoundType:
		return ImVec4(1.f, 0.f, 1.f, 1.f);
	case Audio::Mixer::kSpeechSoundType:
		return ImVec4(1.f, 1.f, 0.f, 1.f);
	}
	return ImVec4(1.f, 1.f, 1.f, 1.f);
}

static void drawActors() {
	if (!state._showActor)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Actors", &state._showStack);
	state._actorFilter.Draw();
	ImGui::BeginChild("Actor_List");
	for (auto &actor : g_twp->_actors) {
		bool selected = actor->getId() == state._selectedActor;
		Common::String key(actor->_key);
		if (state._actorFilter.PassFilter(actor->_key.c_str())) {
			if (key.empty()) {
				key = "??";
			}
			if (ImGui::Selectable(key.c_str(), &selected)) {
				state._selectedActor = actor->getId();
			}
		}
	}
	ImGui::EndChild();
	ImGui::End();
}

static void drawActor() {
	if (!state._showActor)
		return;

	Common::SharedPtr<Object> actor(sqobj(state._selectedActor));
	if (!actor)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Actor", &state._showStack);
	ImGui::Text("Name: %s", actor->_key.c_str());
	ImGui::Text("Costume: %s (%s)", actor->_costumeName.c_str(), actor->_costumeSheet.c_str());
	ImGui::Text("Animation: %s", actor->_animName.c_str());
	Common::String hiddenLayers(Twp::join(actor->_hiddenLayers, ", "));
	ImGui::Text("Hidden Layers: %s", hiddenLayers.c_str());
	ImGui::Text("Facing: %d", actor->_facing);
	ImGui::Text("Facing Lock: %d", actor->_facingLockValue);
	ImGui::ColorEdit3("Talk color", actor->_talkColor.v);
	ImGui::DragFloat2("Talk offset", actor->_talkOffset.getData());
	ImGui::End();
}

static void drawStack() {
	if (!state._showStack)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Stack", &state._showStack);
	ImGui::BeginChild("ScrollingRegion");
	SQInteger size = sq_gettop(g_twp->getVm());
	ImGui::Text("size: %lld", size);
	HSQOBJECT obj;
	for (SQInteger i = 1; i < size; i++) {
		sq_getstackobj(g_twp->getVm(), -i, &obj);
		ImGui::Text("obj type: 0x%X", obj._type);
	}
	ImGui::EndChild();
	ImGui::End();
}

static void drawResources() {
	if (!state._showResources)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Resources", &state._showResources);

	if (ImGui::BeginTable("Resources", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Resolution");
		ImGui::TableHeadersRow();

		for (auto &res : g_twp->_resManager._textures) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			bool selected = state._textureSelected == res._key;
			if (ImGui::Selectable(res._key.c_str(), selected)) {
				state._textureSelected = res._key;
			}
			ImGui::TableNextColumn();
			ImGui::Text("%s", Common::String::format("%d x %d", res._value.width, res._value.height).c_str());
		}

		ImGui::EndTable();
	}
	ImGui::Separator();

	ImVec2 cursor = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + 10.f));
	ImGui::Text("Preview:");
	ImGui::BeginChild("TexturePreview", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY);
	for (auto &res : g_twp->_resManager._textures) {
		if (state._textureSelected == res._key) {
			ImGui::Image((ImTextureID)(intptr_t)res._value.id, ImVec2(res._value.width, res._value.height));
			break;
		}
	}
	ImGui::EndChild();

	ImGui::End();
}

static void drawAudio() {
	if (!state._showAudio)
		return;

	// count the number of active sounds
	int count = 0;
	for (auto &s : g_twp->_audio._slots) {
		if (s.busy)
			count++;
	}

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Sounds", &state._showAudio);
	ImGui::Text("# sounds: %d/%d", count, NUM_AUDIO_SLOTS);
	ImGui::Separator();

	if (ImGui::BeginTable("Threads", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupColumn("");
		ImGui::TableSetupColumn("Id");
		ImGui::TableSetupColumn("Category");
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Loops");
		ImGui::TableSetupColumn("Volume");
		ImGui::TableSetupColumn("Pan");
		ImGui::TableHeadersRow();

		for (int i = 0; i < NUM_AUDIO_SLOTS; i++) {
			auto &sound = g_twp->_audio._slots[i];
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("#%d", i);
			if (sound.busy) {
				float pan = g_twp->_mixer->getChannelBalance(sound.handle) / 128.f;
				float vol = g_twp->_mixer->getChannelVolume(sound.handle) / 255.f;
				ImGui::TableNextColumn();
				ImGui::Text("%d", sound.id);
				ImGui::TableNextColumn();
				ImGui::TextColored(getCategoryColor(sound.soundType), "%s", toString(sound.soundType).c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%s", sound.sndDef->getName().c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%d", sound.loopTimes);
				ImGui::TableNextColumn();
				ImGui::Text("%0.1f", vol);
				ImGui::TableNextColumn();
				ImGui::Text("%0.1f", pan);
				ImGui::SameLine();
				if (ImGui::SmallButton("STOP")) {
					g_twp->_audio.stop(sound.id);
				}
			}
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

static void drawGeneral() {
	ImGuiIO &io = ImGui::GetIO();

	ImGui::Begin("General");

	SQInteger size = sq_gettop(g_twp->getVm());
	ImGui::TextColored(gray, "Stack:");
	ImGui::SameLine();
	ImGui::Text("%lld", size);
	ImGui::TextColored(gray, "Cutscene:");
	ImGui::SameLine();
	ImGui::Text("%s", g_twp->_cutscene ? g_twp->_cutscene->getName().c_str() : "no");
	DialogState dialogState = g_twp->_dialog.getState();
	ImGui::TextColored(gray, "In dialog:");
	ImGui::SameLine();
	ImGui::Text("%s", ((dialogState == Active) ? "yes" : (dialogState == WaitingForChoice ? "waiting for choice" : "no")));
	ImGui::TextColored(gray, "Verb:");
	ImGui::SameLine();
	Common::String verb = g_twp->getTextDb().getText(g_twp->_hud._verb.text);
	ImGui::Text("%s %d", verb.c_str(), g_twp->_hud._verb.id.id);

	auto mousePos = g_twp->_cursor.pos;
	ImGui::TextColored(gray, "Pos (screen):");
	ImGui::SameLine();
	ImGui::Text("(%.0f, %0.f)", mousePos.getX(), mousePos.getY());
	if (g_twp->_room) {
		auto pos = g_twp->screenToRoom(mousePos);
		ImGui::TextColored(gray, "Pos (room):");
		ImGui::SameLine();
		ImGui::Text("(%.0f, %0.f)", pos.getX(), pos.getY());
	}
	ImGui::Separator();
	ImGui::Checkbox("HUD", &g_twp->_inputState._inputHUD);
	ImGui::SameLine();
	ImGui::Checkbox("Input", &g_twp->_inputState._inputActive);
	ImGui::SameLine();
	ImGui::Checkbox("Cursor", &g_twp->_inputState._showCursor);
	ImGui::SameLine();
	ImGui::Checkbox("Verbs", &g_twp->_inputState._inputVerbsActive);
	ImGui::SameLine();
	ImGui::Checkbox("Allow SaveGame", &g_twp->_saveGameManager._allowSaveGame);

	ImGui::Separator();
	bool isSwitcherOn = g_twp->_actorSwitcher._mode == asOn;
	if (ImGui::Checkbox("Switcher ON", &isSwitcherOn)) {
		if (isSwitcherOn) {
			g_twp->_actorSwitcher._mode |= asOn;
		} else {
			g_twp->_actorSwitcher._mode &= ~asOn;
		}
	}
	bool isTemporaryUnselectable = g_twp->_actorSwitcher._mode & asTemporaryUnselectable;
	if (ImGui::Checkbox("Switcher Temp. Unselectable", &isTemporaryUnselectable)) {
		if (isTemporaryUnselectable) {
			g_twp->_actorSwitcher._mode |= asTemporaryUnselectable;
		} else {
			g_twp->_actorSwitcher._mode &= ~asTemporaryUnselectable;
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Debug")) {
		bool allEnabled = true;
		auto channels = DebugMan.getDebugChannels();
		for (auto &channel : channels) {
			bool enabled = DebugMan.isDebugChannelEnabled(channel.channel);
			allEnabled &= enabled;
			if (ImGui::Checkbox(channel.name.c_str(), &enabled)) {
				if (enabled) {
					DebugMan.enableDebugChannel(channel.channel);
				} else {
					DebugMan.disableDebugChannel(channel.channel);
				}
			}
		}
		ImGui::Separator();
		if (ImGui::Checkbox("All", &allEnabled)) {
			if (allEnabled) {
				DebugMan.enableAllDebugChannels();
			} else {
				DebugMan.disableAllDebugChannels();
			}
		}
	}

	// Camera
	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::TextColored(gray, "follow:");
		ImGui::SameLine();
		ImGui::Text("%s", !g_twp->_followActor ? "(none)" : g_twp->_followActor->_key.c_str());
		ImGui::TextColored(gray, "moving:");
		ImGui::SameLine();
		ImGui::Text("%s", g_twp->_camera.isMoving() ? "yes" : "no");
		auto halfScreenSize = g_twp->_room->getScreenSize() / 2.0f;
		auto camPos = g_twp->cameraPos() - halfScreenSize;
		if (ImGui::DragFloat2("Camera pos", camPos.getData())) {
			g_twp->follow(nullptr);
			g_twp->cameraAt(camPos);
		}
		auto bounds = g_twp->_camera.getBounds();
		if (ImGui::DragFloat4("Bounds", bounds.v)) {
			g_twp->_camera.setBounds(bounds);
		}
	}

	// Room
	Common::SharedPtr<Room> room = g_twp->_room;
	if (room) {
		if (ImGui::CollapsingHeader("Room")) {
			ImGui::TextColored(gray, "Sheet:");
			ImGui::SameLine();
			ImGui::Text("%s", room->_sheet.c_str());
			ImGui::TextColored(gray, "Size:");
			ImGui::SameLine();
			ImGui::Text("%0.f x %0.f", room->_roomSize.getX(), room->_roomSize.getY());
			ImGui::TextColored(gray, "Fullscreen:");
			ImGui::SameLine();
			ImGui::Text("%d", room->_fullscreen);
			ImGui::TextColored(gray, "Height:");
			ImGui::SameLine();
			ImGui::Text("%d", room->_height);
			Color overlay = room->_overlayNode.getOverlayColor();
			if (ImGui::ColorEdit4("Overlay", overlay.v))
				room->_overlayNode.setOverlayColor(overlay);
			ImGui::Checkbox("Debug Lights", &g_twp->_lighting->_debug);
			ImGui::ColorEdit4("Ambient Light", room->_lights._ambientLight.v);
			for (int i = 0; i < room->_lights._numLights; ++i) {
				Common::String ss = Common::String::format("Light %d", i + 1);
				if (ImGui::TreeNode(ss.c_str())) {
					auto &light = room->_lights._lights[i];
					ImGui::DragFloat2("Position", light.pos.getData());
					ImGui::ColorEdit4("Color", light.color.v);
					ImGui::DragFloat("Direction angle", &light.coneDirection, 1.0f, 0.0f, 360.f);
					ImGui::DragFloat("Angle", &light.coneAngle, 1.0f, 0.0f, 360.f);
					ImGui::DragFloat("Cutoff", &light.cutOffRadius, 1.0f);
					ImGui::DragFloat("Falloff", &light.coneFalloff, 0.1f, 0.f, 1.0f);
					ImGui::DragFloat("Brightness", &light.brightness, 1.0f, 1.0f, 100.f);
					ImGui::DragFloat("Half Radius", &light.halfRadius, 1.0f, 0.01f, 0.99f);
					ImGui::TreePop();
				}
			}
		}
	}
	ImGui::Separator();

	// Windows
	if (ImGui::CollapsingHeader("Windows")) {
		ImGui::Checkbox("Threads", &state._showThreads);
		ImGui::Checkbox("Objects", &state._showObjects);
		ImGui::Checkbox("Actor", &state._showActor);
		ImGui::Checkbox("Stack", &state._showStack);
		ImGui::Checkbox("Audio", &state._showAudio);
		ImGui::Checkbox("Resources", &state._showResources);
		ImGui::Checkbox("Scene graph", &state._showScenegraph);
	}
	ImGui::Separator();

	// Room shader
	const char *RoomEffects = "None\0Sepia\0EGA\0VHS\0Ghost\0Black & White\0\0";
	if (ImGui::CollapsingHeader("Room Shader")) {
		int effect = static_cast<int>(room->_effect);
		if (ImGui::Combo("effect", &effect, RoomEffects))
			room->_effect = (RoomEffect)effect;
		ImGui::DragFloat("iFade", &g_twp->_shaderParams.iFade, 0.01f, 0.f, 1.f);
		ImGui::DragFloat("wobbleIntensity", &g_twp->_shaderParams.wobbleIntensity, 0.01f, 0.f, 1.f);
		ImGui::DragFloat3("shadows", g_twp->_shaderParams.shadows.v, 0.01f, -1.f, 1.f);
		ImGui::DragFloat3("midtones", g_twp->_shaderParams.midtones.v, 0.01f, -1.f, 1.f);
		ImGui::DragFloat3("highlights", g_twp->_shaderParams.highlights.v, 0.01f, -1.f, 1.f);
	}

	// Fade Effects
	const char *FadeEffects = "None\0In\0Out\0Wobble\0\0";
	if (ImGui::CollapsingHeader("Fade Shader")) {
		ImGui::Separator();
		ImGui::Combo("Fade effect", &state._fadeEffect, FadeEffects);
		ImGui::DragFloat("Duration", &state._fadeDuration, 0.1f, 0.f, 10.f);
		ImGui::Checkbox("Fade to sepia", &state._fadeToSepia);
		ImGui::Text("Elapsed %f", g_twp->_fadeShader->_elapsed);
		ImGui::Text("Fade %f", g_twp->_fadeShader->_fade);
		if (ImGui::Button("GO")) {
			g_twp->fadeTo((FadeEffect)state._fadeEffect, state._fadeDuration, state._fadeToSepia);
		}
	}
	ImGui::Separator();

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}

static void drawNode(Node *node) {
	auto children = node->getChildren();
	bool selected = state._node == node;
	if (children.empty()) {
		if (ImGui::Selectable(node->getName().c_str(), &selected)) {
			state._node = node;
		}
	} else {
		ImGui::PushID(node->getName().c_str());
		if (ImGui::TreeNode("")) {
			ImGui::SameLine();
			if (ImGui::Selectable(node->getName().c_str(), &selected)) {
				state._node = node;
			}
			for (auto &child : children) {
				drawNode(child);
			}
			ImGui::TreePop();
		} else {
			ImGui::SameLine();
			if (ImGui::Selectable(node->getName().c_str(), &selected)) {
				state._node = node;
			}
		}
		ImGui::PopID();
	}
}

static void drawScenegraph() {
	if (!state._showScenegraph)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Scenegraph", &state._showScenegraph);
	drawNode(&g_twp->_scene);
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	if (state._node != nullptr) {
		ImGui::Begin("Node");
		state._node->isVisible();
		bool visible = state._node->isVisible();
		if (ImGui::Checkbox(state._node->getName().c_str(), &visible)) {
			state._node->setVisible(visible);
		}
		int zsort = state._node->getZSort();
		if (ImGui::DragInt("Z-Sort", &zsort)) {
			state._node->setZSort(zsort);
		}
		Math::Vector2d pos = state._node->getPos();
		if (ImGui::DragFloat2("Pos", pos.getData())) {
			state._node->setPos(pos);
		}
		Math::Vector2d offset = state._node->getOffset();
		if (ImGui::DragFloat2("Offset", offset.getData())) {
			state._node->setOffset(offset);
		}
		ImGui::End();
	}
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugConsole))
		return;

	drawGeneral();
	drawThreads();
	drawObjects();
	drawStack();
	drawAudio();
	drawResources();
	drawScenegraph();
	drawActors();
	drawActor();
}

} // namespace Twp
