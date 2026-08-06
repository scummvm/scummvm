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

#include "ripper/wac/database_catalog.h"

#include "ripper/milestones.h"

#include "common/util.h"

namespace Ripper {

// BuildWacInventorySelectionMenu at 0x22c91 scans this 30-entry range.
// The milestone and text-resource columns are sequential in the retail table;
// the handler column records the complete retail dispatch recovered from
// RunWacInventorySelectionLoop at 0x2252a.
static const WacDatabaseCatalogEntry kWacDatabaseCatalog[] = {
	{  0, 0x46, 0xdc, kWacDatabaseHandlerStillImage,          0 },
	{  1, 0x47, 0xdd, kWacDatabaseHandlerBrokenMug,           0 },
	{  2, 0x48, 0xde, kWacDatabaseHandlerBrokenMugCompletion, 0 },
	{  3, 0x49, 0xdf, kWacDatabaseHandlerJournal,             0 },
	{  4, 0x4a, 0xe0, kWacDatabaseHandlerStillImage,          0 },
	{  5, 0x4b, 0xe1, kWacDatabaseHandlerText,                0xb3 },
	{  6, 0x4c, 0xe2, kWacDatabaseHandlerVoiceLock,           0 },
	{  7, 0x4d, 0xe3, kWacDatabaseHandlerOptionalPresentation, 0,
		"mag_wac.pcx", "mag_wac.avi", kMilestoneWacVideoEditorAvailable,
		kMilestoneRevealedMagnottaPhotoAsFake },
	{  8, 0x4e, 0xe4, kWacDatabaseHandlerConditionalStillImage, 0,
		"wacinv8a.pcx", "wacinv8b.pcx", kMilestoneFirstRipperIdentity + 2 },
	{  9, 0x4f, 0xe5, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 10, 0x50, 0xe6, kWacDatabaseHandlerStillImage,          0 },
	{ 11, 0x51, 0xe7, kWacDatabaseHandlerStillImage,          0 },
	{ 12, 0x52, 0xe8, kWacDatabaseHandlerStillImage,          0 },
	{ 13, 0x53, 0xe9, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 14, 0x54, 0xea, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 15, 0x55, 0xeb, kWacDatabaseHandlerText,                0xb6 },
	{ 16, 0x56, 0xec, kWacDatabaseHandlerOptionalPresentation, 0,
		"ed_wac.pcx", "ed_wac.avi", kMilestoneWacVideoEditorAvailable,
		kMilestoneRevealedEddiePhotoAsFake },
	{ 17, 0x57, 0xed, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 18, 0x58, 0xee, kWacDatabaseHandlerRetailNoOp,          0 },
	{ 19, 0x59, 0xef, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 20, 0x5a, 0xf0, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 21, 0x5b, 0xf1, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 22, 0x5c, 0xf2, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 23, 0x5d, 0xf3, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 24, 0x5e, 0xf4, kWacDatabaseHandlerStillImage,          0 },
	{ 25, 0x5f, 0xf5, kWacDatabaseHandlerConfiguredMediaWithVoiceover, 0,
		"rip_game.smk", "rip_game.wav", 0, kMilestonePlayedRipperGame },
	{ 26, 0x60, 0xf6, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 27, 0x61, 0xf7, kWacDatabaseHandlerRetailNoOp,          0 },
	{ 28, 0x62, 0xf8, kWacDatabaseHandlerRetailNoOp,          0 },
	{ 29, 0x63, 0xf9, kWacDatabaseHandlerRetailNoOp,          0 }
};

uint getWacDatabaseCatalogEntryCount() {
	return ARRAYSIZE(kWacDatabaseCatalog);
}

const WacDatabaseCatalogEntry *getWacDatabaseCatalogEntry(uint originalIndex) {
	if (originalIndex >= ARRAYSIZE(kWacDatabaseCatalog))
		return nullptr;
	return &kWacDatabaseCatalog[originalIndex];
}

} // End of namespace Ripper
