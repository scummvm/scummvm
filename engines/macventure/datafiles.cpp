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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MACVENTURE_DATAFILES_H
#define MACVENTURE_DATAFILES_H

#include "macventure/macventure.h"
#include "macventure/windows.h"

#include "common/unzip.h"

namespace MacVenture {

#define MACVENTURE_DATA_BUNDLE Common::String("macventure.zip")

Common::String windowTypeName(MVWindowType windowType) {
	switch (windowType) {
	case kDocument:
		return "Document";
	case kDBox:
		return "DBox";
	case kPlainDBox:
		return "PlainDBox";
	case kAltBox:
		return "AltBox";
	case kNoGrowDoc:
		return "NoGrowDoc";
	case kMovableDBox:
		return "MovableDBox";
	case kZoomDoc:
		return "ZoomDoc";
	case kZoomNoGrow:
		return "ZoomNoGrow";
	case kInvWindow:
		return "InvWindow";
	case kRDoc16:
		return "RDoc16";
	case kRDoc4:
		return "RDoc4";
	case kRDoc6:
		return "RDoc6";
	case kRDoc10:
		return "RDoc10";
	}
	return "";
}

void MacVentureEngine::loadDataBundle() {
	_dataBundle = Common::makeZipArchive(MACVENTURE_DATA_BUNDLE);
	if (!_dataBundle)
		error("Couldn't load data bundle '%s'.", MACVENTURE_DATA_BUNDLE.c_str());
}

Common::SeekableReadStream *MacVentureEngine::getBorderFile(MVWindowType windowType, bool isActive) {
	Common::String filename = windowTypeName(windowType);
	filename += (isActive ? "_act.bmp" : "_inac.bmp");
	if (!_dataBundle->hasFile(filename)) {
		warning("Missing border file '%s' in data bundle", filename.c_str());
		return NULL;
	}

	return _dataBundle->createReadStreamForMember(filename);
}

} // End of namespace MacVenture

#endif
