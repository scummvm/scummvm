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

#include "engines/stark/ui/dialogpanel.h"
#include "engines/stark/ui/clicktext.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/dialogplayer.h"

#include "engines/stark/visual/image.h"
#include "engines/stark/visual/text.h"

namespace Stark {

DialogPanel::DialogPanel(Gfx::Driver *gfx, Cursor *cursor) :
		Window(gfx, cursor),
		_subtitleVisual(nullptr),
		_hasOptions(false) {
	_position = Common::Rect(Gfx::Driver::kOriginalWidth, Gfx::Driver::kBottomBorderHeight);
	_position.translate(0, Gfx::Driver::kTopBorderHeight + Gfx::Driver::kGameViewportHeight);

	_visible = true;

	StaticProvider *staticProvider = StarkServices::instance().staticProvider;
	// TODO: Un-hardcode
	_activeBackGroundTexture = staticProvider->getCursorImage(20);
	_passiveBackGroundTexture = staticProvider->getCursorImage(21);
}

DialogPanel::~DialogPanel() {
	clearOptions();
	delete _subtitleVisual;
}

void DialogPanel::clearOptions() {
	for (uint i = 0; i < _options.size(); i++) {
		delete _options[i];
	}
	_options.clear();
	_hasOptions = false;
}

void DialogPanel::renderOptions() {
	for (uint i = 0; i < _options.size(); i++) {
		_options[i]->render();
	}
}

void DialogPanel::onRender() {
	if (_hasOptions) {
		_activeBackGroundTexture->render(Common::Point(0, 0), false);
		renderOptions();
	} else {
		_passiveBackGroundTexture->render(Common::Point(0, 0), false);
	}
	// TODO: Unhardcode
	if (_subtitleVisual) {
		_subtitleVisual->render(Common::Point(10, 10));
	}
}

void DialogPanel::update() {
}

void DialogPanel::notifySubtitle(const Common::String &subtitle) {
	clearOptions();
	delete _subtitleVisual;

	_subtitleVisual = new VisualText(_gfx);
	_subtitleVisual->setText(subtitle);
	_subtitleVisual->setColor(_aprilColor);
}

void DialogPanel::notifyDialogOptions(const Common::StringArray &options) {
	clearOptions();
	delete _subtitleVisual;
	_subtitleVisual = nullptr;

	int pos = 0;
	for (uint i = 0; i < options.size(); i++) {
		ClickText *text = new ClickText(options[i], Common::Point(0, pos));
		_options.push_back(text);
		pos += text->getHeight();
		// TODO: Add buttons?
		if (pos > Gfx::Driver::kBottomBorderHeight) {
			break;
		}
	}
	_hasOptions = true;
}

void DialogPanel::onMouseMove(const Common::Point &pos) {
	if (_hasOptions && _options.size() > 0) {
		for (uint i = 0; i < _options.size(); i++) {
			if (_options[i]->containsPoint(pos)) {
				_options[i]->handleMouseOver();
			} else {
				_options[i]->setPassive();
			}
		}
	}
}

void DialogPanel::onClick(const Common::Point &pos) {
	if (_hasOptions && _options.size() > 0) {
		for (uint i = 0; i < _options.size(); i++) {
			if (_options[i]->containsPoint(pos)) {
				DialogPlayer *dialogPlayer = StarkServices::instance().dialogPlayer;
				dialogPlayer->selectOption(i);
				return;
			}
		}
	}
}

} // End of namespace Stark
