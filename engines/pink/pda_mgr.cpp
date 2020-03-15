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

#include "pink/pda_mgr.h"
#include "pink/pink.h"
#include "pink/director.h"
#include "pink/objects/actors/pda_button_actor.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/pda_page.h"
#include "pink/objects/actions/action_play_with_sfx.h"

namespace Pink {

static const char * const g_countries[] = {"BRI", "EGY", "BHU", "AUS", "IND", "CHI"};
static const char * const g_domains[] = {"NAT", "CLO", "HIS", "REL", "PLA", "ART", "FOO", "PEO"};

PDAMgr::PDAMgr(PinkEngine *game)
	: _game(game), _lead(nullptr), _page(nullptr), _globalPage(nullptr),
	_cursorMgr(game, nullptr), _countryIndex(0), _domainIndex(0),
	_iteration(0), _handFrame(0), _leftHandAction(kLeft1) {}

PDAMgr::~PDAMgr() {
	delete _globalPage;
	delete _page;
}

void PDAMgr::saveState(Archive &archive) {
	if (_page)
		archive.writeString(_page->getName());
	else
		archive.writeString("");
}

void PDAMgr::execute(const Command &command) {
	switch (command.type) {
	case Command::kGoToPage:
		goToPage(command.arg);
		break;
	case Command::kGoToPreviousPage:
		assert(_previousPages.size() >= 2);
		_previousPages.pop();
		goToPage(_previousPages.pop());
		break;
	case Command::kGoToDomain:
		goToPage(Common::String::format("%.6s", _page->getName().c_str()));
		break;
	case Command::kGoToHelp:
		warning("Command GoToHelp is not supported and won't be");
		break;
	case Command::kNavigateToDomain:
		goToPage(Common::String(g_countries[_countryIndex]) += g_domains[_domainIndex]);
		break;
	case Command::kIncrementCountry:
		_countryIndex = (_countryIndex + 1) % 6;
		updateWheels(1);
		updateLocator();
		break;
	case Command::kDecrementCountry:
		_countryIndex = (_countryIndex + 5) % 6;
		updateWheels(1);
		updateLocator();
		break;
	case Command::kIncrementDomain:
		_domainIndex = (_domainIndex + 1) % 8;
		updateWheels(1);
		break;
	case Command::kDecrementDomain:
		_domainIndex = (_domainIndex + 7) % 8;
		updateWheels(1);
		break;
	case Command::kClose:
		close();
		break;
	default:
		break;
	}
}

void PDAMgr::goToPage(const Common::String pageName) {
	if (_page && !_page->getName().compareToIgnoreCase(pageName))
		return;

	loadGlobal();

	delete _page;
	_page = new PDAPage(pageName, getGame());

	_previousPages.push(_page->getName());

	if (_game->isPeril())
		initPerilButtons();

	_cursorMgr.setPage(_page);
	onMouseMove(_game->getEventManager()->getMousePos());
}

void PDAMgr::onLeftButtonClick(Common::Point point) {
	Actor* rightHand = _globalPage->findActor(kRightHand);
	if (rightHand)
		static_cast<ActionStill*>(rightHand->getAction())->setFrame(1);
	Actor *actor = _game->getDirector()->getActorByPoint(point);
	if (actor)
		actor->onLeftClickMessage();
}

void PDAMgr::onLeftButtonUp() {
	Actor* rightHand = _globalPage->findActor(kRightHand);
	if (rightHand)
		static_cast<ActionStill*>(rightHand->getAction())->setFrame(0);
}

void PDAMgr::onMouseMove(Common::Point point) {
	Actor *actor = _game->getDirector()->getActorByPoint(point);
	if (actor && dynamic_cast<PDAButtonActor *>(actor))
		actor->onMouseOver(point, &_cursorMgr);
	else
		_cursorMgr.setCursor(kPDADefaultCursor, point, Common::String());

	if (!_game->isPeril())
		return;

	float k = (float)point.x / (480 - point.y);
	Actor *leftHand = _globalPage->findActor(kLeftHand);
	if (k > 0.5) {
		if (k > 1) {
			if (k > 1.5 && _leftHandAction != kLeft4) {
				leftHand->setAction(kLeft4Name);
				static_cast<ActionStill*>(leftHand->getAction())->setFrame(_handFrame + 1);
				_leftHandAction = kLeft4;
			} else if (_leftHandAction != kLeft3) {
				leftHand->setAction(kLeft3Name);
				static_cast<ActionStill*>(leftHand->getAction())->setFrame(_handFrame + 1);
				_leftHandAction = kLeft3;
			}
		} else if (_leftHandAction != kLeft2) {
			leftHand->setAction(kLeft2Name);
			static_cast<ActionStill*>(leftHand->getAction())->setFrame(_handFrame + 1);
			_leftHandAction = kLeft2;
		}
	} else if (_leftHandAction != kLeft1) {
		leftHand->setAction(kLeft1Name);
		static_cast<ActionStill*>(leftHand->getAction())->setFrame(_handFrame + 1);
		_leftHandAction = kLeft1;
	}

	if (_iteration == 0) {
		_handFrame = (_handFrame + 1) % 4;
		static_cast<ActionStill*>(leftHand->getAction())->nextFrameLooped();
	}
	_iteration = (_iteration + 1) % 4;
}

void PDAMgr::close() {
	if (!_globalPage)
		return;

	delete _globalPage;
	delete _page;
	_globalPage = nullptr;
	_page = nullptr;

	_lead->onPDAClose();
}

void PDAMgr::loadGlobal() {
	if (_globalPage)
		return;

	delete _globalPage;
	_globalPage = new PDAPage("GLOBAL", getGame());
}

void PDAMgr::initPerilButtons() {
	Actor *prevPageButton = _globalPage->findActor(kPreviousPageButton);
	if (_previousPages.size() < 2)
		prevPageButton->setAction(kInactiveAction);
	else
		prevPageButton->setAction(kIdleAction);

	Actor *navigatorButton = _globalPage->findActor(kNavigatorButton);
	Actor *domainButton = _globalPage->findActor(kDomainButton);
	if (isNavigate(_page->getName())) {
		navigatorButton->setAction(kInactiveAction);
		domainButton->setAction(kInactiveAction);
		updateWheels();
	} else {
		calculateIndexes();
		navigatorButton->setAction(kIdleAction);
		if (isDomain(_page->getName()))
			domainButton->setAction(kInactiveAction);
		else
			domainButton->setAction(kIdleAction);
	}
	updateLocator();
}

void PDAMgr::updateWheels(bool playSfx) {
	Actor *wheel = _page->findActor(kCountryWheel);
	if (playSfx && wheel->getAction()->getName() != g_countries[_countryIndex]) {
		wheel->setAction(Common::String(g_countries[_countryIndex]) + kSfx);
		static_cast<ActionPlayWithSfx*>(wheel->getAction())->update(); // hack
	}
	wheel->setAction(g_countries[_countryIndex]);

	wheel = _page->findActor(kDomainWheel);
	if (playSfx && wheel->getAction()->getName() != g_domains[_domainIndex]) {
		wheel->setAction(Common::String(g_domains[_domainIndex]) + kSfx);
		static_cast<ActionPlayWithSfx*>(wheel->getAction())->update(); // hack
	}
	wheel->setAction(g_domains[_domainIndex]);
}

bool PDAMgr::isNavigate(const Common::String &name) {
	return !name.compareToIgnoreCase(kNavigatePage);
}

bool PDAMgr::isDomain(const Common::String &name) {
	return name.size() == 6;
}

void PDAMgr::updateLocator() {
	Actor *locator = _globalPage->findActor(kLocator);
	if (locator)
		locator->setAction(g_countries[_countryIndex]);
}

void PDAMgr::calculateIndexes() {
	Common::String country = Common::String::format("%.3s", _page->getName().c_str());
	for (uint i = 0; i < 6; ++i) {
		if (country == g_countries[i]) {
			_countryIndex = i;
			break;
		}
	}

	Common::String domain = _page->getName();
	domain.erase(0, 3);
	if (domain.size() >= 4)
		domain.erase(3);
	for (uint i = 0; i < 8; ++i) {
		if (domain == g_domains[i]) {
			_domainIndex = i;
			break;
		}
	}
}

} // End of namespace Pink
