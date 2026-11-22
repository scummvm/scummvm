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

#ifndef NGI_INPUT_H
#define NGI_INPUT_H

namespace NGI {

class Texture;

void setInputDisabled(bool state);

struct Cursor {
	int pictureId;
	Texture *picture;
	int ofsX;
	int ofsY;
	int itemPictureOffsX;
	int itemPictureOffsY;
	int width;
	int height;

	Cursor() { memset(this, 0, sizeof(*this)); }
};

typedef Common::Array<Cursor *> CursorsArray;

class CursorController {
	//CObject obj;
	int _flag;
	int _inputFlags;
	int _cursorHandle;
	int _hCursor;
	int _field_14;
	int _cursorId;
	int _cursorIndex;
	CursorsArray _cursorsArray;
	Common::Rect _cursorBounds;
	Texture *_cursorItemPicture;

 public:
	CursorController();
	~CursorController();

	void setInputDisabled(bool state);
	void addCursor(Cursor *cursor);
	void setCursorMode(bool mode);

	void render(int x, int y);
	void setCursor(int id);

	void setObject(Texture *pic) { _cursorItemPicture = pic; }
};

} // End of namespace NGI

#endif /* NGI_INPUT_H */
