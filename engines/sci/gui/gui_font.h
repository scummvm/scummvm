/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

namespace Sci {

class SciGUIfont {
public:
	SciGUIfont(EngineState *state, SciGUIscreen *screen, GUIResourceId resourceId);
	~SciGUIfont();

	GUIResourceId getResourceId();
	byte getHeight();
	byte getCharWidth(byte chr);
	byte getCharHeight(byte chr);
	byte *getCharData(byte chr);
	void draw(int16 chr, int16 top, int16 left, byte color, byte textface);

private:
	void initData(GUIResourceId resourceId);

	EngineState *_s;
	SciGUIscreen *_screen;

	GUIResourceId _resourceId;
	byte *_resourceData;

	struct charinfo {
		byte w, h;
		int16 offset;
	};
	byte mFontH;
	uint16 mCharMax;
	charinfo* mChars;
};

} // end of namespace Sci
