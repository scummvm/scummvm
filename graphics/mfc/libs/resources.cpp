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

#include "common/stream.h"
#include "common/formats/winexe_ne.h"
#include "graphics/mfc/libs/resources.h"
#include "graphics/mfc/global_functions.h"

namespace Graphics {
namespace MFC {
namespace Libs {

Resources::~Resources() {
	clear();
}

void Resources::addResources(const Common::Path &file) {
	Common::NEResources *res = new Common::NEResources();
	if (!res->loadFromEXE(file))
		error("Could not load %s", file.baseName().c_str());

	push_front(ResourceFile());
	front()._filename = file.baseName();
	front()._res = res;
	_cache.clear();
}

void Resources::removeResources(const Common::Path &file) {
	// Expect resources to be removed in order
	assert(file.baseName() == front()._filename);
	pop_front();
	_cache.clear();
}

HRSRC Resources::findResource(const char *lpName, const char *lpType) {
	if (empty())
		error("Use CWinApp::addResources to register "
			"an .exe or .dll file containing the resources");

	Common::WinResourceID type =
		(HIWORD(lpType) == 0) ?
		Common::WinResourceID((intptr)lpType) :
		Common::WinResourceID(lpType);

	Common::WinResourceID id;
	if (HIWORD(lpName) == 0)
		id = Common::WinResourceID((intptr)lpName);
	else if (*lpName == '#')
		id = Common::WinResourceID(atol(lpName + 1));
	else
		id = Common::WinResourceID(lpName);

	// First check the cache
	for (auto &it : _cache) {
		if (it._type == type && it._id == id)
			return (HRSRC)&it;
	}

	// Get the resource
	for (auto &it : (*this)) {
		Common::WinResources *res = it._res;
		Common::SeekableReadStream *rs = res->getResource(type, id);

		if (rs) {
			// Add it to the cache, since we'll use
			// a pointer to it's entry as the return
			_cache.push_back(Resource(res, type, id, rs->size()));

			delete rs;
			return (HRSRC)&_cache.back();
		}
	}

	return nullptr;
}

HGLOBAL Resources::loadResource(HRSRC hResInfo) {
	Resource *ptr = ((Resource *)hResInfo);
	Common::SeekableReadStream *rs = ptr->_file->getResource(
		ptr->_type, ptr->_id);

	HGLOBAL h = GlobalAlloc(GPTR, ptr->_size);
	byte *data = (byte *)GlobalLock(h);
	rs->read(data, ptr->_size);
	GlobalUnlock(rs);

	delete rs;
	return h;
}

} // namespace Libs
} // namespace MFC
} // namespace Graphics
