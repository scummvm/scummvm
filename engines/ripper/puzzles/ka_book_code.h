/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
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
 */

#ifndef RIPPER_PUZZLES_KA_BOOK_CODE_H
#define RIPPER_PUZZLES_KA_BOOK_CODE_H

#include "audio/mixer.h"

#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class KaBookCodePuzzle {
public:
	enum Result {
		kFailed,
		kSolved,
		kLoadFailed
	};

	explicit KaBookCodePuzzle(RipperEngine *engine);

	Result run();

private:
	bool loadAssets();
	uint measureText(const Common::String &text) const;
	void drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const;
	void drawPrompt(const Common::String &typedCode);

	RipperEngine *_engine;
	BitmapFontAsset _font;
	BitmapAssetFrame _background;
	Audio::SoundHandle _keyHandle;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_KA_BOOK_CODE_H
