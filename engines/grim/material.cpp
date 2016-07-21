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

#include "common/endian.h"
#include "image/tga.h"
#include "graphics/surface.h"

#include "engines/grim/grim.h"
#include "engines/grim/debug.h"
#include "engines/grim/material.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/colormap.h"
#include "engines/grim/resource.h"
#include "engines/grim/textsplit.h"

#define GRIM_MAT_FLAG_HAS_COLORMAP 1
#define GRIM_MAT_FLAG_UNKNOWN 2

namespace Grim {

Common::List<MaterialData *> *MaterialData::_materials = nullptr;

MaterialData::MaterialData(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap) :
		_fname(filename), _cmap(cmap), _refCount(1), _textures(nullptr), _ownCMap(false) {

	if (g_grim->getGameType() == GType_MONKEY4) {
		initEMI(data);
	} else {
		initGrim(data);
	}
}

void MaterialData::initGrim(Common::SeekableReadStream *data) {
	/* Main header. */
	uint32 tag = data->readUint32BE();
	if (tag != MKTAG('M','A','T',' '))
		error("Invalid header for texture %s. Expected 'MAT ', got '%c%c%c%c'", _fname.c_str(),
		                 (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);

	uint32 version = data->readUint32LE();
	if (version != 0x32) {
		Debug::warning(Debug::Materials, "Unexpected version value %08x for material %s.", version, _fname.c_str());
	}
	uint32 flags = data->readUint32LE();
	if ((flags & (GRIM_MAT_FLAG_HAS_COLORMAP | GRIM_MAT_FLAG_UNKNOWN)) != flags) {
		Debug::warning(Debug::Materials, "Unknown flags %08x for material %s.", flags, _fname.c_str());
	}
	_numImages = data->readUint32LE();
	int alt_numImages = data->readUint32LE();
	if (alt_numImages != _numImages) {
		Debug::warning(Debug::Materials, "Different image counts %i != %i for material %s.", _numImages, alt_numImages, _fname.c_str());
	}
	_textures = new Texture*[_numImages];
	/* Structure of the rest of main header is unknown. */
	data->seek(0x38, SEEK_CUR);

	/* Per-image headers, of which only a magic field is known as it affects header length. */
	for (int i = 0; i < _numImages; ++i) {
		uint32 unknown = data->readUint32LE();
		if (unknown != 0x8 && unknown != 0) {
			Debug::warning(Debug::Materials, "Unknown value %08x for texture %i header for material %s.", unknown, i, _fname.c_str());
		}
		data->seek(0x14 + (unknown == 0 ? 0 : 0x10), SEEK_CUR);
	}
	/* Bitmaps. */
	for (int i = 0; i < _numImages; ++i) {
		Texture *t = _textures[i] = new Texture();
		uint32 width, height;
		t->_width = width = data->readUint32LE();
		t->_height = height = data->readUint32LE();
		t->_hasAlpha = data->readUint32LE();
		t->_texture = nullptr;
		t->_colorFormat = BM_RGBA;
		t->_data = nullptr;
		data->seek(0x8, SEEK_CUR);
		uint32 mipmap_count = data->readUint32LE();
		if (mipmap_count == 0) {
			Debug::warning(Debug::Materials, "Invalid mipmap count %i for texture %i of material %s.", mipmap_count, i, _fname.c_str());
			break;
		}
		if (t->_width == 0 || t->_height == 0) {
			Debug::warning(Debug::Materials, "skip load texture: bad texture size (%dx%d) for texture %d of material %s",
						   t->_width, t->_height, i, _fname.c_str());
			break;
		}
		t->_data = new uint8[t->_width * t->_height];
		data->read(t->_data, t->_width * t->_height);
		/* XXX: Discarding lower-res mipmaps */
		while (--mipmap_count) {
			width >>= 1;
			height >>= 1;
			data->seek(width * height, SEEK_CUR);
		}
	}
	/* If flags & GRIM_MAT_FLAG_HAS_COLORMAP there is a color map here. */
	if (flags & GRIM_MAT_FLAG_HAS_COLORMAP) {
		_ownCMap = true;
		_cmap = new CMap(_fname, data);
	}
}

void loadTGA(Common::SeekableReadStream *data, Texture *t) {
	Image::TGADecoder *tgaDecoder = new Image::TGADecoder();
	tgaDecoder->loadStream(*data);
	const Graphics::Surface *tgaSurface = tgaDecoder->getSurface();

	t->_width = tgaSurface->w;
	t->_height = tgaSurface->h;
	t->_texture = nullptr;

	int bpp = tgaSurface->format.bytesPerPixel;
	if (bpp == 4) {
		t->_colorFormat = BM_BGRA;
		t->_bpp = 4;
		t->_hasAlpha = true;
	} else {
		t->_colorFormat = BM_BGR888;
		t->_bpp = 3;
		t->_hasAlpha = false;
	}

	assert(bpp == 3 || bpp == 4); // Assure we have 24/32 bpp

	// Allocate room for the texture.
	t->_data = new uint8[t->_width * t->_height * (bpp)];

	// Copy the texture data, as the decoder owns the current copy.
	memcpy(t->_data, tgaSurface->getPixels(), t->_width * t->_height * (bpp));

	delete tgaDecoder;
}

void MaterialData::initEMI(Common::SeekableReadStream *data) {

	if (_fname.hasSuffix(".sur")) {  // This expects that we want all the materials in the sur-file
		Common::Array<Common::String> texFileNames;
		char readFileName[64];
		TextSplitter *ts = new TextSplitter(_fname, data);
		ts->setLineNumber(2); // Skip copyright-line
		ts->expectString("version\t1.0");
		if (ts->checkString("name:"))
			ts->scanString("name:%s", 1, readFileName);

		while (!ts->checkString("END_OF_SECTION")) {
			ts->scanString("tex:%s", 1, readFileName);
			Common::String mFileName(readFileName);
			texFileNames.push_back(ResourceLoader::fixFilename(mFileName, false));
		}
		_textures = new Texture*[texFileNames.size()];
		for (uint i = 0; i < texFileNames.size(); i++) {
			Common::String name = texFileNames[i];
			if (name.hasPrefix("specialty")) {
				_textures[i] = g_driver->getSpecialtyTexturePtr(name);
			} else {
				_textures[i] = new Texture();
				Common::SeekableReadStream *texData = g_resourceloader->openNewStreamFile(texFileNames[i].c_str(), true);
				if (!texData) {
					warning("Couldn't find tex-file: %s", texFileNames[i].c_str());
					_textures[i]->_width = 0;
					_textures[i]->_height = 0;
					_textures[i]->_texture = new int(1); // HACK to avoid initializing.
					_textures[i]->_data = nullptr;
					continue;
				}
				loadTGA(texData, _textures[i]);
				delete texData;
			}
		}
		_numImages = texFileNames.size();
		delete ts;
		return;
	} else if (_fname.hasSuffix(".tga")) {
		_numImages = 1;
		_textures = new Texture*[1];
		_textures[0] = new Texture();
		loadTGA(data, _textures[0]);
		return;
	} else if (_fname.hasPrefix("specialty")) {
		_numImages = 1;
		_textures = new Texture*[1];
		_textures[0] = g_driver->getSpecialtyTexturePtr(_fname);
	} else {
		warning("Unknown material-format: %s", _fname.c_str());
	}
}

MaterialData::~MaterialData() {
	_materials->remove(this);
	if (_materials->empty()) {
		delete _materials;
		_materials = nullptr;
	}

	for (int i = 0; i < _numImages; ++i) {
		Texture *t = _textures[i];
		if (!t) continue;
		if (t->_isShared) continue; // don't delete specialty textures
		if (t->_width && t->_height && t->_texture)
			g_driver->destroyTexture(t);
		delete[] t->_data;
		delete t;
	}
	delete[] _textures;
	if (_ownCMap) {
		delete _cmap;
	}
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

Material::Material(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap, bool clamp) :
		Object(), _currImage(0) {
	_data = MaterialData::getMaterialData(filename, data, cmap);
	_clampTexture = clamp;
}

Material::Material() :
		Object(), _currImage(0), _data(nullptr), _clampTexture(false) {
}

void Material::reload(CMap *cmap) {
	Common::String fname = _data->_fname;
	--_data->_refCount;
	if (_data->_refCount < 1) {
		delete _data;
	}

	Material *m = g_resourceloader->loadMaterial(fname, cmap, _clampTexture);
	// Steal the data from the new material and discard it.
	_data = m->_data;
	++_data->_refCount;
	delete m;
}

void Material::select() const {
	Texture *t = _data->_textures[_currImage];
	if (t && t->_width && t->_height) {
		if (!t->_texture) {
			g_driver->createTexture(t, (uint8 *)t->_data, _data->_cmap, _clampTexture);
			delete[] t->_data;
			t->_data = nullptr;
		}
		g_driver->selectTexture(t);
	} else {
		warning("Can't select material: %s", getFilename().c_str());
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
