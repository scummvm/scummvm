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

#ifndef CACHED_LISTING_PROVIDER_H_
#define CACHED_LISTING_PROVIDER_H_

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "engines/wintermute/debugger/error.h"
#include "engines/wintermute/debugger/listing_providers/source_listing_provider.h"

namespace Wintermute {

class BasicSourceListingProvider;
class BlankListingProvider;
class Listing;

class CachedSourceListingProvider : public SourceListingProvider {
	BasicSourceListingProvider *_sourceListingProvider;
	BlankListingProvider *_fallbackListingProvider;
	typedef Common::HashMap<Common::Path, SourceListing *, Common::Path::IgnoreCaseAndMac_Hash, Common::Path::IgnoreCaseAndMac_EqualTo> CacheMap;
	CacheMap _cached;
	void invalidateCache();
public:
	CachedSourceListingProvider();
	~CachedSourceListingProvider() override;
	ErrorCode setPath(const Common::Path &path) override;
	Common::Path getPath() const override;
	Listing *getListing(const Common::Path &filename, ErrorCode &err) override;
};

} // End of namespace Wintermute

#endif /* CACHED_LISTING_PROVIDER_H_ */
