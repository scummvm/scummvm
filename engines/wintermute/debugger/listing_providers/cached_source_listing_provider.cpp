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

#include "cached_source_listing_provider.h"
#include "basic_source_listing_provider.h"
#include "blank_listing_provider.h"
#include "source_listing.h"

namespace Wintermute {

CachedSourceListingProvider::CachedSourceListingProvider() {
	_sourceListingProvider = new BasicSourceListingProvider();
	_fallbackListingProvider = new BlankListingProvider();
}

CachedSourceListingProvider::~CachedSourceListingProvider() {
	delete _sourceListingProvider;
	delete _fallbackListingProvider;
	for (Common::HashMap<Common::String, SourceListing*>::iterator it = _cached.begin();
			it != _cached.end(); it++) {
		delete (it->_value);
	}
}

Listing *CachedSourceListingProvider::getListing(const Common::String &filename, Wintermute::ErrorCode &error) {
	if (_cached.contains(filename)) {
		error = OK;
		SourceListing *copy = new SourceListing(*_cached.getVal(filename));
		return copy;
	} else {
		ErrorCode inner;
		SourceListing *res = _sourceListingProvider->getListing(filename, inner);
		if (inner == OK) {
			SourceListing *copy = new SourceListing(*res);
			_cached.setVal(filename, copy); // The cached copy is deleted on destruction
			return res;
		} else {
			delete res;
			return _fallbackListingProvider->getListing(filename, error);
		}
	}
}

void CachedSourceListingProvider::invalidateCache() {
	for (Common::HashMap<Common::String, SourceListing*>::iterator it = _cached.begin();
			it != _cached.end(); it++) {
		delete (it->_value);
	}
	_cached.clear();
}

ErrorCode CachedSourceListingProvider::setPath(const Common::String &path) {
	invalidateCache();
	return _sourceListingProvider->setPath(path);
}

Common::String CachedSourceListingProvider::getPath() const {
	return _sourceListingProvider->getPath();
}

} // End of namespace Wintermute
