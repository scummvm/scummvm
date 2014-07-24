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

#ifndef GRIM_MATERIAL_H
#define GRIM_MATERIAL_H

#include "engines/grim/object.h"

namespace Grim {

class CMap;

class Texture {
public:
	Texture() :
		_width(0), _height(0), _colorFormat(0), _bpp(0), _hasAlpha(false), _texture(nullptr), _data(nullptr), _isShared(false) {};
	int _width;
	int _height;
	int _colorFormat;
	int _bpp;
	bool _hasAlpha;
	void *_texture;
	uint8 *_data;
	bool _isShared;
};

class MaterialData {
public:
	MaterialData(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap);
	~MaterialData();

	static MaterialData *getMaterialData(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap);
	static Common::List<MaterialData *> *_materials;

	Common::String _fname;
	const ObjectPtr<CMap> _cmap;
	int _numImages;
	Texture **_textures;
	int _refCount;

private:
	void initGrim(Common::SeekableReadStream *data);
	void initEMI(Common::SeekableReadStream *data);
};

class Material : public Object {
public:
	// Load a texture from the given data.
	Material(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap, bool clamp);

	void reload(CMap *cmap);
	// Load this texture into the GL context
	virtual void select() const;

	// Set which image in an animated texture to use
	void setActiveTexture(int n);

	int getNumTextures() const;
	int getActiveTexture() const;

	const Common::String &getFilename() const;
	MaterialData *getData() const;

	virtual ~Material();

protected:
	Material();

private:
	MaterialData *_data;
	int _currImage;
	bool _clampTexture;
};

} // end of namespace Grim

#endif
