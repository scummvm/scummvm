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

#include "common/str.h"

#include "scumm/scumm.h"
#include "scumm/macgui/macgui.h"
#include "scumm/macgui/macgui_impl.h"
#include "scumm/macgui/macgui_indy3.h"
#include "scumm/macgui/macgui_loom.h"
#include "scumm/macgui/macgui_v5.h"
#include "scumm/macgui/macgui_v6.h"

namespace Scumm {

MacGui::MacGui(ScummEngine *vm, const Common::Path &resourceFile) {
	switch (vm->_game.id) {
	case GID_INDY3:
		_impl = new MacIndy3Gui(vm, resourceFile);
		break;

	case GID_LOOM:
		_impl = new MacLoomGui(vm, resourceFile);
		break;

	case GID_MONKEY:
	case GID_MONKEY2:
	case GID_INDY4:
		_impl = new MacV5Gui(vm, resourceFile);
		break;

	case GID_TENTACLE:
	case GID_SAMNMAX:
	case GID_DIG:
	case GID_FT:
		_impl = new MacV6Gui(vm, resourceFile);
		break;

	default:
		error("MacGui: Invalid game id %d", vm->_game.id);
		break;
	}
}

MacGui::~MacGui() {
	delete _impl;
}

int MacGui::getNumColors() const {
	return _impl->getNumColors();
}

bool MacGui::initialize() {
	return _impl->initialize();
}

void MacGui::reset() {
	_impl->reset();
}

void MacGui::update(int delta) {
	_impl->update(delta);
}

void MacGui::updateWindowManager() {
	_impl->updateWindowManager();
}

void MacGui::resetAfterLoad() {
	_impl->resetAfterLoad();
}

bool MacGui::handleEvent(Common::Event event) {
	return _impl->handleEvent(event);
}

void MacGui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	_impl->setupCursor(width, height, hotspotX, hotspotY, animate);
}

void MacGui::setPaletteDirty() {
	_impl->setPaletteDirty();
}

const Graphics::Font *MacGui::getFontByScummId(int32 id) {
	return _impl->getFontByScummId(id);
}

void MacGui::drawBanner(char *message) {
	_impl->drawBanner(message);
}

void MacGui::undrawBanner() {
	_impl->undrawBanner();
}

Graphics::Surface *MacGui::textArea() const {
	return _impl->textArea();
}

bool MacGui::runQuitDialog() {
	return _impl->runQuitDialog();
}

bool MacGui::runRestartDialog() {
	return _impl->runRestartDialog();
}

void MacGui::runDraftsInventory() {
	((MacLoomGui *)_impl)->runDraftsInventory();
}

void MacGui::clearTextArea() {
	_impl->clearTextArea();
}

void MacGui::initTextAreaForActor(Actor *a, byte color) {
	_impl->initTextAreaForActor(a, color);
}

void MacGui::printCharToTextArea(int chr, int x, int y, int color) {
	_impl->printCharToTextArea(chr, x, y, color);
}

bool MacGui::isVerbGuiActive() const {
	return _impl->isVerbGuiActive();
}

} // End of namespace Scumm
