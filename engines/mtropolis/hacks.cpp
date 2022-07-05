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

#include "common/system.h"
#include "common/hashmap.h"

#include "mtropolis/assets.h"
#include "mtropolis/detection.h"
#include "mtropolis/hacks.h"
#include "mtropolis/runtime.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/saveload.h"

namespace MTropolis {

Hacks::Hacks() {
	ignoreMismatchedProjectNameInObjectLookups = false;
}

Hacks::~Hacks() {
}

void Hacks::addStructuralHooks(uint32 guid, const Common::SharedPtr<StructuralHooks> &hooks) {
	structuralHooks[guid] = hooks;
}

void Hacks::addModifierHooks(uint32 guid, const Common::SharedPtr<ModifierHooks> &hooks) {
	modifierHooks[guid] = hooks;
}

void Hacks::addAssetHooks(const Common::SharedPtr<AssetHooks> &hooks) {
	assetHooks.push_back(hooks);
}

void Hacks::addSceneTransitionHooks(const Common::SharedPtr<SceneTransitionHooks> &hooks) {
	sceneTransitionHooks.push_back(hooks);
}

void Hacks::addSaveLoadHooks(const Common::SharedPtr<SaveLoadHooks> &hooks) {
	saveLoadHooks.push_back(hooks);
}

namespace HackSuites {

class ObsidianCorruptedAirTowerTransitionFix : public AssetHooks {
public:
	void onLoaded(Asset *asset, const Common::String &name) override;
};

void ObsidianCorruptedAirTowerTransitionFix::onLoaded(Asset *asset, const Common::String &name) {
	if (asset->getAssetType() == kAssetTypeMovie && name == "A105_132.01Fxx.trn") {
		static_cast<MovieAsset *>(asset)->addDamagedFrame(35);
	}
}

class ObsidianInventoryWindscreenHooks : public StructuralHooks {
public:
	void onSetPosition(Structural *structural, Common::Point &pt) override;
};

void ObsidianInventoryWindscreenHooks::onSetPosition(Structural *structural, Common::Point &pt) {
	if (pt.y < 480) {
		// Set direct to screen so it draws over cinematics
		static_cast<VisualElement *>(structural)->setDirectToScreen(true);

		// Move in-bounds
		pt.y -= 60;
	}
}

class ObsidianSecurityFormWindscreenHooks : public StructuralHooks {
public:
	void onSetPosition(Structural *structural, Common::Point &pt) override;

private:
	Common::Array<uint32> _hiddenCards;
};

void ObsidianSecurityFormWindscreenHooks::onSetPosition(Structural *structural, Common::Point &pt) {
	bool cardVisibility = (pt.y > 480);

	// Originally tried manipulating layer order but that's actually not a good solution because
	// the form graphic is not actually dismissed until the cinematic completes.  It's normally not
	// visible because the cinematic is drawn over it, but managing that vis-a-vis the cards is a mess,
	// and the form graphic actually includes a bit of area to the left due to the vidbot's arm being
	// on the desk, which partially overlaps the cards, but not completely.
	Structural *subsection = structural->getParent()->getParent();
	assert(subsection->isSubsection());

	Structural *sharedScene = subsection->getChildren()[0].get();
	assert(sharedScene);

	Structural *cards = nullptr;
	for (const Common::SharedPtr<Structural> &child : sharedScene->getChildren()) {
		if (child->getName() == "Inventory Cards") {
			cards = child.get();
			break;
		}
	}

	if (!cardVisibility)
		_hiddenCards.clear();

	if (cards) {
		for (const Common::SharedPtr<Structural> &child : cards->getChildren()) {
			assert(child->isElement() && static_cast<Element *>(child.get())->isVisual());

			VisualElement *card = static_cast<VisualElement *>(child.get());

			if (cardVisibility) {
				if (Common::find(_hiddenCards.begin(), _hiddenCards.end(), card->getStaticGUID()) != _hiddenCards.end())
					card->setVisible(true);
			} else {
				if (card->isVisible()) {
					_hiddenCards.push_back(card->getStaticGUID());
					card->setVisible(false);
				}
			}
		}
	}

	if (cardVisibility)
		_hiddenCards.clear();
}

void addObsidianBugFixes(const MTropolisGameDescription &desc, Hacks &hacks) {
	// Workaround for bug in Obsidian:
	// When opening the journal in the intro, a script checks if cGSt.cfst.binjournal is false and if so,
	// sets cGSt.cfst.binjournal to true and then sets including setting cJournalConst.aksjournpath to the
	// main journal scene path.  That scene path is used to resolve the scene to go to after clicking
	// the "Continue" button on the warning that pops up.
	//
	// The problem is that cJournalConst uses a project name that doesn't match the retail data, and
	// cJournalConst is unloaded if the player leaves the journal.  This causes a progression blocker if
	// the player leaves the journal without clicking Continue.
	hacks.ignoreMismatchedProjectNameInObjectLookups = true;

	// Fix for corrupted frame in transition from the outer edge in Spider to the air puzzle tower.
	// The data is corrupted in both Mac and Win retail versions.
	hacks.addAssetHooks(Common::SharedPtr<AssetHooks>(new ObsidianCorruptedAirTowerTransitionFix()));
}

void addObsidianImprovedWidescreen(const MTropolisGameDescription &desc, Hacks &hacks) {
	if ((desc.desc.flags & ADGF_DEMO) == 0 && desc.desc.language == Common::EN_ANY && desc.desc.platform == Common::kPlatformWindows) {
		const uint32 inventoryItemGUIDs[] = {
			// Bureau documents
			// 100 area (booths)
			0x4e2d9e,
			0x4de654,

			// 199 area (booths hint room)
			0x4e2555,
			0x4de654,

			// 200 area (library)
			0x4c83d2,
			0x4c5802,

			// 299 area (Cloud Ring)
			0x178d5c,
			0x177754,

			// 300 area (light+phone)
			0x4e0f86,
			0x4e5107,

			// 400 area (maze)
			0x4e5528,
			0x4e55cc,

			// 500 area (Immediate Action)
			0x4e2e7b,
			0x4e0710,

			// 800 area (bookshelves)
			0x9914fb,
			0x990f1f,

			// 600 (sky face), 699 (mountain), and 700 (finale) have no document elements (player loses the documents)

			// Bureau maze keycards
			0x6035f,
			0x62e24,
			0x58d7f,
			0x58212,

			// Spider metal puzzle beaker
			0x12fa7,

			// Inspiration chip
			// 100 area (Junkyard)
			0x5f02e6,

			// 200 area (Plane)
			0x9bd5fc,

			// 300 area (Piazza)
			0x5ef979,
			
			// 400 area (Church)
			0xed9a8f,

			// 500 area (Statue)
			0x5ecdee,
		};

		const uint32 cubeMazeSecurityFormGUID = 0x9602ec;

		Common::SharedPtr<StructuralHooks> invItemHooks(new ObsidianInventoryWindscreenHooks());

		for (uint32 guid : inventoryItemGUIDs)
			hacks.addStructuralHooks(guid, invItemHooks);

		hacks.addStructuralHooks(cubeMazeSecurityFormGUID, Common::SharedPtr<StructuralHooks>(new ObsidianSecurityFormWindscreenHooks()));
	}
	if ((desc.desc.flags & ADGF_DEMO) == 0 && desc.desc.language == Common::EN_ANY && desc.desc.platform == Common::kPlatformMacintosh) {
		const uint32 inventoryItemGUIDs[] = {
			// Bureau documents
			// 100 area (booths)
			0x4dfa22,
			0x4e2d9e,

			// 199 area (booths hint room)
			0x4e2555,
			0x4de654,

			// 200 area (library)
			0x4c83d2,
			0x4c5802,

			// 299 area (Cloud Ring)
			0x178d5c,
			0x177754,

			// 300 area (light+phone)
			0x4e0f86,
			0x4e5107,

			// 400 area (maze)
			0x4e5528,
			0x4e55cc,

			// 500 area (Immediate Action)
			0x4a2e7b,
			0x4e0710,

			// 800 area (bookshelves)
			0x9914fb,
			0x990f1f,

			// 600 (sky face), 699 (mountain), and 700 (finale) have no document elements (player loses the documents)

			// Bureau maze keycards
			0x6035f,
			0x62e24,
			0x58d7f,
			0x58212,

			// Spider metal puzzle beaker
			0x12fa7,

			// Inspiration chip
			// 100 area (Junkyard)
			0x5f02e6,

			// 200 area (Plane)
			0x9bd5fc,

			// 300 area (Piazza)
			0x5ef979,

			// 400 area (Church)
			0xed9a8f,

			// 500 area (Statue)
			0x5ecdee,
		};

		const uint32 cubeMazeSecurityFormGUID = 0x9602ec;

		Common::SharedPtr<StructuralHooks> invItemHooks(new ObsidianInventoryWindscreenHooks());

		for (uint32 guid : inventoryItemGUIDs)
			hacks.addStructuralHooks(guid, invItemHooks);

		hacks.addStructuralHooks(cubeMazeSecurityFormGUID, Common::SharedPtr<StructuralHooks>(new ObsidianSecurityFormWindscreenHooks()));
	}
}

// Auto-save triggers for Obsidian.  Basically, we auto-save on reaching specific scenes when conditions are met.
// There are two types of condition: One is the player reaches the scene from a one-way scene that can not be
// revisited, such as a chapter transition.
//
// The other is a variable latch, which happens if the variable became true since the last time the reset scene
// (the opening credits) was reached, or since the last time the game was loaded.
//
// Variable latches don't work if the latch becomes true and the player saves+reloads their game before they hit
// the auto-save checkpoint, but that's okay.

struct ObsidianAutoSaveTrigger {
	const char *sceneName;
	const char *priorSceneName; // If set, only save when transitioning from this scene
	const char *varTrueLatch;   // If set, only save when this variable was set to
};

static ObsidianAutoSaveTrigger kObsidianAutoSaveTriggers[] = {
	// Arrive at campsite
	{
		"103.2L",
		"102_103_Credits",
		nullptr,
	},
	// Bureau start
	{
		"102.0L",
		nullptr,
		"cgst.clst.cl100st.bbossspoken"
	},
	// Win cube maze from side room
	{
		"445.2L",
		nullptr,
		"cgst.clst.cl400st.bcubiclewon",
	},
	// Win cube maze from back room
	{
		"445.0L",
		nullptr,
		"cgst.clst.cl400st.bcubiclewon",
	},
	// Stamp document and back away
	{
		"445.2L",
		nullptr,
		"cgst.clst.cinv.bstampedsd",
	},
	// Get repair document from cabinet + back away (English version)
	{
		"218.4L",
		nullptr,
		"cgst.clst.cinv.bhavesd",
	},
	// Get repair document from Bridge Repair + back away (non-English versions)
	{
		"109.6L",
		nullptr,
		"cgst.clst.cinv.bhavesd",
	},
	// Fix Immediate Action clock + exit left side
	{
		"504.0L",
		nullptr,
		"cgst.clst.cl500st.bwonl505",
	},
	// Fix Immediate Action clock + exit right side
	{
		"504.4L",
		nullptr,
		"cgst.clst.cl500st.bwonl505",
	},
	// Give document to Immediate Action + return to light
	{
		"306.6L",
		nullptr,
		"cgst.clst.cinv.bgavesd",
	},
	// Get rebel document + leave cabinet (English version)
	{
		"227.4L",
		nullptr,
		"cgst.clst.cinv.bhaveom",
	},
	// Solve dial puzzle
	{
		"699.0L",
		nullptr,
		"cgst.clst.cl308st.bwon",
	},

	// Spider start
	{
		"101.2L",
		"710.0L",
		nullptr,
	},
	// Leave elevator after completing any Spider puzzle
	{
		"121.0L",
		"118.0L_121.0L-s1",
		nullptr,
	},

	// Inspiration start
	{
		"101.4L",
		"121.0L_Lunch_Time",
		nullptr,
	},
	// Complete propulsion puzzle and leave engine room
	{
		"201.4L",
		nullptr,
		"cgst.cbst.cb2st.bengineon",
	},
	// Complete Church puzzle and leave robot
	{
		"412.4L",
		nullptr,
		"cgst.cbst.cb4st.bwon",
	},
	// Complete statue canvas puzzle
	{
		"523.2L_from_521.2L-s2",
		nullptr,
		"cgst.cbst.cb5st.bpaintedblank",
	},

	// Conductor start
	{
		"101.6L",
		"203.4L_WIN",
		nullptr,
	},
	// Freed Max
	{
		"104.0L_Max_Freed",
		nullptr,
		"cgst.ccst.cc1st.bmaxfreed",
	},
};

class ObsidianAutoSaveVarsState {
public:
	ObsidianAutoSaveVarsState();

	static const VariableModifier *findVar(Runtime *runtime, const Common::String &str);

	bool getVarState(const Common::String &varName) const;
	void resyncAllVars(Runtime *runtime);

private:
	Common::HashMap<Common::String, bool> _varState;
};

ObsidianAutoSaveVarsState::ObsidianAutoSaveVarsState() {
	for (const ObsidianAutoSaveTrigger &trigger : kObsidianAutoSaveTriggers) {
		if (trigger.varTrueLatch)
			_varState[trigger.varTrueLatch] = false;
	}
}

const VariableModifier *ObsidianAutoSaveVarsState::findVar(Runtime *runtime, const Common::String &str) {
	size_t scanStartPos = 0;

	const Modifier *modifierScan = nullptr;
	const IModifierContainer *container = runtime->getProject();

	for (;;) {
		size_t dotPos = str.findFirstOf('.', scanStartPos);
		if (dotPos == Common::String::npos)
			dotPos = str.size();

		Common::String childName = str.substr(scanStartPos, dotPos - scanStartPos);
		if (!container)
			return nullptr;

		modifierScan = nullptr;
		for (const Common::SharedPtr<Modifier> &modifier : container->getModifiers()) {
			if (caseInsensitiveEqual(childName, modifier->getName())) {
				modifierScan = modifier.get();
				break;
			}
		}

		if (!modifierScan)
			return nullptr;

		if (modifierScan->isCompoundVariable())
			container = static_cast<const CompoundVariableModifier *>(modifierScan);

		if (dotPos == str.size())
			break;

		scanStartPos = dotPos + 1;
	}

	if (modifierScan && modifierScan->isVariable())
		return static_cast<const VariableModifier *>(modifierScan);

	return nullptr;
}

bool ObsidianAutoSaveVarsState::getVarState(const Common::String &varName) const {
	Common::HashMap<Common::String, bool>::const_iterator it = _varState.find(varName);
	if (it == _varState.end())
		return false;
	return it->_value;
}

void ObsidianAutoSaveVarsState::resyncAllVars(Runtime *runtime) {
	for (Common::HashMap<Common::String, bool>::iterator it = _varState.begin(), itEnd = _varState.end(); it != itEnd; ++it) {
		const VariableModifier *var = findVar(runtime, it->_key);
		if (var) {
			DynamicValue varValue;
			var->varGetValue(nullptr, varValue);
			assert(varValue.getType() == DynamicValueTypes::kBoolean);

			it->_value = varValue.getBool();
		}
	}
}

class ObsidianAutoSaveSceneTransitionHooks : public SceneTransitionHooks {
public:
	explicit ObsidianAutoSaveSceneTransitionHooks(const Common::SharedPtr<ObsidianAutoSaveVarsState> &vars, IAutoSaveProvider *autoSaveProvider);

	void onSceneTransitionEnded(Runtime *runtime, const Common::WeakPtr<Structural> &newScene) override;

private:
	Common::SharedPtr<ObsidianAutoSaveVarsState> _varsState;
	IAutoSaveProvider *_autoSaveProvider;

	Common::String _currentSceneName;
	Common::String _prevSceneName;

	Common::String _resetSceneName;
	Common::String _saveVarName;
};

ObsidianAutoSaveSceneTransitionHooks::ObsidianAutoSaveSceneTransitionHooks(const Common::SharedPtr<ObsidianAutoSaveVarsState> &vars, IAutoSaveProvider *autoSaveProvider)
	: _varsState(vars), _autoSaveProvider(autoSaveProvider) {

	_resetSceneName = Common::String("101_102_Credits");
	_saveVarName = Common::String("cgst");
}

void ObsidianAutoSaveSceneTransitionHooks::onSceneTransitionEnded(Runtime *runtime, const Common::WeakPtr<Structural> &newScene) {
	bool triggerAutoSave = false;

	if (newScene.expired())
		return;

	_prevSceneName = _currentSceneName;
	_currentSceneName = newScene.lock()->getName();

	for (const ObsidianAutoSaveTrigger &trigger : kObsidianAutoSaveTriggers) {
		Common::String triggerSceneName(trigger.sceneName);

		if (!caseInsensitiveEqual(triggerSceneName, _currentSceneName))
			continue;

		if (trigger.priorSceneName && !caseInsensitiveEqual(trigger.priorSceneName, _prevSceneName))
			continue;

		if (trigger.varTrueLatch) {
			Common::String varName(trigger.varTrueLatch);

			// Variable must must have been false since the last game load or reset
			if (_varsState->getVarState(varName))
				continue;

			bool passedLatchTest = false;

			const VariableModifier *var = _varsState->findVar(runtime, varName);
			if (var) {
				DynamicValue varValue;
				var->varGetValue(nullptr, varValue);
				assert(varValue.getType() == DynamicValueTypes::kBoolean);

				passedLatchTest = varValue.getBool();
			}

			if (!passedLatchTest)
				continue;
		}

		triggerAutoSave = true;
		break;
	}

	if (triggerAutoSave) {
		Common::SharedPtr<Modifier> saveVar;

		for (const Common::SharedPtr<Modifier> &child : runtime->getProject()->getModifiers()) {
			if (caseInsensitiveEqual(child->getName(), _saveVarName)) {
				saveVar = child;
				break;
			}
		}

		if (saveVar && saveVar->isModifier()) {
			Modifier *modifier = static_cast<Modifier *>(saveVar.get());
			Common::SharedPtr<ModifierSaveLoad> saveLoad = modifier->getSaveLoad();

			if (saveLoad) {
				CompoundVarSaver saver(saveVar.get());
				_autoSaveProvider->autoSave(&saver);

				_varsState->resyncAllVars(runtime);
			}
		}
	}

	if (caseInsensitiveEqual(_currentSceneName, _resetSceneName))
		_varsState->resyncAllVars(runtime);
}

class ObsidianAutoSaveSaveLoadHooks : public SaveLoadHooks {
public:
	explicit ObsidianAutoSaveSaveLoadHooks(const Common::SharedPtr<ObsidianAutoSaveVarsState> &vars);

	void onSave(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) override;
	void onLoad(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) override;

private:
	Common::SharedPtr<ObsidianAutoSaveVarsState> _varsState;
};


ObsidianAutoSaveSaveLoadHooks::ObsidianAutoSaveSaveLoadHooks(const Common::SharedPtr<ObsidianAutoSaveVarsState> &vars) : _varsState(vars) {
}

void ObsidianAutoSaveSaveLoadHooks::onSave(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) {
	// Reset all variable latches on save
	_varsState->resyncAllVars(runtime);
}

void ObsidianAutoSaveSaveLoadHooks::onLoad(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) {
	// Reset all variable latches on load
	_varsState->resyncAllVars(runtime);
}

void addObsidianAutoSaves(const MTropolisGameDescription &desc, Hacks &hacks, IAutoSaveProvider *autoSaveProvider) {
	Common::SharedPtr<ObsidianAutoSaveVarsState> varsState(new ObsidianAutoSaveVarsState());
	hacks.addSceneTransitionHooks(Common::SharedPtr<SceneTransitionHooks>(new ObsidianAutoSaveSceneTransitionHooks(varsState, autoSaveProvider)));
	hacks.addSaveLoadHooks(Common::SharedPtr<SaveLoadHooks>(new ObsidianAutoSaveSaveLoadHooks(varsState)));
}

} // End of namespace HackSuites

} // End of namespace MTropolis
