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

#include "common/system.h"

#include "engines/advancedDetector.h"

#include "graphics/fonts/ttf.h"
#include "graphics/font.h"

#include "graphics/thumbnail.h"

#include "petka/petka.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/video.h"
#include "petka/objects/object_cursor.h"
#include "petka/interfaces/save_load.h"

namespace Petka {

const uint kFirstSaveLoadPageId = 4990;

InterfaceSaveLoad::InterfaceSaveLoad() {
	_page = 0;
	_loadMode = false;

	_saveRects[0] = Common::Rect(43, 84, 151, 166);
	_saveRects[1] = Common::Rect(43, 209, 151, 291);
	_saveRects[2] = Common::Rect(43, 335, 151, 417);
	_saveRects[3] = Common::Rect(358, 75, 466, 157);
	_saveRects[4] = Common::Rect(360, 200, 468, 282);
	_saveRects[5] = Common::Rect(359, 325, 467, 407);
	_nextPageRect = Common::Rect(596, 403, 624, 431);
	_prevPageRect = Common::Rect(10, 414, 38, 442);
}

void InterfaceSaveLoad::start(int id) {
	QSystem *sys = g_vm->getQSystem();
	QManager *resMgr = g_vm->resMgr();
	QObjectBG *bg = (QObjectBG *)sys->findObject("SAVELOAD");

	_loadMode = (id == kLoadMode);

	_objs.push_back(bg);
	bg->_resourceId = kFirstSaveLoadPageId + _page + (_loadMode ? 0 : 5);

	resMgr->removeResource(bg->_resourceId);
	auto bmp = resMgr->getSurface(bg->_resourceId);

	Graphics::ManagedSurface surf(bmp->w, bmp->h, bmp->format);
	surf.blitFrom(*bmp);

	Common::ScopedPtr<Graphics::Font> font(Graphics::loadTTFFontFromArchive("FreeSans.ttf", 20));

	MetaEngine &metaEngine = PetkaEngine::getMetaEngine();
	for (int i = 0, j = _page * 6; i < 6; ++i, ++j) {
		SaveStateDescriptor save = metaEngine.querySaveMetaInfos(g_vm->_desc->gameId, j);

		auto surface = save.getThumbnail();
		if (!surface)
			continue;

		Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> thumbnail(surface->scale(108, 82, true));
		thumbnail.reset(thumbnail->convertTo(g_system->getOverlayFormat()));

		surf.blitFrom(*thumbnail, Common::Point(_saveRects[i].left, _saveRects[i].top));

		Common::Rect textRect(240, 30);
		textRect.translate(_saveRects[i].left, _saveRects[i].bottom + 1);

		Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> text(new Graphics::Surface);
		text->create(textRect.width(), textRect.height(), g_system->getScreenFormat());
		font->drawString(text.get(), save.getSaveDate() + " " + save.getSaveTime(), 0, 0, textRect.width(), text->format.RGBToColor(0, 0x7F, 00));

		surf.transBlitFrom(*text, Common::Point(textRect.left, textRect.top));
	}

	bmp->copyFrom(surf.rawSurface());

	SubInterface::start(id);
}

void InterfaceSaveLoad::onLeftButtonDown(Common::Point p) {
	int index = findSaveLoadRectIndex(p);
	if (index == -1) {
		if (_prevPageRect.contains(p) && _page > 0) {
			_page--;
			stop();
			start(_loadMode ? kLoadMode : kSaveMode);
		} else if (_nextPageRect.contains(p) && _page < 2) {
			_page++;
			stop();
			start(_loadMode ? kLoadMode : kSaveMode);
		}
	} else {
		stop();
		_loadMode ? g_vm->loadGameState(_page * 6 + index) : g_vm->saveGameState(_page * 6 + index, "", false);
	}
}

void InterfaceSaveLoad::onRightButtonDown(Common::Point p) {
	stop();
}

void InterfaceSaveLoad::onMouseMove(Common::Point p) {
	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	cursor->_animate = findSaveLoadRectIndex(p) != -1 || _nextPageRect.contains(p) || _prevPageRect.contains(p);
	cursor->setPos(p, false);
}

int InterfaceSaveLoad::findSaveLoadRectIndex(Common::Point p) {
	for (uint i = 0; i < ARRAYSIZE(_saveRects); ++i) {
		if (_saveRects[i].contains(p)) {
			return i;
		}
	}
	return -1;
}

void InterfaceSaveLoad::saveScreen() {
	Common::ScopedPtr<Common::MemoryWriteStreamDynamic> thumbnail(new Common::MemoryWriteStreamDynamic(DisposeAfterUse::NO));
	Graphics::saveThumbnail(*thumbnail);
	g_vm->_thumbnail.reset(new Common::MemoryReadStream(thumbnail->getData(), thumbnail->size(), DisposeAfterUse::YES));
}

} // End of namespace Petka

