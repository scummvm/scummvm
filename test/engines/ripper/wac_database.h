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

#include "common/str.h"

#include <cxxtest/TestSuite.h>

class RipperWacDatabaseTestSuite : public CxxTest::TestSuite {
public:
	void testCatalogPreservesRetailAvailabilityAndLabelColumns() {
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntryCount(), 30U);
		for (uint index = 0;
				index < Ripper::getWacDatabaseCatalogEntryCount(); ++index) {
			const Ripper::WacDatabaseCatalogEntry *entry =
				Ripper::getWacDatabaseCatalogEntry(index);
			TS_ASSERT(entry);
			TS_ASSERT_EQUALS(entry->originalIndex, index);
			TS_ASSERT_EQUALS(entry->milestoneFlag, 0x46U + index);
			TS_ASSERT_EQUALS(entry->textResourceId, 0xdcU + index);
		}
		TS_ASSERT(!Ripper::getWacDatabaseCatalogEntry(30));
	}

	void testCatalogKeepsImplementedDispatchIdsExplicit() {
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(0)->handler,
			Ripper::kWacDatabaseHandlerStillImage);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(1)->handler,
			Ripper::kWacDatabaseHandlerBrokenMug);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(2)->handler,
			Ripper::kWacDatabaseHandlerBrokenMugCompletion);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(3)->handler,
			Ripper::kWacDatabaseHandlerJournal);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(5)->handler,
			Ripper::kWacDatabaseHandlerText);
		TS_ASSERT_EQUALS(
			Ripper::getWacDatabaseCatalogEntry(5)->contentResourceId, 0xb3);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(6)->handler,
			Ripper::kWacDatabaseHandlerVoiceLock);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(7)->handler,
			Ripper::kWacDatabaseHandlerOptionalPresentation);
		TS_ASSERT_EQUALS(Common::String(
			Ripper::getWacDatabaseCatalogEntry(7)->imagePath), "mag_wac.pcx");
		TS_ASSERT_EQUALS(Common::String(
			Ripper::getWacDatabaseCatalogEntry(7)->mediaPath), "mag_wac.avi");
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(7)->presentationFlag,
			Ripper::kMilestoneWacVideoEditorAvailable);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(7)->completionFlag,
			Ripper::kMilestoneRevealedMagnottaPhotoAsFake);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(9)->handler,
			Ripper::kWacDatabaseHandlerLoopingMedia);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(13)->handler,
			Ripper::kWacDatabaseHandlerLoopingMedia);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(14)->handler,
			Ripper::kWacDatabaseHandlerLoopingMedia);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(15)->handler,
			Ripper::kWacDatabaseHandlerText);
		TS_ASSERT_EQUALS(
			Ripper::getWacDatabaseCatalogEntry(15)->contentResourceId, 0xb6);
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(16)->handler,
			Ripper::kWacDatabaseHandlerOptionalPresentation);
		TS_ASSERT_EQUALS(Common::String(
			Ripper::getWacDatabaseCatalogEntry(16)->imagePath), "ed_wac.pcx");
		TS_ASSERT_EQUALS(Common::String(
			Ripper::getWacDatabaseCatalogEntry(16)->mediaPath), "ed_wac.avi");
		TS_ASSERT_EQUALS(Ripper::getWacDatabaseCatalogEntry(16)->completionFlag,
			Ripper::kMilestoneRevealedEddiePhotoAsFake);
	}
};
