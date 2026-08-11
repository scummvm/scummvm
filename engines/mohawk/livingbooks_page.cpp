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

#include "mohawk/livingbooks.h"
#include "mohawk/resource.h"

#include "common/textconsole.h"

namespace Mohawk {

LBPage::LBPage(MohawkEngine_LivingBooks *vm) : _vm(vm) {
	_code = nullptr;
	_mhk = nullptr;

	_baseId = 0;
	_cascade = false;
}

void LBPage::open(Archive *mhk, uint16 baseId) {
	_mhk = mhk;
	_baseId = baseId;

	_vm->addArchive(_mhk);
	if (!_vm->hasResource(ID_BCOD, baseId)) {
		// assume that BCOD is mandatory for v4/v5
		if (_vm->getGameType() == GType_LIVINGBOOKSV4 || _vm->getGameType() == GType_LIVINGBOOKSV5)
			error("missing BCOD resource (id %d)", baseId);
		_code = new LBCode(_vm, 0);
	} else {
		_code = new LBCode(_vm, baseId);
	}

	loadBITL(baseId);
	for (uint i = 0; i < _items.size(); i++)
		_vm->addItem(_items[i]);

	for (uint32 i = 0; i < _items.size(); i++)
		_items[i]->init();

	for (uint32 i = 0; i < _items.size(); i++)
		_items[i]->startPhase(kLBPhaseLoad);
}

void LBPage::addClonedItem(LBItem *item) {
	_vm->addItem(item);
	_items.push_back(item);
}

void LBPage::itemDestroyed(LBItem *item) {
	for (uint i = 0; i < _items.size(); i++)
		if (item == _items[i]) {
			_items.remove_at(i);
			return;
		}
	error("itemDestroyed didn't find item");
}

LBPage::~LBPage() {
	delete _code;
	_vm->removeItems(_items);
	for (uint i = 0; i < _items.size(); i++)
		delete _items[i];
	_vm->removeArchive(_mhk);
	delete _mhk;
}

// Only 1 VSRN resource per page
uint16 LBPage::getResourceVersion() {
	Common::SeekableReadStream *versionStream = _vm->getResource(ID_VRSN, _baseId);

	// FIXME: some V2 games have very strange version entries
	if (versionStream->size() != 2)
		debug(1, "Version Record size mismatch");

	uint16 version = versionStream->readUint16BE();

	delete versionStream;
	return version;
}

void LBPage::loadBITL(uint16 resourceId) {
	Common::SeekableReadStreamEndian *bitlStream = _vm->wrapStreamEndian(ID_BITL, resourceId);

	while (true) {
		Common::Rect rect = _vm->readRect(bitlStream);
		uint16 type = bitlStream->readUint16();

		LBItem *res;
		switch (type) {
		case kLBPictureItem:
			res = new LBPictureItem(_vm, this, rect);
			break;
		case kLBAnimationItem:
			res = new LBAnimationItem(_vm, this, rect);
			break;
		case kLBPaletteItem:
			res = new LBPaletteItem(_vm, this, rect);
			break;
		case kLBGroupItem:
			res = new LBGroupItem(_vm, this, rect);
			break;
		case kLBSoundItem:
			res = new LBSoundItem(_vm, this, rect);
			break;
		case kLBLiveTextItem:
			res = new LBLiveTextItem(_vm, this, rect);
			break;
		case kLBMovieItem:
			res = new LBMovieItem(_vm, this, rect);
			break;
		case kLBMiniGameItem:
			res = new LBMiniGameItem(_vm, this, rect);
			break;
		case kLBProxyItem:
			res = new LBProxyItem(_vm, this, rect);
			break;
		default:
			warning("Unknown item type %04x", type);
			// fall through
		case 3: // often used for buttons
			res = new LBItem(_vm, this, rect);
			break;
		}

		res->readFrom(bitlStream);
		_items.push_back(res);

		if (bitlStream->size() == bitlStream->pos())
			break;
	}

	delete bitlStream;
}

} // End of namespace Mohawk
