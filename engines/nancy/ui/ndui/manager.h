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

#ifndef NANCY_UI_NDUI_MANAGER_H
#define NANCY_UI_NDUI_MANAGER_H

#include "common/array.h"
#include "common/hash-str.h"

#include "engines/nancy/puzzledata.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/ui/ndui/common.h"

namespace Graphics {
class ManagedSurface;
}

namespace Nancy {

struct NancyInput;

namespace UI {
namespace NDUI {

class Panel;
class Control;
class Tooltip;
class InvButton;

// Something outside the UI that answers the actions sent to an "Engine_" target of its
// own, for as long as it is registered with the Manager
class EngineTarget {
public:
	virtual ~EngineTarget() {}
	virtual void onEngineAction(const Action &action) = 0;
};

// The Nancy16 UI, owned by the scene. It creates the panels from the NDUI chunks of the
// player character's UI IFF and the IFFs it includes, and stands between them and the
// rest of the engine:
// - Actions, whether fired by a control's event or sent by a scene's UIControl record,
//   go to the panel or control they name. The ones naming an "Engine_" target go to
//   the engine handlers instead: inventory, journal, saving and loading, options...
// - The scene tells it about the inventory, captions, conversations, the clock and scene
//   changes, which it passes on to the controls that show them.
// - Input goes to the topmost panel under the mouse, and keys to the focused edit box.
// - What the UI asks of the engine (new game, load, save, quit) waits for the scene to
//   pick it up, since it can replace the UI itself.
// It also keeps the textures the elements draw, and the templates of the controls that
// get created while the game runs, like journal entries and conversation responses.
class Manager {
public:
	static const uint16 kTooltipZ = 2000;

	// What the UI asked of the engine, which has to wait until the UI is done
	// handling the player's input, since it can replace the UI
	enum RequestType {
		kRequestNone,
		kRequestNewGame,
		kRequestLoad,
		kRequestSave,
		kRequestQuit
	};

	struct Request {
		RequestType type = kRequestNone;
		int slot = -1;
		Common::String name;
		bool hasFollowUpAction = false;	// a quit or load follows the save
	};

	~Manager();

	Request takeRequest();

	// Nancy18 scenes save and load games by name, which is how it does second chances
	void requestSave(const Common::String &name);
	void requestLoad(const Common::String &name);

	// Goes on with the quit or load that was put off to ask whether to save first
	void runPendingAction();

	// Creates every panel in the IFF with the given name, and in the ones it includes
	void load(const Common::String &name);

	// Nancy19 panels are made by scripts, which fill them in afterwards. They have to
	// be taken down before the scripts go away, since their controls call into them.
	Panel *addPanel(byte kind);
	void clearPanels();

	// The control whose event is being handled by a script
	const Common::String &getEventSender() const { return _eventSender; }
	void setEventSender(const Common::String &sender) { _eventSender = sender; }

	void takeAction(const Action &action);
	void takeAction(int32 type, const Common::String &target, double value);
	void takeAction(int32 type, const Common::String &target, const Common::String &text);
	void handleInput(NancyInput &input);
	void registerGraphics();

	// Nancy17: the control under the mouse that has a tooltip, and the panel's control that shows it
	void setHoveredTooltip(const Control *control, Tooltip *tooltip);

	const Graphics::ManagedSurface *getTexture(const Common::String &name);

	// Creates a control from a template, as the last one of a panel
	void createObject(const Common::String &name, const Common::String &templateName, const Common::String &parentName);
	void destroyObject(const Common::String &name);

	// Engine state shown by the UI
	void update();
	void setCaption(const Common::String &text);
	const Common::String &getCaption() const { return _caption; }
	void updatePlayerTime();
	void createItemButtons();
	// Takes an item out of the inventory, and opens the scene that shows it
	void viewInventoryItem(int16 itemID);

	// Nancy17 cell phone
	void createPhoneBook();
	void showPhoneBookEntry(const PhoneBookData::Entry &entry, bool isNew);
	void showInventoryItem(int16 itemID, bool show);
	void onSceneChange(uint16 sceneID);
	void addJournalEntry(const Common::String &pageName, const Common::String &textKey);
	void addTasklistEntry(const Common::String &textKey, int16 flag);
	void startConversation(const Common::String &caption, const Common::Array<Common::String> &responses);
	void endConversation();
	// Once the player has picked a response, the others can't be picked anymore
	void disableOtherResponses(uint pickedResponse);

	// Displays a value of the value table, until it gets unregistered
	void registerValueDisplay(int16 index, const Common::String &display);
	// Nancy19 has the values by name
	void registerValueDisplay(const Common::String &valueName, const Common::String &display);

	// What scenes and the player changed about the panels and controls, kept in saved games
	void saveState(Common::Array<UIStateData::Change> &changes) const;
	// The controls show up as they were saved right away, since a UI that was
	// already on screen has nothing to fade in from
	void restoreState(const Common::Array<UIStateData::Change> &changes);

	void addEngineTarget(const Common::String &name, EngineTarget *target);
	void removeEngineTarget(const Common::String &name);

	// The background color outside of the viewport, picked in the options
	void applyMatteColor();

private:
	struct ValueDisplay {
		int16 index = 0;
		Common::String valueName;	// Nancy19
		Common::String name;
		bool hasShownValue = false;
		int32 shownValue = 0;
	};

	typedef Common::HashMap<Common::String, Common::String, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> StringMap;
	typedef Common::HashMap<Common::String, int16, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FlagMap;

	void takeEngineAction(const Action &action);
	void onInventoryClicked(int16 itemID);
	void onJournalAction(const Action &action);
	void onTasklistAction(const Action &action);
	void playTasklistSound(uint bank, bool play);
	void onSoundAction(const Action &action);

	void onLoaderAction(const Action &action);
	void onSaverAction(const Action &action);
	void onSaveQueryAction(const Action &action);
	void onGameWindowAction(const Action &action);
	void onCellPhoneAction(const Action &action);
	void onItemStreamClicked(const InvButton &button);
	void onVolumeAction(const Action &action, const char *configKey, uint16 sampleChannel);
	void onFlagsAction(const Action &action);
	void askToSaveFirst(const Action &action);
	uint fillSaveList(const Common::String &listName);
	int findSaveSlot(const Common::String &name) const;
	int runScummVMSaveLoadDialog(bool isSave, Common::String &saveName);
	void setSaveScreenshot(const Graphics::ManagedSurface &image, const Common::Rect &src);

	Panel *findPanel(const Common::String &name) const;
	InvButton *findItemButton(int16 itemID) const;

	Common::Array<Panel *> _panels;
	Common::String _eventSender;
	Common::String _caption;
	Common::HashMap<Common::String, Graphics::ManagedSurface *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _textures;
	Common::HashMap<Common::String, Common::Array<byte>, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _templates;

	void updateTooltip(const Common::Point &mousePos);

	// Nancy17 tooltips are drawn on top of every panel
	RenderObject _tooltipWindow{kTooltipZ};
	const Control *_hoveredControl = nullptr;
	Tooltip *_hoveredTooltip = nullptr;
	const Control *_tooltipControl = nullptr;
	uint32 _tooltipHoverStart = 0;
	bool _isTooltipDismissed = false;

	Common::Array<Common::String> _clockDisplays;
	Common::Array<ValueDisplay> _valueDisplays;
	Common::Array<ValueDisplay> _timerDisplays;		// meters showing what is left of a timer

	StringMap _journalPages;		// page title -> page panel
	Common::String _shownJournalPage;
	FlagMap _tasklistFlags;			// entry -> event flag that has to be set to check it off

	bool _isInConversation = false;
	Common::Array<Common::String> _responseNames;

	Common::Array<uint16> _pausedChannels;	// sound channels the UI paused, to resume later

	Common::HashMap<Common::String, EngineTarget *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _engineTargets;

	Common::String _selectedSave;	// in the load dialog
	Common::String _typedSaveName;	// in the save dialog
	Common::String _lastSaveName;

	// What the player was about to do when asked whether to save first
	bool _hasPendingAction = false;
	Action _pendingAction;

	Request _request;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_MANAGER_H
