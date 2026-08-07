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

#include "ripper/wac/still_image.h"

#include "common/debug.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/wac/database.h"
#include "ripper/wac/wac.h"

namespace Ripper {

static const uint kWacDefaultCursor = 14;
static const uint kWacControlCursor = 16;
static const int kWacDatabaseLeft = 400;
static const int kWacDatabaseTop = 50;
static const int kWacDatabaseRight = 590;
static const int kWacDatabaseBottom = 332;
static const int kWacMediaLeft = 50;
static const int kWacMediaTop = 50;
static const int kWacMediaWidth = 350;
static const int kWacMediaHeight = 282;
static const int kWacMediaScrollX = 355;
static const int kWacMediaScrollUpY = 60;
static const int kWacMediaScrollDownY = 90;
static const int kWacMediaScrollStep = 10;
static const uint kWacMediaPaletteFirst = 10;
static const uint kWacMediaPaletteCount = 140;
static const byte kWacDatabaseBackground = 4;
static const uint16 kWacDatabaseSelectionChanged = 0xfffe;
static const uint16 kNoAction = WacManager::kNoAction;
static const uint16 kExitAction = WacManager::kExitAction;

WacStillImageViewer::WacStillImageViewer(WacDatabaseSession *database) :
		_database(database), _scrollOffset(0), _scrollControl(0),
		_alignImageOrigin(false) {
}

bool WacStillImageViewer::load(const Common::String &path) {
	BitmapAssetFrame image;
	if (!_database->engine()->getResources()->loadInterfacePcx(path, image))
		return false;
	if (image.palette.size() <
			(kWacMediaPaletteFirst + kWacMediaPaletteCount) * 3)
		return false;

	_image = Common::move(image);
	// RunWacStillImageScreenWithOptionalAudio at 0x22f1f initializes the
	// retail scroll registry to zero, whose bitmap-presentation path opens a
	// tall PCX on its final 0x11a-row slice. BitmapAssetFrame is top-down, so
	// translate that presentation boundary to its source-row offset here.
	_scrollOffset = _image.height > kWacMediaHeight ?
		_image.height - kWacMediaHeight : 0;
	_scrollControl = 0;
	g_system->getPaletteManager()->setPalette(
		_image.palette.data() + kWacMediaPaletteFirst * 3,
		kWacMediaPaletteFirst, kWacMediaPaletteCount);
	draw();
	debugC(1, kDebugWac,
		"Ripper: loaded WAC database still image='%s' size=%ux%u viewport=%d,%d,%d,%d scrollOffset=%u scrollLimit=%u",
		path.c_str(), _image.width, _image.height,
		kWacMediaLeft, kWacMediaTop, kWacMediaWidth, kWacMediaHeight,
		_scrollOffset,
		_image.height > kWacMediaHeight ?
			_image.height - kWacMediaHeight : 0);
	return true;
}

Common::Point WacStillImageViewer::imageOrigin() const {
	const uint copyWidth = MIN<uint>(_image.width, kWacMediaWidth);
	const uint copyHeight = MIN<uint>(
		_image.height - _scrollOffset, kWacMediaHeight);
	int x = _image.height > kWacMediaHeight ? kWacMediaLeft :
		kWacMediaLeft + (kWacMediaWidth - copyWidth) / 2;
	if (_alignImageOrigin)
		x -= x % 4;
	const int y = _image.height > kWacMediaHeight ? kWacMediaTop :
		kWacMediaTop + (kWacMediaHeight - copyHeight) / 2;
	return Common::Point(x, y);
}

void WacStillImageViewer::draw() const {
	if (_image.pixels.empty())
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = kWacMediaTop; y < kWacMediaTop + kWacMediaHeight; ++y)
		memset(screen->getBasePtr(kWacMediaLeft, y), kWacDatabaseBackground,
			kWacMediaWidth);
	const uint copyWidth = MIN<uint>(_image.width, kWacMediaWidth);
	const uint copyHeight = MIN<uint>(
		_image.height - _scrollOffset, kWacMediaHeight);
	const Common::Point origin = imageOrigin();
	for (uint row = 0; row < copyHeight; ++row) {
		memcpy(screen->getBasePtr(origin.x, origin.y + row),
			_image.pixels.data() +
				(_scrollOffset + row) * _image.width,
			copyWidth);
	}
	g_system->unlockScreen();
	drawScrollControls();
	_database->engine()->getCursor()->refresh();
	presentScreen();
}

void WacStillImageViewer::drawScrollControls() const {
	const Common::Array<BitmapAssetFrame> &arrows =
		_database->databaseScrollArrows();
	if (_image.height <= kWacMediaHeight || arrows.size() < 4)
		return;
	const uint maximumScroll = _image.height - kWacMediaHeight;
	_database->drawBitmap(arrows[
		_scrollControl == 1 && _scrollOffset > 0 ? 1 : 0],
		kWacMediaScrollX, kWacMediaScrollUpY);
	_database->drawBitmap(arrows[
		_scrollControl == 2 && _scrollOffset < maximumScroll ? 3 : 2],
		kWacMediaScrollX, kWacMediaScrollDownY);
}

int WacStillImageViewer::findScrollControl(const Common::Point &point) const {
	const Common::Array<BitmapAssetFrame> &arrows =
		_database->databaseScrollArrows();
	if (_image.height <= kWacMediaHeight || arrows.size() < 4)
		return 0;
	const Common::Rect up(kWacMediaScrollX, kWacMediaScrollUpY,
		kWacMediaScrollX + arrows[0].width,
		kWacMediaScrollUpY + arrows[0].height);
	const Common::Rect down(kWacMediaScrollX, kWacMediaScrollDownY,
		kWacMediaScrollX + arrows[2].width,
		kWacMediaScrollDownY + arrows[2].height);
	if (up.contains(point))
		return 1;
	if (down.contains(point))
		return 2;
	return 0;
}

void WacStillImageViewer::scroll(int delta) {
	if (_image.height <= kWacMediaHeight)
		return;
	const uint maximumScroll = _image.height - kWacMediaHeight;
	int nextScroll = (int)_scrollOffset + delta;
	if (nextScroll < 0)
		nextScroll = 0;
	else if ((uint)nextScroll > maximumScroll)
		nextScroll = maximumScroll;
	if ((uint)nextScroll == _scrollOffset)
		return;
	_scrollOffset = nextScroll;
	debugC(2, kDebugWac,
		"Ripper: scrolled WAC database still image offset=%u limit=%u delta=%d",
		_scrollOffset, maximumScroll, delta);
	draw();
}

uint16 WacStillImageViewer::run(byte entryIndex,
		const Common::String &entryLabel, const Common::String &path) {
	return runInternal(entryIndex, entryLabel, path, Common::String(), 0, 0);
}

uint16 WacStillImageViewer::runWithOptionalPresentation(byte entryIndex,
		const Common::String &entryLabel, const Common::String &imagePath,
		const Common::String &mediaPath, uint presentationFlag,
		uint completionFlag) {
	return runInternal(entryIndex, entryLabel, imagePath, mediaPath,
		presentationFlag, completionFlag);
}

uint16 WacStillImageViewer::runInternal(byte entryIndex,
		const Common::String &entryLabel, const Common::String &imagePath,
		const Common::String &mediaPath, uint presentationFlag,
		uint completionFlag) {
	// RunWacStillImageScreenWithOptionalAudio at 0x22f1f owns the decoded
	// bitmap, palette patch, optional scroll controls, and nested WAC input
	// loop. RunWacInventorySelectionLoop at 0x2252a normalizes Escape (0x1b)
	// after this function returns and resumes the database chooser.
	// RunWacStillImageScreenWithOptionalPresentation at 0x22a32 shares that
	// ownership, aligns the image X coordinate to four pixels, and gates its
	// WACVID1A/B control on flag 0x57.
	const bool hasOptionalPresentation = !mediaPath.empty();
	const bool presentationAvailable = hasOptionalPresentation &&
		_database->engine()->getMilestones()->isSet(presentationFlag);
	_alignImageOrigin = hasOptionalPresentation;
	if (!load(imagePath)) {
		warning("Ripper: could not load WAC database still image '%s'",
			imagePath.c_str());
		return kNoAction;
	}

	Common::Array<BitmapAssetFrame> presentationFrames;
	if (presentationAvailable) {
		presentationFrames.resize(2);
		const char *const frameNames[] = { "wacvid1a", "wacvid1b" };
		for (uint frame = 0; frame < presentationFrames.size(); ++frame) {
			BitmapAssetSequence sequence;
			if (!_database->engine()->getResources()->loadInterfaceBitmapSequence(
					frameNames[frame], sequence) || sequence.frames.empty()) {
				warning("Ripper: could not load WAC presentation control '%s'",
					frameNames[frame]);
				return kNoAction;
			}
			presentationFrames[frame] = Common::move(sequence.frames[0]);
		}
	}
	const Common::Point origin = imageOrigin();
	const Common::Rect presentationBounds = presentationAvailable ?
		Common::Rect(origin.x, origin.y + _image.height + 1,
			origin.x + presentationFrames[0].width,
			origin.y + _image.height + 1 + presentationFrames[0].height) :
		Common::Rect();
	if (presentationAvailable)
		_database->drawBitmap(presentationFrames[0],
			presentationBounds.left, presentationBounds.top);

	_database->engine()->getInput()->discardMouseTransitions();
	debugC(1, kDebugWac,
		"Ripper: entered WAC still-image viewer entry=%u label='%s' function=%s image='%s' scrollable=%d presentation='%s' available=%d gate=0x%x completion=0x%x control=%d,%d,%d,%d",
		entryIndex, entryLabel.c_str(), hasOptionalPresentation ?
			"RunWacStillImageScreenWithOptionalPresentation@0x22a32" :
			"RunWacStillImageScreenWithOptionalAudio@0x22f1f",
		imagePath.c_str(), _image.height > kWacMediaHeight,
		mediaPath.c_str(), presentationAvailable, presentationFlag,
		completionFlag, presentationBounds.left, presentationBounds.top,
		presentationBounds.width(), presentationBounds.height());

	uint16 result = kNoAction;
	bool presentationPressed = false;
	while (!_database->engine()->shouldQuit()) {
		MouseState mouse;
		const uint16 command = _database->serviceDatabaseMediaInput(
			entryIndex, nullptr, 0, 0, &mouse, true);
		if (command == kWacDatabaseSelectionChanged ||
				command == kExitAction || command == 0x1b) {
			result = command;
			break;
		}
		if (command == MediaSequenceCallback::kContinueRefreshPalette) {
			g_system->getPaletteManager()->setPalette(
				_image.palette.data() + kWacMediaPaletteFirst * 3,
				kWacMediaPaletteFirst, kWacMediaPaletteCount);
			draw();
			if (presentationAvailable)
				_database->drawBitmap(presentationFrames[
					presentationPressed ? 1 : 0],
					presentationBounds.left, presentationBounds.top);
		}

		const int scrollControl = findScrollControl(mouse.position);
		if (scrollControl != _scrollControl) {
			_scrollControl = scrollControl;
			drawScrollControls();
			debugC(2, kDebugWac,
				"Ripper: WAC database still-image scroll hover control=%d point=%d,%d",
				_scrollControl, mouse.position.x, mouse.position.y);
		}
		const Common::Rect databaseBounds(kWacDatabaseLeft, kWacDatabaseTop,
			kWacDatabaseRight, kWacDatabaseBottom);
		const bool presentationHover = presentationAvailable &&
			presentationBounds.contains(mouse.position);
		_database->engine()->getCursor()->update(
			databaseBounds.contains(mouse.position) ||
				_database->persistentControlHovered() ||
				scrollControl != 0 || presentationHover ?
				kWacControlCursor : kWacDefaultCursor);
		if ((mouse.pressed & kMouseButtonLeft) != 0 && presentationHover) {
			presentationPressed = true;
			_database->drawBitmap(presentationFrames[1],
				presentationBounds.left, presentationBounds.top);
		}
		if ((mouse.released & kMouseButtonLeft) != 0) {
			if (scrollControl == 1)
				scroll(-kWacMediaScrollStep);
			else if (scrollControl == 2)
				scroll(kWacMediaScrollStep);
			if (presentationPressed) {
				presentationPressed = false;
				_database->drawBitmap(presentationFrames[0],
					presentationBounds.left, presentationBounds.top);
				if (presentationHover) {
					// The retail control temporarily forces video mode 1 around
					// RunMediaPresentation, then sets the caller-supplied EBX flag.
					RipperSettings *settings =
						_database->engine()->getSettings();
					const uint savedVideoMode = settings->getVideoMode();
					settings->setVideoMode(1);
					const bool played = _database->engine()->getMedia()->play(
						mediaPath, true, 0, 0);
					settings->setVideoMode(savedVideoMode);
					_database->engine()->getMilestones()->set(
						completionFlag, true,
						"wac-picture-presentation");
					debugC(1, kDebugWac,
						"Ripper: WAC picture presentation entry=%u media='%s' played=%d gate=0x%x completion=0x%x value=%d videoMode=%u restoredVideoMode=%u",
						entryIndex, mediaPath.c_str(), played,
						presentationFlag, completionFlag,
						_database->engine()->getMilestones()->isSet(
							completionFlag), 1U, savedVideoMode);
				}
			}
		}
		presentScreen();
		g_system->delayMillis(10);
	}

	_database->clearDatabaseMediaViewport();
	_database->drawDatabase();
	_database->engine()->getInput()->discardMouseTransitions();
	debugC(1, kDebugWac,
		"Ripper: left WAC still-image viewer entry=%u image='%s' result=0x%x scrollOffset=%u presentationAvailable=%d completion=0x%x value=%d",
		entryIndex, imagePath.c_str(), result, _scrollOffset,
		presentationAvailable, completionFlag,
		completionFlag != 0 &&
			_database->engine()->getMilestones()->isSet(completionFlag));
	if (result == kWacDatabaseSelectionChanged &&
			_database->_databaseSelection < _database->_databaseEntries.size())
		return _database->dispatchDatabaseEntry(
			_database->_databaseEntries[_database->_databaseSelection]);
	return result == kExitAction ? kExitAction : kNoAction;
}

} // End of namespace Ripper
