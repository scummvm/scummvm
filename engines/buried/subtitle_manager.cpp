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

#include "buried/dialogs.h"
#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/subtitle_manager.h"
#include "buried/window.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Buried {

// Background color for the subtitle box.
static constexpr ColorRGB kBoxBgColor = { 38, 12,12 };
// Color for the subtitle box top border. (Muted-copper-orange)
static constexpr ColorRGB kTopBorderColor = { 140,60, 35 };
// Color for the subtitle box bottom border. (Bright orange)
static constexpr ColorRGB kBottomBorderColor = { 237,109, 66 };
// Color for the subtitle box side borders. (Muted dark red)
static constexpr ColorRGB kSideBorderColor = { 105, 36, 28 };
// Color for the speaker name text. (Neon orange)
static constexpr ColorRGB kSpeakerTextColor = { 237, 109, 66 };
// Color for the subtitle text, chosen to have high contrast with the background. (Cream amber)
static constexpr ColorRGB kDialogTextColor = { 255, 230, 180 };

SubtitleManager::SubtitleManager(BuriedEngine *vm) : _vm(vm), _font(nullptr), _fontBold(nullptr), _fontSize(0) {
	updateFont();
	loadSubtitlesData();
}

SubtitleManager::~SubtitleManager() {
	delete _font;
	delete _fontBold;
}

bool SubtitleManager::areSubtitlesEnabled() {
	return ConfMan.getBool("subtitles");
}

void SubtitleManager::markSubtitlesDirty(Window *targetWindow) {
	if (!areSubtitlesEnabled()) {
		return;
	}

	// Invalidate target window (or main window) to trigger onPaint() repaints
	if (targetWindow) {
		targetWindow->invalidateWindow(false);
	} else if (_vm->_mainWindow) {
		_vm->_mainWindow->invalidateWindow(false);
	}

	// Mark the subtitle box region dirty on the GraphicsManager screen renderer.
	// This is required because the subtitle overlay sits below the jumpsuit visor window bounds,
	// so we explicitly add its screen bounding rect to the hardware dirty rect list for display blitting.
	_vm->_gfx->invalidateRect(getDefaultBoxBounds(), false);
}

void SubtitleManager::forceRepaintSubtitles(Window *targetWindow) {
	if (!areSubtitlesEnabled()) {
		return;
	}

	Window *windowToPaint = targetWindow ? targetWindow : _vm->_mainWindow;
	if (!windowToPaint) {
		return;
	}

	markSubtitlesDirty(windowToPaint);
	windowToPaint->onPaint();
}

void SubtitleManager::updateFont() {
	int requestedSize = kDefaultSubtitleFontSize;
	if (ConfMan.hasKey(kSubtitleFontSizeKey)) {
		requestedSize = ConfMan.getInt(kSubtitleFontSizeKey);
	}

	if (!_font || _fontSize != requestedSize) {
		delete _font;
		delete _fontBold;
		_fontSize = requestedSize;
		_font = _vm->_gfx->createFont(_fontSize, /* bold= */ false);
		_fontBold = _vm->_gfx->createFont(_fontSize, /* bold= */ true);
	}
}

uint32 SubtitleManager::getColor(const ColorRGB &color) const {
	return _vm->_gfx->getColor(color.r, color.g, color.b);
}

// Loads binary subtitle package (buried_subtitles.dat) and returns whether the load was successful.
//
// File Format Specification (Big-Endian):
// 1. Magic Signature (4 bytes): 'BURS' (0x42555253)
// 2. Track Count (uint16): N tracks
// 3. TOC Table (N x 22 bytes):
//    - Media ID (16 bytes): Fixed ASCII string (null-padded)
//    - Payload Offset (uint32): Byte offset to track data
//    - Card Count (uint16): M subtitle cards in track
// 4. Track Payloads:
//    - Start Time (uint32): Start time in milliseconds
//    - End Time (uint32): End time in milliseconds
//    - Speaker Length (uint16): Speaker string length
//    - Speaker String: UTF-8 speaker name
//    - Text Length (uint16): Dialogue string length
//    - Text String: UTF-8 dialogue text
bool SubtitleManager::loadSubtitlesData() {
	Common::File file;
	if (!file.open(Common::Path("buried_subtitles.dat"))) {
		warning("[SubtitleManager] Could not open buried_subtitles.dat");
		return false;
	}

	// Look for "Buried Subtitle" signature to skip trying to parse an errant or malformed file.
	uint32 magic = file.readUint32BE();
	if (magic != MKTAG('B', 'U', 'R', 'S')) {
		warning("[SubtitleManager] Invalid magic in subtitles.dat: 0x%08X", magic);
		return false;
	}

	uint16 numTracks = file.readUint16BE();

	struct TocEntry {
		Common::String mediaId;
		uint32 offset;
		uint16 cardCount;
	};

	Common::Array<TocEntry> toc;
	toc.reserve(numTracks);

	// Load the "table of contents" for the subtitles.
	for (uint16 i = 0; i < numTracks; ++i) {
		char mediaIdBuf[17];
		file.read(mediaIdBuf, 16);
		mediaIdBuf[16] = '\0';

		TocEntry entry;
		entry.mediaId = mediaIdBuf;
		entry.offset = file.readUint32BE();
		entry.cardCount = file.readUint16BE();
		toc.push_back(entry);
	}

	// Load track payload data
	for (const auto &tocEntry : toc) {
		if (file.seek(tocEntry.offset, SEEK_SET)) {
			SubtitleTrack track;
			track.mediaId = tocEntry.mediaId;

			for (uint16 cardInex = 0; cardInex < tocEntry.cardCount; ++cardInex) {
				SubtitleEntry card;
				card.startMs = file.readUint32BE();
				card.endMs = file.readUint32BE();

				uint16 speakerNameLength = file.readUint16BE();
				if (speakerNameLength > 0) {
					card.speaker = file.readString(0, speakerNameLength);
				}

				uint16 textLength = file.readUint16BE();
				if (textLength > 0) {
					card.text = file.readString(0, textLength);
				}

				track.entries.push_back(card);
			}

			_loadedTracks[track.mediaId] = track;
		}
	}

	return true;
}

bool SubtitleManager::hasSubtitleTrack(const Common::String &mediaId) const {
	if (mediaId.empty()) {
		return false;
	}
	return _loadedTracks.contains(mediaId);
}

const SubtitleManager::SubtitleEntry *SubtitleManager::getSubtitleForTime(const Common::String &mediaId, uint32 currentMs) {
	if (!_loadedTracks.contains(mediaId)) {
		return nullptr;
	}

	// Linear search is sufficient here -- tracks typically only have a few entries and none have more than 20.
	const SubtitleTrack &track = _loadedTracks[mediaId];
	for (const auto & entry : track.entries) {
		if (currentMs >= entry.startMs && currentMs <= entry.endMs) {
			return &entry;
		}
	}

	return nullptr;
}

bool SubtitleManager::renderSubtitleForMedia(Graphics::Surface *destSurface, const Common::String &mediaId, uint32 currentMs) {
	return renderSubtitleForMedia(destSurface, getDefaultBoxBounds(), mediaId, currentMs);
}

bool SubtitleManager::renderSubtitleForMedia(Graphics::Surface *destSurface, const Common::Rect &boxRect, const Common::String &mediaId, uint32 currentMs) {
	if (!areSubtitlesEnabled()) {
		return false;
	}

	const SubtitleEntry *sub = getSubtitleForTime(mediaId, currentMs);
	if (!sub) {
		return false;
	}
	renderSubtitle(destSurface, boxRect, *sub);
	return true;
}

bool SubtitleManager::isSubtitledAudioPlaying() const {
	SoundManager *sound = _vm->_sound;
	if (!sound) {
		return false;
	}
	return sound->isAIVoicePlaying() ||
	       sound->isSyncSoundPlaying() ||
	       sound->isInterfaceSoundPlaying() ||
	       sound->isSubtitledSoundEffectPlaying();
}

bool SubtitleManager::renderSubtitlesForActiveAudio(Graphics::Surface *destSurface) {
	if (!destSurface || !areSubtitlesEnabled()) {
		return false;
	}

	SoundManager *sound = _vm->_sound;
	if (!sound) {
		return false;
	}

	if (sound->isSyncSoundPlaying()) {
		return renderSubtitleForMedia(destSurface, sound->getSyncSoundMediaId(), sound->getSyncSoundPlaybackPositionMillis());
	}
	if (sound->isAIVoicePlaying()) {
		return renderSubtitleForMedia(destSurface, sound->getAIVoiceMediaId(), sound->getAIVoicePlaybackPositionMillis());
	}
	if (sound->isInterfaceSoundPlaying()) {
		return renderSubtitleForMedia(destSurface, sound->getInterfaceSoundMediaId(), sound->getInterfaceSoundPlaybackPositionMillis());
	}
	if (sound->isSubtitledSoundEffectPlaying()) {
		return renderSubtitleForMedia(destSurface, sound->getSubtitledSoundEffectMediaId(), sound->getSubtitledSoundEffectPosition());
	}

	return false;
}

bool SubtitleManager::renderSubtitleForVideo(Graphics::Surface *destSurface, const Window *videoWindow, const Common::String &mediaId, uint32 currentMs, const Common::Rect &mediaRect) {
	Common::Rect boxBounds = calculateBoxBoundsForVideo(videoWindow, mediaRect);
	return renderSubtitleForMedia(destSurface, boxBounds, mediaId, currentMs);
}

static byte blendColorComponent(byte srcComp, byte targetComp, float alpha) {
	return (byte)(srcComp * (1.0f - alpha) + targetComp * alpha);
}

static void drawPixel(Graphics::Surface *destSurface, int x, int y, uint32 color) {
	if (destSurface->format.bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)destSurface->getBasePtr(x, y);
		*ptr = (uint16)color;
	} else if (destSurface->format.bytesPerPixel == 4) {
		uint32 *ptr = (uint32 *)destSurface->getBasePtr(x, y);
		*ptr = color;
	} else {
		byte *ptr = (byte *)destSurface->getBasePtr(x, y);
		*ptr = (byte)color;
	}
}

// Blends the existing color at pixel (x,y) on destSurface with the target color.
static void blendPixel(Graphics::Surface *destSurface, int x, int y, ColorRGB target, float alpha) {
	if (destSurface->format.bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)destSurface->getBasePtr(x, y);
		byte r, g, b;
		destSurface->format.colorToRGB(*ptr, r, g, b);
		byte blendedR = blendColorComponent(r, target.r, alpha);
		byte blendedG = blendColorComponent(g, target.g, alpha);
		byte blendedB = blendColorComponent(b, target.b, alpha);
		*ptr = destSurface->format.RGBToColor(blendedR, blendedG, blendedB);
	} else if (destSurface->format.bytesPerPixel == 4) {
		uint32 *ptr = (uint32 *)destSurface->getBasePtr(x, y);
		byte r, g, b;
		destSurface->format.colorToRGB(*ptr, r, g, b);
		byte blendedR = blendColorComponent(r, target.r, alpha);
		byte blendedG = blendColorComponent(g, target.g, alpha);
		byte blendedB = blendColorComponent(b, target.b, alpha);
		*ptr = destSurface->format.RGBToColor(blendedR, blendedG, blendedB);
	} else {
		drawPixel(destSurface, x, y, destSurface->format.RGBToColor(target.r, target.g, target.b));
	}
}

void SubtitleManager::renderSubtitle(Graphics::Surface *destSurface, const Common::Rect &boxRect, const SubtitleEntry &entry) {
	if (!destSurface || !_font || entry.text.empty()) {
		return;
	}

	// Re-check fonts in case the user changed the setting while the game is running.
	updateFont();

	int fontHeight = _font->getFontHeight();

	float alpha = CLIP<float>(kSubtitleBoxOpacity, 0.0f, 1.0f);

	// Render subtitle box scrim & bottom chamfered border.
	//
	// 1. Chamfer Cutouts: For the bottom rows (distFromBottom < kSubtitleChamferHeight), inset the left and right
	//    bounds by (kSubtitleChamferHeight - distFromBottom) * 2 pixels. This applies a 2:1 horizontal-to-vertical
	//    bevel slope (approx. 26.6° angle relative to horizontal) extending kSubtitleChamferHeight pixels vertically.
	// 2. CRT Scanline Interlacing: Alternate row opacity (1.15x alpha on even rows,
	//    0.85x alpha on odd rows) to simulate an interlaced glass CRT monitor.
	for (int y = boxRect.top; y < boxRect.bottom; ++y) {
		if (y < 0 || y >= destSurface->h) {
			continue;
		}

		int distanceFromTop = y - boxRect.top;
		int distanceFromBottom = boxRect.bottom - 1 - y;

		// Insets for the chamfer drawn at the bottom of the box.
		int boxHorizontalInset = 0;
		if (distanceFromBottom < kSubtitleChamferHeight) {
			boxHorizontalInset = (kSubtitleChamferHeight - distanceFromBottom) * 2; // 2:1 bevel slope (approx. 26.6° angle)
		}

		int startX = boxRect.left + boxHorizontalInset;
		int endX = boxRect.right - boxHorizontalInset;

		// CRT Scanline Raster: alternate line alpha for subtle interlaced CRT glass effect.
		// The CRT effect is skipped if the alpha is ever configured to 1 as such a setting would indicate an intent to
		// fully eliminate transparency on the subtitle pane.
		float curAlpha = alpha;
		if (alpha < 1.0f) {
			curAlpha = distanceFromTop % 2 == 0 ? alpha * 1.15f : alpha * 0.85f;
			curAlpha = CLIP<float>(curAlpha, 0.0f, 1.0f);
		}

		for (int x = startX; x < endX; ++x) {
			if (x < 0 || x >= destSurface->w) {
				continue;
			}

			bool isTopBorder = y < boxRect.top + kSubtitleBorderWidth;
			bool isBottomBorder = y >= boxRect.bottom - kSubtitleBorderWidth;
			bool isSideBorder = x < startX + kSubtitleBorderWidth || x >= endX - kSubtitleBorderWidth;

			if (isTopBorder) {
				drawPixel(destSurface, x, y, getColor(kTopBorderColor));
			} else if (isBottomBorder) {
				drawPixel(destSurface, x, y, getColor(kBottomBorderColor));
			} else if (isSideBorder) {
				drawPixel(destSurface, x, y, getColor(kSideBorderColor));
			} else {
				blendPixel(destSurface, x, y, kBoxBgColor, curAlpha);
			}
		}
	}

	uint32 speakerColor = getColor(kSpeakerTextColor);
	uint32 dialogColor = getColor(kDialogTextColor);

	constexpr int paddingX = kSubtitlePadX;
	constexpr int paddingY = kSubtitlePadY;
	const int interiorWidth  = boxRect.width() - paddingX * 2;
	const int innerX  = boxRect.left + paddingX;
	const int innerY = boxRect.top + paddingY;

	int curX = innerX;
	int curY = innerY;

	// Build speaker prefix (e.g. "Arthur: ")
	const Common::String speakerPrefix = entry.speaker.empty() ? "" : (entry.speaker + ": ");
	int speakerWidth = speakerPrefix.empty() ? 0 : _fontBold->getStringWidth(speakerPrefix);

	// Word-wrap text using ScummVM's native Font::wordWrapText, passing speakerWidth as initWidth for line 1
	Common::Array<Common::String> lines;
	_font->wordWrapText(entry.text, interiorWidth, lines, speakerWidth);

	// Draw wrapped text lines
	for (size_t i = 0; i < lines.size() && i < (size_t)kMaxSubtitleLines; i++) {
		if (i == 0) {
			// Special handling to draw the speaker name on the first line.
			if (!speakerPrefix.empty()) {
				_fontBold->drawString(destSurface, speakerPrefix, curX, curY, interiorWidth, speakerColor, Graphics::kTextAlignLeft);
				curX += speakerWidth;
			}
			int line1AvailableTextWidth = interiorWidth - speakerWidth;
			if (line1AvailableTextWidth > 0 && !lines[0].empty()) {
				_font->drawString(destSurface, lines[0], curX, curY, line1AvailableTextWidth, dialogColor, Graphics::kTextAlignLeft);
			}
		} else {
			// Remaining lines only have dialog text.
			if (!lines[i].empty()) {
				_font->drawString(destSurface, lines[i], curX, curY, interiorWidth, dialogColor, Graphics::kTextAlignLeft);
			}
		}
		curX = innerX;
		curY += fontHeight + kSubtitleInterlineSpacing;
	}

	_vm->_gfx->invalidateRect(boxRect, /* erase= */ false);
}


int SubtitleManager::getFontHeight() const {
	return _font->getFontHeight();
}

int SubtitleManager::getBoxHeight() const {
	int maxTextHeight = getFontHeight() * kMaxSubtitleLines;
	int interLineSpacingHeight = kSubtitleInterlineSpacing * (kMaxSubtitleLines - 1);
	int paddingHeight = kSubtitlePadY * 2;
	int borderHeight = kSubtitleBorderWidth * 2;
	return maxTextHeight + interLineSpacingHeight + paddingHeight + borderHeight + kSubtitleChamferHeight;
}

Common::Rect SubtitleManager::getDefaultBoxBounds() {
	int boxHeight = getBoxHeight();
	return {
		kSubtitleBoxX,
		kSubtitleBoxY,
		kSubtitleBoxX + kSubtitleBoxWidth,
		(int16) (kSubtitleBoxY + boxHeight) };
}

Common::Rect SubtitleManager::calculateBoxBoundsForVideo(const Window *videoWindow, const Common::Rect &mediaRect) {
	// The behavior is bifurcated for videos that play in the Jumpsuit view (like INN) and videos that play over
	// a blank screen like the intro and finale videos. These videos do not play in the same screen location.
	// For Jumpsuit videos, we want the subtitle box to align with where it is for non-video media.
	bool isJumpsuitVideo = false;
	for (const Window *w = videoWindow->getParent(); w != nullptr; w = w->getParent()) {
		if (dynamic_cast<const SceneViewWindow *>(w)) {
			isJumpsuitVideo = true;
			break;
		}
	}

	if (isJumpsuitVideo) {
		return getDefaultBoxBounds();
	}

	int boxHeight = getBoxHeight();
	return Common::Rect(
		mediaRect.left,
		mediaRect.bottom,
		mediaRect.right,
		mediaRect.bottom + boxHeight
	);
}

} // End of namespace Buried

