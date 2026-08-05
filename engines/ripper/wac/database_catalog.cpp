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

#include "common/util.h"

namespace Ripper {

// BuildWacInventorySelectionMenu at 0x22c91 scans this 30-entry range.
// The milestone and text-resource columns are sequential in the retail table;
// the handler column records only dispatch branches implemented from
// RunWacInventorySelectionLoop at 0x2252a.
static const WacDatabaseCatalogEntry kWacDatabaseCatalog[] = {
	{  0, 0x46, 0xdc, kWacDatabaseHandlerStillImage,          0 },
	{  1, 0x47, 0xdd, kWacDatabaseHandlerBrokenMug,           0 },
	{  2, 0x48, 0xde, kWacDatabaseHandlerBrokenMugCompletion, 0 },
	{  3, 0x49, 0xdf, kWacDatabaseHandlerJournal,             0 },
	{  4, 0x4a, 0xe0, kWacDatabaseHandlerStillImage,          0 },
	{  5, 0x4b, 0xe1, kWacDatabaseHandlerText,                0xb3 },
	{  6, 0x4c, 0xe2, kWacDatabaseHandlerVoiceLock,           0 },
	{  7, 0x4d, 0xe3, kWacDatabaseHandlerUnavailable,         0 },
	{  8, 0x4e, 0xe4, kWacDatabaseHandlerUnavailable,         0 },
	{  9, 0x4f, 0xe5, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 10, 0x50, 0xe6, kWacDatabaseHandlerStillImage,          0 },
	{ 11, 0x51, 0xe7, kWacDatabaseHandlerStillImage,          0 },
	{ 12, 0x52, 0xe8, kWacDatabaseHandlerUnavailable,         0 },
	{ 13, 0x53, 0xe9, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 14, 0x54, 0xea, kWacDatabaseHandlerLoopingMedia,        0 },
	{ 15, 0x55, 0xeb, kWacDatabaseHandlerText,                0xb6 },
	{ 16, 0x56, 0xec, kWacDatabaseHandlerUnavailable,         0 },
	{ 17, 0x57, 0xed, kWacDatabaseHandlerUnavailable,         0 },
	{ 18, 0x58, 0xee, kWacDatabaseHandlerUnavailable,         0 },
	{ 19, 0x59, 0xef, kWacDatabaseHandlerUnavailable,         0 },
	{ 20, 0x5a, 0xf0, kWacDatabaseHandlerUnavailable,         0 },
	{ 21, 0x5b, 0xf1, kWacDatabaseHandlerUnavailable,         0 },
	{ 22, 0x5c, 0xf2, kWacDatabaseHandlerUnavailable,         0 },
	{ 23, 0x5d, 0xf3, kWacDatabaseHandlerUnavailable,         0 },
	{ 24, 0x5e, 0xf4, kWacDatabaseHandlerUnavailable,         0 },
	{ 25, 0x5f, 0xf5, kWacDatabaseHandlerUnavailable,         0 },
	{ 26, 0x60, 0xf6, kWacDatabaseHandlerUnavailable,         0 },
	{ 27, 0x61, 0xf7, kWacDatabaseHandlerUnavailable,         0 },
	{ 28, 0x62, 0xf8, kWacDatabaseHandlerUnavailable,         0 },
	{ 29, 0x63, 0xf9, kWacDatabaseHandlerUnavailable,         0 }
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
