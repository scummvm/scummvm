/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_BITMAP_H
#define GRIM_BITMAP_H

#include "engines/grim/resource.h"
#include "engines/grim/object.h"

namespace Grim {

// This BitmapData class keeps the actual bitmap data and can be shared
// between Bitmap instances, by using getBitmapData.
// Bitmap still keeps the data that can change between the instances
// i.e. _x, _y and _currImage.
// They are automatically deleted if they are not used by any bitmap anymore.
class BitmapData {
public:
	BitmapData(const char *fname, const char *data, int len);
	BitmapData(const char *data, int w, int h, int bpp, const char *fname);
	BitmapData();
	~BitmapData();
	bool loadTile(const char *filename, const char *data, int len);

	static BitmapData *getBitmapData(const char *fname, const char *data, int len);
	static Common::HashMap<Common::String, BitmapData *> *_bitmaps;

	Common::String _fname;
	char **_data;
	int _numImages;
	int _width, _height, _x, _y;
	int _format;
	int _numTex;
	int _bpp;
	void *_texIds;
	bool _hasTransparency;
	char _filename[32];

	int _refCount;
};

class Bitmap : public Object {
public:
	// Construct a bitmap from the given data.
	Bitmap(const char *filename, const char *data, int len);
	Bitmap(const char *data, int width, int height, int bpp, const char *filename);
	Bitmap();

	const char *getFilename() const { return _data->_fname.c_str(); }

	void draw() const;

	// Set which image in an animated bitmap to use
	void setNumber(int n) { if ((n - 1) >= _data->_numImages) warning("Bitmap::setNumber: no anim image: %d", n); else _currImage = n; }

	int getNumImages() const { return _data->_numImages; }
	int getCurrentImage() const { return _currImage; }
	bool getHasTransparency() const { return _data->_hasTransparency; }
	int getFormat() const { return _data->_format; }
	int getWidth() const { return _data->_width; }
	int getHeight() const { return _data->_height; }
	int getX() const { return _x; }
	int getY() const { return _y; }
	void setX(int xPos) { _x = xPos; }
	void setY(int yPos) { _y = yPos; }

	char *getData(int num) const { return _data->_data[num]; }
	char *getData() const { return _data->_data[_currImage]; }
	void *getTexIds() const { return _data->_texIds; }
	int getNumTex() const { return _data->_numTex; }

	virtual ~Bitmap();

private:
	BitmapData *_data;
	int _currImage;
	int _x, _y;
};

} // end of namespace Grim

#endif
