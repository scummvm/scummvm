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

#include "tetraedge/tetraedge.h"

#include "tetraedge/game/how_to.h"
#include "tetraedge/game/application.h"

namespace Tetraedge {

HowTo::HowTo() : _entered(false) {
}

void HowTo::leave()	{
	Application *app = g_engine->getApplication();
	app->captureFade();
	unload();
	//app->helpOptionMenu().enter();
	app->fade();
	_entered = false;
	error("TODO: Finish HowTo::leave - need app->helpOptionMenu");
}

void HowTo::enter()	{
	if (_entered)
		return;

	_entered = true;
	error("TODO: Implement HowTo::enter");
}

bool HowTo::onDefaultPadButtonUp(uint32 flags) {
	error("TODO: Implement HowTo::onDefaultPadButtonUp");
}

bool HowTo::updateDisplay(uint oldval, uint newval) {
	error("TODO: Implement HowTo::updateDisplay");
}

} // end namespace Tetraedge
