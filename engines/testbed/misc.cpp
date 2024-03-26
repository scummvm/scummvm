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

#include "testbed/misc.h"
#include "common/timer.h"
#include "common/file.h"

#include "graphics/palette.h"

#include "gui/dialog.h"
#include "gui/imagealbum-dialog.h"

#include "image/jpeg.h"

namespace Testbed {

Common::String MiscTests::getHumanReadableFormat(const TimeDate &td) {
	return Common::String::format("%d:%d:%d on %d/%d/%d (dd/mm/yyyy)", td.tm_hour, td.tm_min, td.tm_sec, td.tm_mday, td.tm_mon + 1, td.tm_year + 1900);
}

void MiscTests::timerCallback(void *arg) {
	// Increment arg which actually points to an int
	int &valToModify = *((int *) arg);
	valToModify = 999; // some arbitrary value
}

void MiscTests::criticalSection(void *arg) {
	SharedVars &sv = *((SharedVars *)arg);

	Testsuite::logDetailedPrintf("Before critical section: %d %d\n", sv.first, sv.second);
	sv.mutex->lock();

	// In any case, the two vars must be equal at entry, if mutex works fine.
	// verify this here.
	if (sv.first != sv.second) {
		sv.resultSoFar = false;
	}

	sv.first++;
	g_system->delayMillis(1000);

	// This should bring no change as well in the difference between vars
	// verify this too.
	if (sv.second + 1 != sv.first) {
		sv.resultSoFar = false;
	}

	sv.second *= sv.first;
	Testsuite::logDetailedPrintf("After critical section: %d %d\n", sv.first, sv.second);
	sv.mutex->unlock();

	g_system->getTimerManager()->removeTimerProc(criticalSection);
}

TestExitStatus MiscTests::testDateTime() {

	if (ConfParams.isSessionInteractive()) {
		if (Testsuite::handleInteractiveInput("Testing the date time API implementation", "Continue", "Skip", kOptionRight)) {
			Testsuite::logPrintf("Info! Date time tests skipped by the user.\n");
			return kTestSkipped;
		}

		Testsuite::writeOnScreen("Verifying Date-Time...", Common::Point(0, 100));
	}

	TimeDate t1, t2;
	g_system->getTimeAndDate(t1);
	Testsuite::logDetailedPrintf("Current Time and Date: ");
	Common::String dateTimeNow;
	dateTimeNow = getHumanReadableFormat(t1);

	if (ConfParams.isSessionInteractive()) {
		// Directly verify date
		dateTimeNow = "We expect the current date time to be " + dateTimeNow;
		if (Testsuite::handleInteractiveInput(dateTimeNow, "Correct!", "Wrong", kOptionRight)) {
			return kTestFailed;
		}
	}

	g_system->getTimeAndDate(t1);
	dateTimeNow = getHumanReadableFormat(t1);
	Testsuite::logDetailedPrintf("%s\n", dateTimeNow.c_str());
	// Now, Put some delay
	g_system->delayMillis(2000);
	g_system->getTimeAndDate(t2);
	Testsuite::logDetailedPrintf("Time and Date 2s later: ");
	dateTimeNow = getHumanReadableFormat(t2);
	Testsuite::logDetailedPrintf("%s\n", dateTimeNow.c_str());

	if (t1.tm_year == t2.tm_year && t1.tm_mon == t2.tm_mon && t1.tm_mday == t2.tm_mday) {
		if (t1.tm_mon == t2.tm_mon && t1.tm_year == t2.tm_year) {
			// Ignore lag due to processing time
			if (t1.tm_sec + 2 == t2.tm_sec) {
				return kTestPassed;
			}
		}
	}
	return kTestFailed;
}

TestExitStatus MiscTests::testTimers() {
	int valToModify = 0;
	if (g_system->getTimerManager()->installTimerProc(timerCallback, 100000, &valToModify, "testbedTimer")) {
		g_system->delayMillis(150);
		g_system->getTimerManager()->removeTimerProc(timerCallback);

		if (999 == valToModify) {
			return kTestPassed;
		}
	}
	return kTestFailed;
}

TestExitStatus MiscTests::testMutexes() {

	if (ConfParams.isSessionInteractive()) {
		if (Testsuite::handleInteractiveInput("Testing the Mutual Exclusion API implementation", "Continue", "Skip", kOptionRight)) {
			Testsuite::logPrintf("Info! Mutex tests skipped by the user.\n");
			return kTestSkipped;
		}
		Testsuite::writeOnScreen("Installing mutex", Common::Point(0, 100));
	}

	SharedVars sv = {1, 1, true, new Common::Mutex()};

	if (g_system->getTimerManager()->installTimerProc(criticalSection, 100000, &sv, "testbedMutex")) {
		g_system->delayMillis(150);
	}

	sv.mutex->lock();
	sv.first++;
	g_system->delayMillis(1000);
	sv.second *= sv.first;
	sv.mutex->unlock();

	// wait till timed process exits
	if (ConfParams.isSessionInteractive()) {
		Testsuite::writeOnScreen("Waiting for 3s so that timed processes finish", Common::Point(0, 100));
	}
	g_system->delayMillis(3000);

	Testsuite::logDetailedPrintf("Final Value: %d %d\n", sv.first, sv.second);
	delete sv.mutex;

	if (sv.resultSoFar && 6 == sv.second) {
		return kTestPassed;
	}

	return kTestFailed;
}

TestExitStatus MiscTests::testOpenUrl() {
	Common::String info = "Testing openUrl() method.\n"
		"In this test we'll try to open scummvm.org in your default browser.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : openUrl()\n");
		return kTestSkipped;
	}

	if (!g_system->openUrl("https://scummvm.org/")) {
		Testsuite::logPrintf("Info! openUrl() says it couldn't open the url (probably not supported on this platform)\n");
		return kTestFailed;
	}

	if (Testsuite::handleInteractiveInput("Was ScummVM able to open 'https://scummvm.org/' in your default browser?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! openUrl() is not working!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("openUrl() is OK\n");
	return kTestPassed;
}

class ImageAlbumImageSupplier : public GUI::ImageAlbumImageSupplier {
public:
	void addFile(const Common::Path &path, Common::FormatInfo::FormatID format, bool dontReportFormat);

	bool loadImageSlot(uint slot, const Graphics::Surface *&outSurface, bool &outHasPalette, Graphics::Palette &outPalette, GUI::ImageAlbumImageMetadata &outMetadata) override;
	void releaseImageSlot(uint slot) override;
	bool getFileFormatForImageSlot(uint slot, Common::FormatInfo::FormatID &outFormat) const override;
	Common::SeekableReadStream *createReadStreamForSlot(uint slot) override;
	uint getNumSlots() const override;
	Common::U32String getDefaultFileNameForSlot(uint slot) const override;

private:
	struct FileInfo {
		FileInfo(const Common::Path &path, Common::FormatInfo::FormatID format, bool dontReportFormat);

		Common::Path _path;
		Common::FormatInfo::FormatID _format;
		bool _dontReportFormat;

		Common::SharedPtr<Image::ImageDecoder> _decoder;
	};

	Common::Array<FileInfo> _slots;
};

void ImageAlbumImageSupplier::addFile(const Common::Path &path, Common::FormatInfo::FormatID format, bool dontReportFormat) {
	_slots.push_back(FileInfo(path, format, dontReportFormat));
}

bool ImageAlbumImageSupplier::loadImageSlot(uint slot, const Graphics::Surface *&outSurface, bool &outHasPalette, Graphics::Palette &outPalette, GUI::ImageAlbumImageMetadata &outMetadata) {

	FileInfo &fi = _slots[slot];

	switch (fi._format) {
	case Common::FormatInfo::kBMP:
		fi._decoder.reset(new Image::BitmapDecoder());
		break;
#ifdef USE_JPEG
	case Common::FormatInfo::kJPEG:
		fi._decoder.reset(new Image::JPEGDecoder());
		break;
#endif
	default:
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> readStream(createReadStreamForSlot(slot));
	if (!readStream)
		return false;

	if (!fi._decoder->loadStream(*readStream))
		return false;

	outSurface = fi._decoder->getSurface();
	outHasPalette = fi._decoder->hasPalette();
	if (fi._decoder->hasPalette())
		outPalette.set(fi._decoder->getPalette(), 0, fi._decoder->getPaletteColorCount());
	outMetadata = GUI::ImageAlbumImageMetadata();

	return true;
}

void ImageAlbumImageSupplier::releaseImageSlot(uint slot) {
	_slots[slot]._decoder.reset();
}

bool ImageAlbumImageSupplier::getFileFormatForImageSlot(uint slot, Common::FormatInfo::FormatID &outFormat) const {
	if (_slots[slot]._dontReportFormat)
		return false;

	outFormat = _slots[slot]._format;
	return true;
}

Common::SeekableReadStream *ImageAlbumImageSupplier::createReadStreamForSlot(uint slot) {
	Common::ScopedPtr<Common::File> f(new Common::File());
	if (!f->open(_slots[slot]._path))
		return nullptr;

	return f.release();
}

uint ImageAlbumImageSupplier::getNumSlots() const {
	return _slots.size();
}

Common::U32String ImageAlbumImageSupplier::getDefaultFileNameForSlot(uint slot) const {
	return Common::U32String(_slots[slot]._path.baseName());
}

ImageAlbumImageSupplier::FileInfo::FileInfo(const Common::Path &path, Common::FormatInfo::FormatID format, bool dontReportFormat) {
	_path = path;
	_format = format;
	_dontReportFormat = dontReportFormat;
}

TestExitStatus MiscTests::testImageAlbum() {
	Common::String info = "Testing ImageAlbum method.\n"
						  "In this test we'll try to display some images,\n"
						  "and you should be able to save them if the backend supports it.\n"
						  "The second image will not report a file format to the backend, the third (if it exists) will be JPEG.\n";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : ImageAlbum()\n");
		return kTestSkipped;
	}

	ImageAlbumImageSupplier imageSupplier;
	imageSupplier.addFile("imagealbum/image1.bmp", Common::FormatInfo::kBMP, false);
	imageSupplier.addFile("imagealbum/image2.bmp", Common::FormatInfo::kBMP, true);
#ifdef USE_JPEG
	imageSupplier.addFile("imagealbum/image3.jpg", Common::FormatInfo::kJPEG, false);
#endif

	GUI::Dialog *dialog = GUI::createImageAlbumDialog(Common::U32String("Image Album"), &imageSupplier, 0);
	dialog->runModal();
	delete dialog;

	if (Testsuite::handleInteractiveInput("Did the image album work as expected?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! ImageAlbum is not working!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("ImageAlbum is OK\n");
	return kTestPassed;
}

MiscTestSuite::MiscTestSuite() {
	addTest("Datetime", &MiscTests::testDateTime, false);
	addTest("Timers", &MiscTests::testTimers, false);
	addTest("Mutexes", &MiscTests::testMutexes, false);
	addTest("openUrl", &MiscTests::testOpenUrl, true);
	addTest("ImageAlbum", &MiscTests::testImageAlbum, true);
}

} // End of namespace Testbed
