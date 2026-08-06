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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_WAC_DATABASE_CATALOG_H
#define RIPPER_WAC_DATABASE_CATALOG_H

#include "common/scummsys.h"

namespace Ripper {

enum WacDatabaseHandlerKind {
	kWacDatabaseHandlerRetailNoOp,
	kWacDatabaseHandlerStillImage,
	kWacDatabaseHandlerConditionalStillImage,
	kWacDatabaseHandlerBrokenMug,
	kWacDatabaseHandlerBrokenMugCompletion,
	kWacDatabaseHandlerJournal,
	kWacDatabaseHandlerVoiceLock,
	kWacDatabaseHandlerLoopingMedia,
	kWacDatabaseHandlerScriptMediaWithVoiceover,
	kWacDatabaseHandlerOptionalPresentation,
	kWacDatabaseHandlerText
};

struct WacDatabaseCatalogEntry {
	constexpr WacDatabaseCatalogEntry(byte originalIndex_, uint16 milestoneFlag_,
		uint16 textResourceId_, WacDatabaseHandlerKind handler_,
		uint16 contentResourceId_, const char *imagePath_ = nullptr,
		const char *mediaPath_ = nullptr, uint16 presentationFlag_ = 0,
		uint16 completionFlag_ = 0) : originalIndex(originalIndex_),
		milestoneFlag(milestoneFlag_), textResourceId(textResourceId_),
		handler(handler_), contentResourceId(contentResourceId_),
		imagePath(imagePath_), mediaPath(mediaPath_),
		presentationFlag(presentationFlag_), completionFlag(completionFlag_) {
	}

	byte originalIndex;
	uint16 milestoneFlag;
	uint16 textResourceId;
	WacDatabaseHandlerKind handler;
	uint16 contentResourceId;
	const char *imagePath;
	const char *mediaPath;
	uint16 presentationFlag;
	uint16 completionFlag;
};

uint getWacDatabaseCatalogEntryCount();
const WacDatabaseCatalogEntry *getWacDatabaseCatalogEntry(uint originalIndex);

} // End of namespace Ripper

#endif // RIPPER_WAC_DATABASE_CATALOG_H
