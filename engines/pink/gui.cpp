/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macmenu.h"

#include "pink/pink.h"
#include "pink/director.h"
#include "pink/objects/actors/lead_actor.h"

namespace Pink {

enum {
	kNewGameAction = 0,
	kLoadSave,
	kSaveAction,
	kSaveAsAction,
	kSongsAction,
	kSoundSettingsAction,
	kLastSavesAction,
	kPauseAction,
	kExitAction
};

enum {
	kShowContent = 9,
	kShowGreece,
	kShowIndonesia,
	kShowIsrael,
	kShowKenya,
	kShowRussia
};

enum {
	kShowPDAAction = 15,

	kShowAustraliaArt = 16,
	kShowAustraliaCloth,
	kShowAustraliaFood,
	kShowAustraliaHistory,
	kShowAustraliaNature,
	kShowAustraliaPeople,
	kShowAustraliaPlaces,
	kShowAustraliaReligion,

	kShowBhutanArt = 24,
	kShowBhutanCloth,
	kShowBhutanFood,
	kShowBhutanHistory,
	kShowBhutanNature,
	kShowBhutanPeople,
	kShowBhutanPlaces,
	kShowBhutanReligion,

	kShowChinaArt = 32,
	kShowChinaCloth,
	kShowChinaFood,
	kShowChinaHistory,
	kShowChinaNature,
	kShowChinaPeople,
	kShowChinaPlaces,
	kShowChinaReligion,

	kShowEnglandArt = 40,
	kShowEnglandCloth,
	kShowEnglandFood,
	kShowEnglandHistory,
	kShowEnglandNature,
	kShowEnglandPeople,
	kShowEnglandPlaces,
	kShowEnglandReligion,

	kShowEgyptArt = 48,
	kShowEgyptCloth,
	kShowEgyptFood,
	kShowEgyptHistory,
	kShowEgyptNature,
	kShowEgyptPeople,
	kShowEgyptPlaces,
	kShowEgyptReligion,

	kShowIndiaArt = 56,
	kShowIndiaCloth,
	kShowIndiaFood,
	kShowIndiaHistory,
	kShowIndiaNature,
	kShowIndiaPeople,
	kShowIndiaPlaces,
	kShowIndiaReligion
};

enum {
	kShowGameWebPage = 64,
	kShowTechSupport,
	kShowWinnnerPage,
	kShowWanderlustInteractive,
	kShowMGM,
	kShowDiagnostics,
	kShowAbout
};

static void menuCommandsCallback(int action, Common::U32String &, void *data) {
	PinkEngine *engine = (PinkEngine *)data;
	bool isPeril = engine->isPeril();
	if (!isPeril && action > 14) {
		action += 49;
	} else if (isPeril && action >= 9) {
		action += 6;
	}
	engine->executeMenuCommand(action);
}

void PinkEngine::initMenu(Common::PEResources &exeResources) {
	_director->getWndManager().setEnginePauseCallback(this, &pauseEngine);

	_menu = Graphics::MacMenu::createMenuFromPEexe(exeResources, &_director->getWndManager());
	_menu->calcDimensions();
	_menu->setCommandsCallback(&menuCommandsCallback, this);
}

void PinkEngine::executeMenuCommand(uint id) {
	switch (id) {
	case kNewGameAction: {
		const Common::String moduleName = _modules[0]->getName();
		initModule(moduleName, "", nullptr);
		break;
	}
	case kLoadSave:
	case kSaveAction:
	case kSaveAsAction:
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

	case kShowAustraliaArt:
		_actor->loadPDA("AUSART");
		break;
	case kShowAustraliaCloth:
		_actor->loadPDA("AUSCLO");
		break;
	case kShowAustraliaFood:
		_actor->loadPDA("AUSFOO");
		break;
	case kShowAustraliaHistory:
		_actor->loadPDA("AUSHIS");
		break;
	case kShowAustraliaNature:
		_actor->loadPDA("AUSNAT");
		break;
	case kShowAustraliaPeople:
		_actor->loadPDA("AUSPEO");
		break;
	case kShowAustraliaPlaces:
		_actor->loadPDA("AUSPLA");
		break;
	case kShowAustraliaReligion:
		_actor->loadPDA("AUSREL");
		break;

	case kShowBhutanArt:
		_actor->loadPDA("BHUART");
		break;
	case kShowBhutanCloth:
		_actor->loadPDA("BHUCLO");
		break;
	case kShowBhutanFood:
		_actor->loadPDA("BHUFOO");
		break;
	case kShowBhutanHistory:
		_actor->loadPDA("BHUHIS");
		break;
	case kShowBhutanNature:
		_actor->loadPDA("BHUNAT");
		break;
	case kShowBhutanPeople:
		_actor->loadPDA("BHUPEO");
		break;
	case kShowBhutanPlaces:
		_actor->loadPDA("BHUPLA");
		break;
	case kShowBhutanReligion:
		_actor->loadPDA("BHUREL");
		break;

	case kShowChinaArt:
		_actor->loadPDA("CHIART");
		break;
	case kShowChinaCloth:
		_actor->loadPDA("CHICLO");
		break;
	case kShowChinaFood:
		_actor->loadPDA("CHIFOO");
		break;
	case kShowChinaHistory:
		_actor->loadPDA("CHIHIS");
		break;
	case kShowChinaNature:
		_actor->loadPDA("CHINAT");
		break;
	case kShowChinaPeople:
		_actor->loadPDA("CHIPEO");
		break;
	case kShowChinaPlaces:
		_actor->loadPDA("CHIPLA");
		break;
	case kShowChinaReligion:
		_actor->loadPDA("CHIREL");
		break;

	case kShowEnglandArt:
		_actor->loadPDA("BRIART");
		break;
	case kShowEnglandCloth:
		_actor->loadPDA("BRICLO");
		break;
	case kShowEnglandFood:
		_actor->loadPDA("BRIFOO");
		break;
	case kShowEnglandHistory:
		_actor->loadPDA("BRIHIS");
		break;
	case kShowEnglandNature:
		_actor->loadPDA("BRINAT");
		break;
	case kShowEnglandPeople:
		_actor->loadPDA("BRIPEO");
		break;
	case kShowEnglandPlaces:
		_actor->loadPDA("BRIPLA");
		break;
	case kShowEnglandReligion:
		_actor->loadPDA("BRIREL");
		break;

	case kShowEgyptArt:
		_actor->loadPDA("BRIART");
		break;
	case kShowEgyptCloth:
		_actor->loadPDA("BRICLO");
		break;
	case kShowEgyptFood:
		_actor->loadPDA("BRIFOO");
		break;
	case kShowEgyptHistory:
		_actor->loadPDA("BRIHIS");
		break;
	case kShowEgyptNature:
		_actor->loadPDA("BRINAT");
		break;
	case kShowEgyptPeople:
		_actor->loadPDA("BRIPEO");
		break;
	case kShowEgyptPlaces:
		_actor->loadPDA("BRIPLA");
		break;
	case kShowEgyptReligion:
		_actor->loadPDA("BRIREL");
		break;

	case kShowIndiaArt:
		_actor->loadPDA("INDART");
		break;
	case kShowIndiaCloth:
		_actor->loadPDA("INDCLO");
		break;
	case kShowIndiaFood:
		_actor->loadPDA("INDFOO");
		break;
	case kShowIndiaHistory:
		_actor->loadPDA("INDHIS");
		break;
	case kShowIndiaNature:
		_actor->loadPDA("INDNAT");
		break;
	case kShowIndiaPeople:
		_actor->loadPDA("INDPEO");
		break;
	case kShowIndiaPlaces:
		_actor->loadPDA("INDPLA");
		break;
	case kShowIndiaReligion:
		_actor->loadPDA("INDREL");
		break;

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

	default:
		break;
	}
}

} // End of namespace Pink
