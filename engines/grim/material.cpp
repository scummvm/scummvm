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

#include "common/endian.h"

#include "image/tga.h"
#include "image/png.h"

#include "graphics/blit.h"
#include "graphics/surface.h"

#include "engines/grim/grim.h"
#include "engines/grim/debug.h"
#include "engines/grim/material.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/colormap.h"
#include "engines/grim/resource.h"
#include "engines/grim/textsplit.h"

namespace Grim {

Common::List<MaterialData *> *MaterialData::_materials = nullptr;

MaterialData::MaterialData(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap) :
		_fname(filename), _cmap(cmap), _refCount(1), _textures(nullptr) {

	if (g_grim->getGameType() == GType_MONKEY4) {
		initEMI(data);
	} else {
		initGrim(data);
	}
}

static void loadImage(Image::ImageDecoder *decoder, Texture *t) {
#ifdef SCUMM_BIG_ENDIAN
	const Graphics::PixelFormat format_3bpp(3, 8, 8, 8, 0, 16, 8,  0, 0);
	const Graphics::PixelFormat format_4bpp(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	const Graphics::PixelFormat format_3bpp(3, 8, 8, 8, 0, 0, 8, 16, 0);
	const Graphics::PixelFormat format_4bpp(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif

	const Graphics::Surface *surface = decoder->getSurface();

	t->_width = surface->w;
	t->_height = surface->h;
	t->_texture = nullptr;

	int bpp = surface->format.bytesPerPixel;
	assert(decoder->hasPalette() || bpp == 3 || bpp == 4); // Assure we have 8/24/32 bpp

	// Allocate room for the texture.
	if (bpp == 4) {
		t->_bpp = bpp;
		t->_hasAlpha = true;
	} else {
		t->_bpp = 3;
		t->_hasAlpha = false;
	}
	t->_data = new uint8[t->_width * t->_height * t->_bpp];

	// Copy the texture data, as the decoder owns the current copy.
	if (decoder->hasPalette()) {
		uint32 map[256];
		Graphics::convertPaletteToMap(map,
		                              decoder->getPalette(),
		                              decoder->getPaletteColorCount(),
		                              format_3bpp);
		Graphics::crossBlitMap(t->_data, (const byte *)surface->getPixels(),
		                       t->_width * t->_bpp, surface->pitch,
		                       t->_width, t->_height, t->_bpp, map);
	} else {
		Graphics::crossBlit(t->_data, (const byte *)surface->getPixels(),
		                    t->_width * t->_bpp, surface->pitch,
		                    t->_width, t->_height,
		                   (t->_bpp == 4) ? format_4bpp : format_3bpp, surface->format);
	}
}

static void loadPNG(Common::SeekableReadStream *data, Texture *t) {
	Image::PNGDecoder *pngDecoder = new Image::PNGDecoder();
	pngDecoder->loadStream(*data);
	loadImage(pngDecoder, t);
	delete pngDecoder;
}

static void loadTGA(Common::SeekableReadStream *data, Texture *t) {
	Image::TGADecoder *tgaDecoder = new Image::TGADecoder();
	tgaDecoder->loadStream(*data);
	loadImage(tgaDecoder, t);
	delete tgaDecoder;
}

void MaterialData::initGrim(Common::SeekableReadStream *data) {
	 if (_fname.hasSuffix(".png")) {
		_numImages = 1;
		_textures = new Texture*[1];
		_textures[0] = new Texture();
		loadPNG(data, _textures[0]);
		return;
	}
	uint32 tag = data->readUint32BE();
	if (tag != MKTAG('M','A','T',' '))
		error("Invalid header for texture %s. Expected 'MAT ', got '%c%c%c%c'", _fname.c_str(),
		                 (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);

	data->seek(12, SEEK_SET);
	_numImages = data->readUint32LE();
	_textures = new Texture*[_numImages];
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
		Texture *t = _textures[i] = new Texture();
		t->_width = data->readUint32LE();
		t->_height = data->readUint32LE();
		t->_hasAlpha = data->readUint32LE();
		t->_texture = nullptr;
		t->_data = nullptr;
		if (t->_width == 0 || t->_height == 0) {
			Debug::warning(Debug::Materials, "skip load texture: bad texture size (%dx%d) for texture %d of material %s",
						   t->_width, t->_height, i, _fname.c_str());
			break;
		}
		t->_bpp = 1;
		t->_data = new uint8[t->_width * t->_height];
		data->seek(12, SEEK_CUR);
		data->read(t->_data, t->_width * t->_height);
	}
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
		// We need to allow null cmaps for remastered overlays
		if (m->_fname == filename && (!(m->_cmap || cmap) || m->_cmap->getFilename() == cmap->getFilename())) {
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
