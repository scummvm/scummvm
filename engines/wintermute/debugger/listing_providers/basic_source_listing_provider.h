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

#ifndef BASIC_SOURCE_LISTING_PROVIDER_H_
#define BASIC_SOURCE_LISTING_PROVIDER_H_

#include "engines/wintermute/debugger/listing_provider.h"
#include "source_listing_provider.h"
#include "source_listing.h"
#include "common/fs.h"

namespace Wintermute {

class BasicSourceListingProvider : public SourceListingProvider {
	Common::FSDirectory *_fsDirectory;
public:
	BasicSourceListingProvider();
	~BasicSourceListingProvider() override;
	SourceListing *getListing(const Common::String &filename, ErrorCode &err) override;
	ErrorCode setPath(const Common::String &path) override;
	Common::String getPath() const override;
};

}
#endif /* BASIC_SOURCE_LISTING_PROVIDER_H_ */
