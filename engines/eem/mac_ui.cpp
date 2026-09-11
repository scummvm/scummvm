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

#include "common/ptr.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "image/pict.h"

#include "eem/audio.h"
#include "eem/eem.h"
#include "eem/music.h"
#include "eem/site.h"

namespace EEM {

namespace {

struct MacPuzzleItem {
	Common::Rect rect;
	byte type;
	uint16 picture;
	Common::String text;
};

Common::Rect readPuzzleRect(Common::SeekableReadStream &stream) {
	const int top = stream.readSint16BE();
	const int left = stream.readSint16BE();
	const int bottom = stream.readSint16BE();
	const int right = stream.readSint16BE();
	return Common::Rect(left, top, right, bottom);
}

bool loadPuzzleDialog(uint16 id, Common::Rect &rect, Common::Array<MacPuzzleItem> &items) {
	Common::ScopedPtr<Common::SeekableReadStream> dialog(
		openMacResource(Common::Path("EEM London CD"), MKTAG('D', 'L', 'O', 'G'), id));
	if (!dialog || dialog->size() < 20)
		return false;
	rect = readPuzzleRect(*dialog);
	dialog->seek(18);
	Common::ScopedPtr<Common::SeekableReadStream> list(
		openMacResource(Common::Path("EEM London CD"), MKTAG('D', 'I', 'T', 'L'), dialog->readUint16BE()));
	if (!list || list->size() < 2)
		return false;
	const uint count = list->readUint16BE() + 1;
	for (uint i = 0; i < count; ++i) {
		if (list->size() - list->pos() < 14)
			return false;
		list->skip(4);
		MacPuzzleItem item;
		item.rect = readPuzzleRect(*list);
		item.rect.translate(rect.left, rect.top);
		item.type = list->readByte();
		const uint size = list->readByte();
		if (size > list->size() - list->pos())
			return false;
		item.picture = 0;
		if ((item.type & 0x7f) == 64 && size == 2) {
			item.picture = list->readUint16BE();
		} else {
			for (uint j = 0; j < size; ++j)
				item.text += (char)list->readByte();
		}
		if (size & 1)
			list->skip(1);
		items.push_back(item);
	}
	return !list->err();
}

bool loadPuzzlePicture(uint16 id, Graphics::ManagedSurface &surface, Graphics::Palette &palette,
					   bool usePicturePalette = false) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		openMacResource(Common::Path("EEM London CD"), MKTAG('P', 'I', 'C', 'T'), id));
	Image::PICTDecoder decoder;
	if (!stream || !decoder.loadStream(*stream) || !decoder.getSurface())
		return false;
	const Graphics::Palette &sourcePalette = decoder.getPalette();
	if (usePicturePalette && sourcePalette.size())
		palette.set(sourcePalette.data(), 0, sourcePalette.size());
	Graphics::Surface *converted = decoder.getSurface()->convertTo(
		Graphics::PixelFormat::createFormatCLUT8(), sourcePalette.data(), sourcePalette.size(),
		palette.data(), palette.size());
	if (!converted)
		return false;
	surface.copyFrom(*converted);
	converted->free();
	delete converted;
	return true;
}

void drawPuzzleButton(Graphics::ManagedSurface &surface, const Common::Rect &rect, byte color) {
	surface.hLine(rect.left + 3, rect.top, rect.right - 4, color);
	surface.hLine(rect.left + 3, rect.bottom - 1, rect.right - 4, color);
	surface.vLine(rect.left, rect.top + 3, rect.bottom - 4, color);
	surface.vLine(rect.right - 1, rect.top + 3, rect.bottom - 4, color);
	surface.drawLine(rect.left, rect.top + 3, rect.left + 3, rect.top, color);
	surface.drawLine(rect.right - 4, rect.top, rect.right - 1, rect.top + 3, color);
	surface.drawLine(rect.left, rect.bottom - 4, rect.left + 3, rect.bottom - 1, color);
	surface.drawLine(rect.right - 4, rect.bottom - 1, rect.right - 1, rect.bottom - 4, color);
}

} // namespace

// Mac London CODE 6:5554/5a1a; the Mac release has no TRAVEL*.ANM files.
bool EEMEngine::playMacLondonTravelAnimation(uint8 travelKind) {
	static const struct TravelScene {
		uint16 background, palette;
		uint16 body[2], animation[2], vehicle[2][2];
		int partnerX[2], partnerY, vehicleY[2];
		int step;
		uint frameDelay;
	} kScenes[] = {
		{ 0x314, 0x38, { 0x315, 0x316 }, { 0x20, 0x21 },
			{ { 0x2bf, 0x2bf }, { 0x2bf, 0x2bf } }, { 32, 32 }, 88, { 307, 30 }, 9, 48 },
		{ 0x2bc, 0x37, { 0x2ba, 0x2b9 }, { 0x0e, 0x23 },
			{ { 0x2b7, 0x2b8 }, { 0x2b7, 0x2b8 } }, { 0, 1 }, 88, { 311, 25 }, 8, 60 },
		{ 0x2ae, 0x41, { 0, 0 }, { 0x22, 0x22 },
			{ { 0x2b4, 0x2b3 }, { 0x2b2, 0x2b1 } }, { 1, 1 }, 84, { 311, 15 }, 12, 80 }
	};
	if (travelKind == 0 || travelKind > ARRAYSIZE(kScenes))
		return false;
	const TravelScene &scene = kScenes[travelKind - 1];
	const uint partner = _partner == kPartnerJake ? 0 : 1;
	const bool train = travelKind == 1;
	Picture background, body, vehicles[2];
	Animation animation, doors;
	byte palette[kPalSize];
	if (!getSitePalette(scene.palette, palette) ||
		!_picsArchive.getPicture(scene.background, background) ||
		!_picsArchive.getPicture(scene.vehicle[partner][0], vehicles[0]) ||
		!_picsArchive.getPicture(scene.vehicle[partner][1], vehicles[1]) ||
		!_aniArchive.loadAnimation(scene.animation[partner], animation) || animation.empty() ||
		(scene.body[partner] && !_picsArchive.getPicture(scene.body[partner], body)) ||
		(train && (!_aniArchive.loadAnimation(0x1f, doors) || doors.empty()))) {
		warning("Mac London travel: unable to load scene %u", travelKind);
		return false;
	}

	const bool cursorVisible = CursorMan.isVisible();
	CursorMan.showMouse(false);
	fadeCurrentPaletteToBlack();
	const Graphics::Palette colors(palette, 256);
	const MacSpritePaletteMap paletteMap = {
		colors.findBestColor(255, 255, 255), colors.findBestColor(0, 0, 0)
	};
	Graphics::ManagedSurface base(screenWidth(), screenHeight(), Graphics::PixelFormat::createFormatCLUT8());
	base.simpleBlitFrom(background.surface);
	remapMacSurfaceEndpoints(base, paletteMap);
	if (scene.body[partner])
		blitMacMaskedSurface(base.surfacePtr(), body, 0, 87, false, paletteMap);
	Graphics::ManagedSurface frame(base.w, base.h, base.format);
	uint partnerFrame = train ? 0 : MIN<uint>(1, animation.size() - 1);
	uint doorFrame = train ? doors.size() - 1 : 0;
	auto drawFrame = [&](uint pass, int x) {
		frame.simpleBlitFrom(base);
		blitMacAnimFrameAnchored(frame.surfacePtr(), vehicles[pass], x, scene.vehicleY[pass], paletteMap);
		blitMacAnimFrameAnchored(frame.surfacePtr(), animation[partnerFrame],
								scene.partnerX[partner], scene.partnerY, paletteMap);
		if (train)
			blitMacAnimFrameAnchored(frame.surfacePtr(), doors[doorFrame], 311, 155, paletteMap);
		g_system->copyRectToScreen(frame.getPixels(), frame.pitch, 0, 0, frame.w, frame.h);
		g_system->updateScreen();
	};
	drawFrame(0, (base.w + vehicles[0].surface.w) & ~3);
	fadePaletteFromBlack(palette);
	startLondonTravelMusic(travelKind);

	// Pace the original's CPU-dependent travel loop at 30 Hz.
	const uint kTravelUpdatesPerSecond = 30;
	const uint32 startMs = g_system->getMillis();
	uint tick = 1;
	uint frameClock = 0;
	bool skipped = false;
	for (uint pass = 0; pass < 2 && !skipped && !shouldQuit(); ++pass) {
		const int width = vehicles[pass].surface.w;
		int x = (pass ? -width : base.w + width) & ~3;
		const int step = pass ? scene.step : -scene.step;
		const uint frameDelay = train && pass ? 128 : scene.frameDelay;
		while (!skipped && !shouldQuit()) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER ||
					event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN)
					skipped = true;
			}
			if (skipped || shouldQuit())
				break;
			const int32 remaining = startMs + tick * 1000 / kTravelUpdatesPerSecond - g_system->getMillis();
			if (remaining > 0) {
				g_system->delayMillis(MIN<int32>(remaining, 10));
				continue;
			}
			++tick;
			x += step;
			drawFrame(pass, x);
			frameClock += 6;
			if (frameClock > frameDelay) {
				if (train) {
					if (!pass) {
						if (doorFrame)
							--doorFrame;
						if (!doorFrame)
							partnerFrame = (partnerFrame + 1) % animation.size();
					} else {
						if (x < 520)
							partnerFrame = (partnerFrame + 1) % animation.size();
						if (x > 500)
							doorFrame = MIN<uint>(doorFrame + 1, doors.size() - 1);
					}
				} else if (pass) {
					partnerFrame = MIN<uint>(partnerFrame + 1, animation.size() - 1);
				} else {
					partnerFrame = (partnerFrame + 1) % animation.size();
				}
				frameClock = 0;
			}
			if (pass ? x > base.w : x < -width)
				break;
		}
	}
	if (!shouldQuit())
		fadeCurrentPaletteToBlack();
	finishTravelMusic(skipped);
	CursorMan.showMouse(cursorVisible);
	return true;
}

// Mac London CODE 7:4598 uses DLOG/DITL controls and PICT resources.
bool EEMEngine::doMacLondonPuzzle(Common::SeekableReadStream &stream) {
	const uint type = stream.readUint16BE();
	const uint16 dialogId = stream.readUint16BE();
	const uint16 backgroundId = stream.readUint16BE();
	stream.skip(2);
	const uint pictureCount = stream.readUint16BE();
	if (type > 2 || pictureCount > 10 || stream.size() - stream.pos() < pictureCount * 6 + 6)
		return false;
	Common::Array<uint16> pictureIds;
	for (uint i = 0; i < pictureCount; ++i) {
		pictureIds.push_back(stream.readUint16BE());
		stream.skip(4);
	}
	stream.skip(6);
	const Common::String question = parseString(stream.readLine(), _playerName, _partner);
	Common::String answer;
	Common::Array<Common::Rect> selectionRects;
	if (type == 0) {
		stream.skip(8);
		answer = stream.readLine();
	} else {
		const uint count = stream.readUint16BE();
		if (count > pictureCount || stream.size() - stream.pos() < count * 8)
			return false;
		for (uint i = 0; i < count; ++i) {
			if (type == 1)
				selectionRects.push_back(readPuzzleRect(stream));
			else
				stream.skip(8);
		}
	}
	Common::Rect dialogRect;
	Common::Array<MacPuzzleItem> items;
	if (stream.err() || !loadPuzzleDialog(dialogId, dialogRect, items) || items.size() < 5) {
		warning("Unable to load Mac London puzzle dialog %u", dialogId);
		return false;
	}
	for (uint i = 0; i < selectionRects.size(); ++i)
		selectionRects[i].translate(dialogRect.left, dialogRect.top);

	byte oldPalette[kPalSize];
	g_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	Graphics::Palette palette(oldPalette, 256);
	Graphics::ManagedSurface background;
	if (!loadPuzzlePicture(backgroundId, background, palette, true)) {
		warning("Mac London puzzle PICT %u missing", backgroundId);
		return false;
	}
	const byte black = palette.findBestColor(0, 0, 0);
	const byte white = palette.findBestColor(255, 255, 255);
	Graphics::ManagedSurface saved(screenWidth(), screenHeight(), Graphics::PixelFormat::createFormatCLUT8());
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return false;
	saved.simpleBlitFrom(*screen);
	g_system->unlockScreen();
	Graphics::ManagedSurface base;
	Graphics::Surface *converted = saved.surfacePtr()->convertTo(
		Graphics::PixelFormat::createFormatCLUT8(), oldPalette, 256, palette.data(), palette.size());
	base.copyFrom(*converted);
	converted->free();
	delete converted;
	base.fillRect(dialogRect, white);
	Common::Rect border = dialogRect;
	border.grow(1);
	base.frameRect(border, black);
	for (uint i = 0; i < items.size(); ++i) {
		if (!items[i].picture)
			continue;
		Graphics::ManagedSurface picture;
		if (items[i].picture == backgroundId)
			picture.copyFrom(background);
		else if (!loadPuzzlePicture(items[i].picture, picture, palette))
			continue;
		base.blitFrom(picture, Common::Rect(picture.w, picture.h), items[i].rect);
	}
	Common::Array<Graphics::ManagedSurface> selectionPictures;
	selectionPictures.resize(selectionRects.size());
	for (uint i = 0; i < selectionPictures.size(); ++i)
		loadPuzzlePicture(pictureIds[i], selectionPictures[i], palette);
	Graphics::MacFontManager fontManager(0, Common::EN_ANY);
	const Graphics::Font *font = fontManager.getFont(Graphics::MacFont(Graphics::kMacFontSystem, 12));
	const int lineHeight = font->getFontHeight();
	Common::Array<uint> controls;
	for (uint i = 0; i < items.size(); ++i) {
		if (!(items[i].type & 0x80))
			controls.push_back(i);
	}
	int selected = -1;
	uint focus = 0;
	Common::String input;
	uint caret = 0;
	bool accepted = false;
	bool done = false;
	setInteractiveMouseCursor(false);
	g_system->getPaletteManager()->setPalette(palette.data(), 0, 256);
	if (type == 0)
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	while (!done && !shouldQuit()) {
		Graphics::ManagedSurface frame;
		frame.copyFrom(base);
		if (selected >= 0 && (uint)selected < selectionPictures.size() && !selectionPictures[selected].empty())
			frame.blitFrom(selectionPictures[selected], Common::Rect(selectionPictures[selected].w,
				selectionPictures[selected].h), selectionRects[selected]);
		for (uint i = 0; i < items.size(); ++i) {
			const MacPuzzleItem &item = items[i];
			const byte itemType = item.type & 0x7f;
			const Common::Rect &r = item.rect;
			if (itemType == 4) {
				drawPuzzleButton(frame, r, black);
				font->drawString(frame.surfacePtr(), item.text, r.left,
					r.top + (r.height() - lineHeight) / 2, r.width(), black, Graphics::kTextAlignCenter);
				if (i == 0 && (type != 0 || !input.empty())) {
					Common::Rect ring = r;
					ring.grow(3);
					drawPuzzleButton(frame, ring, black);
				}
			} else if (itemType == 6) {
				frame.drawEllipse(r.left, r.top + 3, r.left + 11, r.top + 14, black, false);
				if (selected == (int)i - 3)
					frame.drawEllipse(r.left + 3, r.top + 6, r.left + 8, r.top + 11, black, true);
				font->drawString(frame.surfacePtr(), item.text, r.left + 18, r.top, r.width() - 18, black);
			} else if (itemType == 8) {
				Common::Array<Common::String> lines;
				font->wordWrapText(item.text == "^0" ? question : item.text, r.width(), lines);
				for (uint j = 0; j < lines.size(); ++j)
					font->drawString(frame.surfacePtr(), lines[j], r.left, r.top + j * lineHeight, r.width(), black);
			} else if (itemType == 16) {
				frame.fillRect(r, white);
				Common::Rect edge = r;
				edge.grow(3);
				frame.frameRect(edge, black);
				font->drawString(frame.surfacePtr(), input, r.left, r.top, r.width(), black);
				if ((g_system->getMillis() / 400) & 1) {
					const int x = MIN<int>(r.right - 1, r.left + font->getStringWidth(input.substr(0, caret)));
					frame.vLine(x, r.top, r.top + lineHeight - 1, black);
				}
			}
		}
		g_system->copyRectToScreen(frame.getPixels(), frame.pitch, 0, 0, frame.w, frame.h);
		g_system->updateScreen();
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			int clicked = -1;
			if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				done = true;
				break;
			} else if (event.type == Common::EVENT_KEYDOWN) {
				const Common::KeyCode key = event.kbd.keycode;
				if (key == Common::KEYCODE_ESCAPE)
					clicked = 1;
				else if (key == Common::KEYCODE_RETURN || key == Common::KEYCODE_KP_ENTER)
					clicked = 0;
				else if (key == Common::KEYCODE_TAB && !controls.empty()) {
					focus = (focus + ((event.kbd.flags & Common::KBD_SHIFT) ? controls.size() - 1 : 1)) % controls.size();
					const Common::Rect &r = items[controls[focus]].rect;
					g_system->warpMouse((r.left + r.right) / 2, (r.top + r.bottom) / 2);
				} else if (type == 0) {
					if (key == Common::KEYCODE_BACKSPACE && caret > 0)
						input.deleteChar(--caret);
					else if (key == Common::KEYCODE_DELETE && caret < input.size())
						input.deleteChar(caret);
					else if (key == Common::KEYCODE_LEFT && caret > 0)
						--caret;
					else if (key == Common::KEYCODE_RIGHT && caret < input.size())
						++caret;
					else if (event.kbd.ascii >= ' ' && event.kbd.ascii < 127 && input.size() < answer.size() + 3)
						input.insertChar((char)event.kbd.ascii, caret++);
				} else if (key == Common::KEYCODE_SPACE && !controls.empty())
					clicked = controls[focus];
			} else if (event.type == Common::EVENT_LBUTTONDOWN) {
				for (uint i = 0; i < controls.size(); ++i) {
					if (items[controls[i]].rect.contains(event.mouse)) {
						focus = i;
						clicked = controls[i];
						break;
					}
				}
			}
			if (clicked == 0 && (type != 0 || !input.empty())) {
				accepted = true;
				done = true;
			} else if (clicked == 1) {
				done = true;
			} else if (clicked >= 2 && type != 0) {
				const int choice = clicked - (type == 1 ? 2 : 3);
				if (choice >= 0 && (uint)choice < pictureCount)
					selected = choice;
			}
			if (done)
				break;
		}
		g_system->delayMillis(15);
	}
	if (type == 0)
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	g_system->getPaletteManager()->setPalette(oldPalette, 0, 256);
	g_system->copyRectToScreen(saved.getPixels(), saved.pitch, 0, 0, saved.w, saved.h);
	input.trim();
	input.toUppercase();
	const bool correct = accepted && (type == 0 ? input == answer : selected == 0);
	if (!correct && !shouldQuit()) {
		const byte *kd = _mystery.kdTextIndex();
		const uint16 text = kd ? READ_LE_UINT16(kd + 12) : 0xffff;
		if (text != 0xffff) {
			if (_music && _musicOn)
				_music->playMus(37, false);
			drawKDBalloonOverCurrentScreen(parseString(_mystery.textAt(text), _playerName, _partner));
			if (_audio)
				_audio->sayKDDigital(kd, 6, _partner);
			waitForInput(0xffffffffu);
			if (_audio)
				_audio->stopSpool();
		}
	}
	stopMusic();
	g_system->copyRectToScreen(saved.getPixels(), saved.pitch, 0, 0, saved.w, saved.h);
	g_system->updateScreen();
	setInteractiveMouseCursor(false);
	return correct;
}

} // namespace EEM
