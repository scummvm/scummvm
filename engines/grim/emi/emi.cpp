/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/foreach.h"

#include "engines/grim/emi/emi.h"
#include "engines/grim/emi/emi_registry.h"
#include "engines/grim/emi/lua_v2.h"
#include "engines/grim/primitives.h"
#include "engines/grim/set.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/actor.h"


namespace Grim {

EMIEngine *g_emi = NULL;

EMIEngine::EMIEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language) :
		GrimEngine(syst, gameFlags, gameType, platform, language), _sortOrderInvalidated(false) {
	g_emi = this;
	g_emiregistry = new EmiRegistry();
}



EMIEngine::~EMIEngine() {
	g_emi = NULL;
	delete g_emiregistry;
	g_emiregistry = NULL;
}

LuaBase *EMIEngine::createLua() {
	return new Lua_V2();
}

const char *EMIEngine::getUpdateFilename() {
	if (getGamePlatform() == Common::kPlatformWindows && !(getGameFlags() & ADGF_DEMO)) {
		switch (getGameLanguage()) {
			case Common::FR_FRA:
				return "MonkeyUpdate_FRA.exe";
				break;
			case Common::DE_DEU:
				return "MonkeyUpdate_DEU.exe";
				break;
			case Common::IT_ITA:
				return "MonkeyUpdate_ITA.exe";
				break;
			case Common::PT_BRA:
				return "MonkeyUpdate_BRZ.exe";
				break;
			case Common::ES_ESP:
				return "MonkeyUpdate_ESP.exe";
				break;
			case Common::EN_ANY:
			case Common::EN_GRB:
			case Common::EN_USA:
			default:
				return "MonkeyUpdate.exe";
				break;
		}
	} else
		return 0;
}

void EMIEngine::pushText(Common::List<TextObject *> *objects) {
	_textstack.push_back(objects);
}

Common::List<TextObject *> *EMIEngine::popText() {
	Common::List<TextObject *> *object = _textstack.front();
	_textstack.pop_front();
	return object;
}

void EMIEngine::drawNormalMode() {

	// Draw Primitives
	foreach (PrimitiveObject *p, PrimitiveObject::getPool()) {
		p->draw();
	}

	_currSet->setupCamera();

	g_driver->set3DMode();

	if (_setupChanged) {
		cameraPostChangeHandle(_currSet->getSetup());
		_setupChanged = false;
	}

	// Draw actors
	buildActiveActorsList();
	sortActiveActorsList();

	Bitmap *background = _currSet->getCurrSetup()->_bkgndBm;
	background->_data->load();
	uint32 numLayers = background->_data->_numLayers;
	int32 currentLayer = numLayers - 1;
	foreach (Actor *a, _activeActors) {
		int sortorder = a->getSortOrder();
		if (sortorder < 0)
			break;

		while (sortorder <= currentLayer * 10 && currentLayer >= 0) {
			background->drawLayer(currentLayer--);
		}

		if (a->isVisible() && ! a->isInOverworld())
			a->draw();
	}
	while (currentLayer >= 0) {
		background->drawLayer(currentLayer--);
	}

	foreach (Actor *a, _activeActors) {
		if (a->isInOverworld())
			a->draw();
	}

	flagRefreshShadowMask(false);

}

void EMIEngine::invalidateActiveActorsList() {
	GrimEngine::invalidateActiveActorsList();
	invalidateSortOrder();
}

void EMIEngine::invalidateSortOrder() {
	_sortOrderInvalidated = true;
}

bool EMIEngine::compareActor(Actor *x, Actor *y) {
	if (x->getSortOrder() == y->getSortOrder()) {
		return x->getId() < y->getId();
	}
	return x->getSortOrder() > y->getSortOrder();
}

void EMIEngine::sortActiveActorsList() {
	if (!_sortOrderInvalidated) {
		return;
	}

	_sortOrderInvalidated = false;

	Common::sort(_activeActors.begin(), _activeActors.end(), compareActor);
}

} // end of namespace Grim
