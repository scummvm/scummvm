/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef SUBTITLES_H_
#define SUBTITLES_H_

#include "engines/myst3/gfx.h"

#include "common/array.h"

namespace Myst3 {

class Myst3Engine;
class ResourceDescription;

class Subtitles : public Window {
public:
	static Subtitles *create(Myst3Engine *vm, uint32 id);
	virtual ~Subtitles();

	// Window API
	Common::Rect getPosition() const override;
	Common::Rect getOriginalPosition() const override;

	void setFrame(int32 frame);
	void drawOverlay() override;

protected:
	struct Phrase {
		uint32 offset;
		int32 frame;
		Common::String string;
	};

	Subtitles(Myst3Engine *vm);

	void loadFontSettings(int32 id);
	virtual void loadResources() = 0;
	virtual bool loadSubtitles(int32 id) = 0;
	virtual void drawToTexture(const Phrase *phrase) = 0;
	void freeTexture();

	int32 checkOverridenId(int32 id);
	ResourceDescription loadText(int32 id, bool overriden);

	Myst3Engine *_vm;

	Common::Array<Phrase> _phrases;

	int32 _frame;
	Texture *_texture;

	// Font settings
	Common::String _fontFace;
	uint _fontSize;
	bool _fontBold;
	uint _surfaceHeight;
	uint _singleLineTop;
	uint _line1Top;
	uint _line2Top;
	uint _surfaceTop;
	int32 _fontCharsetCode;
};

} // End of namespace Myst3

#endif // SUBTITLES_H_
