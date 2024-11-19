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
	Character *objectToPickUp = nullptr;

	// Handle when the character has moved into a non-walkable area, push them out if
	// they did and return true, return false otherwise
	bool HandleWalkability(Character *c);

	// fn0037_0E8C proc
	uint8 LookupWalkability(const Common::Point &p) const;
	bool IsWalkable(const Common::Point &p) const;

	
	bool IsLineSegmentWalkable(const Common::Point &p1, const Common::Point &p2, bool print = false);


	public:
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

	// Returns false if we are at the end of the path already or the path is not valid
	bool TryFollowPath();

	// Set by opcode 11h
	bool ExecuteScriptOnFinishLerp = false;

	// TODO: Handle properly
	uint8 animationIndex = 1;

	bool isAnimationMirrored() const;
	uint8 getMirroredAnimation(uint8 original) const;

		// TODO: Will need time handling
	Macs2::AnimFrame *GetCurrentAnimationFrame();
		Macs2::AnimFrame *GetCurrentPortrait();
		void StartLerpTo(const Common::Point &target, uint32 duration, bool ignoreObstacles = false);

		void StartPickup(Character *object);

		// Handles setting this character up to send an event to the script executor when finished
		// and will send the event right away in case the last movement is already done
		// TODO: Check if the code also handles it this way
		void RegisterWaitForMovementFinishedEvent();
		void Update();
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
	
	byte _pal[256 * 3] = { 0 };
	int _offset = 0;

	uint32 _frameDelayFlag = 100;
	int32 _nextFrameFlag = _frameDelayFlag;
	uint32 _lastMillis = 0;
	bool _hasTicked = false;
	uint32 _flagFrameIndex = 0;

	uint32 _guyFrameIndex = 0;

	// TODO: Probably the start of a mode enum
	bool _isShowingStringBox = false;
	Common::StringArray _drawnStringBox;
	bool _continueScriptAfterUI = false;

	bool _isShowingInventory = false;

	SpeechActData currentSpeechActData;


	Graphics::ManagedSurface _backgroundSurface;

	void drawDarkRectangle(uint16 x, uint16 y, uint16 width, uint16 height);

	void drawStringBackground(uint16 x, uint16 y, uint16 width, uint16 height);

	void drawBackgroundAnimations(Graphics::ManagedSurface &s);
	void drawBackgroundAnimationNumbers(Graphics::ManagedSurface &s);
	void drawCurrentSpeaker(Graphics::ManagedSurface &s);

	void renderString(uint16 x, uint16 y, Common::String s);

	void showStringBox(const Common::StringArray &sa);

	void drawGlyphs(Macs2::GlyphData *data, int count, uint16 x, uint16 y, Graphics::ManagedSurface &s);

	void handleFading();

	void drawPathfindingPoints(Graphics::ManagedSurface &s);

	void drawDebugOutput(Graphics::ManagedSurface &s);

	void drawPath(Graphics::ManagedSurface &s);

	int currentFadeValue = -1;
	int fadeDelta = 4;


public:
	bool started = false;

	// As long as this debug bool is active, apply any click possible whenever it makes sense
	bool autoclickActive = false;

	Common::Array<Character *> characters;

	// Sets the source for the to-be-opened inventory and updats the array of inventory objects
	void SetInventorySource(GameObject *newInventorySource);

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

	View1();
	virtual ~View1() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage& msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	void draw() override;
	bool tick() override;

	void drawInventory(Graphics::ManagedSurface &s);
	GameObject *getClickedInventoryItem(const Common::Point &p);

	void setStringBox(const Common::StringArray& sa);
	void clearStringBox();

	void startFading();

	void DrawSprite(int16 x, int16 y, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth = false, uint8 depth = 0);
	void DrawSprite(const Common::Point &pos, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s, bool mirrored, bool useDepth = false, uint8 depth = 0);
	void DrawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, uint16 width, uint16 height, const byte * const data, Graphics::ManagedSurface &s);
	void DrawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, const Sprite& sprite, Graphics::ManagedSurface &s);
	void DrawSpriteAdvanced(uint16 x, uint16 y, uint16 width, uint16 height, uint16 scaling, const byte *data, Graphics::ManagedSurface &s);
	void DrawSpriteAdvanced(const Common::Point &pos, uint16 width, uint16 height, uint16 scaling, const Sprite& sprite, Graphics::ManagedSurface &s);

	void DrawCharacters(Graphics::ManagedSurface &s);

	void ShowSpeechAct(uint16 characterIndex, const Common::Array<Common::String> &strings, const Common::Point &position, bool onRightSide = false);
	void DrawBorder(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);
	void DrawBorderSide(const Common::Point &pos, const Common::Point &size, Graphics::ManagedSurface &s);
	void DrawHorizontalBorderHighlight(const Common::Point &pos, int16 width, uint8 unknown, Graphics::ManagedSurface &s);
	void DrawVerticalBorderHighlight(const Common::Point &pos, int16 height, uint8 unknown, Graphics::ManagedSurface &s);

	void ShowDialogueChoice(const Common::Array<Common::StringArray> &choices, const Common::Point &position, bool onRightSide = false);

	void TriggerDialogueChoice(uint8 index);

	uint16 CalculateCharacterScaling(uint16 characterY, bool updateDebugValues = false);

	uint16 GetHitObjectID(const Common::Point &pos) const;
};

} // namespace Macs2

#endif
