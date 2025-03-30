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

#include "common/file.h"
#include "common/memstream.h"
#include "common/formats/winexe_ne.h"
#include "bagel/hodjnpodj/views/resource_view.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {

ResourceView::ResourceView(const Common::String &name, const Common::String &resFilename) :
		View(name), _resourceFilename(resFilename) {
}

ResourceView::~ResourceView() {
	delete _resources;
}

bool ResourceView::msgOpen(const OpenMessage &msg) {
	SearchMan.add("Resources", this, 10, false);

	if (!_resourceFilename.empty()) {
		_resources = Common::NEResources::createFromEXE(Common::Path(_resourceFilename));

		// Load the data for each mapped file
		for (ResourceFiles::iterator it = _resourceFiles.begin();
				it != _resourceFiles.end(); ++it) {
			ResourceEntry &re = it->_value;
			Common::SeekableReadStream *src =
				_resources->getResource(re._type, re._id);
			assert(src);

			re.resize(src->size());
			src->read(&re[0], re.size());

			delete src;
		}
	}

	g_events->setCursor(IDC_ARROW);

	return View::msgOpen(msg);
}

bool ResourceView::msgClose(const CloseMessage &msg) {
	SearchMan.remove("Resources");
	delete _resources;
	_resources = nullptr;

	// Free the data for each mapped file
	for (ResourceFiles::iterator it = _resourceFiles.begin();
		it != _resourceFiles.end(); ++it) {
		ResourceEntry &re = it->_value;
		re.clear();
	}

	return View::msgClose(msg);
}

bool ResourceView::hasFile(const Common::Path &path) const {
	return _resourceFiles.contains(path.toString());
}

int ResourceView::listMembers(Common::ArchiveMemberList &list) const {
	return 0;
}

const Common::ArchiveMemberPtr ResourceView::getMember(const Common::Path &path) const {
	return Common::ArchiveMemberPtr();
}

Common::SeekableReadStream *ResourceView::createReadStreamForMember(const Common::Path &path) const {
	Common::NEResources winResources;
	Common::String filename = path.toString();

	// See if it's a filename that's been registered
	if (!_resourceFiles.contains(filename))
		return nullptr;

	const ResourceEntry &re = _resourceFiles[filename];
	return new Common::MemoryReadStream(&re[0], re.size());
}

} // namespace HodjNPodj
} // namespace Bagel
