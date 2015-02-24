/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/ui/dialoginterface.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/dialogplayer.h"

#include "engines/stark/visual/image.h"

namespace Stark {

DialogInterface::DialogInterface() : _texture(nullptr), _hasOptions(false) {
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;
	// TODO: Un-hardcode
	_activeBackGroundTexture = staticProvider->getCursorImage(20);
	_passiveBackGroundTexture = staticProvider->getCursorImage(21);
}

DialogInterface::~DialogInterface() {
	delete _texture;
}

void DialogInterface::render() {
	Gfx::Driver *gfx = StarkServices::instance().gfx;
	gfx->setScreenViewport(false);
	if (_hasOptions) {
		_activeBackGroundTexture->render(Common::Point(0, 401));
	} else {
		_passiveBackGroundTexture->render(Common::Point(0, 401));
	}
	// TODO: Unhardcode
	if (_texture) {
		gfx->drawSurface(_texture, Common::Point(10, 400));
	}
}

void DialogInterface::update() {
}

void DialogInterface::notifySubtitle(const Common::String &subtitle) {
	_hasOptions = false;
	delete _texture;
	Gfx::Driver *gfx = StarkServices::instance().gfx;
	_texture = gfx->createTextureFromString(subtitle, 0xFFFF0000);
}

void DialogInterface::notifyDialogOptions(const Common::StringArray &options) {
	warning("Was notified");
	Common::String str;
	for (int i = 0; i < options.size(); i++) {
		str += options[i] + '\n';
	}
	delete _texture;
	Gfx::Driver *gfx = StarkServices::instance().gfx;
	_texture = gfx->createTextureFromString(str, 0xFFFF0000);
	_hasOptions = true;
}

} // End of namespace Stark
