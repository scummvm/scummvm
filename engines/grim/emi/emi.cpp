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
#include "engines/grim/emi/lua_v2.h"
#include "engines/grim/primitives.h"
#include "engines/grim/set.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/actor.h"


namespace Grim {

EMIEngine *g_emi = NULL;

EMIEngine::EMIEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language) :
		GrimEngine(syst, gameFlags, gameType, platform, language) {
	g_emi = this;
}


EMIEngine::~EMIEngine() {
	g_emi = NULL;
}

LuaBase *EMIEngine::createLua() {
	return new Lua_V2();
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

	Bitmap *background = _currSet->getCurrSetup()->_bkgndBm;
	uint32 numLayers = background->_data->_numLayers;
	background->_data->load();
	int32 currentLayer = numLayers - 1;
	foreach (Actor *a, _activeActors) {
		if (a->getSortOrder() < 0)
			break;

		while (a->getSortOrder() <= currentLayer * 10 && currentLayer >= 0) {
			background->drawLayer(currentLayer--);
		}

		if (a->isVisible())
			a->draw();
	}
	while (currentLayer >= 0) {
		background->drawLayer(currentLayer--);
	}


	flagRefreshShadowMask(false);

}

} // end of namespace Grim
