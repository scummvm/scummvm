#include "twp/debugtools.h"
#include "imgui/imgui.h"
#include "twp/twp.h"
#include "twp/thread.h"
#include "twp/dialog.h"

namespace Twp {

static struct {
	bool _showThreads = false;
	bool _showObjects = false;
	bool _showStack = false;
	bool _showAudio = false;
	bool _showResources = false;
	ImGuiTextFilter _objFilter;
	int _fadeEffect = 0;
	float _fadeDuration = 0.f;
	bool _fadeToSepia = false;
	Common::String _textureSelected;
} state;

ImVec4 gray(0.6f, 0.6f, 0.6f, 1.f);

static void drawThreads() {
	if (!state._showThreads)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	const auto &threads = g_engine->_threads;
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
	for (auto &layer : g_engine->_room->_layers) {
		for (auto &obj : layer->_objects) {
			if (state._objFilter.PassFilter(obj->_key.c_str())) {
				ImGui::PushID(obj->getId());
				bool visible = obj->_node->isVisible();
				if (ImGui::Checkbox("", &visible)) {
					obj->_node->setVisible(visible);
				}
				ImGui::SameLine();
				Common::String name = obj->_key == "" ? obj->getName() : Common::String::format("%s(%s) %d", obj->getName().c_str(), obj->_key.c_str(), obj->getId());
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

static void drawStack() {
	if (!state._showStack)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Stack");
	ImGui::BeginChild("ScrollingRegion");
	SQInteger size = sq_gettop(g_engine->getVm());
	ImGui::Text("size: %lld", size);
	HSQOBJECT obj;
	for (SQInteger i = 1; i < size; i++) {
		sq_getstackobj(g_engine->getVm(), -i, &obj);
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

		for (auto &res : g_engine->_resManager._textures) {
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
	ImGui::BeginChild("TexturePreview", ImVec2(0, 0), ImGuiChildFlags_Border|ImGuiChildFlags_ResizeX|ImGuiChildFlags_ResizeY);
	for (auto &res : g_engine->_resManager._textures) {
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
	for (auto &s : g_engine->_audio._slots) {
		if (s.busy)
			count++;
	}

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Sounds", &state._showAudio);
	ImGui::Text("# sounds: %d/32", count);
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

		for (int i = 0; i < 32; i++) {
			auto &sound = g_engine->_audio._slots[i];
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("#%d", i);
			if (sound.busy) {
				float pan = g_engine->_mixer->getChannelBalance(sound.handle) / 128.f;
				float vol = g_engine->_mixer->getChannelVolume(sound.handle) / 255.f;
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
					g_engine->_audio.stop(sound.id);
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

	SQInteger size = sq_gettop(g_engine->getVm());
	ImGui::TextColored(gray, "Stack:");
	ImGui::SameLine();
	ImGui::Text("%lld", size);
	ImGui::TextColored(gray, "In cutscene:");
	ImGui::SameLine();
	ImGui::Text("%s", g_engine->_cutscene ? "yes" : "no");
	DialogState dialogState = g_engine->_dialog.getState();
	ImGui::TextColored(gray, "In dialog:");
	ImGui::SameLine();
	ImGui::Text("%s", ((dialogState == Active) ? "yes" : (dialogState == WaitingForChoice ? "waiting for choice" : "no")));
	ImGui::TextColored(gray, "Verb:");
	ImGui::SameLine();
	Common::String verb = g_engine->getTextDb().getText(g_engine->_hud._verb.text);
	ImGui::Text("%s %d", verb.c_str(), g_engine->_hud._verb.id.id);

	auto mousePos = g_engine->_cursor.pos;
	ImGui::TextColored(gray, "Pos (screen):");
	ImGui::SameLine();
	ImGui::Text("(%.0f, %0.f)", mousePos.getX(), mousePos.getY());
	if (g_engine->_room) {
		auto pos = g_engine->screenToRoom(mousePos);
		ImGui::TextColored(gray, "Pos (room):");
		ImGui::SameLine();
		ImGui::Text("(%.0f, %0.f)", pos.getX(), pos.getY());
	}
	ImGui::Separator();
	ImGui::Checkbox("HUD", &g_engine->_inputState._inputHUD);
	ImGui::SameLine();
	ImGui::Checkbox("Input", &g_engine->_inputState._inputActive);
	ImGui::SameLine();
	ImGui::Checkbox("Cursor", &g_engine->_inputState._showCursor);
	ImGui::SameLine();
	ImGui::Checkbox("Verbs", &g_engine->_inputState._inputVerbsActive);
	ImGui::Separator();

	// Camera
	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::TextColored(gray, "follow:");
		ImGui::SameLine();
		ImGui::Text("%s", !g_engine->_followActor ? "(none)" : g_engine->_followActor->_key.c_str());
		ImGui::TextColored(gray, "moving:");
		ImGui::SameLine();
		ImGui::Text("%s", g_engine->_camera.isMoving() ? "yes" : "no");
		auto halfScreenSize = g_engine->_room->getScreenSize() / 2.0f;
		auto camPos = g_engine->cameraPos() - halfScreenSize;
		if (ImGui::DragFloat2("Camera pos", camPos.getData())) {
			g_engine->follow(nullptr);
			g_engine->cameraAt(camPos);
		}
		auto bounds = g_engine->_camera.getBounds();
		if (ImGui::DragFloat4("Bounds", bounds.v)) {
			g_engine->_camera.setBounds(bounds);
		}
	}

	// Room
	Room *room = g_engine->_room;
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
		ImGui::Checkbox("Stack", &state._showStack);
		ImGui::Checkbox("Audio", &state._showAudio);
		ImGui::Checkbox("Resources", &state._showResources);
	}
	ImGui::Separator();

	// Room shader
	const char *RoomEffects = "None\0Sepia\0EGA\0VHS\0Ghost\0Black & White\0\0";
	if (ImGui::CollapsingHeader("Room Shader")) {
		int effect = static_cast<int>(room->_effect);
		if (ImGui::Combo("effect", &effect, RoomEffects))
			room->_effect = (RoomEffect)effect;
		ImGui::DragFloat("iFade", &g_engine->_shaderParams.iFade, 0.01f, 0.f, 1.f);
		ImGui::DragFloat("wobbleIntensity", &g_engine->_shaderParams.wobbleIntensity, 0.01f, 0.f, 1.f);
		ImGui::DragFloat3("shadows", g_engine->_shaderParams.shadows.v, 0.01f, -1.f, 1.f);
		ImGui::DragFloat3("midtones", g_engine->_shaderParams.midtones.v, 0.01f, -1.f, 1.f);
		ImGui::DragFloat3("highlights", g_engine->_shaderParams.highlights.v, 0.01f, -1.f, 1.f);
	}

	// Fade Effects
	const char *FadeEffects = "None\0In\0Out\0Wobble\0\0";
	if (ImGui::CollapsingHeader("Fade Shader")) {
		ImGui::Separator();
		ImGui::Combo("Fade effect", &state._fadeEffect, FadeEffects);
		ImGui::DragFloat("Duration", &state._fadeDuration, 0.1f, 0.f, 10.f);
		ImGui::Checkbox("Fade to sepia", &state._fadeToSepia);
		ImGui::Text("Elapsed %f", g_engine->_fadeShader->_elapsed);
		ImGui::Text("Fade %f", g_engine->_fadeShader->_fade);
		if (ImGui::Button("GO")) {
			g_engine->fadeTo((FadeEffect)state._fadeEffect, state._fadeDuration, state._fadeToSepia);
		}
	}
	ImGui::Separator();

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}

void onImGuiRender() {
	drawGeneral();
	drawThreads();
	drawObjects();
	drawStack();
	drawAudio();
	drawResources();
}

} // namespace Twp
