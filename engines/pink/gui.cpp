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

#include "common/config-manager.h"
#include "common/translation.h"

#include "engines/metaengine.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macmenu.h"

#include "gui/message.h"

#include "pink/pink.h"
#include "pink/screen.h"
#include "pink/objects/actors/lead_actor.h"

namespace Pink {

enum {
	kNewGameAction			= 57600,
	kLoadSave				= 57601,
	kSaveAction				= 57603,
	kSaveAsAction			= 57604,
	kSongsAction			= 32865,
	kSoundSettingsAction	= 32772,
	kLastSavesAction		= 57616,
	kPauseAction			= 32854,
	kExitAction				= 57665
};

enum {
	kShowContent	= 32771,
	kShowGreece		= 32866,
	kShowIndonesia  = 32867,
	kShowIsrael		= 32868,
	kShowKenya		= 32869,
	kShowRussia		= 32870
};

enum {
	kShowPDAAction			= 32793,

	kShowAustraliaArt		= 32796,
	kShowAustraliaCloth		= 32797,
	kShowAustraliaFood		= 32798,
	kShowAustraliaHistory	= 32799,
	kShowAustraliaNature	= 32800,
	kShowAustraliaPeople	= 32801,
	kShowAustraliaPlaces	= 32802,
	kShowAustraliaReligion	= 32803,

	kShowBhutanArt			= 32804,
	kShowBhutanCloth		= 32805,
	kShowBhutanFood			= 32806,
	kShowBhutanHistory		= 32807,
	kShowBhutanNature		= 32808,
	kShowBhutanPeople		= 32809,
	kShowBhutanPlaces		= 32810,
	kShowBhutanReligion		= 32811,

	kShowChinaArt 			= 32812,
	kShowChinaCloth			= 32813,
	kShowChinaFood			= 32814,
	kShowChinaHistory		= 32815,
	kShowChinaNature		= 32816,
	kShowChinaPeople		= 32817,
	kShowChinaPlaces		= 32818,
	kShowChinaReligion		= 32819,

	kShowEnglandArt			= 32820,
	kShowEnglandCloth		= 32821,
	kShowEnglandFood		= 32822,
	kShowEnglandHistory		= 32823,
	kShowEnglandNature		= 32824,
	kShowEnglandPeople		= 32825,
	kShowEnglandPlaces		= 32826,
	kShowEnglandReligion	= 32827,

	kShowEgyptArt			= 32828,
	kShowEgyptCloth			= 32829,
	kShowEgyptFood			= 32830,
	kShowEgyptHistory		= 32831,
	kShowEgyptNature		= 32832,
	kShowEgyptPeople		= 32833,
	kShowEgyptPlaces		= 32834,
	kShowEgyptReligion		= 32835,

	kShowIndiaArt			= 32836,
	kShowIndiaCloth			= 32837,
	kShowIndiaFood			= 32838,
	kShowIndiaHistory		= 32839,
	kShowIndiaNature		= 32840,
	kShowIndiaPeople		= 32841,
	kShowIndiaPlaces		= 32842,
	kShowIndiaReligion		= 32843
};

enum {
	kShowGameWebPage			= 32844,
	kShowTechSupport			= 32778,
	kShowOnlineHints			= 32777,
	kShowWanderLustWebPage		= 32775,
	kShowHelp					= 32846,
	kShowHints					= 32847,

	kShowWinnnerPage			= 32779,
	kShowWanderlustInteractive	= 32849,
	kShowMGM					= 32848,
	kShowDiagnostics			= 32850,
	kShowAbout					= 57664
};

enum {
	kRecentSaveId		= 9,
	kRecentSavesOffset	= 400000
};

static const int kMaxSaves = 10;

static void menuCommandsCallback(int action, Common::U32String &, void *data) {
	PinkEngine *engine = (PinkEngine *)data;

	engine->executeMenuCommand(action);
}

struct SaveStateDescriptorTimeComparator {
	bool operator()(const SaveStateDescriptor &x, const SaveStateDescriptor &y) const {
		return x.getSaveDate() == y.getSaveDate() ? x.getSaveTime() > y.getSaveTime() : x.getSaveDate() > y.getSaveDate();
	}
};

SaveStateList PinkEngine::listSaves() const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern = getMetaEngine()->getSavegameFile(kSavegameFilePattern, _targetName.c_str());
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);
		Common::ScopedPtr<Common::InSaveFile> in(saveFileMan->openForLoading(*file));
		if (in) {
			SaveStateDescriptor desc;
			desc.setSaveSlot(slotNum);
			if (Pink::readSaveHeader(*in.get(), desc))
				saveList.push_back(desc);
		}
	}

	// Sort saves based on save time.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorTimeComparator());
	return saveList;
}

void PinkEngine::initMenu() {
	_screen->getWndManager().setEngine(this);

	_menu = Graphics::MacMenu::createMenuFromPEexe(_exeResources, &_screen->getWndManager());
	if (getLanguage() == Common::HE_ISR) {
		_menu->setAlignment(Graphics::kTextAlignRight);
	}

	Graphics::MacMenuSubMenu *subMenu = _menu->getSubmenu(nullptr, 0);
	if (subMenu) {

		if (isPerilDemo()) {
			// From the first submenu ("Game"), disable the (zero indexed) item 5 ("Songs").
			// Use setEnabled() rather than removeMenuItem() since the latter removes the item
			// out of the list changing the index for the items that follow.
			// The effect is that "Songs" will be greyed out for the demo, since it's not available for it.
			// The original demo does not have the "Songs" item in the menu at all.
			_menu->setEnabled( _menu->getSubMenuItem(_menu->getMenuItem(0), 5), false);
		}

		SaveStateList saves = listSaves();
		if (!saves.empty()) {
			_menu->removeMenuItem(subMenu, kRecentSaveId);
			int maxSaves = saves.size() > kMaxSaves ? kMaxSaves : saves.size();
			for (int i = 0; i < maxSaves; ++i) {
				_menu->insertMenuItem(subMenu, Common::U32String::format("%i. %S", i + 1, saves[i].getDescription().c_str()),
										kRecentSaveId + i, saves[i].getSaveSlot() + kRecentSavesOffset);
			}
		}
	}

	_menu->calcDimensions();
	_menu->setCommandsCallback(&menuCommandsCallback, this);
}

void PinkEngine::executeMenuCommand(uint id) {
	if (executePageChangeCommand(id))
		return;

	if (id >= kRecentSavesOffset) {
		int slotNum = id - kRecentSavesOffset;
		Common::Error loadError = loadGameState(slotNum);
		if (loadError.getCode() != Common::kNoError) {
			GUI::MessageDialog errorDialog(loadError.getDesc());
			errorDialog.runModal();
		}
		return;
	}

	switch (id) {
	case kNewGameAction: {
		initModule(_modules[0]->getName(), "", nullptr);
		break;
	}
	case kLoadSave:
		loadGameDialog();
		break;

	case kSaveAction:
	case kSaveAsAction:
		saveGameDialog();
		break;

	case kSoundSettingsAction:
	case kLastSavesAction:
	case kPauseAction:
	case kExitAction:
		openMainMenuDialog();
		break;

	case kSongsAction:
		initModule("Muzik", "", nullptr);
		break;

	case kShowPDAAction:
		_actor->loadPDA(kNavigatePage);
		break;

	// Cases kShowAustraliaArt - kShowIndiaReligion are handled in executePageChangeCommand()

	case kShowContent:
		_actor->loadPDA("TOC");
		break;
	case kShowGreece:
		_actor->loadPDA("GREOVER");
		break;
	case kShowIndonesia:
		_actor->loadPDA("INDOVER");
		break;
	case kShowIsrael:
		_actor->loadPDA("ISROVER");
		break;
	case kShowKenya:
		_actor->loadPDA("KENOVER");
		break;
	case kShowRussia:
		_actor->loadPDA("SIBOVER");
		break;

	case kShowGameWebPage:
		openLocalWebPage("PINK.HTM");
		break;
	case kShowTechSupport:
		openLocalWebPage("SUPPORT.HTM");
		break;
	case kShowWinnnerPage:
		openLocalWebPage("WINNER.HTM");
		break;
	case kShowWanderLustWebPage:
		openLocalWebPage("LUST.HTM");
		break;
	case kShowOnlineHints:
		openLocalWebPage("HINTS.HTM");
		break;

	default:
		{
			GUI::MessageDialog dialog(_("This menu item is not yet implemented"));
			dialog.runModal();
			warning("Unprocessed command id %d", id);
		}
		break;
	}
}

// Australia, Bhutan, China, Britain, Egypt, India
static const char *pageChangePrefixes[] = { "AUS", "BHU", "CHI", "BRI", "EGY", "IND" };
// Art, Cloth, Food, History, Nature, People, Places, Religion
static const char *pageChangeSuffixes[] = { "ART", "CLO", "FOO", "HIS", "NAT", "PEO", "PLA", "REL" };

bool PinkEngine::executePageChangeCommand(uint id) {
	if (id >= kShowAustraliaArt && id <= kShowIndiaReligion) {
		Common::String prefix = pageChangePrefixes[(id - kShowAustraliaArt) / 8];
		Common::String suffix = pageChangeSuffixes[(id - kShowAustraliaArt) % 8];
		_actor->loadPDA(prefix + suffix);
		return true;
	}

	return false;
}

void PinkEngine::openLocalWebPage(const Common::String &pageName) const {
	Common::FSNode gameFolder = Common::FSNode(ConfMan.getPath("path"));
	Common::FSNode filePath = gameFolder.getChild("INSTALL").getChild(pageName);
	if (filePath.exists()) {
		Common::String fullUrl = Common::String::format("file:///%s", filePath.getPath().toString('/').c_str());
		_system->openUrl(fullUrl);
	}
}

} // End of namespace Pink
