/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#ifndef GRIM_MATERIAL_H
#define GRIM_MATERIAL_H

#include "engines/grim/object.h"

namespace Grim {

class CMap;

class Texture {
public:
	int _width;
	int _height;
	bool _hasAlpha;
	void *_texture;
	char *_data;
};

class MaterialData {
public:
	MaterialData(const Common::String &filename, const char *data, int len, CMap *cmap);
	~MaterialData();

	static MaterialData *getMaterialData(const Common::String &filename, const char *data, int len, CMap *cmap);
	static Common::List<MaterialData *> *_materials;

	Common::String _fname;
	const ObjectPtr<CMap> _cmap;
	int _numImages;
	Texture *_textures;
	int _refCount;

private:
	void initGrim(const Common::String &filename, const char *data, int len, CMap *cmap);
	void initEMI(const Common::String &filename, const char *data, int len);
};

class Material : public Object {
public:
	// Load a texture from the given data.
	Material(const Common::String &filename, const char *data, int len, CMap *cmap);

	void reload(CMap *cmap);
	// Load this texture into the GL context
	void select() const;

	// Set which image in an animated texture to use
	void setActiveTexture(int n);

	int getNumTextures() const;
	int getActiveTexture() const;

	const Common::String &getFilename() const;
	MaterialData *getData() const;

	~Material();

private:
	MaterialData *_data;
	int _currImage;
};

} // end of namespace Grim

#endif
