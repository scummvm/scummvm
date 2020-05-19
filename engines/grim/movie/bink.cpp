/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/archive.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/substream.h"
#include "engines/grim/grim.h"
#include "engines/grim/localize.h"
#include "engines/grim/textobject.h"
#include "engines/grim/textsplit.h"
#include "graphics/surface.h"
#include "video/bink_decoder.h"

#include "engines/grim/movie/bink.h"

#ifdef USE_BINK

namespace Grim {

MoviePlayer *CreateBinkPlayer(bool demo) {
	return new BinkPlayer(demo);
}

BinkPlayer::BinkPlayer(bool demo) : MoviePlayer(), _demo(demo) {
	_videoDecoder = new Video::BinkDecoder();
	_videoDecoder->setDefaultHighColorFormat(Graphics::PixelFormat(4, 8, 8, 8, 0, 8, 16, 24, 0));
	_subtitleIndex = _subtitles.begin();
}

bool BinkPlayer::bikCheck(Common::SeekableReadStream *stream, uint32 pos) {
	stream->seek(pos);
	uint32 tag = stream->readUint32BE();
	return (tag & 0xFFFFFF00) == MKTAG('B', 'I', 'K', 0);
}

void BinkPlayer::deinit() {
	g_grim->setMovieSubtitle(nullptr);
	MoviePlayer::deinit();
}

void BinkPlayer::handleFrame() {
	MoviePlayer::handleFrame();

	if (!_showSubtitles || _subtitleIndex == _subtitles.end())
		return;

	unsigned int startFrame, endFrame, curFrame;
	startFrame = _subtitleIndex->_startFrame;
	endFrame = _subtitleIndex->_endFrame;
	curFrame = _videoDecoder->getCurFrame();
	if (startFrame <= curFrame && curFrame <= endFrame) {
		if (!_subtitleIndex->active) {
			TextObject *textObject = new TextObject();
			textObject->setDefaults(&g_grim->_sayLineDefaults);
			Color c(255, 255, 255);
			textObject->setFGColor(c);
			textObject->setIsSpeech();
			if (g_grim->getMode() == GrimEngine::SmushMode) {
				// TODO: How to center exactly and put the text exactly
				// at the bottom  even if there are multiple lines?
				textObject->setX(640 / 2);
				textObject->setY(40);
			}
			textObject->setText(g_localizer->localize(_subtitleIndex->_textId.c_str()), false);
			g_grim->setMovieSubtitle(textObject);
			_subtitleIndex->active = true;
		}
	} else if (endFrame < curFrame) {
		if (_subtitleIndex->active) {
			g_grim->setMovieSubtitle(nullptr);
			_subtitleIndex->active = false;
			_subtitleIndex++;
		}
	}
}

bool BinkPlayer::loadFile(const Common::String &filename) {
	_fname = filename;

	if (_demo) {
		Common::String subname = filename + ".sub";
		// The demo uses a .lab suffix
		_fname = filename + ".lab";
		bool ret = MoviePlayer::loadFile(_fname);

		// Load subtitles from adjacent .sub file, if present
		Common::SeekableReadStream *substream = SearchMan.createReadStreamForMember(subname);
		if (substream) {
			TextSplitter tsSub("", substream);
			while (!tsSub.isEof()) {
				unsigned int start, end;
				char textId[256];

				// extract single subtitle entry
				tsSub.scanString("%d\t%d\t%s", 3, &start, &end, textId);

				Subtitle st(start, end, textId);
				_subtitles.push_back(st);
			}
			delete substream;
			_subtitleIndex = _subtitles.begin();
		}

		return ret;
	}

	_fname += ".m4b";

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(_fname);
	if (!stream) {
		warning("BinkPlayer::loadFile(): Can't create stream for: %s", _fname.c_str());
		return false;
	}

	// set the default start of the bink video in case there is no SMUSH header
	uint32 startBinkPos = 0x0;

	// clear existing subtitles
	_subtitles.clear();

	char header[6];
	// read the first 5 bytes of the header
	stream->read(header, 5);
	header[5] = 0;

	if (!strcmp(header, "SMUSH")) {
		// handle SMUSH header
		unsigned char smushHeader[0x2000];

		// read the first part
		uint32 consumed = 16;
		stream->read(smushHeader, consumed);

		// decode the first part
		for (unsigned int i = 0; i < consumed; i++) {
			smushHeader[i] ^= 0xd2;
		}

		Common::MemoryReadStream msStart(smushHeader, consumed);
		TextSplitter tsStart("", &msStart);

		// extract the length / the start of the following BINK header
		tsStart.scanString("%d", 1, &startBinkPos);

		assert(startBinkPos < sizeof(smushHeader));

		// read the rest (5 bytes less because of the string "SMUSH" at the beginning)
		stream->read(smushHeader+consumed, startBinkPos - consumed - 5);

		// decode the reset
		for (unsigned int i = consumed; i < startBinkPos - 5; i++) {
			smushHeader[i] ^= 0xd2;
		}
		consumed = startBinkPos - 5;

		Common::MemoryReadStream msSmush(smushHeader, consumed);
		TextSplitter tsSmush("", &msSmush);

		// skip the first line which contains the length
		tsSmush.nextLine();

		tsSmush.expectString("BEGINDATA");
		while (!tsSmush.checkString("ENDOFDATA")) {
			unsigned int start, end;
			char textId[256];

			// extract single subtitle entry
			tsSmush.scanString("%d\t%d\t%s", 3, &start, &end, textId);

			Subtitle st(start, end, textId);
			_subtitles.push_back(st);
		}
		tsSmush.expectString("ENDOFDATA");
	}

	// set current subtitle index to the first subtitle
	_subtitleIndex = _subtitles.begin();

	if (!bikCheck(stream, startBinkPos)) {
		warning("BinkPlayer::loadFile(): Could not find BINK header for: %s", _fname.c_str());
		delete stream;
		return false;
	}

	Common::SeekableReadStream *bink = nullptr;
	bink = new Common::SeekableSubReadStream(stream, startBinkPos, stream->size(), DisposeAfterUse::YES);
	return _videoDecoder->loadStream(bink);
}

} // end of namespace Grim

#endif // USE_BINK
