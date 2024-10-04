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

#include "twine/slideshow.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include "image/pcx.h"
#include "twine/movies.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

class Slideshow {
private:
	TwinEEngine *_engine;
	int _textY = 0;
	bool _aborted = false;
	int _lineHeight = 40;
	uint32 _pal[NUMOFCOLORS]{};

	void setPalette(const uint8 *in, int colors) {
		uint8 *paletteOut = (uint8 *)_pal;
		for (int i = 0; i < colors; i++) {
			paletteOut[0] = in[0];
			paletteOut[1] = in[1];
			paletteOut[2] = in[2];
			paletteOut[3] = 0xFF;
			paletteOut += 4;
			in += 3;
		}
		_engine->setPalette(_pal);
	}

	bool loadPCX(const Common::Path &pcx, bool onlyPalette = false) {
		Image::PCXDecoder decoder;
		Common::File file;
		if (!file.open(pcx)) {
			return false;
		}

		if (!decoder.loadStream(file)) {
			return false;
		}
		const Graphics::Surface *src = decoder.getSurface();
		if (src == nullptr) {
			return false;
		}

		Graphics::ManagedSurface &target = _engine->_frontVideoBuffer;
		target.blitFrom(*src);

		if (decoder.hasPalette()) {
			setPalette(decoder.getPalette(), decoder.getPaletteColorCount());
		}
		return true;
	}

	bool loadFont() {
		Common::File font;
		if (!font.open("LBA.FNT")) {
			return false;
		}
		_engine->_resources->_fontBufSize = (int)font.size();
		_engine->_resources->_fontPtr = (uint8 *)malloc(_engine->_resources->_fontBufSize);
		font.read(_engine->_resources->_fontPtr, _engine->_resources->_fontBufSize);

		_engine->_text->setFont(4, 8);
		return true;
	}

	void scriptCls() {
		_textY = 0;
		_engine->_screens->clearScreen();
	}

	void scriptPause(const Common::String &params) {
		_engine->_frontVideoBuffer.update();
		const int seconds = atoi(params.c_str());
		if (_engine->delaySkip(1000 * seconds)) {
			_aborted = true;
		}
	}

	void scriptColor(const Common::String &params) {
		const int color = atoi(params.c_str());
		_engine->_text->setFontColor(color);
	}

	void scriptText(const Common::String &params) {
		if (!params.empty()) {
			_pal[255] = _pal[15] = 0xffffffff;
			_engine->setPalette(_pal);
			const int32 length = _engine->_text->sizeFont(params.c_str());
			const int x = 0;
			_engine->_text->drawText(x, _textY, params.c_str());
			_engine->_frontVideoBuffer.addDirtyRect(Common::Rect(x, _textY, x + length, _textY + _lineHeight));
		}
		_textY += _lineHeight;
	}

	void scriptRText(const Common::String &params) {
		if (!params.empty()) {
			_pal[255] = _pal[15] = 0xffffffff;
			_engine->setPalette(_pal);
			const int32 length = _engine->_text->sizeFont(params.c_str());
			const int x = _engine->width() - length;
			_engine->_text->drawText(x, _textY, params.c_str());
			_engine->_frontVideoBuffer.update();
			_engine->_frontVideoBuffer.addDirtyRect(Common::Rect(x, _textY, x + length, _textY + _lineHeight));
		}
		_textY += _lineHeight;
	}

	void scriptTitle(const Common::String &params) {
		if (!params.empty()) {
			_pal[255] = _pal[15] = 0xffffffff;
			_engine->setPalette(_pal);
			const int32 length = _engine->_text->sizeFont(params.c_str());
			const int x = _engine->width() / 2 - length / 2;
			_engine->_text->drawText(x, _textY, params.c_str());
			_engine->_frontVideoBuffer.addDirtyRect(Common::Rect(x, _textY, x + length, _textY + _lineHeight));
		}
		_textY += _lineHeight;
	}

	void scriptFadeIn() {
		_engine->_screens->fadeIn(_pal);
	}

	void scriptFadeOut() {
		_engine->_screens->fadeOut(_pal);
	}

	void scriptPCX(const Common::String &params) {
		loadPCX(Common::Path(params + ".PCX"));
	}

	void scriptShow() {
		// TODO: _engine->setPalette(_pal);
		// or updateScreen?
	}

	void scriptFLA(const Common::String &params) {
		if (!_engine->_movie->playMovie(params.c_str())) {
			_aborted = true;
		}
	}

	bool playScript() {
		Common::File scriptFile;
		if (!scriptFile.open("LBA_SHOW.SHO")) {
			return false;
		}
		while (!scriptFile.eos() && !scriptFile.err()) {
			if (_aborted) {
				break;
			}
			const Common::String &line = scriptFile.readLine();
			if (line.empty()) {
				continue;
			}
			Common::String cmd;
			Common::String params;
			const size_t index = line.findFirstOf(' ');
			if (index == Common::String::npos) {
				cmd = line;
			} else {
				cmd = line.substr(0, index);
				params = line.substr(index + 1);
			}
			debug("cmd: '%s %s'", cmd.c_str(), params.c_str());
			if (cmd == "cls") {
				scriptCls();
			} else if (cmd == "text") {
				scriptText(params);
			} else if (cmd == "rtext") {
				scriptRText(params);
			} else if (cmd == "title") {
				scriptTitle(params);
			} else if (cmd == "color") {
				scriptColor(params);
			} else if (cmd == "pause") {
				scriptPause(params);
			} else if (cmd == "pcx") {
				scriptPCX(params);
			} else if (cmd == "fla") {
				scriptFLA(params);
			} else if (cmd == "fade_in") {
				scriptFadeIn();
			} else if (cmd == "fade_out") {
				scriptFadeOut();
			} else if (cmd == "show") {
				scriptShow();
			} else if (cmd == "restart") {
				return true;
			} else {
				warning("Unknown command %s (%s)", cmd.c_str(), params.c_str());
			}
		}

		return false;
	}

public:
	Slideshow(TwinEEngine *engine) : _engine(engine) {
	}

	~Slideshow() {
		free(_engine->_resources->_fontPtr);
		_engine->_resources->_fontPtr = nullptr;
	}

	void play() {
		loadFont();
		loadPCX("ADELINE.PCX", true);
		while (playScript()) {
		}
	}
};

void playSlideShow(TwinEEngine *engine) {
	Slideshow slideshow(engine);
	slideshow.play();
}

} // namespace TwinE
