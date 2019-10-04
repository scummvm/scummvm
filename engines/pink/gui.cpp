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

static void menuCommandsCallback(int action, Common::U32String &, void *data) {
	PinkEngine *engine = (PinkEngine *)data;

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
		warning("Unprocessed command id %d", id);
		break;
	}
}

} // End of namespace Pink
