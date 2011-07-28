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

#include "common/endian.h"

#include "engines/grim/debug.h"
#include "engines/grim/material.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/colormap.h"
#include "engines/grim/resource.h"

namespace Grim {

Common::List<MaterialData *> *MaterialData::_materials = NULL;

MaterialData::MaterialData(const Common::String &filename, const char *data, int len, CMap *cmap) :
	_fname(filename), _cmap(cmap), _refCount(1) {

	if (len < 4 || memcmp(data, "MAT ", 4) != 0)
		error("invalid magic loading texture");

	_numImages = READ_LE_UINT32(data + 12);
	/* Discovered by diffing orange.mat with pink.mat and blue.mat .
	 * Actual meaning unknown, so I prefer to use it as an enum-ish
	 * at the moment, to detect unexpected values.
	 */
	uint32 offset = READ_LE_UINT32(data + 0x4c);
	if (offset == 0x8)
		data += 16;
	else if (offset != 0)
		error("Unknown offset: %d", offset);
	_width = READ_LE_UINT32(data + 60 + _numImages * 40);
	_height = READ_LE_UINT32(data + 64 + _numImages * 40);
	_hasAlpha = READ_LE_UINT32(data + 68 + _numImages * 40);

	if (_width == 0 || _height == 0) {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("skip load texture: bad texture size (%dx%d) for texture %s", _width, _height, _fname.c_str());
		return;
	}

	data += 84 + _numImages * 40;

	g_driver->createMaterial(this, data, cmap);
}

MaterialData::~MaterialData() {
	_materials->remove(this);
	if (_materials->empty()) {
		delete _materials;
		_materials = NULL;
	}

	if (_width && _height)
		g_driver->destroyMaterial(this);
}

MaterialData *MaterialData::getMaterialData(const Common::String &filename, const char *data, int len, CMap *cmap) {
	if (!_materials) {
		_materials = new Common::List<MaterialData *>();
	}

	for (Common::List<MaterialData *>::iterator i = _materials->begin(); i != _materials->end(); ++i) {
		MaterialData *m = *i;
		if (m->_fname == filename && m->_cmap->getFilename() == cmap->getFilename()) {
			++m->_refCount;
			return m;
		}
	}

	MaterialData *m = new MaterialData(filename, data, len, cmap);
	_materials->push_back(m);
	return m;
}

Material::Material(const Common::String &filename, const char *data, int len, CMap *cmap) :
		Object(), _currImage(0) {
	_data = MaterialData::getMaterialData(filename, data, len, cmap);
}

void Material::reload(CMap *cmap) {
	Common::String fname = _data->_fname;
	--_data->_refCount;
	if (_data->_refCount < 1) {
		delete _data;
	}

	Material *m = g_resourceloader->loadMaterial(fname, cmap);
	// Steal the data from the new material and discard it.
	_data = m->_data;
	++_data->_refCount;
	delete m;
}

void Material::select() const {
	if (_data->_width && _data->_height)
		g_driver->selectMaterial(this);
}

Material::~Material() {
	--_data->_refCount;
	if (_data->_refCount < 1) {
		delete _data;
	}
}

int Material::getNumImages() const {
	return _data->_numImages;
}

int Material::getCurrentImage() const {
	return _currImage;
}

const Common::String &Material::getFilename() const {
	return _data->_fname;
}

MaterialData *Material::getData() const {
	return _data;
}

} // end of namespace Grim
