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
#include "lastexpress/data/animation.h"
#include "lastexpress/data/background.h"
#include "lastexpress/data/cursor.h"
#include "lastexpress/data/scene.h"
#include "lastexpress/data/sequence.h"
#include "lastexpress/data/subtitle.h"

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/beetle.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/menu/clock.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

#include "common/debug-channels.h"
#include "common/md5.h"

#include "backends/imgui/imgui.h"

namespace LastExpress {

#ifdef USE_IMGUI

typedef struct ImGuiState {
	LastExpressEngine *_engine = nullptr;
	ImGuiTextFilter _filter;
	int _currentTab = 0;
	int _selectedCharacter = -1;
	bool _forceReturnToListView = false;
} ImGuiState;

ImGuiState *_state = nullptr;

void onImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 

	_state = new ImGuiState();
	_state->_engine = (LastExpressEngine *)g_engine;
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugConsole)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}

	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	// Create a single debug window
	ImGui::SetNextWindowSize(ImVec2(1400, 900), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_FirstUseEver);

	// Static variables to track layout sizes
	static float rightPanelWidth = 300.0f;
	static float bottomPanelHeight = 280.0f;
	static float rightTopPanelHeight = 200.0f;

	// Disable the debugger when the NIS engine is running...
	if (_state->_engine->getNISManager()->getNISFlag() & kNisFlagPlaying)
		return;

	// Disable the debugger when the fighting engine is running...
	if (_state->_engine->_fight)
		return;

	if (ImGui::Begin("Last Express Debugger")) {
		// Get available content area
		ImVec2 windowSize = ImGui::GetContentRegionAvail();

		// Right panel splitter
		{
			ImGui::SameLine(windowSize.x - rightPanelWidth - 8);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.3f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::Button("##vsplitter", ImVec2(8, windowSize.y - bottomPanelHeight));
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			if (ImGui::IsItemActive())
				rightPanelWidth += ImGui::GetIO().MouseDelta.x * -1.0f;

			// Clamp to reasonable values
			rightPanelWidth = CLIP<float>(rightPanelWidth, 150.0f, windowSize.x * 0.7f);
		}

		// Bottom panel splitter
		{
			float splitterY = windowSize.y - bottomPanelHeight - 8;
			ImGui::SetCursorPosY(splitterY);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.3f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::Button("##hsplitter", ImVec2(windowSize.x, 8));
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			if (ImGui::IsItemActive())
				bottomPanelHeight += ImGui::GetIO().MouseDelta.y * -1.0f;

			// Clamp to reasonable values
			bottomPanelHeight = CLIP<float>(bottomPanelHeight, 150.0f, windowSize.y * 0.7f);
		}

		// Right panel splitter (between top and bottom sections)
		{
			ImGui::SetCursorPos(ImVec2(windowSize.x - rightPanelWidth, rightTopPanelHeight));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.3f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::Button("##rightsplitter", ImVec2(rightPanelWidth, 8));
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			if (ImGui::IsItemActive())
				rightTopPanelHeight += ImGui::GetIO().MouseDelta.y;

			// Clamp to reasonable values
			rightTopPanelHeight = CLIP<float>(rightTopPanelHeight, 100.0f, windowSize.y - bottomPanelHeight - 100.0f);
		}

		// Main area dimensions (left of right panel, above bottom panel)
		float mainAreaWidth = windowSize.x - rightPanelWidth - 8;
		float mainAreaHeight = windowSize.y - bottomPanelHeight - 8;

		// Top-left: Character Debugger
		ImGui::SetCursorPos(ImVec2(0, 16));
		ImGui::BeginChild("CharacterDebugger", ImVec2(mainAreaWidth, mainAreaHeight), true);
		{
			// Tab bar and all character debugger content
			ImGuiTabItemFlags flags = 0;

			// Tab bar for different views
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

			// Filter for character names
			_state->_filter.Draw("Filter Characters", 180);
			ImGui::SameLine();
			if (ImGui::Button("Clear")) {
				_state->_filter.Clear();
			}

			// Show corresponding view based on selected tab
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
			}
		}
		ImGui::EndChild();

		// Top-right panels
		// Top-right is Clock
		ImGui::SetCursorPos(ImVec2(mainAreaWidth + 8, 16));
		ImGui::BeginChild("Clock", ImVec2(rightPanelWidth, rightTopPanelHeight), true);
		{
			_state->_engine->getClock()->showCurrentTime();
		}
		ImGui::EndChild();

		// Bottom-right is Where's Cath?
		ImGui::SetCursorPos(ImVec2(mainAreaWidth + 8, rightTopPanelHeight + 8));
		ImGui::BeginChild("CathInfo", ImVec2(rightPanelWidth, mainAreaHeight - rightTopPanelHeight - 8), true);
		{
			ImGui::Text("Where's Cath?");
			ImGui::Separator();
			_state->_engine->getLogicManager()->showCurrentTrainNode();
		}
		ImGui::EndChild();

		// Bottom-right is Where's Cath?
		ImGui::SetCursorPos(ImVec2(mainAreaWidth + 8, rightTopPanelHeight * 2 - 16));
		ImGui::BeginChild("EngineInfo", ImVec2(rightPanelWidth, mainAreaHeight - (rightTopPanelHeight * 2 - 16)), true);
		{
			ImGui::Text("Engine Info");
			ImGui::Separator();
			_state->_engine->showEngineInfo();
		}
		ImGui::EndChild();


		// Bottom panel: Train Map (full width)
		ImGui::SetCursorPos(ImVec2(0, windowSize.y - bottomPanelHeight));
		ImGui::BeginChild("TrainMap", ImVec2(windowSize.x, bottomPanelHeight), true);
		{
			// Draw train map without the window decorations
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
	int timeSource = (int)_engine->getLogicManager()->getGameTime();

	int hours = timeSource % 1296000 / 54000;
	int minutes = timeSource % 54000 / 900 % 60 % 60;
	int seconds = (timeSource % 900) / 15;

	char clockText[32];
	Common::sprintf_s(clockText, "%02d:%02d:%02d", hours, minutes, seconds);

	ImDrawList *drawList = ImGui::GetWindowDrawList();
	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImVec2 windowCenter = ImVec2(
		windowPos.x + ImGui::GetWindowSize().x / 2,
		windowPos.y + ImGui::GetWindowSize().y / 2);

	drawList->AddText(
		ImGui::GetFont(),
		4.0f * ImGui::GetFontSize(),
		ImVec2(windowPos.x + 40, windowCenter.y - 20),
		IM_COL32(255, 255, 255, 255),
		clockText
	);

	char dayText[32];
	int dayOffset = timeSource / 1296000;
	Common::sprintf_s(dayText, "July %d, 1914", 24 + dayOffset); // Calculate date based on days since start

	drawList->AddText(
		ImGui::GetFont(),
		2.0f * ImGui::GetFontSize(),
		ImVec2(windowPos.x + 60, windowCenter.y + 35),
		IM_COL32(255, 255, 255, 255),
		dayText
	);


	ImGui::Text("Game time: %d", _engine->getLogicManager()->getGameTime());
	ImGui::Text("Game time ticks: %d", _engine->getLogicManager()->getGameTimeTicks());
	ImGui::Text("Game time ticks delta: %d", _engine->getLogicManager()->getGameTimeTicksDelta());
	ImGui::Text("Grace timer: %d", _engine->_gracePeriodTimer);
	ImGui::Separator();
}

void LogicManager::showCurrentTrainNode() {
	ImGui::Text("Flags: %u", _trainData[_trainNodeIndex].car);
	ImGui::Text("Direction: %u", _trainData[_trainNodeIndex].cathDir);
	ImGui::Text("Node position:");
	ImGui::BulletText("Car %u", _trainData[_trainNodeIndex].nodePosition.car);
	ImGui::BulletText("Location %u", _trainData[_trainNodeIndex].nodePosition.location);
	ImGui::BulletText("Position %u", _trainData[_trainNodeIndex].nodePosition.position);

	ImGui::Text("Parameters: %u %u %u", _trainData[_trainNodeIndex].parameter1, _trainData[_trainNodeIndex].parameter2, _trainData[_trainNodeIndex].parameter3);
	ImGui::Text("Scene filename: %s", _trainData[_trainNodeIndex].sceneFilename);
}

void LogicManager::showCharacterDebugger() {
	ImGuiTabItemFlags flags = 0;

	// Tab bar for different views
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

	// Filter for character names
	_state->_filter.Draw("Filter Characters", 180);
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		_state->_filter.Clear();
	}

	// Show corresponding view based on selected tab
	switch (_state->_currentTab) {
	case 0: // List View
		renderCharacterList(_state->_selectedCharacter);
		break;
	case 1:                         // Grid View
		renderCharacterGrid(false, _state->_selectedCharacter); // Show all characters
		break;
	case 2:                        // Pinned Characters
		renderCharacterGrid(true, _state->_selectedCharacter); // Show only pinned characters
		break;
	}
}

// List view with details panel
void LogicManager::renderCharacterList(int &selectedCharacter) {
	// Left panel: Character list
	ImGui::BeginChild("CharacterList", ImVec2(200, 0), true);

	for (int i = 0; i < 40; i++) {
		// Get character and skip if it doesn't match filter
		Character *character = &getCharacter(i);
		if (!character)
			continue;

		// Format display string and filter
		char buffer[64];
		Common::sprintf_s(buffer, "%s (%d)", getCharacterName(i), i);
		if (!_state->_filter.PassFilter(buffer))
			continue;

		// Check if character is pinned
		bool isPinned = isCharacterPinned(i);
		if (isPinned) {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 220, 0, 255)); // Yellow for pinned
		}

		// Make the character selectable
		if (ImGui::Selectable(buffer, selectedCharacter == i)) {
			selectedCharacter = i;
		}

		if (isPinned) {
			ImGui::PopStyleColor();
		}

		// Right-click menu for pinning/unpinning
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

	if (selectedCharacter >= 0 && selectedCharacter < 40) {
		Character *character = &getCharacter(selectedCharacter);
		if (character) {
			renderCharacterDetails(character, selectedCharacter);
		}
	} else {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select a character from the list to view details");
	}

	ImGui::EndChild();
}

Common::StringArray LogicManager::getCharacterFunctionNames(int character) {
	return _engine->isDemo() ? _demoFuncNames[character] : _funcNames[character];
}

void LogicManager::showTrainMapWindow() {
	// Colors for different cars
	ImVec4 restaurantColor = ImVec4(0.95f, 0.6f, 0.6f, 1.0f);    // Pink/red for restaurant
	ImVec4 loungeColor = ImVec4(0.8f, 0.6f, 0.8f, 1.0f);         // Violet for lounge
	ImVec4 redSleepingColor = ImVec4(0.95f, 0.7f, 0.7f, 1.0f);   // Light red for red sleeping car
	ImVec4 greenSleepingColor = ImVec4(0.7f, 0.95f, 0.7f, 1.0f); // Light green for green sleeping car
	ImVec4 vestibuleColor = ImVec4(0.8f, 0.7f, 0.6f, 1.0f);      // Brown for vestibule
	ImVec4 kronosColor = ImVec4(0.95f, 0.9f, 0.6f, 1.0f);        // Yellow for Kronos car
	ImVec4 characterColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);      // Yellow for characters
	ImVec4 cathColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);           // Red for Cath
	ImVec4 voidColor = ImVec4(0.2f, 0.2f, 0.3f, 1.0f);           // Dark blue/purple for void
	ImVec4 corridorColor = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);       // Gray for corridors

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
		int id;           // Car ID number
		const char *name; // Car name
		ImVec4 color;     // Car color
		float width;      // Relative width (1.0 = standard)
		int compartments; // Number of compartments
		bool isSleeper;   // Is it a sleeping car?
	};

	CarInfo cars[] = {
		{kCarRestaurant, "Restaurant Car", restaurantColor, 1.6f, 6, false},
		{kCarRedSleeping, "Red Sleeping Car", redSleepingColor, 1.2f, 8, true},
		{kCarGreenSleeping, "Green Sleeping Car", greenSleepingColor, 1.2f, 8, true},
		{kCarVestibule, "Vestibule", vestibuleColor, 0.3f, 1, false},
		{kCarKronos, "Kronos Car", kronosColor, 1.0f, 3, false}};

	const int carCount = sizeof(cars) / sizeof(cars[0]);

	// Calculate total relative width to scale properly
	float totalRelativeWidth = 0;
	for (int i = 0; i < carCount; i++) {
		totalRelativeWidth += cars[i].width;
	}

	const float availableWidth = ImGui::GetContentRegionAvail().x - 20; // Subtract padding
	const float unitWidth = availableWidth / totalRelativeWidth;
	const float carHeight = 140;
	const float corridorHeight = 50;

	// Character position mapping
	struct CharPos {
		int car;
		int compartment;
		bool inCorridor;
		int charIndex; // Store the character index
		int position;
	};
	Common::Array<CharPos> charPositions;
	Common::Array<CharPos> voidCharPositions; // For characters in the void (car 0)

	// Collect character positions
	for (int i = 0; i < 40; i++) {
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

	// Draw train cars
	float carX = 10; // Starting position

	for (int c = 0; c < carCount; c++) {
		CarInfo &car = cars[c];
		float carWidth = unitWidth * car.width;

		// Draw car outline
		ImDrawList *drawList = ImGui::GetWindowDrawList();
		ImVec2 carMin;
		carMin.x = ImGui::GetWindowPos().x + carX;
		carMin.y = ImGui::GetWindowPos().y + 30;
		ImVec2 carMax;
		carMax.x = carMin.x + carWidth;
		carMax.y = carMin.y + carHeight;

		// Draw car body
		drawList->AddRectFilled(
			carMin,
			carMax,
			ImGui::ColorConvertFloat4ToU32(car.color),
			5.0f);

		// Corridor position - default is center for non-sleeper cars
		float corridorY = carMin.y + (carHeight - corridorHeight) / 2;

		// For sleeper cars, special layout
		if (car.isSleeper) {
			// Sleeping car with 8 compartments with specific positioning
			// Position 8600 (left) to 2000 (right) for compartments

			// Calculate the actual start and end points for compartments
			float leftmostPos = 8600.0f;
			float rightmostPos = 2000.0f;

			// Calculate what percentage of the car width the compartments occupy
			float leftEdgeRatio = 1.0f - (leftmostPos / 10000.0f);
			float rightEdgeRatio = 1.0f - (rightmostPos / 10000.0f);

			// Calculate the actual pixel positions
			float compartmentLeftEdge = carMin.x + (carWidth * leftEdgeRatio);
			float compartmentRightEdge = carMin.x + (carWidth * rightEdgeRatio);

			// Draw corridor at the bottom (full width of car)
			corridorY = carMin.y + carHeight - corridorHeight;
			drawList->AddRectFilled(
				ImVec2(carMin.x, corridorY),
				ImVec2(carMax.x, corridorY + corridorHeight),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f);

			// Draw corridor-colored areas on both sides of the compartments
			// Left side (beyond compartments)
			drawList->AddRectFilled(
				ImVec2(carMin.x, carMin.y),
				ImVec2(compartmentLeftEdge, corridorY),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f);

			// Right side (beyond compartments)
			drawList->AddRectFilled(
				ImVec2(compartmentRightEdge, carMin.y),
				ImVec2(carMax.x, corridorY),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f);

			// Calculate width of each compartment
			float compartmentWidth = (compartmentRightEdge - compartmentLeftEdge) / 8;

			// Draw the 8 compartments between the calculated edges
			for (int i = 0; i <= 8; i++) {
				float x = compartmentLeftEdge + (i * compartmentWidth);

				// Draw compartment divisions
				drawList->AddLine(
					ImVec2(x, carMin.y),
					ImVec2(x, corridorY),
					IM_COL32(0, 0, 0, 255),
					1.0f);
			}
		} else if (car.id == kCarRestaurant) {
			// Restaurant car with two sections: lounge and restaurant
			// Position 0-3749 = lounge (right side), 3750-10000 = restaurant (left side)

			// Calculate boundaries
			float loungeRatio = 3750.0f / 10000.0f;
			float loungeWidth = carWidth * loungeRatio;
			float restaurantWidth = carWidth - loungeWidth;

			// Draw lounge section (right side)
			drawList->AddRectFilled(
				ImVec2(carMin.x + restaurantWidth, carMin.y),
				ImVec2(carMax.x, carMax.y),
				ImGui::ColorConvertFloat4ToU32(loungeColor),
				0.0f);

			// Dividing line between lounge and restaurant
			drawList->AddLine(
				ImVec2(carMin.x + restaurantWidth, carMin.y),
				ImVec2(carMin.x + restaurantWidth, carMax.y),
				IM_COL32(0, 0, 0, 255),
				2.0f);

			// Center corridor for the entire car
			corridorY = carMin.y + (carHeight - corridorHeight) / 2;

			// Draw corridor through both sections
			drawList->AddRectFilled(
				ImVec2(carMin.x, corridorY),
				ImVec2(carMax.x, corridorY + corridorHeight),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f);

			// Draw restaurant tables (in the restaurant section only)
			float tableWidth = restaurantWidth / 3;

			for (int i = 0; i < 3; i++) {
				// Draw table divisions
				float tableX = carMin.x + i * tableWidth;

				drawList->AddLine(
					ImVec2(tableX, carMin.y),
					ImVec2(tableX, corridorY),
					IM_COL32(0, 0, 0, 255),
					1.0f);

				drawList->AddLine(
					ImVec2(tableX, corridorY + corridorHeight),
					ImVec2(tableX, carMax.y),
					IM_COL32(0, 0, 0, 255),
					1.0f);

				// Draw tables as white rectangles
				drawList->AddRectFilled(
					ImVec2(tableX + 5, carMin.y + 5),
					ImVec2(tableX + tableWidth - 5, corridorY - 5),
					IM_COL32(255, 255, 255, 255),
					3.0f);

				drawList->AddRectFilled(
					ImVec2(tableX + 5, corridorY + corridorHeight + 5),
					ImVec2(tableX + tableWidth - 5, carMax.y - 5),
					IM_COL32(255, 255, 255, 255),
					3.0f);
			}
		} else if (car.id == kCarKronos) {
			// Kronos car with 3 sections
			float sectionWidth = carWidth / 3;

			// Draw corridor
			drawList->AddRectFilled(
				ImVec2(carMin.x, corridorY),
				ImVec2(carMax.x, corridorY + corridorHeight),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f);

			for (int i = 0; i < 2; i++) {
				drawList->AddLine(
					ImVec2(carMin.x + (i + 1) * sectionWidth, carMin.y),
					ImVec2(carMin.x + (i + 1) * sectionWidth, carMax.y),
					IM_COL32(0, 0, 0, 255),
					1.0f);
			}
		} else {
			// Default corridor for other cars
			drawList->AddRectFilled(
				ImVec2(carMin.x, corridorY),
				ImVec2(carMax.x, corridorY + corridorHeight),
				ImGui::ColorConvertFloat4ToU32(corridorColor),
				0.0f);
		}

		// Draw car name and ID
		char carName[32];
		Common::sprintf_s(carName, "%s (%d)", car.name, car.id);
		ImVec2 textSize = ImGui::CalcTextSize(carName);
		drawList->AddText(
			ImVec2(carMin.x + (carWidth / 2) - (textSize.x / 2), carMax.y + 5),
			IM_COL32(255, 255, 255, 255),
			carName);

		// Draw characters in this car
		for (int i = 0; i < charPositions.size(); i++) {
			if (charPositions[i].car == car.id) {
				// Determine character position
				float charX, charY;

				// If in corridor, position based on position value (0-10000)
				if (charPositions[i].inCorridor) {
					// Calculate position along the car based on the 0-10000 range
					// where 0 = rightmost edge and 10000 = leftmost edge
					float positionRatio = (float)charPositions[i].position / 10000.0f;

					// Flip the ratio since 0 is right and 10000 is left
					positionRatio = 1.0f - positionRatio;

					// Set horizontal position based on the ratio
					charX = carMin.x + (carWidth * positionRatio);

					// Set vertical position in the corridor
					charY = corridorY + corridorHeight / 2;

					if (charPositions[i].charIndex == kCharacterCath) {
						charY -= 10;
					} else {
						charY += 10;
					}
				} else {
					// For characters not in corridor, position in compartment
					int compartment = charPositions[i].compartment;

					// Position adapts to car type
					if (car.isSleeper) {
						// Calculate compartment positions based on 8600-2000 range
						float leftmostPos = 8600.0f;
						float rightmostPos = 2000.0f;
						float leftEdgeRatio = 1.0f - (leftmostPos / 10000.0f);
						float rightEdgeRatio = 1.0f - (rightmostPos / 10000.0f);
						float compartmentLeftEdge = carMin.x + (carWidth * leftEdgeRatio);
						float compartmentRightEdge = carMin.x + (carWidth * rightEdgeRatio);
						float compartmentWidth = (compartmentRightEdge - compartmentLeftEdge) / 8;

						// Ensure compartment is in valid range for this car type
						compartment = compartment % 8;

						// Calculate character position based on compartment
						charX = compartmentLeftEdge + (compartment * compartmentWidth) + (compartmentWidth / 2);
						charY = carMin.y + (corridorY - carMin.y) / 2;
					} else if (car.id == kCarRestaurant) {
						// Restaurant car with lounge (0-3749) and restaurant (3750-10000) sections
						float loungeRatio = 3750.0f / 10000.0f;
						float loungeWidth = carWidth * loungeRatio;
						float restaurantWidth = carWidth - loungeWidth;

						// Center corridor for restaurant car
						float restCorridorY = carMin.y + (carHeight - corridorHeight) / 2;

						// Check if character is in lounge or restaurant section based on position
						bool inLounge = (charPositions[i].position < 3750);

						if (inLounge) {
							// Lounge area (right side of car)
							charX = carMin.x + restaurantWidth + (loungeWidth / 2);
							charY = restCorridorY + (compartment % 2 == 0 ? -restCorridorY / 2 : corridorHeight + restCorridorY / 2);
						} else {
							// Restaurant area (left side of car)
							float tableWidth = restaurantWidth / 3;

							// Ensure compartment is in valid range for this car type
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
						// Kronos car layout (3 sections)
						float sectionWidth = carWidth / 3;

						// Ensure compartment is in valid range for this car type
						compartment = compartment % 3;

						charX = carMin.x + compartment * sectionWidth + sectionWidth / 2;
						charY = carMin.y + carHeight / 2;
					} else {
						// Default positioning for vestibule car
						charX = carMin.x + carWidth / 2;
						charY = carMin.y + carHeight / 2;
					}
				}

				// Draw character marker (circle)
				drawList->AddCircleFilled(
					ImVec2(charX, charY),
					8.0f,
					ImGui::ColorConvertFloat4ToU32(charPositions[i].charIndex == kCharacterCath ? cathColor : characterColor),
					12);

				// Character ID
				char charId[8];
				Common::sprintf_s(charId, "%d", charPositions[i].charIndex);
				drawList->AddText(
					ImVec2(charX - ImGui::CalcTextSize(charId).x / 2, charY - ImGui::CalcTextSize(charId).y / 2),
					charPositions[i].charIndex == kCharacterCath ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255),
					charId);

				// Tooltip for character when hovered
				ImGui::SetCursorScreenPos(ImVec2(charX - 8, charY - 8));
				ImGui::InvisibleButton(charId, ImVec2(16, 16));
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("%s", getCharacterName(charPositions[i].charIndex));
					ImGui::Text("Car: %d, Loc: %d, Pos: %d",
								charPositions[i].car,
								getCharacter(charPositions[i].charIndex).characterPosition.location,
								getCharacter(charPositions[i].charIndex).characterPosition.position);
					ImGui::EndTooltip();
				}
			}
		}

		// Move to next car
		carX += carWidth + 5;
	}

	// Draw "The Void" section at the bottom
	if (voidCharPositions.size() > 0) {
		ImDrawList *drawList = ImGui::GetWindowDrawList();

		// Draw void area
		ImVec2 voidMin;
		voidMin.x = ImGui::GetWindowPos().x + 10;
		voidMin.y = ImGui::GetWindowPos().y + 205; // Below the train cars
		ImVec2 voidMax;
		voidMax.x = ImGui::GetWindowPos().x + ImGui::GetContentRegionAvail().x - 10;
		voidMax.y = voidMin.y + 60;

		// Draw void rectangle
		drawList->AddRectFilled(
			voidMin,
			voidMax,
			ImGui::ColorConvertFloat4ToU32(voidColor),
			5.0f);

		// Draw void label
		const char *voidLabel = "THE VOID (id: 0)";
		ImVec2 textSize = ImGui::CalcTextSize(voidLabel);
		drawList->AddText(
			ImVec2(voidMin.x + 10, voidMin.y + 5),
			IM_COL32(255, 255, 255, 255),
			voidLabel);

		// Draw characters in the void
		float charSpacing = (voidMax.x - voidMin.x - 20) / (voidCharPositions.size() + 1);
		for (int i = 0; i < voidCharPositions.size(); i++) {
			float charX = voidMin.x + 10 + charSpacing * (i + 1);
			float charY = voidMin.y + 35;

			// Draw character marker (circle)
			drawList->AddCircleFilled(
				ImVec2(charX, charY),
				8.0f,
				ImGui::ColorConvertFloat4ToU32(characterColor),
				12);

			// Character ID
			char charId[16];
			Common::sprintf_s(charId, "%d", voidCharPositions[i].charIndex);
			drawList->AddText(
				ImVec2(charX - ImGui::CalcTextSize(charId).x / 2, charY - ImGui::CalcTextSize(charId).y / 2),
				IM_COL32(0, 0, 0, 255),
				charId);

			// Tooltip for character when hovered
			ImGui::SetCursorScreenPos(ImVec2(charX - 8, charY - 8));
			ImGui::InvisibleButton(charId, ImVec2(16, 16));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("%s", getCharacterName(voidCharPositions[i].charIndex));
				ImGui::Text("Car: 0, Loc: %d, Pos: %d",
							getCharacter(voidCharPositions[i].charIndex).characterPosition.location,
							getCharacter(voidCharPositions[i].charIndex).characterPosition.position);
				ImGui::EndTooltip();
			}
		}
	}

}

// Grid view showing multiple characters at once
void LogicManager::renderCharacterGrid(bool onlyPinned, int &selectedCharacter) {
	// Number of characters to display per row
	const int charsPerRow = 3;
	int displayed = 0;

	// Calculate fixed width for each card (subtract some padding)
	float windowWidth = ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x - ImGui::GetWindowPos().x;
	float cardWidth = (windowWidth / charsPerRow) - 8;

	for (int i = 0; i < 40; i++) {
		// Get character
		Character *character = &getCharacter(i);
		if (!character)
			continue;

		// Skip if filtering by pin status
		if (onlyPinned && !isCharacterPinned(i))
			continue;

		// Skip if doesn't match text filter
		char buffer[64];
		Common::sprintf_s(buffer, "%s (%d)", getCharacterName(i), i);
		if (!_state->_filter.PassFilter(buffer))
			continue;

		// Start a new row if needed
		if (displayed > 0 && displayed % charsPerRow == 0) {
			ImGui::NewLine();
		} else if (displayed > 0) {
			ImGui::SameLine(displayed % charsPerRow * (cardWidth + 8));
		}

		displayed++;

		// Create a card for the character with fixed width
		ImGui::PushID(i);
		ImGui::BeginChild(ImGui::GetID((void *)(intptr_t)i), ImVec2(cardWidth, 180), true);

		// Character name with pin status
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
		if (character->callbacks[character->currentCall] - 1 < funcNames.size())
			funcCurrName = funcNames[character->callbacks[character->currentCall] - 1];

		// Cath (the player) doesn't have logic functions...
		if (i != 0) {
			ImGui::Text("Current logic function: %s (%d)", funcCurrName.c_str(), character->callbacks[character->currentCall]);
		}

		ImGui::Separator();

		// Key information
		ImGui::Text("Position: Car %u, Loc %u, Pos %u",
					character->characterPosition.car,
					character->characterPosition.location,
					character->characterPosition.position
		);

		ImGui::Text("Sequence: %s", character->sequenceName);
		ImGui::Text("Direction: %d", character->direction);
		ImGui::Text("Current Frame: Seq1 %d / Seq2 %d", character->currentFrameSeq1, character->currentFrameSeq2);

		// Link to details view
		if (ImGui::Button("View Details")) {
			// Switch to list view and select this character
			_state->_selectedCharacter = i; // Store the selected character index
			_state->_forceReturnToListView = true;
		}

		ImGui::EndChild();
		ImGui::PopID();
	}

	if (displayed == 0) {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
						   onlyPinned ? "No pinned characters match the filter" : "No characters match the filter");
	}
}

// Detailed view for a single character
void LogicManager::renderCharacterDetails(Character *character, int index) {
	// Title and header
	ImGui::Text("%s (Character ID: %d)", getCharacterName(index), index);

	// Pin/unpin button
	ImGui::SameLine(ImGui::GetWindowWidth() - 100);
	if (ImGui::Button(isCharacterPinned(index) ? "Unpin Character" : "Pin Character")) {
		toggleCharacterPin(index);
	}

	ImGui::Separator();

	// Always visible basic info section
	ImGui::Text("Position: Car %u, Location %u, Position %u",
				character->characterPosition.car,
				character->characterPosition.location,
				character->characterPosition.position);

	Common::StringArray funcNames = getCharacterFunctionNames(index);

	Common::String funcCurrName = "NONE";
	if (character->callbacks[character->currentCall] - 1 < funcNames.size())
		funcCurrName = funcNames[character->callbacks[character->currentCall] - 1];

	// Cath (the player) doesn't have logic functions...
	if (index != 0) {
		ImGui::Text("Current logic function: %s (%d)", funcCurrName.c_str(), character->callbacks[character->currentCall]);
	}

	// Group related data in collapsing sections
	if (ImGui::CollapsingHeader("Logic call stack", ImGuiTreeNodeFlags_DefaultOpen)) {
		// Start with the current call
		int currentDepth = character->currentCall;

		if (currentDepth < 0 || character->callbacks[currentDepth] == 0) {
			ImGui::Text("No active logic functions");
		} else {
			ImGui::Text("Call stack depth: %d", currentDepth + 1);

			// Display the call stack with proper indentation
			for (int i = currentDepth; i >= 0; i--) {
				// Get the function number at this stack level
				int functionId = character->callbacks[i];

				// Calculate indent amount - ensure it's never zero (use 0.1f as minimum)
				float indentAmount = (currentDepth - i) * 20.0f;
				indentAmount = indentAmount > 0.0f ? indentAmount : 0.1f;

				// Apply indent
				ImGui::Indent(indentAmount);

				// Display function with a bullet point
				ImGui::BulletText("Level %d: %s (#%d)",
								  currentDepth - i,
								  funcNames[functionId - 1].c_str(),
								  functionId);

				// Remove indent
				ImGui::Unindent(indentAmount);
			}
		}
	}

	// Group related data in collapsing sections
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
		ImGui::Text("Car2: %d", character->car2);
		ImGui::Text("Position2: %d", character->position2);
		ImGui::Text("Field_4A9: %d", character->field_4A9);
		ImGui::Text("Field_4AA: %d", character->field_4AA);
	}

	if (ImGui::CollapsingHeader("Call Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (int call = 0; call < 9; call++) {
			char label[32];
			Common::sprintf_s(label, "Call %d", call);

			// Skip calls with all zero parameters
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
	ImGui::BulletText("Is drawn: %s", _state->_engine->getGraphicsManager()->canDrawMouse() ? "yes" : "no");
	ImGui::BulletText("Has left clicked: %s", _state->_engine->mouseHasLeftClicked() ? "yes" : "no");
	ImGui::BulletText("Has right clicked: %s", _state->_engine->mouseHasRightClicked() ? "yes" : "no");
}

#endif

Debugger::Debugger(LastExpressEngine *engine) : _engine(engine), _command(nullptr), _numParams(0), _commandParams(nullptr) {

	//////////////////////////////////////////////////////////////////////////
	// Register the debugger commands

	// General
	registerCmd("help",      WRAP_METHOD(Debugger, cmdHelp));

	// Data
	registerCmd("ls",        WRAP_METHOD(Debugger, cmdListFiles));
	registerCmd("dump",      WRAP_METHOD(Debugger, cmdDumpFiles));

	registerCmd("showframe", WRAP_METHOD(Debugger, cmdShowFrame));
	registerCmd("showbg",    WRAP_METHOD(Debugger, cmdShowBg));
	registerCmd("playseq",   WRAP_METHOD(Debugger, cmdPlaySeq));
	registerCmd("playsnd",   WRAP_METHOD(Debugger, cmdPlaySnd));
	registerCmd("playsbe",   WRAP_METHOD(Debugger, cmdPlaySbe));
	registerCmd("playnis",   WRAP_METHOD(Debugger, cmdPlayNis));

	// Scene & interaction
	registerCmd("loadscene", WRAP_METHOD(Debugger, cmdLoadScene));
	registerCmd("fight",     WRAP_METHOD(Debugger, cmdFight));
	registerCmd("beetle",    WRAP_METHOD(Debugger, cmdBeetle));

	// Game
	registerCmd("delta",     WRAP_METHOD(Debugger, cmdTimeDelta));
	registerCmd("time",      WRAP_METHOD(Debugger, cmdTime));
	registerCmd("show",      WRAP_METHOD(Debugger, cmdShow));
	registerCmd("character",    WRAP_METHOD(Debugger, cmdEntity));

	// Misc
	registerCmd("chapter",   WRAP_METHOD(Debugger, cmdSwitchChapter));
	registerCmd("clear",     WRAP_METHOD(Debugger, cmdClear));

	resetCommand();

	_soundStream = new StreamedSound();
}

Debugger::~Debugger() {
	SAFE_DELETE(_soundStream);
	resetCommand();

	_command = nullptr;
	_commandParams = nullptr;

	// Zero passed pointers
	_engine = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////
bool Debugger::hasCommand() const {
	return (_numParams != 0);
}

void Debugger::resetCommand() {
	SAFE_DELETE(_command);

	if (_commandParams)
		for (int i = 0; i < _numParams; i++)
			free(_commandParams[i]);

	free(_commandParams);
	_commandParams = nullptr;
	_numParams = 0;
}

int Debugger::getNumber(const char *arg) const {
	return strtol(arg, (char **)nullptr, 0);
}

void Debugger::copyCommand(int argc, const char **argv) {
	_commandParams = (char **)malloc(sizeof(char *) * (uint)argc);
	if (!_commandParams)
		return;

	_numParams = argc;

	for (int i = 0; i < _numParams; i++) {
		size_t ln = strlen(argv[i]) + 1;
		_commandParams[i] = (char *)malloc(ln);
		if (_commandParams[i] == nullptr)
			error("[Debugger::copyCommand] Cannot allocate memory for command parameters");

		memset(_commandParams[i], 0, strlen(argv[i]) + 1);
		Common::strcpy_s(_commandParams[i], ln, argv[i]);
	}

	// Exit the debugger!
	cmdExit(0, nullptr);
}

void Debugger::callCommand() {
	if (_command)
		(*_command)(_numParams, const_cast<const char **>(_commandParams));
}

bool Debugger::loadArchive(int index) {
	if (index < 1 || index > 3) {
		debugPrintf("Invalid cd number (was: %d, valid: [1-3])\n", index);
		return false;
	}

	if (!_engine->getResourceManager()->loadArchive((ArchiveIndex)index))
		return false;

	getScenes()->loadSceneDataFile((ArchiveIndex)index);

	return true;
}

// Restore loaded archive
void Debugger::restoreArchive() const {

	ArchiveIndex index = kArchiveCd1;

	switch (getProgress().chapter) {
	default:
	case kChapter1:
		index = kArchiveCd1;
		break;

	case kChapter2:
	case kChapter3:
		index = kArchiveCd2;
		break;

	case kChapter4:
	case kChapter5:
		index = kArchiveCd3;
		break;
	}

	_engine->getResourceManager()->loadArchive(index);
	getScenes()->loadSceneDataFile(index);
}

//////////////////////////////////////////////////////////////////////////
// Debugger commands
//////////////////////////////////////////////////////////////////////////
bool Debugger::cmdHelp(int, const char **) {
	debugPrintf("Debug flags\n");
	debugPrintf("-----------\n");
	debugPrintf(" debugflag_list - Lists the available debug flags and their status\n");
	debugPrintf(" debugflag_enable - Enables a debug flag\n");
	debugPrintf(" debugflag_disable - Disables a debug flag\n");
	debugPrintf("\n");
	debugPrintf("Commands\n");
	debugPrintf("--------\n");
	debugPrintf(" ls - list files in the archive\n");
	debugPrintf(" dump - dump a list of files in all archives\n");
	debugPrintf("\n");
	debugPrintf(" showframe - show a frame from a sequence\n");
	debugPrintf(" showbg - show a background\n");
	debugPrintf(" playseq - play a sequence\n");
	debugPrintf(" playsnd - play a sound\n");
	debugPrintf(" playsbe - play a subtitle\n");
	debugPrintf(" playnis - play an animation\n");
	debugPrintf("\n");
	debugPrintf(" loadscene - load a scene\n");
	debugPrintf(" fight - start a fight\n");
	debugPrintf(" beetle - start the beetle game\n");
	debugPrintf("\n");
	debugPrintf(" delta - Adjust the time delta\n");
	debugPrintf(" show - show game data\n");
	debugPrintf(" character - show character data\n");
	debugPrintf("\n");
	debugPrintf(" loadgame - load a saved game\n");
	debugPrintf(" chapter - switch to a specific chapter\n");
	debugPrintf(" clear - clear the screen\n");
	debugPrintf("\n");
	return true;
}

/**
 * Command: list files in archive
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdListFiles(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {
		Common::Path filter(const_cast<char *>(argv[1]));

		// Load the proper archive
		if (argc == 3) {
			if (!loadArchive(getNumber(argv[2])))
				return true;
		}

		Common::ArchiveMemberList list;
		int count = _engine->getResourceManager()->listMatchingMembers(list, filter);

		debugPrintf("Number of matches: %d\n", count);
		for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it)
			debugPrintf(" %s\n", (*it)->getName().c_str());

		// Restore archive
		if (argc == 3)
			restoreArchive();
	} else {
		debugPrintf("Syntax: ls <filter> (use * for all) (<cd number>)\n");
	}

	return true;
}

/**
 * Command: Dump the list of files in the archive
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdDumpFiles(int argc, const char **) {
#define OUTPUT_ARCHIVE_FILES(name, filename) { \
	_engine->getResourceManager()->reset(); \
	_engine->getResourceManager()->loadArchive(filename); \
	Common::ArchiveMemberList list; \
	int count = _engine->getResourceManager()->listMatchingMembers(list, "*"); \
	debugC(1, kLastExpressDebugResource, "\n\n--------------------------------------------------------------------\n"); \
	debugC(1, kLastExpressDebugResource, "-- " #name " (%d files)\n", count); \
	debugC(1, kLastExpressDebugResource, "--------------------------------------------------------------------\n\n"); \
	debugC(1, kLastExpressDebugResource, "Filename,Size,MD5\n"); \
	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it) { \
		Common::SeekableReadStream *stream = getArchiveMember((*it)->getName()); \
		if (!stream) { \
			debugPrintf("ERROR: Cannot create stream for file: %s\n", (*it)->getName().c_str()); \
			restoreArchive(); \
			return true; \
		} \
		Common::String md5str = Common::computeStreamMD5AsString(*stream); \
		debugC(1, kLastExpressDebugResource, "%s, %d, %s", (*it)->getName().c_str(), (int)stream->size(), md5str.c_str()); \
		delete stream; \
	} \
}

	if (argc == 1) {
		// For each archive file, dump the list of files
		if (_engine->isDemo()) {
			OUTPUT_ARCHIVE_FILES("DEMO", "DEMO.HPF");
		} else {
			OUTPUT_ARCHIVE_FILES("HD", "HD.HPF");
			OUTPUT_ARCHIVE_FILES("CD 1", "CD1.HPF");
			OUTPUT_ARCHIVE_FILES("CD 2", "CD2.HPF");
			OUTPUT_ARCHIVE_FILES("CD 3", "CD3.HPF");
		}

		// Restore current loaded archive
		restoreArchive();
	} else {
		debugPrintf("Syntax: dump");
	}

	return true;
}

/**
 * Command: Shows a frame
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdShowFrame(int argc, const char **argv) {
	if (argc == 3 || argc == 4) {
		Common::String filename(const_cast<char *>(argv[1]));
		filename += ".seq";

		if (argc == 4) {
			if (!loadArchive(getNumber(argv[3])))
				return true;
		}

		if (!_engine->getResourceManager()->hasFile(Common::Path(filename))) {
			debugPrintf("Cannot find file: %s\n", filename.c_str());
			return true;
		}

		// Store command
		if (!hasCommand()) {
			_command = WRAP_METHOD(Debugger, cmdShowFrame);
			copyCommand(argc, argv);

			return cmdExit(0, nullptr);
		} else {
			Sequence sequence(filename);
			if (sequence.load(getArchiveMember(filename))) {
				_engine->getCursor()->show(false);
				clearBg(GraphicsManagerOld::kBackgroundOverlay);

				AnimFrame *frame = sequence.getFrame((uint16)getNumber(argv[2]));
				if (!frame) {
					debugPrintf("Invalid frame index '%s'\n", argv[2]);
					resetCommand();
					return true;
				}

				_engine->getGraphicsManagerOld()->draw(frame, GraphicsManagerOld::kBackgroundOverlay);
				delete frame;

				askForRedraw();
				redrawScreen();

				_engine->_system->delayMillis(1000);
				_engine->getCursor()->show(true);
			}

			resetCommand();

			if (argc == 4)
				restoreArchive();
		}
	} else {
		debugPrintf("Syntax: cmd_showframe <seqname> <index> (<cd number>)\n");
	}
	return true;
}

/**
 * Command: shows a background
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdShowBg(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {
		Common::String filename(const_cast<char *>(argv[1]));

		if (argc == 3) {
			if (!loadArchive(getNumber(argv[2])))
				return true;
		}

		if (!_engine->getResourceManager()->hasFile(Common::Path(filename + ".BG"))) {
			debugPrintf("Cannot find file: %s\n", (filename + ".BG").c_str());
			return true;
		}

		// Store command
		if (!hasCommand()) {
			_command = WRAP_METHOD(Debugger, cmdShowBg);
			copyCommand(argc, argv);

			return cmdExit(0, nullptr);
		} else {
			clearBg(GraphicsManagerOld::kBackgroundC);

			Background *background = _engine->getResourceManager()->loadBackground(filename);
			if (background) {
				_engine->getGraphicsManagerOld()->draw(background, GraphicsManagerOld::kBackgroundC);
				delete background;
				askForRedraw();
			}

			redrawScreen();

			if (argc == 3)
				restoreArchive();

			// Pause for a second to be able to see the background
			_engine->_system->delayMillis(1000);

			resetCommand();
		}
	} else {
		debugPrintf("Syntax: showbg <bgname> (<cd number>)\n");
	}
	return true;
}

/**
 * Command: plays a sequence.
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdPlaySeq(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {
		Common::String filename(const_cast<char *>(argv[1]));
		filename += ".seq";

		if (argc == 3) {
			if (!loadArchive(getNumber(argv[2])))
				return true;
		}

		if (!_engine->getResourceManager()->hasFile(Common::Path(filename))) {
			debugPrintf("Cannot find file: %s\n", filename.c_str());
			return true;
		}

		// Store command
		if (!hasCommand()) {
			_command = WRAP_METHOD(Debugger, cmdPlaySeq);
			copyCommand(argc, argv);

			return cmdExit(0, nullptr);
		} else {
			Sequence *sequence = new Sequence(filename);
			if (sequence->load(getArchiveMember(filename))) {

				// Check that we have at least a frame to show
				if (sequence->count() == 0) {
					delete sequence;
					return false;
				}

				_engine->getCursor()->show(false);

				SequenceFrame player(sequence, 0, true);
				do {
					// Clear screen
					clearBg(GraphicsManagerOld::kBackgroundA);

					_engine->getGraphicsManagerOld()->draw(&player, GraphicsManagerOld::kBackgroundA);

					askForRedraw();
					redrawScreen();

					// Handle right-click to interrupt sequence
					Common::Event ev;
					if (_engine->getEventManager()->pollEvent(ev) && ev.type == Common::EVENT_RBUTTONUP)
						break;

					_engine->_system->delayMillis(175);

					// go to the next frame
				} while (player.nextFrame());
				_engine->getCursor()->show(true);
			} else {
				// Sequence player is deleting his reference to the sequence, but we need to take care of it if the
				// sequence could not be loaded
				delete sequence;
			}

			resetCommand();

			if (argc == 3)
				restoreArchive();
		}
	} else {
		debugPrintf("Syntax: playseq <seqname> (<cd number>)\n");
	}
	return true;
}

/**
 * Command: plays a sound
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdPlaySnd(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {

		if (argc == 3) {
			if (!loadArchive(getNumber(argv[2])))
				return true;
		}

		// Add .SND at the end of the filename if needed
		Common::String name(const_cast<char *>(argv[1]));
		if (!name.contains('.'))
			name += ".SND";

		if (!_engine->getResourceManager()->hasFile(Common::Path(name))) {
			debugPrintf("Cannot find file: %s\n", name.c_str());
			return true;
		}

		_engine->_system->getMixer()->stopAll();

		_soundStream->load(getArchiveMember(name), kVolumeFull, false);

		if (argc == 3)
			restoreArchive();
	} else {
		debugPrintf("Syntax: playsnd <sndname> (<cd number>)\n");
	}
	return true;
}

/**
 * Command: plays subtitles
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdPlaySbe(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {
		Common::String filename(const_cast<char *>(argv[1]));

		if (argc == 3) {
			if (!loadArchive(getNumber(argv[2])))
				return true;
		}

		filename += ".sbe";

		if (!_engine->getResourceManager()->hasFile(Common::Path(filename))) {
			debugPrintf("Cannot find file: %s\n", filename.c_str());
			return true;
		}

		// Store command
		if (!hasCommand()) {
			_command = WRAP_METHOD(Debugger, cmdPlaySbe);
			copyCommand(argc, argv);

			return cmdExit(0, nullptr);
		} else {
			SubtitleManagerOld subtitle(_engine->getFont());
			if (subtitle.load(getArchiveMember(filename))) {
				_engine->getCursor()->show(false);
				for (uint16 i = 0; i < subtitle.getMaxTime(); i += 25) {
					clearBg(GraphicsManagerOld::kBackgroundAll);

					subtitle.setTime(i);
					_engine->getGraphicsManagerOld()->draw(&subtitle, GraphicsManagerOld::kBackgroundOverlay);

					askForRedraw();
					redrawScreen();

					// Handle right-click to interrupt sequence
					Common::Event ev;
					if (!_engine->getEventManager()->pollEvent(ev))
						break;

					if (ev.type == Common::EVENT_RBUTTONUP)
						break;

					_engine->_system->delayMillis(500);
				}
				_engine->getCursor()->show(true);
			}

			if (argc == 3)
				restoreArchive();

			resetCommand();
		}
	} else {
		debugPrintf("Syntax: playsbe <sbename> (<cd number>)\n");
	}
	return true;
}

/**
 * Command: plays a NIS animation sequence.
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdPlayNis(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {
		Common::String name(const_cast<char *>(argv[1]));

		if (argc == 3) {
			if (!loadArchive(getNumber(argv[2])))
				return true;
		}

		// If we got a nis filename, check that the file exists
		if (name.contains('.') && !_engine->getResourceManager()->hasFile(Common::Path(name))) {
			debugPrintf("Cannot find file: %s\n", name.c_str());
			return true;
		}

		// Store command
		if (!hasCommand()) {
			_command = WRAP_METHOD(Debugger, cmdPlayNis);
			copyCommand(argc, argv);

			return cmdExit(0, nullptr);
		} else {
			// Make sure we are not called in a loop
			_numParams = 0;

			// Check if we got a nis filename or an animation index
			if (name.contains('.')) {
				Animation animation;
				if (animation.load(getArchiveMember(name))) {
					_engine->getCursor()->show(false);
					animation.play();
					_engine->getCursor()->show(true);
				}
			} else {
				getActionOld()->playAnimation((EventIndex)atoi(name.c_str()), true);
			}

			if (argc == 3)
				restoreArchive();

			resetCommand();
		}
	} else {
		debugPrintf("Syntax: playnis <nisname.nis or animation index> (<cd number>)\n");
	}
	return true;
}

/**
 * Command: loads a scene
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdLoadScene(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {
		int cd = 1;
		SceneIndex index = (SceneIndex)getNumber(argv[1]);

		// Check args
		if (argc == 3) {
			if (!loadArchive(getNumber(argv[2])))
				return true;
		}

		if (index > 2500) {
			debugPrintf("Error: invalid index value (0-2500)");
			return true;
		}

		// Store command
		if (!hasCommand()) {
			_command = WRAP_METHOD(Debugger, cmdLoadScene);
			copyCommand(argc, argv);

			return cmdExit(0, nullptr);
		} else {

			clearBg(GraphicsManagerOld::kBackgroundAll);

			/************  DEBUG  *************************/
			// Use to find scenes with certain values

			//for (int i = index; i < 2500; i++) {
			//	loadSceneObject(scene, i);

			//	if (scene.getHeader() && scene.getHeader()->car == 5 && scene.getHeader()->position == 81) {
			//		debugPrintf("Found scene: %d", i);

			//		// Draw scene found
			//		_engine->getGraphicsManagerOld()->draw(&scene, GraphicsManagerOld::kBackgroundC);

			//		askForRedraw();
			//		redrawScreen();
			//		_engine->_system->delayMillis(500);

			//		break;
			//	}
			//}

			//delete _sceneLoader;
			//resetCommand();
			//return true;

			/*********************************************/
			Scene *scene = getScenes()->get(index);
			if (!scene) {
				debugPrintf("Cannot load scene %i from CD %i", index, cd);
				resetCommand();

				return true;
			}

			_engine->getGraphicsManagerOld()->draw(scene, GraphicsManagerOld::kBackgroundC);

			askForRedraw();
			redrawScreen();

			// Pause for a second to be able to see the scene
			_engine->_system->delayMillis(500);

			if (argc == 3)
				restoreArchive();

			resetCommand();
		}
	} else {
		debugPrintf("Syntax: loadscene <scene index> (<cd number>)\n");
	}
	return true;
}

/**
 * Command: starts a fight sequence
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdFight(int argc, const char **argv) {
	if (argc == 2) {
		FightType type = (FightType)getNumber(argv[1]);

		// Load proper data file
		ArchiveIndex index = kArchiveCd1;
		switch (type) {
		default:
			goto error;

		case kFightMilos:
			index = kArchiveCd1;
			break;

		case kFightAnna:
			index = kArchiveCd2;
			break;

		case kFightIvo:
		case kFightSalko:
		case kFightVesna:
			index = kArchiveCd3;
			break;
		}

		if (!loadArchive(index)) {
			debugPrintf("Error: failed to load archive %d\n", index);
			return true;
		}

		// Store command
		if (!hasCommand()) {
			_command = WRAP_METHOD(Debugger, cmdFight);
			copyCommand(argc, argv);

			return false;
		} else {
			// Make sure we are not called in a loop
			_numParams = 0;

			clearBg(GraphicsManagerOld::kBackgroundAll);
			askForRedraw();
			redrawScreen();

			SceneIndex lastScene = getState()->scene;

			//getFight()->setup(type) ? debugPrintf("Lost fight!\n") : debugPrintf("Won fight!\n");

			// Pause for a second to be able to see the final scene
			_engine->_system->delayMillis(1000);

			// Restore loaded archive
			restoreArchive();

			// Stop audio and restore scene
			getSoundQueue()->stopAllSound();

			clearBg(GraphicsManagerOld::kBackgroundAll);

			Scene *scene = getScenes()->get(lastScene);
			_engine->getGraphicsManagerOld()->draw(scene, GraphicsManagerOld::kBackgroundC);

			askForRedraw();
			redrawScreen();

			resetCommand();
		}
	} else {
error:
		debugPrintf("Syntax: fight <id> (id=2001-2005)\n");
	}

	return true;
}

/**
 * Command: starts the beetle sequence
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdBeetle(int argc, const char **argv) {
	if (argc == 1) {
		// Load proper data file (beetle game in in Cd2)
		if (!loadArchive(kArchiveCd2)) {
			debugPrintf("Error: failed to load archive 2");
			return true;
		}

		// Store command
		if (!hasCommand()) {
			_command = WRAP_METHOD(Debugger, cmdBeetle);
			copyCommand(argc, argv);

			return false;
		} else {
			clearBg(GraphicsManagerOld::kBackgroundAll);
			askForRedraw();
			redrawScreen();

			// Save current state
			SceneIndex previousScene = getState()->scene;
			ObjectLocation previousLocation = getInventory()->get(kItemBeetle)->location;
			ChapterIndex previousChapter = (ChapterIndex)getProgress().chapter;

			// Setup scene & inventory
			getProgress().chapter = kChapter2;
			Scene *scene = getScenes()->get(kSceneBeetle);
			getInventory()->get(kItemBeetle)->location = kObjectLocation3;

			askForRedraw();
			redrawScreen();

			// Load the beetle game
			Action *action = nullptr;
			Beetle *beetle = new Beetle(_engine);
			if (!beetle->isLoaded())
				beetle->load();

			// Play the game
			Common::Event ev;
			bool playgame = true;
			while (playgame) {
				// Update beetle
				beetle->update();

				askForRedraw();
				redrawScreen();

				while (g_system->getEventManager()->pollEvent(ev)) {

					switch (ev.type) {
					default:
						break;

					case Common::EVENT_KEYDOWN:
						// Exit beetle game on escape
						if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
							playgame = false;

						break;

					case Common::EVENT_MOUSEMOVE: {
						// Update cursor
						CursorStyle style = kCursorNormal;
						SceneHotspot *hotspot = nullptr;
						if (scene->checkHotSpot(ev.mouse, &hotspot)) {
							if (!action)
								action = new Action(_engine);

							style = action->getCursor(*hotspot);
						}

						_engine->getCursor()->setStyle(style);
						break;
					}

					case Common::EVENT_LBUTTONUP:
					case Common::EVENT_RBUTTONUP:
						// Update coordinates
						getLogic()->getGameState()->setCoordinates(ev.mouse);

						if (beetle->catchBeetle())
							playgame = false;
						break;
					}

					_engine->_system->delayMillis(10);
				}
			}

			// Cleanup
			beetle->unload();
			delete beetle;
			delete action;

			// Pause for a second to be able to see the final scene
			_engine->_system->delayMillis(1000);

			// Restore state
			getProgress().chapter = previousChapter;
			getInventory()->get(kItemBeetle)->location = previousLocation;

			// Restore loaded archive
			restoreArchive();

			// Stop audio and restore scene
			getSoundQueue()->stopAllSound();

			clearBg(GraphicsManagerOld::kBackgroundAll);

			Scene *oldScene = getScenes()->get(previousScene);
			_engine->getGraphicsManagerOld()->draw(oldScene, GraphicsManagerOld::kBackgroundC);

			askForRedraw();
			redrawScreen();

			resetCommand();
		}
	} else {
		debugPrintf("Syntax: beetle\n");
	}

	return true;
}

/**
 * Command: adjusts the time delta
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdTimeDelta(int argc, const char **argv) {
	if (argc == 2) {
		int delta = getNumber(argv[1]);

		if (delta <= 0 || delta > 500)
			goto label_error;

		getState()->timeDelta = (uint)delta;
	} else {
label_error:
		debugPrintf("Syntax: delta <time delta> (delta=1-500)\n");
	}

	return true;
}

/**
 * Command: Convert between in-game time and human readable time
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdTime(int argc, const char **argv) {
	if (argc == 2) {
		int32 time = getNumber(argv[1]);

		if (time < 0)
			goto label_error;

		// Convert to human-readable form
		uint8 hours = 0;
		uint8 minutes = 0;
		State::getHourMinutes((uint32)time, &hours, &minutes);

		debugPrintf("%02d:%02d\n", hours, minutes);
	} else {
label_error:
		debugPrintf("Syntax: time <time to convert> (time=0-INT_MAX)\n");
	}

	return true;
}

/**
 * Command: show game logic data
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdShow(int argc, const char **argv) {
#define OUTPUT_DUMP(name, text) \
	debugPrintf(#name "\n"); \
	debugPrintf("--------------------------------------------------------------------\n\n"); \
	debugPrintf("%s", text); \
	debugPrintf("\n");

	if (argc == 2) {

		Common::String name(const_cast<char *>(argv[1]));

		if (name == "state" || name == "st") {
			OUTPUT_DUMP("Game state", getState()->toString().c_str());
		} else if (name == "progress" || name == "pr") {
			OUTPUT_DUMP("Progress", getProgress().toString().c_str());
		} else if (name == "flags" || name == "fl") {
			OUTPUT_DUMP("Flags", getFlags()->toString().c_str());
		} else if (name == "inventory" || name == "inv") {
			OUTPUT_DUMP("Inventory", getInventory()->toString().c_str());
		} else if (name == "objects" || name == "obj") {
			OUTPUT_DUMP("Objects", getObjects()->toString().c_str());
		} else if (name == "savepoints" || name == "pt") {
			OUTPUT_DUMP("SavePoints", getSavePoints()->toString().c_str());
		} else if (name == "scene" || name == "sc") {
			OUTPUT_DUMP("Current scene", getScenes()->get(getState()->scene)->toString().c_str());
		} else {
			goto label_error;
		}

	} else {
label_error:
		debugPrintf("Syntax: state <option>\n");
		debugPrintf("          state / st\n");
		debugPrintf("          progress / pr\n");
		debugPrintf("          flags / fl\n");
		debugPrintf("          inventory / inv\n");
		debugPrintf("          objects / obj\n");
		debugPrintf("          savepoints / pt\n");
		debugPrintf("          scene / sc\n");
	}

	return true;

#undef OUTPUT_DUMP
}

/**
 * Command: shows character data
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdEntity(int argc, const char **argv) {
	if (argc == 2) {
		CharacterIndex index = (CharacterIndex)getNumber(argv[1]);

		if (index > 39)
			goto label_error;

		debugPrintf("Entity %s\n", ENTITY_NAME(index));
		debugPrintf("--------------------------------------------------------------------\n\n");
		debugPrintf("%s", getEntities()->getData(index)->toString().c_str());

		// The Player character does not have any callback data
		if (index != kCharacterCath) {
			EntityData *data = getEntities()->get(index)->getParamData();
			for (uint callback = 0; callback < 9; callback++) {
				debugPrintf("Call parameters %d:\n", callback);
				for (byte ix = 0; ix < 4; ix++)
					debugPrintf("  %s", data->getParameters(callback, ix)->toString().c_str());
			}
		}

		debugPrintf("\n");
	} else {
label_error:
		debugPrintf("Syntax: character <index>\n");
		for (int i = 0; i < 40; i += 4)
			debugPrintf(" %s - %d        %s - %d        %s - %d        %s - %d\n", ENTITY_NAME(i), i, ENTITY_NAME(i+1), i+1, ENTITY_NAME(i+2), i+2, ENTITY_NAME(i+3), i+3);
	}

	return true;
}

/**
 * Command: switch to a specific chapter
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdSwitchChapter(int argc, const char **argv) {
	if (argc == 2) {
		int id = getNumber(argv[1]);

		if (id <= 1 || id > 6)
			goto error;

		// Store command
		if (!hasCommand()) {
			_command = WRAP_METHOD(Debugger, cmdSwitchChapter);
			copyCommand(argc, argv);

			return false;
		} else {
			// Sets the current chapter and then call Logic::switchChapter to proceed to the next chapter
			getState()->progress.chapter = (ChapterIndex)(id - 1);

			getLogic()->switchChapter();

			resetCommand();
		}
	} else {
error:
		debugPrintf("Syntax: chapter <id> (id=2-6)\n");
	}

	return true;
}

/**
 * Command: clears the screen
 *
 * @param argc The argument count.
 * @param argv The values.
 *
 * @return true if it was handled, false otherwise
 */
bool Debugger::cmdClear(int argc, const char **) {
	if (argc == 1) {
		clearBg(GraphicsManagerOld::kBackgroundAll);
		askForRedraw();
		redrawScreen();
	} else {
		debugPrintf("Syntax: clear - clear the screen\n");
	}

	return true;
}

} // End of namespace LastExpress
