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
	VM_HELP
};

enum class FadeMode {
	None,
	FromBlack,
	ToBlack
};

class Button {
public:
	Common::Point _position;
	Common::Point _size;
	Common::String _caption;

	bool isPointInside(const Common::Point &p) const;

	void render(Graphics::ManagedSurface &s);
};

class Character {
private:
	Common::Point _startPosition;

	uint32 _startTime;
	uint32 _duration;

	bool _lerpIgnoresObstacles = false;

	// If this is set, a lerp to a location becomes picking up
	Macs2::GameObject *_pickedUpObject = nullptr;

	// Frame counter for pickup animation (runtime+0x215).
	// Increments each frame while orientation == 0x11.
	// At _pickupFrameStart: item is transferred to inventory.
	// At _pickupFrameEnd: animation ends, orientation restored.
public:
	uint16 _pickupFrameCounter = 0;
	bool _pickupItemTransferred = false;

	uint8 _previousOrientation;
private:

	// Handle when the character has moved into a non-walkable area, push them out if
	// they did and return true, return false otherwise
	bool HandleWalkability(Character *c);

	// fn0037_0E8C proc
	uint8 lookupWalkability(const Common::Point &p) const;

public:
	Character();

	// Walk state from walkAlongPath (1008:1b8f) - runtime offsets +0x00..+0x0A, +0x18, +0x33
	Common::Point _endPosition;          // runtime[+0x00, +0x02]: next waypoint
	int16 _stepDeltaX = 0;              // runtime[+0x04]: abs(endX - startX)
	int16 _stepDeltaY = 0;              // runtime[+0x06]: abs(endY - startY)
	int16 _stepError = 0;               // runtime[+0x18]: Bresenham error accumulator
	bool _isLerping = false;            // walking active
	bool _stepDirectionSet = false;     // runtime[+0x33]: direction has been calculated

	Common::Array<uint16> _path;
	int16 _currentPathIndex;
	Common::Point _pathFinalDestination;
	bool _isFollowingPath = false;
	Common::Array<uint8> _pathfindingOverlay;

	bool isWalkable(const Common::Point &p) const;
	bool calculatePath(Common::Point target);
	bool canNodeConnectSourceToTarget(uint16 nodeIndex, const Common::Point &charPos, const Common::Point &target, const bool *reachable, int nodeCount);
	void floodFillConnectedNodes(int nodeIndex, bool *visited, int nodeCount);
	// Returns false if we are at the end of the path already or the path is not valid
	bool walkAlongPath();
	void startLerpTo(const Common::Point &target, uint32 duration, bool ignoreObstacles = false);
	void startPickup(Macs2::GameObject *object);

	Common::Point getPosition() const;
	void setPosition(const Common::Point &newPosition);
	Macs2::GameObject *_gameObject;

	uint16 getVerticalOffset() const;

	// Set by opcode 11h
	bool _executeScriptOnFinishLerp = false;

	// Runtime[+0x20D..+0x213]: per-character dirty rectangle for partial redraws.
	// Tracks the screen region covered by the character's sprite (old + new position union).
	// Uses inclusive coordinates matching the original binary's convention.
	int16 _dirtyLeft = 0;
	int16 _dirtyTop = 0;
	int16 _dirtyRight = 0;
	int16 _dirtyBottom = 0;

	// Runtime[+0x225..+0x22B]: last frame's sprite draw bounds (x, y, width, height).
	// Used at frame start to initialize _dirtyRect before erasing.
	int16 _lastDrawX = 0;
	int16 _lastDrawY = 0;
	uint16 _lastDrawWidth = 0;
	uint16 _lastDrawHeight = 0;

	// TODO: Handle properly
	uint8 _animationIndex = 1;
	uint16 _motionTargetVerticalOffset = 0;
	uint16 _motionVerticalOffsetDelta = 0;
	uint16 _motionDistanceUnits = 0;
	uint16 _motionProgress = 0;
	uint16 _motionStartVerticalOffset = 0;
	bool _hasMotionVerticalOffset = false;
	bool _shouldMirrorCurrentAnimation = false;

	bool isAnimationMirrored() const;
	uint8 getMirroredAnimation(uint8 original) const;

	// TODO: Will need time handling
	Macs2::AnimFrame *getCurrentAnimationFrame();
	Macs2::AnimFrame *getCurrentPortrait(bool onRightSide = false, uint16 frameIndex = 2);

	// Handles setting this character up to send an event to the script executor when finished
	// and will send the event right away in case the last movement is already done
	// TODO: Check if the code also handles it this way
	void registerWaitForMovementFinishedEvent();
	void update();
};

// cf https://stackoverflow.com/a/51497820
template<typename T, T V>
struct is_in_list_value {};

template<typename T, T V>
constexpr bool is_in_list_helper(T const &t, is_in_list_value<T, V>) {
	return t == V;
}

template<typename T, T V, T W, T... Rest>
constexpr bool is_in_list_helper(T const &t, is_in_list_value<T, V>, is_in_list_value<T, W>, is_in_list_value<T, Rest>...) {
	return (t == V) || is_in_list_helper(t, is_in_list_value<T, W>(), is_in_list_value<T, Rest>()...);
}

template<typename T, T... ts>
constexpr bool is_in_list(T const &t) {
	return is_in_list_helper(t, is_in_list_value<T, ts>()...);
}


struct SpeechActData {
	Character *speaker = nullptr;
	Common::Array<Common::String> strings;
	Common::Point position;
	bool onRightSide;
	// Mouth animation counter from handleTimerCallback (1008:d38b).
	// Decremented each frame. Controls which portrait frame is drawn:
	// >1: draw frame 2 from primary portrait blob (+0x14C)
	// ==0: draw frame 1 from alternate portrait blob (+0x15C) (mouth open)
	// <0: draw frame 2 from alternate portrait blob (+0x15C) (mouth closed)
	int16 mouthAnimCounter = 0;
	bool mouthAnimActive = false;
};

struct ScalingValues {
	uint16 characterY;
	uint16 scalingFactor;
};

class View1 : public UIElement {
	// TODO: Clean up private and public
public:
	ScalingValues _scalingValues;

	ViewMode _currentMode = ViewMode::VM_GAME;

	// Saved scene visuals for help screen restore (avoids changeScene on exit)
	byte _savedPalVanilla[256 * 3] = {0};
	Graphics::ManagedSurface _savedDepthMap;

	AnimFrame *getInventoryIcon(GameObject *gameObject);

	// TODO: use Graphics::Palette
	byte _pal[256 * 3] = {0};
	int _offset = 0;
	bool _paletteDirty = true;

	// Background animation timing from gameTick (1008:e556).
	// The original game increments a tick counter each frame (~70Hz DOS timer)
	// and advances background animations when the counter exceeds a threshold:
	// Background animation timing from gameTick (1008:e556).
	// g_wBgAnimTickCounter is incremented once per game frame (~20fps).
	// Background animation tick counter (mode 2: threshold 0x27, mode 3: threshold 1)
	static constexpr uint32 kGameFrameRate = 20;

	uint32 _bgAnimTickCounter = 0;

	uint32 _flagFrameIndex = 0;

	uint32 _guyFrameIndex = 0;

	// TODO: Probably the start of a mode enum
	bool _isShowingTextBox = false;
	Common::StringArray _drawnStringBox;
	bool _continueScriptAfterUI = false;
	bool _reopenInventoryAfterText = false;
	uint16 _dialogueChoiceCount = 0;
	Common::Array<uint16> _dialogueChoiceLineCounts;


	// Save/Load panel from handleSaveLoadPanelClick (1008:86a4).
	// Opened by right-click during script execution or action bar button 8.
	//
	// Sub-mode 1 (Load): clicking a slot loads the game from that file
	// Sub-mode 2 (Save): clicking a slot opens text input for save name
	//
	// Button bar (7 buttons):
	//   Button 1 = Enter load mode (sub-mode=1)
	//   Button 2 = Enter save mode (sub-mode=2)
	//   Button 3 = Toggle music on/off
	//   Button 4 = Page scroll (cycles 0->1->2->0)
	//   Button 5 = Confirm save (double-click pattern)
	//   Button 6 = Confirm load (double-click pattern)
	//   Button 7 = Play music / close panel
	enum class SaveLoadSubMode {
		None = 0,
		Load = 1,
		Save = 2
	};
	SaveLoadSubMode _saveLoadSubMode = SaveLoadSubMode::None;
	uint16 _saveLoadPageIndex = 0;
	bool _saveConfirmArmed = false;
	bool _loadConfirmArmed = false;
	bool _helpButtonDisabled = false;     // g_wMapDisabledFlag (1020:23b4): disables action bar help button AND save/load panel buttons 1-2
	uint16 _clickedButtonIndex = 0;       // g_wClickedButtonIndex: last clicked button (0=none)
	Common::String _saveSlotNames[30]; // 3 pages x 10 slots

	// Save/Load panel geometry (binary globals: g_wUiPanelX/Y/Width/Height, g_wActionBarButtonWidth/Height)
	Common::Rect _saveLoadPanelRect;
	Common::Rect _saveLoadButtonRects[7];
	uint16 _saveLoadButtonWidth = 0;      // g_wActionBarButtonWidth (after +6)
	uint16 _saveLoadButtonHeight = 0;     // g_wActionBarButtonHeight (after +6)

	void openOriginalSaveLoadPanel();
	void drawOriginalSaveLoadPanel(Graphics::ManagedSurface &s);
	void handleOriginalSaveLoadClick(const Common::Point &pos);
	void closeOriginalSaveLoadPanel();

	SpeechActData currentSpeechActData;

	Graphics::ManagedSurface _backgroundSurface;

	void drawDarkRectangle(uint16 x, uint16 y, uint16 width, uint16 height);

	void drawBackgroundAnimations(Graphics::ManagedSurface &s);
	void drawCurrentSpeaker(Graphics::ManagedSurface &s);

	void renderString(uint16 x, uint16 y, Common::String s);
	void renderString(const Common::Point pos, const Common::String &s);
	void renderStringWithFont(uint16 x, uint16 y, const Common::String &s, const GlyphData *glyphs, uint16 numGlyphs);
	int measureStringWithFont(const Common::String &s, const GlyphData *glyphs, uint16 numGlyphs);

	void showStringBox(const Common::StringArray &sa);

	void drawGlyphs(Macs2::GlyphData *data, int count, uint16 x, uint16 y, Graphics::ManagedSurface &s);

	void drawPathfindingPoints(Graphics::ManagedSurface &s);

	void drawDebugOutput(Graphics::ManagedSurface &s);

	void drawPath(Graphics::ManagedSurface &s);

	int _currentFadeValue = -1;
	int _fadeDelta = 4;
	FadeMode _fadeMode = FadeMode::None;
	bool _cursorSuppressedForFade = false;
	bool _cursorWasVisibleBeforeFade = false;

	void beginFadeCursorSuppression();
	void endFadeCursorSuppression(const byte *palette);

	bool handleInventoryClick(const MouseDownMessage &msg);
	bool handleActionBarClick(const MouseDownMessage &msg);

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
	bool handleInput(const MouseDownMessage &msg);
	bool handleHelpClick(const MouseDownMessage &msg);

public:
	View1();
	virtual ~View1() {}

	bool _started = false;

	// As long as this debug bool is active, apply any click possible whenever it makes sense
	bool _autoclickActive = false;

	Common::Array<Character *> _characters;

	// Sets the source for the to-be-opened inventory and updats the array of inventory objects
	void setInventorySource(GameObject *newInventorySource);
	void openInventory(GameObject *newInventorySource);
	void closeInventory();

	bool isInventorySourceProtagonist() const;

	// If this is the protagonist, we have our normal inventory
	// If this is another object, it is the inventory of a storage container
	GameObject *_inventorySource;

	void transferInventoryItem(GameObject *item, GameObject *targetContainer);

	int findInventoryItem(GameObject *item);

	// TODO: Find a better place for those
	// The inventory items for the currently opened inventory
	Common::Array<GameObject *> _inventoryItems;

	// If this is not null, we are using this object
	GameObject *_activeInventoryItem = nullptr;

	Character *getCharacterByIndex(uint16 index);

	int getCharacterArrayIndex(const Character *c) const;

	// Updates the cursor from the mode set in the engine - TODO: Clean up, this should not
	// be so separated
	void updateCursor(const byte *palette = nullptr);
	bool isCursorSuppressedForFade() const { return _cursorSuppressedForFade; }

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;

	// Debug: last hovered area/hotspot (updated every mouse move for ImGui)
	uint16 _hoverAreaId = 0;
	uint16 _hoverHotspotId = 0;

	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	void draw() override;
	bool tick() override;

	void drawInventory(Graphics::ManagedSurface &s);
	GameObject *getClickedInventoryItem(const Common::Point &p);

	Common::Point _inventoryGridUpperLeft;
	Common::Point _inventorySlotSize;

	Common::Point _stringBoxPosition;

	Common::Rect _mainMenuRect;

	// Binary g_wUiPanelState: 0=none, 1=action bar, 2=protagonist inv, 3=container inv, 4=save/load
	enum UiPanelState : uint16 {
		kUiPanelNone = 0,
		kUiPanelActionBar = 1,
		kUiPanelInventory = 2,
		kUiPanelContainerInventory = 3,
		kUiPanelSaveLoad = 4
	};
	UiPanelState _uiPanelState = kUiPanelNone;

	// Binary g_wIsShowingDialoguePanel (1020:1008)
	bool _isShowingDialogueChoicePanel = false;

	// Binary g_wPendingPanelRequest (1020:1034): deferred panel open request.
	// Set while action bar is active; processed by gameTick when _uiPanelState returns to kUiPanelNone.
	// Values: 0=none, 1=protagonist inventory, 2=container inventory, 3=save/load
	enum PendingPanelRequest : uint16 {
		kPanelRequestNone = 0,
		kPanelRequestInventory = 1,
		kPanelRequestContainerInventory = 2,
		kPanelRequestSaveLoad = 3,
		kPanelRequestSaveLoadActive = 4   // Set by handleSaveLoadPanelClick to keep panel alive
	};
	PendingPanelRequest _pendingPanelRequest = kPanelRequestNone;

	// Binary g_wSavedCursorMode [scene+0xFEA]: saved before opening action bar panel
	Script::MouseMode _cursorModeBeforeMenu = Script::MouseMode::Walk;

	void openMainMenu(Common::Point clickedPosition);

	void drawMainMenu(Graphics::ManagedSurface &s);

	void setStringBox(const Common::StringArray &sa);
	void setStringBoxAt(const Common::StringArray &sa, const Common::Point &pos);
	void handleTextBoxInput();
	void dismissDialoguePanel();
	bool handleDialogueChoiceClick(int clickY, int clickX);

	void startFading(uint16 speed = 4);
	void startFadeToBlack(uint16 speed = 4);
	void startFadingWithSpeed(uint16 speed);

	void drawSprite(int16 x, int16 y, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth = false, uint8 depth = 0);
	void drawSprite(const Common::Point &pos, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth = false, uint8 depth = 0);
	void drawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, uint16 width, uint16 height, const byte *const data, Graphics::ManagedSurface &s);
	void drawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, const Sprite &sprite, Graphics::ManagedSurface &s);
	void drawSpriteAdvanced(uint16 x, uint16 y, uint16 width, uint16 height, uint16 scaling, const byte *data, Graphics::ManagedSurface &s);
	void drawSpriteAdvanced(const Common::Point &pos, uint16 width, uint16 height, uint16 scaling, const Sprite &sprite, Graphics::ManagedSurface &s);

	// The definitive version that can do everything
	void drawSpriteSuperAdvanced(const Common::Point &pos, const Sprite &sprite, uint16 scaling, bool mirrored, bool useDepth, uint8 depth, Graphics::ManagedSurface &s, uint8 shadowIntensity = 0);

	void drawCharacters(Graphics::ManagedSurface &s);
	void drawAllCharacters();

	void showSpeechAct(uint16 characterIndex, const Common::Array<Common::String> &strings, const Common::Point &position, bool onRightSide = false);
	void drawBorder(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);
	void drawBorderSide(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);
	// fn0037_3AD4 proc
	void drawBorderOuterHighlights(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);
	// ;; fn0037_3CDE: 0037:3CDE
	void drawPressedBorderOuterHighlights(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);

	Macs2::AnimFrame *getUISprite(uint32 offset);

	// fn0037_3737 proc
	void drawHorizontalBorderHighlight(const Common::Point &pos, int16 width, uint32 spriteAddress, Graphics::ManagedSurface &s);
	// 0037h:3876h
	void drawVerticalBorderHighlight(const Common::Point &pos, int16 height, uint32 spriteAddress, Graphics::ManagedSurface &s);

	void drawImageResources(Graphics::ManagedSurface &s);

	void showDialogueChoice(uint16 speakerObjectID, const Common::Array<Common::StringArray> &choices, const Common::Point &position, bool onRightSide = false);

	void triggerDialogueChoice(uint8 index);

	struct OverlayTextEntry {
		Common::Point position;
		uint8 alignment = 0;
		Common::String text;
	};

	void addOverlayTextEntry(const OverlayTextEntry &entry);
	void clearOverlayTextEntries();
	void drawOverlayTextEntries();

	Common::Array<OverlayTextEntry> _overlayTextEntries;

	uint16 calculateCharacterScaling(uint16 characterY, bool updateDebugValues = false);

	uint16 getHitObjectID(const Common::Point &pos) const;

	Common::Array<Common::Rect> _inventoryButtonLocations;

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
		Talk = 0,         // Sets cursor mode to 0x13 (Talk)
		Look = 1,         // Sets cursor mode to 0x14 (Look)
		Use = 2,          // Sets cursor mode to 0x15 (Use)
		Walk = 3,         // Sets cursor mode to 0x16 (Walk)
		Inventory = 4,    // Opens inventory panel (g_wHasSavedUiBackground=1)
		InventoryUse = 5, // Uses selected inventory item (cursor 0x17), only if item selected
		Map = 6,          // Opens help/info screen (sets scene+0x61db=1 for map panel mode)
		SaveLoad = 7,     // Opens dialogue/save panel (g_wHasSavedUiBackground=3)
		Close = 8         // Implicit close (clicking outside any button)
	};

	Common::Array<Common::Rect> _mainMenuButtonLocations;

	uint16 _inventoryPage = 0;
};

} // namespace Macs2

#endif
