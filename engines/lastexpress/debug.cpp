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

#include "lastexpress/debug.h"

// Data

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/beetle.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/savegame.h"

#include "lastexpress/menu/clock.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"

#include "common/debug-channels.h"
#include "common/md5.h"

#include "backends/imgui/imgui.h"

namespace LastExpress {

#ifdef USE_IMGUI

typedef struct ImGuiState {
	LastExpressEngine *_engine = nullptr;
	ImGuiTextFilter _filter;
	int _currentTab = 0;
	int _selectedCharacter = 0;
	bool _forceReturnToListView = false;
	float _rightPanelWidth = 0.0f;
	float _bottomPanelHeight = 0.0f;
	float _rightTopPanelHeight = 0.0f;
	int _ticksToAdvance = 0;
	ImTextureID _textureID = ImTextureID_Invalid;
	int _selectedGlobalVarRow = -1;
} ImGuiState;

ImGuiState *_state = nullptr;

void onImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	_state = new ImGuiState();
	_state->_engine = (LastExpressEngine *)g_engine;
}

void onImGuiRender() {
	if (_state->_engine->shouldQuit() || _state->_engine->_exitFromMenuButton)
		return;

	if (!debugChannelSet(-1, kDebugConsole)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}

	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	ImGui::SetNextWindowSize(ImVec2(1400, 1000), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_FirstUseEver);

	_state->_rightPanelWidth = 300.0f;
	_state->_bottomPanelHeight = 280.0f;
	_state->_rightTopPanelHeight = 200.0f;

	// Disable the debugger when the NIS engine is running...
	if (_state->_engine->getNISManager()->getNISFlag() & kNisFlagPlaying)
		return;

	// Disable the debugger when the fighting engine is running...
	if (_state->_engine->_fight)
		return;

	if (ImGui::Begin("Last Express Debugger")) {
		ImVec2 windowSize = ImGui::GetContentRegionAvail();

		// Right panel splitter...
		{
			ImGui::SameLine(windowSize.x - _state->_rightPanelWidth - 8);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.3f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::Button("##vsplitter", ImVec2(8, windowSize.y - _state->_bottomPanelHeight));
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			if (ImGui::IsItemActive())
				_state->_rightPanelWidth += ImGui::GetIO().MouseDelta.x * -1.0f;

			_state->_rightPanelWidth = CLIP<float>(_state->_rightPanelWidth, 150.0f, windowSize.x * 0.7f);
		}

		// Bottom panel splitter...
		{
			float splitterY = windowSize.y - _state->_bottomPanelHeight - 8;
			ImGui::SetCursorPosY(splitterY);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.3f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::Button("##hsplitter", ImVec2(windowSize.x, 8));
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			if (ImGui::IsItemActive())
				_state->_bottomPanelHeight += ImGui::GetIO().MouseDelta.y * -1.0f;

			_state->_bottomPanelHeight = CLIP<float>(_state->_bottomPanelHeight, 150.0f, windowSize.y * 0.7f);
		}

		// Right panel splitter (between top and bottom sections)...
		{
			ImGui::SetCursorPos(ImVec2(windowSize.x - _state->_rightPanelWidth, _state->_rightTopPanelHeight));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.3f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::Button("##rightsplitter", ImVec2(_state->_rightPanelWidth, 8));
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			if (ImGui::IsItemActive())
				_state->_rightTopPanelHeight += ImGui::GetIO().MouseDelta.y;

			_state->_rightTopPanelHeight = CLIP<float>(_state->_rightTopPanelHeight, 100.0f, windowSize.y - _state->_bottomPanelHeight - 100.0f);
		}

		float mainAreaWidth = windowSize.x - _state->_rightPanelWidth - 8;
		float mainAreaHeight = windowSize.y - _state->_bottomPanelHeight - 8;

		// Top-left: Character Debugger
		ImGui::SetCursorPos(ImVec2(0, 16));
		ImGui::BeginChild("CharacterDebugger", ImVec2(mainAreaWidth, mainAreaHeight), true);
		{
			ImGuiTabItemFlags flags = 0;

			// Tab bar for different views...
			if (ImGui::BeginTabBar("CharacterViews")) {
				if (_state->_forceReturnToListView) {
					_state->_forceReturnToListView = false;
					flags |= ImGuiTabItemFlags_SetSelected;
				}

				if (ImGui::BeginTabItem("List View", nullptr, flags)) {
					_state->_currentTab = 0;
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Grid View")) {
					_state->_currentTab = 1;
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Pinned Characters")) {
					_state->_currentTab = 2;
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Current Scene")) {
					_state->_currentTab = 3;
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Global Vars")) {
					_state->_currentTab = 4;
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			if (_state->_currentTab >= 0 && _state->_currentTab <= 2) {
				// Update the character filter...
				_state->_filter.Draw("Filter Characters", 180);
				ImGui::SameLine();
				if (ImGui::Button("Clear")) {
					_state->_filter.Clear();
				}
			}

			// Show corresponding view based on selected tab...
			switch (_state->_currentTab) {
			case 0: // List View
				_state->_engine->getLogicManager()->renderCharacterList(_state->_selectedCharacter);
				break;
			case 1: // Grid View
				_state->_engine->getLogicManager()->renderCharacterGrid(false, _state->_selectedCharacter);
				break;
			case 2: // Pinned Characters
				_state->_engine->getLogicManager()->renderCharacterGrid(true, _state->_selectedCharacter);
				break;
			case 3: // Current Scene
				_state->_engine->getLogicManager()->renderCurrentSceneDebugger();
				break;
			case 4: // Global Vars
				_state->_engine->getLogicManager()->renderGlobalVars();
				break;
			default:
				break;
			}
		}
		ImGui::EndChild();

		// Top-right area

		// Top-right is Clock
		ImGui::SetCursorPos(ImVec2(mainAreaWidth + 8, 16));
		ImGui::BeginChild("Clock", ImVec2(_state->_rightPanelWidth, _state->_rightTopPanelHeight), true);
		{
			_state->_engine->getClock()->showCurrentTime();
		}
		ImGui::EndChild();

		// Bottom-right is Where's Cath?
		ImGui::SetCursorPos(ImVec2(mainAreaWidth + 8, _state->_rightTopPanelHeight + 8));
		ImGui::BeginChild("CathInfo", ImVec2(_state->_rightPanelWidth, mainAreaHeight - _state->_rightTopPanelHeight - 8), true);
		{
			ImGui::Text("Where's Cath?");
			ImGui::Separator();
			_state->_engine->getLogicManager()->showCurrentTrainNode();
		}
		ImGui::EndChild();

		// Bottom-right is Where's Cath?
		ImGui::SetCursorPos(ImVec2(mainAreaWidth + 8, _state->_rightTopPanelHeight * 2 - 16));
		ImGui::BeginChild("EngineInfo", ImVec2(_state->_rightPanelWidth, mainAreaHeight - (_state->_rightTopPanelHeight * 2 - 16)), true);
		{
			ImGui::Text("Engine Info");
			ImGui::Separator();
			_state->_engine->showEngineInfo();
		}
		ImGui::EndChild();


		// Bottom panel: Train Map
		ImGui::SetCursorPos(ImVec2(0, windowSize.y - _state->_bottomPanelHeight));
		ImGui::BeginChild("TrainMap", ImVec2(windowSize.x, _state->_bottomPanelHeight), true);
		{
			_state->_engine->getLogicManager()->showTrainMapWindow();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void onImGuiCleanup() {
	delete _state;
	_state = nullptr;
}

void Clock::showCurrentTime() {
	int timeSource = _engine->getMenu()->isShowingMenu() ? _engine->getClock()->getTimeShowing() : (int)_engine->getLogicManager()->getGameTime();

	int hours = timeSource % 1296000 / 54000;
	int minutes = timeSource % 54000 / 900 % 60 % 60;
	int seconds = (timeSource % 900) / 15;

	char clockText[32];
	Common::sprintf_s(clockText, "%02d:%02d:%02d", hours, minutes, seconds);

	ImDrawList *drawList = ImGui::GetWindowDrawList();
	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 windowCenter = ImVec2(
		windowPos.x + ImGui::GetWindowSize().x / 2,
		windowPos.y + ImGui::GetWindowSize().y / 2
	);

	drawList->AddText(
		ImGui::GetFont(),
		4.0f * ImGui::GetFontSize(),
		ImVec2(windowPos.x + 40, windowCenter.y - 20),
		IM_COL32(255, 255, 255, 255),
		clockText
	);

	char dayText[32];
	int dayOffset = timeSource / 1296000;
	Common::sprintf_s(dayText, "July %d, 1914", 24 + dayOffset); // The game starts in July 24, 1914...

	drawList->AddText(
		ImGui::GetFont(),
		2.0f * ImGui::GetFontSize(),
		ImVec2(windowPos.x + 60, windowCenter.y + 35),
		IM_COL32(255, 255, 255, 255),
		dayText
	);


	ImGui::Text("Game time: %d", _engine->getLogicManager()->getGameTime());
	ImGui::Text("Real time: %d", _engine->getLogicManager()->getRealTime());
	ImGui::Text("Time speed: %d", _engine->getLogicManager()->getTimeSpeed());
	ImGui::Text("Grace timer: %d", _engine->_gracePeriodTimer);
	ImGui::Separator();
}

Common::String LogicManager::translateNodeProperty(int property) {
	Common::StringArray properties = {
		"No Property",
		"Has Door",
		"Has Item",
		"Has 2 Items",
		"Has Door Item",
		"Has 3 Items",
		"Model Pad",
		"Soft Point",
		"Soft Point Item",
		"Auto Walk",
		"Sleeping On Bed",
		"Beetle Area",
		"Pulling Emergency Stop",
		"Rebecca's Diary",
		"Stops Fast Walk"
	};

	if (property >= 0 && property <= 9) {
		return properties[property];
	} else if (property >= 128 && property <= 133) {
		return properties[property - 128 + 9];
	} else {
		return "Unknown property " + Common::String(property);
	}
}

void LogicManager::showCurrentTrainNode() {
	ImGui::Text("Node property: %s", translateNodeProperty(_trainData[_activeNode].property).c_str());
	ImGui::Text("Direction: %u", _trainData[_activeNode].cathDir);
	ImGui::Text("Node position:");
	ImGui::BulletText("Car %u", _trainData[_activeNode].nodePosition.car);
	ImGui::BulletText("Location %u", _trainData[_activeNode].nodePosition.location);
	ImGui::BulletText("Position %u", _trainData[_activeNode].nodePosition.position);

	ImGui::Text("Parameters: %u %u %u", _trainData[_activeNode].parameter1, _trainData[_activeNode].parameter2, _trainData[_activeNode].parameter3);
	ImGui::Text("Scene filename: %s", _trainData[_activeNode].sceneFilename);
}

void LogicManager::showCharacterDebugger() {
	ImGuiTabItemFlags flags = 0;

	if (ImGui::BeginTabBar("CharacterViews")) {
		if (_state->_forceReturnToListView) {
			_state->_forceReturnToListView = false;
			flags |= ImGuiTabItemFlags_SetSelected;
		}

		if (ImGui::BeginTabItem("List View", nullptr, flags)) {
			_state->_currentTab = 0;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Grid View")) {
			_state->_currentTab = 1;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Pinned Characters")) {
			_state->_currentTab = 2;
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	_state->_filter.Draw("Filter Characters", 180);
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		_state->_filter.Clear();
	}

	switch (_state->_currentTab) {
	case 0: // List View
		renderCharacterList(_state->_selectedCharacter);
		break;
	case 1:  // Grid View
		renderCharacterGrid(false, _state->_selectedCharacter);
		break;
	case 2: // Pinned Characters
		renderCharacterGrid(true, _state->_selectedCharacter);
		break;
	}
}

void LogicManager::renderCharacterList(int &selectedCharacter) {
	ImGui::BeginChild("CharacterList", ImVec2(200, 0), true);

	for (int i = 0; i < 40; i++) {
		Character *character = &getCharacter(i);
		if (!character)
			continue;

		char buffer[64];
		Common::sprintf_s(buffer, "%s (%d)", getCharacterName(i), i);
		if (!_state->_filter.PassFilter(buffer))
			continue;

		bool isPinned = isCharacterPinned(i);
		if (isPinned) {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 220, 0, 255)); // Yellow for pinned...
		}

		if (ImGui::Selectable(buffer, selectedCharacter == i)) {
			selectedCharacter = i;
		}

		if (isPinned) {
			ImGui::PopStyleColor();
		}

		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem(isPinned ? "Unpin Character" : "Pin Character")) {
				toggleCharacterPin(i);
			}
			ImGui::EndPopup();
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	// Right panel: Character details
	ImGui::BeginChild("CharacterDetails", ImVec2(0, 0), true);

	if (selectedCharacter < 40) {
		Character *character = &getCharacter(selectedCharacter);
		if (character) {
			renderCharacterDetails(character, selectedCharacter);
		}
	} else {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select a character from the list to view details");
	}

	ImGui::EndChild();
}

void LogicManager::renderCurrentSceneDebugger() {
	if (_state->_textureID)
		g_system->freeImGuiTexture((void *)_state->_textureID);

	// Let's blit the current background on the ImGui window...
	Graphics::Surface temp;
	temp.create(640, 480, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_backBuffer, (PixMap *)temp.getPixels(), 0, 0, 640, 480);

	_state->_textureID = (ImTextureID)(intptr_t)g_system->getImGuiTexture(temp);

	temp.free();

	ImVec2 imagePos = ImGui::GetCursorScreenPos();
	ImVec2 imageSize(640, 480);

	ImGui::Image(_state->_textureID, imageSize);

	// Set hovering flag...
	ImVec2 mousePos = ImGui::GetMousePos();
	ImVec2 relativeMousePos(mousePos.x - imagePos.x, mousePos.y - imagePos.y);

	bool mouseOverImage = (relativeMousePos.x >= 0 && relativeMousePos.x < 640 &&
						   relativeMousePos.y >= 0 && relativeMousePos.y < 480);

	ImDrawList *drawList = ImGui::GetWindowDrawList();

	Link *hoveredLink = nullptr;
	int hoveredLinkIndex = -1;

	// Draw all the Link hotspots and collision lines...
	Node *currentNode = &_trainData[_activeNode];
	if (currentNode && currentNode->link) {
		Link *currentLink = currentNode->link;
		int linkIndex = 0;

		while (currentLink) {
			// Calculate screen coordinates for this link's bounding box...
			ImVec2 topLeft(imagePos.x + currentLink->left, imagePos.y + currentLink->top);
			ImVec2 bottomRight(imagePos.x + currentLink->right, imagePos.y + currentLink->bottom);

			// Check if mouse is hovering over this link...
			bool isHovered = mouseOverImage &&
							 relativeMousePos.x >= currentLink->left && relativeMousePos.x <= currentLink->right &&
							 relativeMousePos.y >= currentLink->top && relativeMousePos.y <= currentLink->bottom;

			// Store the hovered link with highest priority,
			// if they have the same priority, the last one in list wins...
			if (isHovered) {
				if (hoveredLink == nullptr ||
					currentLink->location > hoveredLink->location ||
					currentLink->location == hoveredLink->location) {
					hoveredLink = currentLink;
					hoveredLinkIndex = linkIndex;
				}
			}

			ImU32 rectColor = IM_COL32(255, 0, 0, 100);   // Semi-transparent red
			ImU32 borderColor = IM_COL32(255, 0, 0, 255); // Solid red border

			// Highlight hovered link...
			if (isHovered) {
				rectColor = IM_COL32(255, 255, 255, 150);
			}

			drawList->AddRectFilled(topLeft, bottomRight, rectColor);
			drawList->AddRect(topLeft, bottomRight, borderColor, 0.0f, 0, 2.0f);

			// Draw collision lines...
			Line7 *currentLine = currentLink->lineList;
			int lineIndex = 0;
			while (currentLine) {
				ImU32 lineColor = IM_COL32(0, 255, 255, 255);

				switch (currentLine->lineType) {
				case 0:
					lineColor = IM_COL32(0, 255, 255, 255); // Cyan
					break;
				case 1:
				default:
					lineColor = IM_COL32(255, 128, 0, 255); // Orange
					break;
				}

				if (currentLine->slope == 0) {
					// Horizontal line: y = -intercept / 1000
					float y = -(float)currentLine->intercept / 1000.0f;
					if (y >= currentLink->top && y <= currentLink->bottom) {
						ImVec2 lineStart(imagePos.x + currentLink->left, imagePos.y + y);
						ImVec2 lineEnd(imagePos.x + currentLink->right, imagePos.y + y);
						drawList->AddLine(lineStart, lineEnd, lineColor, 2.0f);
					}
				} else {
					// Non-horizontal line
					float slope = (float)currentLine->slope;
					float intercept = (float)currentLine->intercept;

					// Calculate line endpoints across the entire screen bounds...
					Common::Array<ImVec2> intersectionPoints;

					// Left edge (x = currentLink->left)
					float yLeft = -(slope * currentLink->left + intercept) / 1000.0f;
					if (yLeft >= currentLink->top && yLeft <= currentLink->bottom) {
						intersectionPoints.push_back(ImVec2(imagePos.x + currentLink->left, imagePos.y + yLeft));
					}

					// Right edge (x = currentLink->right)
					float yRight = -(slope * currentLink->right + intercept) / 1000.0f;
					if (yRight >= currentLink->top && yRight <= currentLink->bottom) {
						intersectionPoints.push_back(ImVec2(imagePos.x + currentLink->right, imagePos.y + yRight));
					}

					// Top edge (y = currentLink->top)
					float xTop = -(1000.0f * currentLink->top + intercept) / slope;
					if (xTop >= currentLink->left && xTop <= currentLink->right) {
						intersectionPoints.push_back(ImVec2(imagePos.x + xTop, imagePos.y + currentLink->top));
					}

					// Bottom edge (y = currentLink->bottom)
					float xBottom = -(1000.0f * currentLink->bottom + intercept) / slope;
					if (xBottom >= currentLink->left && xBottom <= currentLink->right) {
						intersectionPoints.push_back(ImVec2(imagePos.x + xBottom, imagePos.y + currentLink->bottom));
					}

					// Draw line if we have at least 2 intersection points...
					if (intersectionPoints.size() >= 2) {
						drawList->AddLine(intersectionPoints[0], intersectionPoints[1], lineColor, 2.0f);
					}
				}

				currentLine = currentLine->next;
				lineIndex++;
			}

			currentLink = currentLink->next;
			linkIndex++;
		}
	}

	// Show tooltip for hovered link...
	if (hoveredLink && mouseOverImage) {
		ImGui::BeginTooltip();
		ImGui::Text("Link %d", hoveredLinkIndex);
		ImGui::Text("Bounds: (%d,%d) - (%d,%d)", hoveredLink->left, hoveredLink->top, hoveredLink->right, hoveredLink->bottom);

		if (hoveredLink->scene) {
			ImGui::Text("Leads to scene: %s (%d)", _trainData[hoveredLink->scene].sceneFilename, hoveredLink->scene);
		} else {
			ImGui::Text("Leads to scene: None (0)");
		}

		ImGui::Text("Location: %d", hoveredLink->location);

		const char *actionName;
		if (_engine->getMenu()->isShowingMenu()) {
			const char *menuActionNames[] = {
				"None", "PlayGame", "Credits", "Quit", "Action4",
				"Action5", "SwitchEggs", "Rewind", "FastForward", "Action9",
				"GoToParis", "GoToStrasbourg", "GoToMunich", "GoToVienna", "GoToBudapest",
				"GoToBelgrad", "GoToCostantinople", "VolumeDown", "VolumeUp", "BrightnessDown",
				"BrightnessUp"
			};

			actionName = (hoveredLink->action < 21) ? menuActionNames[hoveredLink->action] : "Unknown";
		} else {
			const char *gameActionNames[] = {
				"None", "Inventory", "SendCathMessage", "PlaySound", "PlayMusic",
				"Knock", "Compartment", "PlaySounds", "PlayAnimation", "SetDoor",
				"SetModel", "SetItem", "KnockInside", "TakeItem", "DropItem",
				"LinkOnGlobal", "Rattle", "DummyAction1", "LeanOutWindow", "AlmostFall",
				"ClimbInWindow", "ClimbLadder", "ClimbDownTrain", "KronosSanctum", "EscapeBaggage",
				"EnterBaggage", "BombPuzzle", "Conductors", "KronosConcert", "LetterInAugustSuitcase",
				"CatchBeetle", "ExitCompartment", "OutsideTrain", "FirebirdPuzzle", "OpenMatchBox",
				"OpenBed", "DummyAction2", "HintDialog", "MusicEggBox", "FindEggUnderSink",
				"Bed", "PlayMusicChapter", "PlayMusicChapterSetupTrain", "SwitchChapter", "EasterEgg"
			};

			actionName = (hoveredLink->action < 45) ? gameActionNames[hoveredLink->action] : "Unknown";
		}

		ImGui::Text("Action: %d (%s)", hoveredLink->action, actionName);

		const char *cursorNames[] = {
			"Normal", "Forward", "Backward", "TurnRight", "TurnLeft",
			"Up", "Down", "Left", "Right", "Hand",
			"HandKnock", "Magnifier", "HandPointer", "Sleep", "Talk",
			"Talk2", "MatchBox", "Telegram", "PassengerList", "Article",
			"Scarf", "Paper", "Parchemin", "Match", "Whistle",
			"Key", "Bomb", "Firebird", "Briefcase", "Corpse",
			"PunchLeft", "PunchRight", "Portrait", "PortraitSelected", "PortraitGreen",
			"PortraitGreenSelected", "PortraitYellow", "PortraitYellowSelected", "HourGlass", "EggBlue",
			"EggRed", "EggGreen", "EggPurple", "EggTeal", "EggGold",
			"EggClock", "Normal2", "Blank"
		};

		const char *cursorName;
		if (hoveredLink->cursor == 128) {
			cursorName = "Process";
		} else if (hoveredLink->cursor == 255) {
			cursorName = "KeepValue";
		} else if (hoveredLink->cursor < 48) {
			cursorName = cursorNames[hoveredLink->cursor];
		} else {
			cursorName = "Unknown";
		}

		ImGui::Text("Cursor: %d (%s)", hoveredLink->cursor, cursorName);
		ImGui::Text("Params: %d, %d, %d", hoveredLink->param1, hoveredLink->param2, hoveredLink->param3);

		// Show line information...
		if (hoveredLink->lineList) {
			ImGui::Separator();
			Line7 *line = hoveredLink->lineList;
			int lineNum = 0;

			while (line) {
				ImGui::Text("Line %d: slope=%d, intercept=%d, type=%d",
							lineNum, line->slope, line->intercept, line->lineType);

				line = line->next;
				lineNum++;
			}
		}

		ImGui::EndTooltip();
	}
}

void LogicManager::renderGlobalVars() {
	_state->_selectedGlobalVarRow = -1;

	if (ImGui::BeginTable("GlobalVars", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
		ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 400.0f);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		const char *globalNames[58] = {
			"",
			"Jacket",
			"CorpseMovedFromFloor",
			"ReadLetterInAugustSuitcase",
			"FoundCorpse",
			"CharacterSearchingForCath",
			"PhaseOfTheNight",
			"CathIcon",
			"CorpseHasBeenThrown",
			"FrancoisHasSeenCorpseThrown",
			"AnnaIsEating",
			"Chapter",
			"DoneSavePointAfterLeftCompWithNewJacket",
			"MetAugust",
			"IsDayTime",
			"PoliceHasBoardedAndGone",
			"ConcertIsHappening",
			"KahinaKillTimeoutActive",
			"MaxHasToStayInBaggage",
			"UnknownDebugFlag",
			"TrainIsRunning",
			"AnnaIsInBaggageCar",
			"DoneSavePointAfterLeavingSuitcaseInCathComp",
			"TatianaFoundOutEggStolen",
			"OverheardAugustInterruptingAnnaAtDinner",
			"MetTatianaAndVassili",
			"OverheardTatianaAndAlexeiAtBreakfast",
			"KnowAboutAugust",
			"KnowAboutKronos",
			"EggIsOpen",
			"CanPlayKronosSuitcaseLeftInCompMusic",
			"CanPlayEggSuitcaseMusic",
			"CanPlayEggUnderSinkMusic",
			"CathInSpecialState",
			"OverheardAlexeiTellingTatianaAboutBomb",
			"OverheardAlexeiTellingTatianaAboutWantingToKillVassili",
			"OverheardTatianaAndAlexeiPlayingChess",
			"OverheardMilosAndVesnaConspiring",
			"OverheardVesnaAndMilosDebatingAboutCath",
			"FrancoisSawABlackBeetle",
			"OverheardMadameAndFrancoisTalkingAboutWhistle",
			"MadameDemandedMaxInBaggage",
			"MadameComplainedAboutMax",
			"MetMadame",
			"KnowAboutRebeccaDiary",
			"OverheardSophieTalkingAboutCath",
			"MetSophieAndRebecca",
			"KnowAboutRebeccaAndSophieRelationship",
			"RegisteredTimeAtWhichCathGaveFirebirdToKronos",
			"MetMahmud",
			"AlmostFallActionIsAvailable",
			"MetMilos",
			"MetMonsieur",
			"MetHadija",
			"MetYasmin",
			"MetAlouan",
			"MetFatima",
			"TatianaScheduledToVisitCath"
		};

		for (int i = 1; i < 58; i++) {
			ImGui::TableNextRow();

			// Make the entire row selectable, so I don't have to buy new glasses to see the name of the variable...
			ImGui::TableSetColumnIndex(0);
			bool isSelected = (_state->_selectedGlobalVarRow == i);
			if (ImGui::Selectable(("##row" + Common::String(i)).c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
				_state->_selectedGlobalVarRow = i;
			}

			ImGui::SameLine();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%d", i);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", globalNames[i]);

			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%d", _globals[i]);
		}

		ImGui::EndTable();
	}
}

Common::StringArray LogicManager::getCharacterFunctionNames(int character) {
	return _engine->isDemo() ? _demoFuncNames[character] : _funcNames[character];
}

void LogicManager::showTrainMapWindow() {
	ImVec4 restaurantColor = ImVec4(0.95f, 0.6f, 0.6f, 1.0f);    // Pink/red
	ImVec4 loungeColor = ImVec4(0.8f, 0.6f, 0.8f, 1.0f);         // Violet
	ImVec4 redSleepingColor = ImVec4(0.95f, 0.7f, 0.7f, 1.0f);   // Light red
	ImVec4 greenSleepingColor = ImVec4(0.7f, 0.95f, 0.7f, 1.0f); // Light green
	ImVec4 vestibuleColor = ImVec4(0.8f, 0.7f, 0.6f, 1.0f);      // Brown
	ImVec4 kronosColor = ImVec4(0.95f, 0.9f, 0.6f, 1.0f);        // Yellow
	ImVec4 characterColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);      // Yellow
	ImVec4 cathColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);           // Red
	ImVec4 voidColor = ImVec4(0.2f, 0.2f, 0.3f, 1.0f);           // Dark blue/purple
	ImVec4 corridorColor = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);       // Gray

	Common::HashMap<int, int> compartmentsPositions;
	compartmentsPositions[2740] = 7;
	compartmentsPositions[3050] = 6;
	compartmentsPositions[4070] = 5;
	compartmentsPositions[4840] = 4;
	compartmentsPositions[5790] = 3;
	compartmentsPositions[6470] = 2;
	compartmentsPositions[7500] = 1;
	compartmentsPositions[8200] = 0;

	// Train layout
	struct CarInfo {
		int id;
		const char *name;
		ImVec4 color;
		float width;
		int compartments;
		bool isSleeper;
	};

	CarInfo carsChapter1[] = {
		{ kCarRestaurant,    "Restaurant Car",     restaurantColor,    1.6f, 6, false },
		{ kCarRedSleeping,   "Red Sleeping Car",   redSleepingColor,   1.2f, 8, true  },
		{ kCarGreenSleeping, "Green Sleeping Car", greenSleepingColor, 1.2f, 8, true  },
		{ kCarVestibule,     "Vestibule",          vestibuleColor,     0.3f, 1, false },
		{ kCarKronos,        "Kronos Car",         kronosColor,        1.0f, 3, false }
	};

	CarInfo carsChapters23[] = {
		{ kCarLocomotive,    "Locomotive",         vestibuleColor,     0.5f, 1, false },
		{ kCarCoalTender,    "Coal Tender",        voidColor,          1.0f, 1, false },
		{ kCarBaggage,       "Baggage Car",        vestibuleColor,     1.0f, 1, false },
		{ kCarRestaurant,    "Restaurant Car",     restaurantColor,    1.6f, 6, false },
		{ kCarRedSleeping,   "Red Sleeping Car",   redSleepingColor,   1.2f, 8, true  },
		{ kCarGreenSleeping, "Green Sleeping Car", greenSleepingColor, 1.2f, 8, true  },
		{ kCarVestibule,     "Vestibule",          vestibuleColor,     0.3f, 1, false },
		{ kCarKronos,        "Kronos Car",         kronosColor,        1.0f, 3, false },
		{ kCarBaggageRear,   "Rear Baggage Car",   vestibuleColor,     1.0f, 1, false }
	};

	CarInfo carsChapter4[] = {
		{ kCarLocomotive,    "Locomotive",         vestibuleColor,     0.5f, 1, false },
		{ kCarCoalTender,    "Coal Tender",        voidColor,          1.0f, 1, false },
		{ kCarBaggage,       "Baggage Car",        vestibuleColor,     1.0f, 1, false },
		{ kCarRestaurant,    "Restaurant Car",     restaurantColor,    1.6f, 6, false },
		{ kCarRedSleeping,   "Red Sleeping Car",   redSleepingColor,   1.2f, 8, true  },
		{ kCarGreenSleeping, "Green Sleeping Car", greenSleepingColor, 1.2f, 8, true  },
		{ kCarVestibule,     "Vestibule",          vestibuleColor,     0.3f, 1, false },
		{ kCarBaggageRear,   "Rear Baggage Car",   vestibuleColor,     1.0f, 1, false }
	};

	CarInfo carsChapter5[] = {
		{ kCarLocomotive,    "Locomotive",         vestibuleColor,     0.5f, 1, false },
		{ kCarCoalTender,    "Coal Tender",        voidColor,          1.0f, 1, false },
		{ kCarBaggage,       "Baggage Car",        vestibuleColor,     1.0f, 1, false },
		{ kCarRestaurant,    "Restaurant Car",     restaurantColor,    1.6f, 6, false }
	};

	CarInfo *cars = nullptr;
	int carCount = 0;

	switch (_state->_engine->getLogicManager()->_globals[kGlobalChapter]) {
	case 1:
		cars = carsChapter1;
		carCount = ARRAYSIZE(carsChapter1);
		break;
	case 2:
	case 3:
	default:
		cars = carsChapters23;
		carCount = ARRAYSIZE(carsChapters23);
		break;
	case 4:
	case 5:
		if (_state->_engine->getLogicManager()->_globals[kGlobalChapter] == 5 &&
			(_state->_engine->getLogicManager()->_doneNIS[kEventAugustUnhookCars] || _state->_engine->getLogicManager()->_doneNIS[kEventAugustUnhookCarsBetrayal])) {
			cars = carsChapter5;
			carCount = ARRAYSIZE(carsChapter5);
		} else {
			cars = carsChapter4;
			carCount = ARRAYSIZE(carsChapter4);
		}

		break;
	}

	// Calculate total relative width to scale properly...
	float totalRelativeWidth = 0.2f;
	for (int i = 0; i < carCount; i++) {
		totalRelativeWidth += cars[i].width;
	}

	const float availableWidth = ImGui::GetContentRegionAvail().x - 20; // Subtract padding
	const float unitWidth = availableWidth / totalRelativeWidth;
	const float carHeight = 140;
	const float corridorHeight = 50;

	// Character position mapping...
	struct CharPos {
		int car;
		int compartment;
		bool inCorridor;
		int charIndex;
		int position;
	};

	Common::Array<CharPos> charPositions;
	Common::Array<CharPos> voidCharPositions;

	// Collect character positions...
	for (int i = 0; i < 40; i++) {
		// These are basically invisible entities, not real characters...
		if (i == kCharacterClerk || i == kCharacterMaster || i == kCharacterMitchell)
			continue;

		Character *character = &getCharacter(i);
		if (!character)
			continue;

		CharPos pos;
		pos.car = character->characterPosition.car;
		pos.compartment = compartmentsPositions.getValOrDefault(character->characterPosition.position);
		pos.inCorridor = character->characterPosition.location != 1;
		pos.charIndex = i;
		pos.position = character->characterPosition.position;

		if (pos.car == 0) {
			voidCharPositions.push_back(pos);
		} else {
			charPositions.push_back(pos);
		}
	}

	// Draw train cars...
	float carX = 10; // Starting position!

	for (int c = 0; c < carCount; c++) {
		CarInfo &car = cars[c];
		float carWidth = unitWidth * car.width;

		// Draw car outline...
		ImDrawList *drawList = ImGui::GetWindowDrawList();
		ImVec2 carMin;
		carMin.x = ImGui::GetWindowPos().x + carX;
		carMin.y = ImGui::GetWindowPos().y + 30;
		ImVec2 carMax;
		carMax.x = carMin.x + carWidth;
		carMax.y = carMin.y + carHeight;

		// Draw car body...
		drawList->AddRectFilled(
			carMin,
			carMax,
			ImGui::ColorConvertFloat4ToU32(car.color),
			5.0f
		);

		// Corridor position - default is center for non-sleeper cars...
		float corridorY = carMin.y + (carHeight - corridorHeight) / 2;

		// For sleeper cars, special layout...
		if (car.isSleeper) {
			// Sleeping car with 8 compartments;
			// position 8600 (left) to 2000 (right) mark the start and
			// the end (empirical values ;-) )...

			// Calculate the actual start and end points for compartments...
			float leftmostPos = 8600.0f;
			float rightmostPos = 2000.0f;

			// Calculate what percentage of the car width the compartments occupy...
			float leftEdgeRatio = 1.0f - (leftmostPos / 10000.0f);
			float rightEdgeRatio = 1.0f - (rightmostPos / 10000.0f);

			// Calculate the actual pixel positions...
			float compartmentLeftEdge = carMin.x + (carWidth * leftEdgeRatio);
			float compartmentRightEdge = carMin.x + (carWidth * rightEdgeRatio);

			// Draw corridor at the bottom (full width of car)...
			corridorY = carMin.y + carHeight - corridorHeight;
			drawList->AddRectFilled(
				ImVec2(carMin.x, corridorY),
				ImVec2(carMax.x, corridorY + corridorHeight),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f
			);

			// Draw corridor-colored areas on both sides of the compartments
			// (where the compartements rooms end and the door to the next one is nearby)

			// Left side (beyond compartments)
			drawList->AddRectFilled(
				ImVec2(carMin.x, carMin.y),
				ImVec2(compartmentLeftEdge, corridorY),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f
			);

			// Right side (beyond compartments)
			drawList->AddRectFilled(
				ImVec2(compartmentRightEdge, carMin.y),
				ImVec2(carMax.x, corridorY),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f
			);

			// Calculate width of each compartment...
			float compartmentWidth = (compartmentRightEdge - compartmentLeftEdge) / 8;

			// Draw the 8 compartments between the calculated edges...
			for (int i = 0; i <= 8; i++) {
				float x = compartmentLeftEdge + (i * compartmentWidth);

				// Draw compartment vertical walls...
				drawList->AddLine(
					ImVec2(x, carMin.y),
					ImVec2(x, corridorY),
					IM_COL32(0, 0, 0, 255),
					1.0f
				);
			}
		} else if (car.id == kCarRestaurant) {
			// Restaurant car with two sections: lounge and restaurant
			// Position 0-3749 = lounge (right side), 3750-10000 = restaurant (left side)

			// Calculate bounds...
			float loungeRatio = 3750.0f / 10000.0f;
			float loungeWidth = carWidth * loungeRatio;
			float restaurantWidth = carWidth - loungeWidth;

			// Draw lounge section (right side)...
			drawList->AddRectFilled(
				ImVec2(carMin.x + restaurantWidth, carMin.y),
				ImVec2(carMax.x, carMax.y),
				ImGui::ColorConvertFloat4ToU32(loungeColor),
				0.0f
			);

			// Dividing wall between lounge and restaurant...
			drawList->AddLine(
				ImVec2(carMin.x + restaurantWidth, carMin.y),
				ImVec2(carMin.x + restaurantWidth, carMax.y),
				IM_COL32(0, 0, 0, 255),
				2.0f
			);

			// Center corridor for the entire car...
			corridorY = carMin.y + (carHeight - corridorHeight) / 2;

			// Draw corridor through both sections...
			drawList->AddRectFilled(
				ImVec2(carMin.x, corridorY),
				ImVec2(carMax.x, corridorY + corridorHeight),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f
			);

			// Draw restaurant tables...
			float tableWidth = restaurantWidth / 3;

			for (int i = 0; i < 3; i++) {
				float tableX = carMin.x + i * tableWidth;

				drawList->AddLine(
					ImVec2(tableX, carMin.y),
					ImVec2(tableX, corridorY),
					IM_COL32(0, 0, 0, 255),
					1.0f
				);

				drawList->AddLine(
					ImVec2(tableX, corridorY + corridorHeight),
					ImVec2(tableX, carMax.y),
					IM_COL32(0, 0, 0, 255),
					1.0f
				);

				drawList->AddRectFilled(
					ImVec2(tableX + 5, carMin.y + 5),
					ImVec2(tableX + tableWidth - 5, corridorY - 5),
					IM_COL32(255, 255, 255, 255),
					3.0f
				);

				drawList->AddRectFilled(
					ImVec2(tableX + 5, corridorY + corridorHeight + 5),
					ImVec2(tableX + tableWidth - 5, carMax.y - 5),
					IM_COL32(255, 255, 255, 255),
					3.0f
				);
			}
		} else if (car.id == kCarKronos) {
			// Kronos car with 3 sections
			float sectionWidth = carWidth / 3;

			// Draw corridor
			drawList->AddRectFilled(
				ImVec2(carMin.x, corridorY),
				ImVec2(carMax.x, corridorY + corridorHeight),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f
			);

			for (int i = 0; i < 2; i++) {
				drawList->AddLine(
					ImVec2(carMin.x + (i + 1) * sectionWidth, carMin.y),
					ImVec2(carMin.x + (i + 1) * sectionWidth, carMax.y),
					IM_COL32(0, 0, 0, 255),
					1.0f
				);
			}
		} else {
			// Default corridor for other cars...
			drawList->AddRectFilled(
				ImVec2(carMin.x, corridorY),
				ImVec2(carMax.x, corridorY + corridorHeight),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f
			);
		}

		// Draw car name and ID...
		char carName[32];
		Common::sprintf_s(carName, "%s\n(%d)", car.name, car.id);
		ImVec2 textSize = ImGui::CalcTextSize(carName);
		drawList->AddText(
			ImVec2(carMin.x + (carWidth / 2) - (textSize.x / 2), carMax.y + 5),
			IM_COL32(255, 255, 255, 255),
			carName
		);

		// Draw characters in this car...
		for (uint i = 0; i < charPositions.size(); i++) {
			if (charPositions[i].car == car.id) {
				float charX, charY;

				if (charPositions[i].inCorridor) {
					// Calculate position along the car based on the 0-10000 range
					// where 0 = rightmost edge and 10000 = leftmost edge...
					float positionRatio = (float)charPositions[i].position / 10000.0f;

					// Flip the ratio since 0 is right and 10000 is left...
					positionRatio = 1.0f - positionRatio;

					// Set horizontal position based on the ratio...
					charX = carMin.x + (carWidth * positionRatio);

					// Set vertical position in the corridor...
					charY = corridorY + corridorHeight / 2;

					// Let's see the player marker a little better, shall we? :-)
					if (charPositions[i].charIndex == kCharacterCath) {
						charY -= 10;
					} else {
						charY += 10;
					}
				} else {
					// For characters not in corridor...
					int compartment = charPositions[i].compartment;

					if (car.isSleeper) {
						// Calculate compartment positions based on the 8600-2000 range...
						float leftmostPos = 8600.0f;
						float rightmostPos = 2000.0f;
						float leftEdgeRatio = 1.0f - (leftmostPos / 10000.0f);
						float rightEdgeRatio = 1.0f - (rightmostPos / 10000.0f);
						float compartmentLeftEdge = carMin.x + (carWidth * leftEdgeRatio);
						float compartmentRightEdge = carMin.x + (carWidth * rightEdgeRatio);
						float compartmentWidth = (compartmentRightEdge - compartmentLeftEdge) / 8;

						compartment = compartment % 8; // All 16 compartments are consecutive, so we mod by 8

						// Calculate character position based on compartment
						charX = compartmentLeftEdge + (compartment * compartmentWidth) + (compartmentWidth / 2);
						charY = carMin.y + (corridorY - carMin.y) / 2;
					} else if (car.id == kCarRestaurant) {
						// Restaurant car with lounge (0-3749) and restaurant (3750-10000) sections...
						float loungeRatio = 3750.0f / 10000.0f;
						float loungeWidth = carWidth * loungeRatio;
						float restaurantWidth = carWidth - loungeWidth;

						// Center corridor for restaurant car...
						float restCorridorY = carMin.y + (carHeight - corridorHeight) / 2;

						// Check if character is in lounge or restaurant section based on position...
						bool inLounge = (charPositions[i].position < 3750);

						if (inLounge) {
							// Lounge area (right side of car)...
							charX = carMin.x + restaurantWidth + (loungeWidth / 2);
							charY = carMin.y + carHeight / 2;
						} else {
							// Restaurant area (left side of car)...
							float tableWidth = restaurantWidth / 3;

							compartment = compartment % 6;

							if (compartment < 3) { // Top side
								charX = carMin.x + compartment * tableWidth + tableWidth / 2;
								charY = carMin.y + (restCorridorY - carMin.y) / 2;
							} else { // Bottom side
								charX = carMin.x + (compartment - 3) * tableWidth + tableWidth / 2;
								charY = restCorridorY + corridorHeight + (carMax.y - (restCorridorY + corridorHeight)) / 2;
							}
						}
					} else if (car.id == kCarKronos) {
						float sectionWidth = carWidth / 3;

						compartment = compartment % 3;

						charX = carMin.x + compartment * sectionWidth + sectionWidth / 2;
						charY = carMin.y + carHeight / 2;
					} else {
						// Vestibule car or whatever else...
						charX = carMin.x + carWidth / 2;
						charY = carMin.y + carHeight / 2;
					}
				}

				// Draw character marker...
				drawList->AddCircleFilled(
					ImVec2(charX, charY),
					8.0f,
					ImGui::ColorConvertFloat4ToU32(charPositions[i].charIndex == kCharacterCath ? cathColor : characterColor),
					12
				);

				char charId[8];
				Common::sprintf_s(charId, "%d", charPositions[i].charIndex);
				drawList->AddText(
					ImVec2(charX - ImGui::CalcTextSize(charId).x / 2, charY - ImGui::CalcTextSize(charId).y / 2),
					charPositions[i].charIndex == kCharacterCath ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255),
					charId
				);

				// Tooltip!
				ImGui::SetCursorScreenPos(ImVec2(charX - 8, charY - 8));
				ImGui::InvisibleButton(charId, ImVec2(16, 16));
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("%s", getCharacterName(charPositions[i].charIndex));
					ImGui::Text("Car: %d, Loc: %d, Pos: %d",
								charPositions[i].car,
								getCharacter(charPositions[i].charIndex).characterPosition.location,
								getCharacter(charPositions[i].charIndex).characterPosition.position
					);
					ImGui::EndTooltip();
				}
			}
		}

		// Move to next car...
		carX += carWidth + 5;
	}

	// Draw "The Void" section at the bottom...
	if (voidCharPositions.size() > 0) {
		ImDrawList *drawList = ImGui::GetWindowDrawList();

		// Area...
		ImVec2 voidMin;
		voidMin.x = ImGui::GetWindowPos().x + 10;
		voidMin.y = ImGui::GetWindowPos().y + 205; // Below the train cars
		ImVec2 voidMax;
		voidMax.x = ImGui::GetWindowPos().x + ImGui::GetContentRegionAvail().x - 10;
		voidMax.y = voidMin.y + 60;

		// Rectangle...
		drawList->AddRectFilled(
			voidMin,
			voidMax,
			ImGui::ColorConvertFloat4ToU32(voidColor),
			5.0f
		);

		// Label...
		const char *voidLabel = "THE VOID (id: 0)";
		drawList->AddText(
			ImVec2(voidMin.x + 10, voidMin.y + 5),
			IM_COL32(255, 255, 255, 255),
			voidLabel
		);

		// Character markers...
		float charSpacing = (voidMax.x - voidMin.x - 20) / (voidCharPositions.size() + 1);
		for (uint i = 0; i < voidCharPositions.size(); i++) {
			float charX = voidMin.x + 10 + charSpacing * (i + 1);
			float charY = voidMin.y + 35;

			drawList->AddCircleFilled(
				ImVec2(charX, charY),
				8.0f,
				ImGui::ColorConvertFloat4ToU32(characterColor),
				12
			);

			char charId[16];
			Common::sprintf_s(charId, "%d", voidCharPositions[i].charIndex);
			drawList->AddText(
				ImVec2(charX - ImGui::CalcTextSize(charId).x / 2, charY - ImGui::CalcTextSize(charId).y / 2),
				IM_COL32(0, 0, 0, 255),
				charId
			);

			// Tooltip! :-)
			ImGui::SetCursorScreenPos(ImVec2(charX - 8, charY - 8));
			ImGui::InvisibleButton(charId, ImVec2(16, 16));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("%s", getCharacterName(voidCharPositions[i].charIndex));
				ImGui::Text("Car: 0, Loc: %d, Pos: %d",
							getCharacter(voidCharPositions[i].charIndex).characterPosition.location,
							getCharacter(voidCharPositions[i].charIndex).characterPosition.position
				);
				ImGui::EndTooltip();
			}
		}
	}
}

void LogicManager::renderCharacterGrid(bool onlyPinned, int &selectedCharacter) {
	const int charsPerRow = 3;
	int displayed = 0;

	float windowWidth = ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x - ImGui::GetWindowPos().x;
	float cardWidth = (windowWidth / charsPerRow) - 8;

	for (int i = 0; i < 40; i++) {
		// Get (and filter) character...
		Character *character = &getCharacter(i);
		if (!character)
			continue;

		if (onlyPinned && !isCharacterPinned(i))
			continue;

		char buffer[64];
		Common::sprintf_s(buffer, "%s (%d)", getCharacterName(i), i);
		if (!_state->_filter.PassFilter(buffer))
			continue;

		if (displayed > 0 && displayed % charsPerRow == 0) {
			ImGui::NewLine();
		} else if (displayed > 0) {
			ImGui::SameLine(displayed % charsPerRow * (cardWidth + 8));
		}

		displayed++;

		// Create a card for the character...
		ImGui::PushID(i);
		ImGui::BeginChild(ImGui::GetID((void *)(intptr_t)i), ImVec2(cardWidth, 180), true);

		if (isCharacterPinned(i)) {
			ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s (%d)", getCharacterName(i), i);
			ImGui::SameLine(ImGui::GetWindowWidth() - 50);
			if (ImGui::SmallButton("Unpin")) {
				toggleCharacterPin(i);
			}
		} else {
			ImGui::Text("%s (%d)", getCharacterName(i), i);
			ImGui::SameLine(ImGui::GetWindowWidth() - 40);
			if (ImGui::SmallButton("Pin")) {
				toggleCharacterPin(i);
			}
		}

		ImGui::Separator();

		Common::StringArray funcNames = getCharacterFunctionNames(i);

		Common::String funcCurrName = "NONE";
		if ((uint)(character->callbacks[character->currentCall] - 1) < funcNames.size())
			funcCurrName = funcNames[character->callbacks[character->currentCall] - 1];

		// Cath (the player) doesn't have logic functions...
		if (i != 0) {
			ImGui::Text("Current logic function: %s (%d)", funcCurrName.c_str(), character->callbacks[character->currentCall]);
		}

		ImGui::Separator();

		ImGui::Text("Position: Car %u, Loc %u, Pos %u",
					character->characterPosition.car,
					character->characterPosition.location,
					character->characterPosition.position
		);

		ImGui::Text("Sequence: %s", character->sequenceName);
		ImGui::Text("Direction: %d", character->direction);
		ImGui::Text("Current Frame: Seq1 %d / Seq2 %d", character->currentFrameSeq1, character->currentFrameSeq2);

		if (ImGui::Button("View Details")) {
			_state->_selectedCharacter = i;
			_state->_forceReturnToListView = true;
		}

		ImGui::EndChild();
		ImGui::PopID();
	}

	if (displayed == 0) {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
						   onlyPinned ? "No pinned characters match the filter" : "No characters match the filter"
		);
	}
}

void LogicManager::renderCharacterDetails(Character *character, int index) {
	ImGui::Text("%s (Character ID: %d)", getCharacterName(index), index);

	ImGui::SameLine(ImGui::GetWindowWidth() - 100);
	if (ImGui::Button(isCharacterPinned(index) ? "Unpin Character" : "Pin Character")) {
		toggleCharacterPin(index);
	}

	ImGui::Separator();

	ImGui::Text("Position: Car %u, Location %u, Position %u",
				character->characterPosition.car,
				character->characterPosition.location,
				character->characterPosition.position
	);

	Common::StringArray funcNames = getCharacterFunctionNames(index);

	Common::String funcCurrName = "NONE";
	if ((uint)(character->callbacks[character->currentCall] - 1) < funcNames.size())
		funcCurrName = funcNames[character->callbacks[character->currentCall] - 1];

	// Cath (the player) doesn't have logic functions...
	if (index != 0) {
		ImGui::Text("Current logic function: %s (%d)", funcCurrName.c_str(), character->callbacks[character->currentCall]);
	}

	if (ImGui::CollapsingHeader("Logic call stack", ImGuiTreeNodeFlags_DefaultOpen)) {
		// Start with the current call...
		int currentDepth = character->currentCall;

		if (currentDepth < 0 || character->callbacks[currentDepth] == 0) {
			ImGui::Text("No active logic functions");
		} else {
			ImGui::Text("Call stack depth: %d", currentDepth + 1);

			// Display the call stack...
			for (int i = currentDepth; i >= 0; i--) {
				int functionId = character->callbacks[i];

				float indentAmount = (currentDepth - i) * 20.0f;
				indentAmount = indentAmount > 0.0f ? indentAmount : 0.1f;

				ImGui::Indent(indentAmount);


				ImGui::BulletText("Level %d: %s (#%d)",
								  currentDepth - i,
								  funcNames[functionId - 1].c_str(),
								  functionId
				);

				ImGui::Unindent(indentAmount);
			}
		}
	}


	if (ImGui::CollapsingHeader("Animation State", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Sequence: %s", character->sequenceName);
		ImGui::Text("Sequence 2: %s", character->sequenceName2);
		ImGui::Text("Prefix: %s", character->sequenceNamePrefix);
		ImGui::Text("Copy: %s", character->sequenceNameCopy);
		ImGui::Text("Current Frame: Seq1 %d / Seq2 %d", character->currentFrameSeq1, character->currentFrameSeq2);
		ImGui::Text("Waited ticks until cycle restart: %d", character->waitedTicksUntilCycleRestart);
		ImGui::Text("Elapsed Frames: %d", character->elapsedFrames);
	}

	if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Direction: %d", character->direction);
		ImGui::Text("Walk Counter: %d", character->walkCounter);
		ImGui::Text("Walk Step Size: %d", character->walkStepSize);
		ImGui::Text("Direction Switch: %d", character->directionSwitch);
	}

	if (ImGui::CollapsingHeader("State", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Current Call: %d", character->currentCall);
		ImGui::Text("Inventory Item: %d", character->inventoryItem);
		ImGui::Text("Clothes: %d", character->clothes);
		ImGui::Text("Attached Conductor: %d", character->attachedConductor);
		ImGui::Text("Process Entity: %d", character->doProcessEntity);
		ImGui::Text("Previous Car: %d", character->car2);
		ImGui::Text("Previous Position: %d", character->position2);
		ImGui::Text("Position fudge flag: %d", character->needsPosFudge);
		ImGui::Text("Position fudge flag (secondary): %d", character->needsSecondaryPosFudge);
	}

	if (ImGui::CollapsingHeader("Call Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (int call = 0; call < 9; call++) {
			char label[32];
			Common::sprintf_s(label, "Call %d", call);

			// Skip calls with all zero parameters...
			bool hasNonZeroParams = false;
			for (int param = 0; param < 32; param++) {
				if (character->callParams[call].parameters[param] != 0) {
					hasNonZeroParams = true;
					break;
				}
			}

			if (!hasNonZeroParams) {
				continue;
			}

			if (ImGui::TreeNode(label)) {
				for (int param = 0; param < 32; param++) {
					if (character->callParams[call].parameters[param] != 0) {
						ImGui::Text("Param %d: %d", param,
									character->callParams[call].parameters[param]);
					}
				}

				ImGui::TreePop();
			}
		}
	}

	if (ImGui::CollapsingHeader("Callbacks")) {
		bool hasCallbacks = false;

		for (int i = 0; i < 16; i++) {
			if (character->callbacks[i] != 0) {
				ImGui::Text("Callback %d: %d", i, character->callbacks[i]);
				hasCallbacks = true;
			}
		}

		if (!hasCallbacks) {
			ImGui::Text("No active callbacks");
		}
	}
}

const char *LogicManager::getCharacterName(int index) const {
	if (index < 0 || index >= 40) {
		return "Unknown";
	}

	return _characterNames[index];
}

bool LogicManager::isCharacterPinned(int index) const {
	if (index < 0 || index >= 40) {
		return false;
	}
	return _pinnedCharacters[index];
}

void LogicManager::toggleCharacterPin(int index) {
	if (index >= 0 && index < 40) {
		_pinnedCharacters[index] = !_pinnedCharacters[index];
	}
}

void LastExpressEngine::showEngineInfo() {
	ImGui::Text("Mouse status:");
	ImGui::BulletText("Is drawn: %s", getGraphicsManager()->canDrawMouse() ? "yes" : "no");
	ImGui::BulletText("Has left clicked: %s", mouseHasLeftClicked() ? "yes" : "no");
	ImGui::BulletText("Has right clicked: %s", mouseHasRightClicked() ? "yes" : "no");
	ImGui::BulletText("Fast walk active: %s", getLogicManager()->_doubleClickFlag ? "yes" : "no");

	if (!getMenu()->isShowingMenu()) {
		ImGui::Separator();

		ImGui::Text("Utilities (careful!):");
		ImGui::Checkbox("Lock grace period", &_lockGracePeriod);

		ImGui::NewLine();

		ImGui::Text("Advance time by:");

		ImGui::InputInt("ticks", &_state->_ticksToAdvance, 100, 1000);
		ImGui::SameLine();
		bool shouldAdvance = ImGui::Button("Go");
		if (shouldAdvance) {
			getLogicManager()->_gameTime += _state->_ticksToAdvance;
			getLogicManager()->_realTime += _state->_ticksToAdvance;
			_state->_ticksToAdvance = 0;
		}
	}
}

#endif

Debugger::Debugger(LastExpressEngine *engine) : _engine(engine) {
	//////////////////////////////////////////////////////////////////////////
	// Register the debugger commands
	//////////////////////////////////////////////////////////////////////////

	// General
	registerCmd("help",      WRAP_METHOD(Debugger, cmdHelp));
}

Debugger::~Debugger() {
	// Zero passed pointers
	_engine = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Debugger commands
//////////////////////////////////////////////////////////////////////////
bool Debugger::cmdHelp(int, const char **) {
	debugPrintf("No commands");
	debugPrintf("\n");
	return true;
}

} // End of namespace LastExpress
