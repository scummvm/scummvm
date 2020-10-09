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

#include "engines/stark/ui/menu/locationscreen.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"

#include "engines/stark/resources/item.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/sound.h"

#include "engines/stark/ui/cursor.h"

#include "engines/stark/visual/text.h"

#include "audio/mixer.h"
#include "common/system.h"

namespace Stark {

StaticLocationScreen::StaticLocationScreen(Gfx::Driver *gfx, Cursor *cursor,
                                           const char *locationName, Screen::Name screenName) :
		SingleWindowScreen(screenName, gfx, cursor),
		_locationName(locationName),
		_location(nullptr),
		_hoveredWidgetIndex(-1) {
	_position = Common::Rect(Gfx::Driver::kOriginalWidth, Gfx::Driver::kOriginalHeight);
	_visible = true;
}

StaticLocationScreen::~StaticLocationScreen() {
	freeWidgets();
}

void StaticLocationScreen::open() {
	_location = StarkStaticProvider->loadLocation(_locationName);
}

void StaticLocationScreen::close() {
	freeWidgets();

	StarkStaticProvider->unloadLocation(_location);
	_location = nullptr;
}

void StaticLocationScreen::freeWidgets() {
	for (uint i = 0; i < _widgets.size(); i++) {
		delete _widgets[i];
	}

	_widgets.clear();
	_hoveredWidgetIndex = -1;
}

void StaticLocationScreen::onGameLoop() {
	for (uint i = 0; i < _widgets.size(); i++) {
		StaticLocationWidget *widget = _widgets[i];
		if (widget->isVisible()) {
			widget->onGameLoop();
		}
	}
}

void StaticLocationScreen::onMouseMove(const Common::Point &pos) {
	int newHoveredWidget = -1;

	for (uint i = 0; i < _widgets.size(); i++) {
		StaticLocationWidget *widget = _widgets[i];
		widget->onMouseMove(pos);

		if (widget->isVisible() && widget->isMouseInside(pos)) {
			newHoveredWidget = i;
		}
	}

	// The first widget is always the background. It is ignored below.

	if (newHoveredWidget != _hoveredWidgetIndex) {
		if (_hoveredWidgetIndex > 0 && uint(_hoveredWidgetIndex) < _widgets.size()) {
			_widgets[_hoveredWidgetIndex]->onMouseLeave();
		}

		if (newHoveredWidget > 0) {
			_widgets[newHoveredWidget]->onMouseEnter();
		}

		_hoveredWidgetIndex = newHoveredWidget;
	}

	_cursor->setCursorType(_hoveredWidgetIndex > 0 ? Cursor::kActive : Cursor::kDefault);
}

void StaticLocationScreen::onClick(const Common::Point &pos) {
	for (uint i = 1; i < _widgets.size(); i++) {
		StaticLocationWidget *widget = _widgets[i];
		if (widget->isVisible() && widget->isMouseInside(pos)) {
			widget->onClick();
			break;
		}
	}
}

void StaticLocationScreen::onRender() {
	for (uint i = 0; i < _widgets.size(); i++) {
		if (_widgets[i]->isVisible()) {
			_widgets[i]->render();
		}
	}
}

void StaticLocationScreen::onScreenChanged() {
	for (uint i = 0; i < _widgets.size(); i++) {
		_widgets[i]->onScreenChanged();
	}
}

void StaticLocationScreen::waitForSoundsToComplete() {
	while (g_system->getMixer()->hasActiveChannelOfType(Audio::Mixer::kSFXSoundType)) {
		StarkGfx->clearScreen();
		g_system->delayMillis(10);
		StarkGfx->flipBuffer();
	}
}

StaticLocationWidget::StaticLocationWidget(const char *renderEntryName, WidgetOnClickCallback *onClickCallback,
                                           WidgetOnMouseMoveCallback *onMouseMoveCallback):
		_onClick(onClickCallback),
		_onMouseMove(onMouseMoveCallback),
		_renderEntry(nullptr),
		_item(nullptr),
		_soundMouseEnter(nullptr),
		_soundMouseClick(nullptr),
		_visible(true) {
	if (renderEntryName) {
		Resources::Location *location = StarkStaticProvider->getLocation();
		_renderEntry = location->getRenderEntryByName(renderEntryName);

		if (_renderEntry == nullptr) {
			debug("Widget disabled: unable to find render entry with name '%s' in location '%s'",
					renderEntryName, location->getName().c_str());
			setVisible(false);
		} else {
			_item = _renderEntry->getOwner();
		}
	}
}

void StaticLocationWidget::render() {
	if (_renderEntry) {
		_renderEntry->render();
	}
}

bool StaticLocationWidget::isVisible() const {
	return _visible;
}

void StaticLocationWidget::setVisible(bool visible) {
	_visible = visible;
}

bool StaticLocationWidget::isMouseInside(const Common::Point &mousePos) const {
	if (!_renderEntry) return false;

	Common::Point relativePosition;
	return _renderEntry->containsPoint(mousePos, relativePosition, Common::Rect());
}

void StaticLocationWidget::onClick() {
	onMouseLeave();

	if (_soundMouseClick) {
		_soundMouseClick->play();
		_soundMouseClick->setStopOnDestroy(false);
	}

	if (_onClick) {
		(*_onClick)();
	}
}

void StaticLocationWidget::onGameLoop() {
	if (_item) {
		_item->onGameLoop();
	}
}

void StaticLocationWidget::onMouseEnter() {
	if (_soundMouseEnter && !_soundMouseEnter->isPlaying()) {
		_soundMouseEnter->play();
	}
}

void StaticLocationWidget::onMouseLeave() {
	if (_soundMouseEnter) {
		_soundMouseEnter->stop();
	}
}

void StaticLocationWidget::setupSounds(int16 enterSound, int16 clickSound) {
	if (enterSound != -1) {
		_soundMouseEnter = StarkStaticProvider->getLocationSound(enterSound);
	}

	if (clickSound != -1) {
		_soundMouseClick = StarkStaticProvider->getLocationSound(clickSound);
	}
}

void StaticLocationWidget::setTextColor(const Color &textColor) {
	if (!_renderEntry) return;

	VisualText *text = _renderEntry->getText();
	assert(text);

	text->setColor(textColor);
}

void StaticLocationWidget::onScreenChanged() {
	if (!_renderEntry) return;

	VisualText *text = _renderEntry->getText();
	if (text) {
		text->resetTexture();
	}
}

void StaticLocationWidget::onMouseMove(const Common::Point &mousePos) {
	if (_onMouseMove) {
		(*_onMouseMove)(*this, mousePos);
	}
}

Common::Point StaticLocationWidget::getPosition() const {
	if (_renderEntry) {
		return _renderEntry->getPosition();
	}

	return Common::Point(0, 0);
}

StaticLocationWidget::~StaticLocationWidget() {
	delete _onClick;
	delete _onMouseMove;
}

} // End of namespace Stark
