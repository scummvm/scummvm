/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
		initEMI(data);
	} else {
		initGrim(data);
	}
}

void MaterialData::initGrim(Common::SeekableReadStream *data) {
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
		t->_colorFormat = BM_RGBA;
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
}

void loadTGA(Common::SeekableReadStream *data, Texture *t) {
	int descField = data->readByte();
	assert(descField == 0);	// Verify that description-field is empty
	
	data->seek(1, SEEK_CUR);
	
	int format = data->readByte();
	if (!(format == 2 || format == 10)) {
		error("Unsupported TGA-format detected: %d", format);
	}
	
	data->seek(9, SEEK_CUR);
	t->_width = data->readUint16LE();
	t->_height = data->readUint16LE();
	t->_hasAlpha = false;
	t->_texture = NULL;
	
	int bpp = data->readByte();
	if (bpp == 32) {
		t->_colorFormat = BM_RGBA;
		t->_bpp = 4;
	} else {
		t->_colorFormat = BM_BGR888;
		t->_bpp = 3;
	}
	
	uint8 desc = data->readByte();
	uint8 flipped = !(desc & 32);
	
	assert(bpp == 24 || bpp == 32); // Assure we have 24/32 bpp
	t->_data = new char[t->_width * t->_height * (bpp / 8)];
	
	if (format == 2) {
		// Since certain TGA's are flipped (relative to the tex-coords) and others not
		// We'll have to handle that here, otherwise we could just do 1.0f - texCoords
		// When drawing/loading
		if (flipped) {
			char *writePtr = t->_data + (t->_width * (t->_height - 1) * bpp / 8);
			for (int i = 0; i < t->_height; i++) {
				data->read(writePtr, t->_width * (bpp / 8));
				writePtr -= (t->_width * bpp / 8);
			}
		} else {
			data->read(t->_data, t->_width * t->_height * (bpp / 8));		
		}
	} else if (format == 10) {
		// Decode Run-Length Encoding
		char *writePtr = t->_data;
		while (!data->eos()) {
			byte head = data->readByte();
			if (head & 0x80) {
				byte num = (head & 0x7f) + 1;
				uint32 d = data->readUint32LE();
				for (int i = 0; i < num; ++i) {
					*((uint32 *)writePtr) = d;
					writePtr += 4;
				}
			} else {
				++head;
				data->read(writePtr, head*4);
				writePtr += head*4;
			}
		}
	}
}
	
void MaterialData::initEMI(Common::SeekableReadStream *data) {
	Common::Array<Common::String> texFileNames;
	char readFileName[64];

	if (_fname.hasSuffix(".sur")) {  // This expects that we want all the materials in the sur-file
		TextSplitter *ts = new TextSplitter(data);
		ts->setLineNumber(2); // Skip copyright-line
		ts->expectString("version\t1.0");
		if (ts->checkString("name:"))
			ts->scanString("name:%s", 1, readFileName);
		
		while(!ts->checkString("END_OF_SECTION")) {
			ts->scanString("tex:%s", 1, readFileName);
			Common::String mFileName(readFileName);
			texFileNames.push_back(ResourceLoader::fixFilename(mFileName, false));
		}
		Common::SeekableReadStream *texData;
		_textures = new Texture[texFileNames.size()];
		for (uint i = 0; i < texFileNames.size(); i++) {
			texData = g_resourceloader->openNewStreamFile(texFileNames[i].c_str(), true);
			if (!texData) {
				warning("Couldn't find tex-file: %s", texFileNames[i].c_str());
				_textures[i]._width = 0;
				_textures[i]._height = 0;
				_textures[i]._texture = new int(1); // HACK to avoid initializing.
				continue;
			}
			loadTGA(texData, _textures + i);
			delete texData;
		}
		_numImages = texFileNames.size();
		delete ts;
		return;
	} else if(_fname.hasSuffix(".tga")) {
		_numImages = 1;
		_textures = new Texture();
		loadTGA(data, _textures);
		//	texFileNames.push_back(filename);
		return;
		
	} else {
		warning("Unknown material-format: %s", _fname.c_str());
	}
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
		if (m->_fname == filename && g_grim->getGameType() == GType_MONKEY4) {
			++m->_refCount;
			return m;
		}
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

Material::Material() :
		Object(), _currImage(0), _data(NULL) {
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
	if (_data) {
		--_data->_refCount;
		if (_data->_refCount < 1) {
			delete _data;
		}
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
