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

#include "engines/grim/grim.h"
#include "engines/grim/debug.h"
#include "engines/grim/material.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/colormap.h"
#include "engines/grim/resource.h"
#include "engines/grim/textsplit.h"

namespace Grim {

Common::List<MaterialData *> *MaterialData::_materials = NULL;

MaterialData::MaterialData(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap) :
	_fname(filename), _cmap(cmap), _refCount(1) {

	if (g_grim->getGameType() == GType_MONKEY4) {
		initEMI(filename, data);
	} else {
		initGrim(filename, data, cmap);
	}
}

void MaterialData::initGrim(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap) {
	uint32 tag = data->readUint32BE();
	if (tag != MKTAG('M','A','T',' '))
		error("invalid magic loading texture");

	data->seek(12, SEEK_SET);
	_numImages = data->readUint32LE();
	_textures = new Texture[_numImages];
	/* Discovered by diffing orange.mat with pink.mat and blue.mat .
	 * Actual meaning unknown, so I prefer to use it as an enum-ish
	 * at the moment, to detect unexpected values.
	 */
	data->seek(0x4c, SEEK_SET);
	uint32 offset = data->readUint32LE();
	if (offset == 0x8)
		offset = 16;
	else if (offset != 0)
		error("Unknown offset: %d", offset);

	data->seek(60 + _numImages * 40 + offset, SEEK_SET);
	for (int i = 0; i < _numImages; ++i) {
		Texture *t = _textures + i;
		t->_width = data->readUint32LE();
		t->_height = data->readUint32LE();
		t->_hasAlpha = data->readUint32LE();
		t->_texture = NULL;
		t->_data = NULL;
		if (t->_width == 0 || t->_height == 0) {
			Debug::warning(Debug::Materials, "skip load texture: bad texture size (%dx%d) for texture %d of material %s",
						t->_width, t->_height, i, _fname.c_str());
			break;
		}
		t->_data = new char[t->_width * t->_height];
		data->seek(12, SEEK_CUR);
		data->read(t->_data, t->_width * t->_height);
	}
	delete data;
}

void MaterialData::initEMI(const Common::String &filename, Common::SeekableReadStream *data) {
	Common::Array<Common::String> texFileNames;
	char *readFileName = new char[64];

	if (filename.hasSuffix(".sur")) {  // This expects that we want all the materials in the sur-file
		TextSplitter *ts = new TextSplitter(data);
		ts->setLineNumber(1); // Skip copyright-line
		ts->expectString("VERSION 1.0");
		while(!ts->checkString("END_OF_SECTION")) {
			ts->scanString("TEX:\t\t\t%s", 1, readFileName);
			Common::String mFileName(readFileName);
			texFileNames.push_back(mFileName);
		}
		for (uint i = 0; i < texFileNames.size(); i++) {
			warning("SUR-file texture: %s", texFileNames[i].c_str());
			// TODO: Add the necessary loading here.
		}
		_numImages = texFileNames.size();
	} if(!filename.hasSuffix(".tga")) {
		_numImages = 1;
		texFileNames.push_back(filename);
	} else {
		warning("Unknown material-format: %s", filename.c_str());
	}
	return; // Leave the rest till we have models to put materials on.

	//TODO: The following code was written when data was a data buffer, instead a stream
	//but it wasn't adapetd since there wasn't a way to check it
	/*int format = data[1];
	assert(format == 2);	// Verify that we have uncompressed TGA (2)
	data += 12;
	_numImages = 1;
	_textures = new Texture();
	_textures->_width = READ_LE_UINT16(data);
	_textures->_height = READ_LE_UINT16(data + 2);
	_textures->_hasAlpha = false;
	int bpp = data[4];
	assert(bpp == 24); // Assure we have 24 bpp
	data += 6;
	g_driver->createMaterial(_textures, data, 0);*/
}

MaterialData::~MaterialData() {
	_materials->remove(this);
	if (_materials->empty()) {
		delete _materials;
		_materials = NULL;
	}

	for (int i = 0; i < _numImages; ++i) {
		Texture *t = _textures + i;
		if (t->_width && t->_height && t->_texture)
			g_driver->destroyMaterial(t);
		delete[] t->_data;
	}
	delete[] _textures;
}

MaterialData *MaterialData::getMaterialData(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap) {
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

	MaterialData *m = new MaterialData(filename, data, cmap);
	_materials->push_back(m);
	return m;
}

Material::Material(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap) :
		Object(), _currImage(0) {
	_data = MaterialData::getMaterialData(filename, data, cmap);
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
	Texture *t = _data->_textures + _currImage;
	if (t->_width && t->_height) {
		if (!t->_texture) {
			g_driver->createMaterial(t, t->_data, _data->_cmap);
			delete[] t->_data;
			t->_data = NULL;
		}
		g_driver->selectMaterial(t);
	}
}

Material::~Material() {
	--_data->_refCount;
	if (_data->_refCount < 1) {
		delete _data;
	}
}

void Material::setActiveTexture(int n) {
	_currImage = n;
}

int Material::getNumTextures() const {
	return _data->_numImages;
}

int Material::getActiveTexture() const {
	return _currImage;
}

const Common::String &Material::getFilename() const {
	return _data->_fname;
}

MaterialData *Material::getData() const {
	return _data;
}

} // end of namespace Grim
