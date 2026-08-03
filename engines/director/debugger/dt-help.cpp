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

#include "director/director.h"
#include "director/debugger/dt-internal.h"

namespace Director {
namespace DT {

const ShortcutDef kShortcutDefs[kActCount] = {
	{ "continue",          "Continue / Break",       "Resume execution, or pause it while running.",              ImGuiKey_F5 },
	{ "stepOver",          "Step Over",              "Step over the current Lingo statement.",                    ImGuiKey_F10 },
	{ "stepInto",          "Step Into",              "Step into the called handler.",                             ImGuiKey_F11 },
	{ "stepOut",           "Step Out",               "Run until the current handler returns.",                    ImGuiMod_Shift | ImGuiKey_F11 },
	{ "quickOpen",         "Quick Open",             "Search and jump to any cast member or handler.",            ImGuiMod_Ctrl | ImGuiKey_P },
	{ "pickFromStage",     "Pick From Stage",        "Arm pick mode: the next stage click selects that sprite.",  ImGuiMod_Ctrl | ImGuiKey_K },
	{ "toggleControlPanel","Toggle Control Panel",   "Show or hide the Control Panel window.",                    ImGuiMod_Ctrl | ImGuiKey_2 },
	{ "toggleCast",        "Toggle Cast",            "Show or hide the Cast window.",                             ImGuiMod_Ctrl | ImGuiKey_3 },
	{ "toggleScore",       "Toggle Score",           "Show or hide the Score window.",                            ImGuiMod_Ctrl | ImGuiKey_4 },
	{ "toggleMouseIgnore", "Toggle Mouse Passthrough","Ignore the debugger's mouse so clicks reach the stage.",   ImGuiMod_Ctrl | ImGuiKey_F1 },
};

void initShortcuts() {
	for (int i = 0; i < kActCount; i++)
		_state->_shortcuts[i] = kShortcutDefs[i].defaultChord;
}

void resetShortcuts() {
	initShortcuts();
}

bool actionTriggered(DebuggerAction act) {
	// While rebinding, keypresses feed the capture UI, not the actions.
	if (_state->_shortcutCapture >= 0)
		return false;
	ImGuiKeyChord chord = _state->_shortcuts[act];
	if (chord == ImGuiKey_None)
		return false;
	return ImGui::Shortcut(chord, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteOverFocused);
}

static Common::String chordName(ImGuiKeyChord chord) {
	if (chord == ImGuiKey_None)
		return "(unbound)";
	Common::String s;
	if (chord & ImGuiMod_Ctrl)  s += "Ctrl+";
	if (chord & ImGuiMod_Shift) s += "Shift+";
	if (chord & ImGuiMod_Alt)   s += "Alt+";
	if (chord & ImGuiMod_Super) s += "Super+";
	const ImGuiKeyChord modMask = ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiMod_Alt | ImGuiMod_Super;
	ImGuiKey key = (ImGuiKey)(chord & ~modMask);
	if (key != ImGuiKey_None)
		s += ImGui::GetKeyName(key);
	else if (!s.empty())
		s.deleteLastChar(); // drop the trailing '+' when only modifiers are held
	return s;
}

static bool isModifierKey(ImGuiKey k) {
	return k == ImGuiKey_LeftCtrl || k == ImGuiKey_RightCtrl ||
		k == ImGuiKey_LeftShift || k == ImGuiKey_RightShift ||
		k == ImGuiKey_LeftAlt || k == ImGuiKey_RightAlt ||
		k == ImGuiKey_LeftSuper || k == ImGuiKey_RightSuper;
}

// While rebinding, accumulate the held chord and commit it on full release.
// Escape cancels; a bare Backspace/Delete clears the binding.
static void captureChord() {
	if (_state->_shortcutCapture < 0) {
		_state->_shortcutPending = ImGuiKey_None;
		return;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
		_state->_shortcutCapture = -1;
		_state->_shortcutPending = ImGuiKey_None;
		return;
	}

	const ImGuiKeyChord modMask = ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiMod_Alt | ImGuiMod_Super;
	ImGuiKeyChord mods = ImGui::GetIO().KeyMods;

	// The topmost non-modifier key currently held (if any).
	ImGuiKey heldKey = ImGuiKey_None;
	for (ImGuiKey k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k = (ImGuiKey)(k + 1)) {
		if (isModifierKey(k) || k == ImGuiKey_Escape)
			continue;
		if (ImGui::IsKeyDown(k)) {
			heldKey = k;
			break;
		}
	}

	bool anyHeld = (heldKey != ImGuiKey_None) || (mods != 0);
	if (anyHeld) {
		// Keep every modifier seen this gesture (so releasing Ctrl early doesn't
		// drop it) plus the last non-modifier key, making release order irrelevant.
		ImGuiKeyChord accumMods = (_state->_shortcutPending & modMask) | mods;
		ImGuiKey key = (heldKey != ImGuiKey_None) ? heldKey : (ImGuiKey)(_state->_shortcutPending & ~modMask);
		_state->_shortcutPending = accumMods | key;
		return;
	}

	// Every key is released. Commit only if a real (non-modifier) key was captured.
	ImGuiKeyChord pending = _state->_shortcutPending;
	ImGuiKey key = (ImGuiKey)(pending & ~modMask);
	if (key == ImGuiKey_None) {
		// Only modifiers were pressed; discard and keep waiting for a real chord.
		_state->_shortcutPending = ImGuiKey_None;
		return;
	}
	if ((pending & modMask) == 0 && (key == ImGuiKey_Backspace || key == ImGuiKey_Delete))
		_state->_shortcuts[_state->_shortcutCapture] = ImGuiKey_None; // clear
	else
		_state->_shortcuts[_state->_shortcutCapture] = pending;
	_state->_shortcutCapture = -1;
	_state->_shortcutPending = ImGuiKey_None;
}

void showHelp() {
	if (!_state->_w.help)
		return;

	ImGui::SetNextWindowSize(ImVec2(600, 640), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Help", &_state->_w.help)) {
		captureChord();

		if (ImGui::CollapsingHeader("Keyboard shortcuts", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::TextDisabled("Click a key to rebind. Esc cancels, Backspace/Delete clears.");
			if (ImGui::BeginTable("##shortcuts", 3,
					ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
				ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableSetupColumn("Description");
				ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 130.0f);
				ImGui::TableHeadersRow();

				for (int i = 0; i < kActCount; i++) {
					ImGui::TableNextRow();
					ImGui::PushID(i);

					ImGui::TableSetColumnIndex(0);
					ImGui::TextUnformatted(kShortcutDefs[i].label);

					ImGui::TableSetColumnIndex(1);
					ImGui::TextWrapped("%s", kShortcutDefs[i].help);

					ImGui::TableSetColumnIndex(2);
					Common::String key;
					if (_state->_shortcutCapture == i)
						key = (_state->_shortcutPending != ImGuiKey_None) ? chordName(_state->_shortcutPending) + " ..." : Common::String("press keys...");
					else
						key = chordName(_state->_shortcuts[i]);
					if (ImGui::Button(key.c_str(), ImVec2(-FLT_MIN, 0)))
						_state->_shortcutCapture = (_state->_shortcutCapture == i) ? -1 : i;

					ImGui::PopID();
				}
				ImGui::EndTable();
			}
			if (ImGui::Button("Reset to defaults"))
				resetShortcuts();
			ImGui::SameLine();
			if (ImGui::Button("Save"))
				saveCurrentState();
		}

		if (ImGui::CollapsingHeader("Tips", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::BulletText("Quick Open finds cast members by name/number and handlers by name;\nEnter picks the top match.");
			ImGui::BulletText("Pick From Stage: arm it, then click a sprite on the stage to open it in\nCast Details (also selects its Score cell). Puppeted sprites work too.");
			ImGui::BulletText("Breakpoints: each function breakpoint has a Condition field. Enter a Lingo\nexpression and it only fires when the expression is true.");
			ImGui::BulletText("Vars: right-click a variable for \"Copy value\". Click a variable name to\nadd/remove it from Watched Vars.");
			ImGui::BulletText("Cast list: click the Name/ID/Type column headers to sort.");
			ImGui::BulletText("Cast Details on a movie cast member has a Movie tab showing its embedded\nscore, laid out like the film-loop viewer.");
			ImGui::BulletText("Score: click a cell to open that cast member in Cast Details.");
			ImGui::BulletText("Settings: Enable Multi-Viewport lets debugger windows leave the main window.");
			ImGui::BulletText("Save/Load state (View menu) persists open windows, layout, and these shortcuts.");
		}
	}
	ImGui::End();
}

} // namespace DT
} // namespace Director
