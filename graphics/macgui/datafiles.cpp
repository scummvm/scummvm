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

/*
 * Based on MacVenture engine, based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "graphics/macgui/macwindowmanager.h"

#include "common/archive.h"
#include "common/compression/unzip.h"

namespace Graphics {

#define MACGUI_DATA_BUNDLE Common::String("macgui.dat")

struct BorderName {
	uint32 type;
	const char *name;
	BorderOffsets offsets;
};

static const BorderName borders[] = {
	{0x00, 					"StandardClose",			{ 1,  2, 19,  2,		 2,  2,		false,	25, 0, 0}},
	{0x01, 					"ThickNoTitle",		 		{ 5,  5,  5,  5,		-1, -1,		false,	0,  0, 0}},
	{0x02, 					"ThinNoTitle",		 		{ 1,  1,  1,  1,		-1, -1,		false,	0,  0, 0}},
	{0x03, 					"ThinNoTitleShadow",		{ 1,  3,  1,  3,		-1, -1,		false,	0,  0, 0}},
	{0x04, 					"StandardClose",			{ 1,  2, 19,  2,		 2,  2,		false,	25, 0, 0}},
	{0x05, 					"Thick",					{ 5,  5, 20,  5,		 2,  3,		false,	13, 0, 0}},
	{0x06, 					"ThinNoTitle",		 		{ 1,  1,  1,  1,		-1, -1,		false,	0,  0, 0}},
	{0x07, 					"ThinNoTitleShadow",		{ 1,  3,  1,  3,		-1, -1, 	false,	0,  0, 0}},
	{0x08, 					"StandardCloseZoom",		{ 1,  2, 19,  2,		 2,  2,		false,	25, 0, 0}},
	{0x09, 					"ThickZoom",				{ 5,  5, 20,  5,		 2,  3,		false,	13, 0, 0}},
	{0x0A, 					"ThinNoTitle",		 		{ 1,  1,  1,  1,		-1, -1,		false,	0,  0, 0}},
	{0x0B, 					"ThinNoTitleShadow",  		{ 1,  3,  1,  3,		-1, -1,		false,	0,  0, 0}},
	{0x0C, 					"StandardCloseZoom",		{ 1,  2, 19,  2,		 2,  2,		false,	25, 0, 0}},
	{0x0D, 					"ThickZoom",				{ 5,  5, 20,  5,		 2,  3,		false,	13, 0, 0}},
	{0x0E, 					"ThinNoTitle",		 		{ 1,  1,  1,  1,		-1, -1,		false,	0,  0, 0}},
	{0x0F, 					"ThinNoTitleShadow",  		{ 1,  3,  1,  3,		-1, -1,		false,	0,  0, 0}},
	{0x10, 					"RoundClose",		 		{ 1,  1, 19,  6,		 1,  1,		true,	25, 0, 0}},
	{kBorderScroll + 0x00,	"Win95BorderScrollbar",		{ 1,  17, 1,  1,		 1,  1,		true,	25, 15, 17}},
	{kBorderScroll + 0x01, 	"Win95NoBorderScrollbar",	{ 1,  17, 1,  1,		 1,  1,		true,	25, 15, 17}},
	{kBorderScroll + 0x02, 	"MacOSNoBorderScrollbar",	{ 1,  17, 1,  1,		 1,  1,		true,	25, 17, 17}},
	{0xFF, 					"No type",			 		{-1, -1, -1, -1,		-1, -1,		false,	0,  0, 0}}
};

Common::String windowTypeName(uint32 windowType) {
	int i = 0;
	while (borders[i].type != 0xFF) {
		if (borders[i].type == windowType) {
			return borders[i].name;
		}
		i++;
	}
	return "ThinNoTitle";
}

void MacWindowManager::loadDataBundle() {
	_dataBundle = Common::makeZipArchive(MACGUI_DATA_BUNDLE);
	if (!_dataBundle) {
		warning("MACGUI: Couldn't load data bundle '%s'.", MACGUI_DATA_BUNDLE.c_str());
	}
}

void MacWindowManager::cleanupDataBundle() {
	delete _dataBundle;
}

BorderOffsets MacWindowManager::getBorderOffsets(uint32 windowType) {
	int i = 0;
	while (borders[i].type != 0xFF) {
		if (borders[i].type == windowType)
			break;
		i++;
	}
	return borders[i].offsets;
}

Common::SeekableReadStream *MacWindowManager::getBorderFile(uint32 windowType, uint32 flags) {
	if (!_dataBundle)
		return NULL;

	Common::String filename = windowTypeName(windowType);
	filename += (flags & kWindowBorderActive) ? "_act" : "_inac";
	filename += (flags & kWindowBorderTitle) ? "_title" : "";
	filename += ".bmp";
	if (!_dataBundle->hasFile(filename)) {
		warning("Missing border file '%s' in data bundle", filename.c_str());
		return NULL;
	}

	return _dataBundle->createReadStreamForMember(filename);
}

Common::SeekableReadStream *MacWindowManager::getFile(const Common::String &filename) {
	if (!_dataBundle)
		return NULL;

	if (!_dataBundle->hasFile(filename)) {
		warning("Missing file '%s' in data bundle", filename.c_str());
		return NULL;
	}

	return _dataBundle->createReadStreamForMember(filename);
}

} // End of namespace Graphics
