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

#include "engines/wintermute/debugger/listing_providers/basic_source_listing_provider.h"
#include "engines/wintermute/base/base_file_manager.h"

namespace Wintermute {
BasicSourceListingProvider::BasicSourceListingProvider() : _fsDirectory(nullptr) {
}

BasicSourceListingProvider::~BasicSourceListingProvider() {
}

SourceListing *BasicSourceListingProvider::getListing(const Common::Path &filename, ErrorCode &_err) {
	_err = OK;
	if (!_fsDirectory) {
		_err = SOURCE_PATH_NOT_SET;
		return nullptr;
	};

	Common::SeekableReadStream *file = _fsDirectory->createReadStreamForMember(filename);
	Common::Array<Common::String> strings;

	if (!file) {
		_err = NO_SUCH_SOURCE;
	} else {
		if (file->err()) {
			_err = UNKNOWN_ERROR;
		}
		while (!file->eos()) {
			strings.push_back(file->readLine());
			if (file->err()) {
				_err = UNKNOWN_ERROR;
			}
		}
	}

	if (_err == OK) {
		return new SourceListing(strings);
	} else {
		return nullptr;
	}
}

ErrorCode BasicSourceListingProvider::setPath(const Common::Path &path) {
	if (path.empty())
		return ILLEGAL_PATH;
	delete _fsDirectory;
	Common::FSNode node(path);
	if (node.exists() && node.isDirectory()) {
		_fsDirectory = new Common::FSDirectory(node, 64);
		return OK;
	} else {
		return COULD_NOT_OPEN;
	}
}

Common::Path BasicSourceListingProvider::getPath() const {
	if (!_fsDirectory) return Common::Path();
	return _fsDirectory->getFSNode().getPath();
}

}
