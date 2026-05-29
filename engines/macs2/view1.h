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

#ifndef MACS2_VIEW1_H
#define MACS2_VIEW1_H

#include "macs2/events.h"
#include "macs2/macs2.h"

namespace Macs2 {

class GameObject;

// TODO: Implement the different view modes
enum class ViewMode {
	VM_GAME,
	VM_MAP
};

enum class FadeMode {
	None,
	FromBlack,
	ToBlack
};

class Button {

public:
	Common::Point Position;
	Common::Point Size;
	Common::String Caption;

	bool IsPointInside(const Common::Point &p) const;

	void Render(Graphics::ManagedSurface &s);
};

class Character {
private:
	Common::Point StartPosition;
	Common::Point EndPosition;

	uint32 StartTime;
	uint32 Duration;

	bool IsLerping = false;
	// TODO: Need to consistently reset this one to false
	bool LerpIgnoresObstacles = false;

	// If this is set, a lerp to a location becomes picking up
	// TODO: Replace by more proper task implementation later
	Macs2::GameObject *pickedUpObject = nullptr;

	// Simple timer to keep track of how long we play the picking up animation
	float pickupAnimationEndTime = -1.0f;

	uint8 previousOrientation;

	// Handle when the character has moved into a non-walkable area, push them out if
	// they did and return true, return false otherwise
	bool HandleWalkability(Character *c);

	// fn0037_0E8C proc
	uint8 LookupWalkability(const Common::Point &p) const;

public:
	bool IsWalkable(const Common::Point &p) const;
	bool IsLineSegmentWalkable(const Common::Point &p1, const Common::Point &p2, bool print = false);
	Common::Array<uint8> PathfindingOverlay;
	Character();

	Common::Array<uint16> Path;
	int16 CurrentPathIndex;
	Common::Point PathFinalDestination;

	bool FindPath(Common::Point target);

	bool VisitPathfindingNode(uint16 index, Common::Array<bool> &visited, const Common::Point &target);

	bool IsFollowingPath = false;

	Common::Point GetPosition() const;
	void SetPosition(const Common::Point &newPosition);
	Macs2::GameObject *GameObject;

	uint16 GetVerticalOffset() const;

	// Returns false if we are at the end of the path already or the path is not valid
	bool TryFollowPath();

	// Set by opcode 11h
	bool ExecuteScriptOnFinishLerp = false;

	// TODO: Handle properly
	uint8 animationIndex = 1;
	uint16 motionTargetVerticalOffset = 0;
	uint16 motionVerticalOffsetDelta = 0;
	uint16 motionDistanceUnits = 0;
	uint16 motionProgress = 0;
	uint16 motionStartVerticalOffset = 0;
	bool hasMotionVerticalOffset = false;
	bool shouldMirrorCurrentAnimation = false;

	bool isAnimationMirrored() const;
	uint8 getMirroredAnimation(uint8 original) const;

	// TODO: Will need time handling
	Macs2::AnimFrame *GetCurrentAnimationFrame();
	Macs2::AnimFrame *GetCurrentPortrait(bool onRightSide = false);
	void StartLerpTo(const Common::Point &target, uint32 duration, bool ignoreObstacles = false);

	void StartPickup(Macs2::GameObject *object);

	// Handles setting this character up to send an event to the script executor when finished
	// and will send the event right away in case the last movement is already done
	// TODO: Check if the code also handles it this way
	void RegisterWaitForMovementFinishedEvent();
	void Update();
};

// cf https://stackoverflow.com/a/51497820
// TODO: Figure this one out
template<typename T, T... ts>
constexpr bool is_in_list(T const &t) {
	using unused = bool[];

	bool ret{false};

	(void)unused{false, ret |= t == ts...};

	return ret;
}

struct SpeechActData {
	Character *speaker = nullptr;
	Common::Array<Common::String> strings;
	Common::Point position;
	bool onRightSide;
};

struct ScalingValues {
	uint16 characterY;
	uint16 scalingFactor;
};

class View1 : public UIElement {
	// TODO: Clean up private and public
public:
	ScalingValues scalingValues;

	ViewMode currentMode = ViewMode::VM_GAME;

	AnimFrame *GetInventoryIcon(GameObject *gameObject);

	byte _pal[256 * 3] = {0};
	int _offset = 0;
	bool _paletteDirty = true;

	// Background animation timing from gameTick (1008:e556).
	// The original game increments a tick counter each frame (~70Hz DOS timer)
	// and advances background animations when the counter exceeds a threshold:
	//   Mode 2 (word5203==2): threshold = 0x27 (39 ticks, ~557ms at 70Hz)
	//   Mode 3 (word5203==3): threshold from scene data (variable, typically smaller)
	// We convert to milliseconds for ScummVM's variable frame rate.
	static constexpr uint32 kBgAnimTicksMode2 = 39;  // 0x27 ticks
	static constexpr uint32 kDosTimerHz = 70;
	static constexpr uint32 kBgAnimDelayMode2Ms = (kBgAnimTicksMode2 * 1000) / kDosTimerHz; // ~557ms

	uint32 _frameDelayFlag = kBgAnimDelayMode2Ms;
	int32 _nextFrameFlag = _frameDelayFlag;
	uint32 _lastMillis = 0;
	bool _hasTicked = false;
	uint32 _flagFrameIndex = 0;

	uint32 _guyFrameIndex = 0;

	// TODO: Probably the start of a mode enum
	bool _isShowingStringBox = false;
	Common::StringArray _drawnStringBox;
	bool _continueScriptAfterUI = false;
	bool _isShowingDialogueChoice = false;
	uint16 _dialogueChoiceCount = 0;

	bool _isShowingInventory = false;

	// Map/Save/Load panel from handleMapPanelClick (1008:86a4).
	// This is a combined panel opened by right-click during script execution
	// or by action bar button 8. It has two areas:
	//
	// 1. Text slot list (10 slots, 12px tall each, max 21 chars):
	//    - Sub-mode 1 (save): clicking a slot opens text input for save name
	//    - Sub-mode 2 (map/travel): clicking a slot triggers travel
	//    Slot names stored at scene data offset 0x1f52 + slot*0x15 (Pascal strings)
	//
	// 2. Bottom button bar (7 buttons):
	//    Button 1 = Enter save mode (sub-mode=1)
	//    Button 2 = Enter map/travel mode (sub-mode=2)
	//    Button 3 = Toggle music on/off
	//    Button 4 = Page scroll (cycles 0->1->2->0)
	//    Button 5 = Confirm save (double-click pattern)
	//    Button 6 = Confirm load (double-click pattern)
	//    Button 7 = Play music / close panel
	//
	// The panel also supports the "map mode" (scene+0x61db != 0) where
	// clicking on the depth map previews/selects scenes.
	enum class MapPanelSubMode {
		None = 0,
		SaveSlots = 1,
		MapTravel = 2
	};
	bool _isMapPanelActive = false;
	MapPanelSubMode _mapPanelSubMode = MapPanelSubMode::None;
	uint16 _mapPanelPageIndex = 0;

	SpeechActData currentSpeechActData;

	Graphics::ManagedSurface _backgroundSurface;

	void drawDarkRectangle(uint16 x, uint16 y, uint16 width, uint16 height);

	void drawStringBackground(uint16 x, uint16 y, uint16 width, uint16 height);

	void drawBackgroundAnimations(Graphics::ManagedSurface &s);
	void drawBackgroundAnimationNumbers(Graphics::ManagedSurface &s);
	void drawCurrentSpeaker(Graphics::ManagedSurface &s);

	void renderString(uint16 x, uint16 y, Common::String s);
	void renderString(const Common::Point pos, const Common::String &s);

	void showStringBox(const Common::StringArray &sa);

	void drawGlyphs(Macs2::GlyphData *data, int count, uint16 x, uint16 y, Graphics::ManagedSurface &s);

	void handleFading();

	void drawPathfindingPoints(Graphics::ManagedSurface &s);

	void drawDebugOutput(Graphics::ManagedSurface &s);

	void drawPath(Graphics::ManagedSurface &s);

	int currentFadeValue = -1;
	int fadeDelta = 4;
	FadeMode fadeMode = FadeMode::None;
	bool _cursorSuppressedForFade = false;
	bool _cursorWasVisibleBeforeFade = false;

	void beginFadeCursorSuppression();
	void endFadeCursorSuppression(const byte *palette);

public:
	bool started = false;

	// As long as this debug bool is active, apply any click possible whenever it makes sense
	bool autoclickActive = false;

	Common::Array<Character *> characters;

	// Sets the source for the to-be-opened inventory and updats the array of inventory objects
	void SetInventorySource(GameObject *newInventorySource);
	void OpenInventory(GameObject *newInventorySource);
	void CloseInventory();

	bool IsInventorySourceProtagonist() const;

	// If this is the protagonist, we have our normal inventory
	// If this is another object, it is the inventory of a storage container
	GameObject *inventorySource;

	void TransferInventoryItem(GameObject *item, GameObject *targetContainer);

	int FindInventoryItem(GameObject *item);

	// TODO: Find a better place for those
	// The inventory items for the currently opened inventory
	Common::Array<GameObject *> inventoryItems;

	// If this is not null, we are using this object
	GameObject *activeInventoryItem = nullptr;

	Character *GetCharacterByIndex(uint16 index);

	int GetCharacterArrayIndex(const Character *c) const;
	bool HasDuplicateCharacters() const;

	// Updates the cursor from the mode set in the engine - TODO: Clean up, this should not
	// be so separated
	void UpdateCursor(const byte *palette = nullptr);
	bool isCursorSuppressedForFade() const { return _cursorSuppressedForFade; }

	View1();
	virtual ~View1() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;

	// Input state machine from handleInput (1008:e8bf).
	// The original game's input handler has two major branches:
	//
	// 1. NORMAL MODE (scene data offset 0x61db == 0):
	//    Two sub-modes based on whether a script is executing (0f88):
	//    a) Script NOT executing: Full interaction
	//       - Left click + Walk mode (0x16): pathfind to click position
	//       - Left click + other modes: hit-test hotspots/characters, run script
	//       - Right click: open action bar popup (g_wUiPanelState=1)
	//       - Button flag 8 (skip): fast-forward script to opcode 0x1D
	//    b) Script IS executing: Limited interaction
	//       - Left click: dismiss text boxes, dialogue choices, timer clicks
	//       - Right click: open map/save panel (state=4) if no UI is blocking
	//
	// 2. MAP MODE (scene data offset 0x61db != 0):
	//    - Left click: getDepthAtPoint() at click position
	//      depth 1..0xF9: preview that scene image
	//      depth 0xFF: full scene change back to main scene
	//
	// UI Panel States (g_wUiPanelState):
	//   0 = Normal gameplay
	//   1 = Action bar (right-click popup)
	//   2 = Inventory panel
	//   3 = Dialogue/save panel
	//   4 = Map panel
	//
	// Panel dismissal uses keyboard scancode in PTR_LOOP_1020_0fe8:
	//   6 = close inventory/dialogue panels
	//   7 = close map panel
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	void draw() override;
	bool tick() override;

	void drawInventory(Graphics::ManagedSurface &s);
	void drawInventory2(Graphics::ManagedSurface &s);
	GameObject *getClickedInventoryItem(const Common::Point &p);
	GameObject *getClickedInventoryItem2(const Common::Point &p);

	Common::Point inventoryGridUpperLeft;
	Common::Point inventorySlotSize;

	Common::Point stringBoxPosition;

	Common::Rect mainMenuRect;

	bool isShowingMainMenu = false;

	void openMainMenu(Common::Point clickedPosition);

	void drawMainMenu(Graphics::ManagedSurface &s);

	void setStringBox(const Common::StringArray &sa);
	void setStringBoxAt(const Common::StringArray &sa, const Common::Point &pos);
	void clearStringBox(bool continueScript = true);

	void startFading();
	void startFadeToBlack();

	void DrawSprite(int16 x, int16 y, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth = false, uint8 depth = 0);
	void DrawSprite(const Common::Point &pos, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth = false, uint8 depth = 0);
	void DrawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, uint16 width, uint16 height, const byte *const data, Graphics::ManagedSurface &s);
	void DrawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, const Sprite &sprite, Graphics::ManagedSurface &s);
	void DrawSpriteAdvanced(uint16 x, uint16 y, uint16 width, uint16 height, uint16 scaling, const byte *data, Graphics::ManagedSurface &s);
	void DrawSpriteAdvanced(const Common::Point &pos, uint16 width, uint16 height, uint16 scaling, const Sprite &sprite, Graphics::ManagedSurface &s);

	// The definitive version that can do everything
	void DrawSpriteSuperAdvanced(const Common::Point &pos, const Sprite &sprite, uint16 scaling, bool mirrored, bool useDepth, uint8 depth, Graphics::ManagedSurface &s);

	void DrawCharacters(Graphics::ManagedSurface &s);

	void ShowSpeechAct(uint16 characterIndex, const Common::Array<Common::String> &strings, const Common::Point &position, bool onRightSide = false);
	void DrawBorder(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);
	void DrawBorderSide(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);
	// fn0037_3AD4 proc
	void DrawBorderOuterHighlights(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);
	// ;; fn0037_3CDE: 0037:3CDE
	void DrawPressedBorderOuterHighlights(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);

	Macs2::Sprite *GetUISprite(uint32 offset);

	// fn0037_3737 proc
	void DrawHorizontalBorderHighlight(const Common::Point &pos, int16 width, uint32 spriteAddress, Graphics::ManagedSurface &s);
	// 0037h:3876h
	void DrawVerticalBorderHighlight(const Common::Point &pos, int16 height, uint32 spriteAddress, Graphics::ManagedSurface &s);

	void DrawImageResources(Graphics::ManagedSurface &s);

	void ShowDialogueChoice(uint16 speakerObjectID, const Common::Array<Common::StringArray> &choices, const Common::Point &position, bool onRightSide = false);

	void TriggerDialogueChoice(uint8 index);

	struct OverlayTextEntry {
		Common::Point position;
		uint8 alignment = 0;
		Common::String text;
	};

	void addOverlayTextEntry(const OverlayTextEntry &entry);
	void clearOverlayTextEntries();
	void drawOverlayTextEntries();

	Common::Array<OverlayTextEntry> _overlayTextEntries;

	uint16 CalculateCharacterScaling(uint16 characterY, bool updateDebugValues = false);

	uint16 GetHitObjectID(const Common::Point &pos) const;

	Common::Array<Common::Rect> inventoryButtonLocations;

	enum class InventoryButtonIndex {
		Look = 0,
		Hand = 1,
		Up = 2,
		Down = 3,
		Drop = 4,
		Close = 5
	};

	// Action bar button layout from handleActionBarClick (1008:42dc).
	// The action bar is a 3x3 grid of buttons, centered on the right-click
	// position. Each button is sized to fit the largest cursor icon + 6px padding.
	// The panel is clamped to screen bounds (320x200).
	enum class MainMenuButtonIndex {
		Talk = 0,       // Sets cursor mode to 0x13 (Talk)
		Look = 1,       // Sets cursor mode to 0x14 (Look)
		Use = 2,        // Sets cursor mode to 0x15 (Use)
		Walk = 3,       // Sets cursor mode to 0x16 (Walk)
		Inventory = 4,  // Opens inventory panel (g_wHasSavedUiBackground=1)
		InventoryUse = 5, // Uses selected inventory item (cursor 0x17), only if item selected
		Map = 6,        // Enters map mode (sets scene+0x61db=1), only if not disabled
		SaveLoad = 7,   // Opens dialogue/save panel (g_wHasSavedUiBackground=3)
		Close = 8       // Implicit close (clicking outside any button)
	};

	Common::Array<Common::Rect> mainMenuButtonLocations;

	uint16 inventoryPage = 0;
};

} // namespace Macs2

#endif
