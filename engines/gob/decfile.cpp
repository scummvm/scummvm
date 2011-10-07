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
 */

#include "common/str.h"
#include "common/stream.h"
#include "common/substream.h"

#include "gob/gob.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/surface.h"
#include "gob/video.h"
#include "gob/rxyfile.h"
#include "gob/decfile.h"

namespace Gob {

DECFile::Layer::Layer() : surface(0), coordinates(0) {
}

DECFile::Layer::~Layer() {
	delete coordinates;
	delete surface;
}


DECFile::DECFile(GobEngine *vm, const Common::String &fileName,
                 uint16 width, uint16 height, uint8 bpp) : _vm(vm),
	_width(width), _height(height), _bpp(bpp), _hasPadding(false) {

	_backdrop = new Surface(_width, _height, _bpp);

	Common::SeekableReadStream *dec = _vm->_dataIO->getFile(fileName);
	if (dec) {
		Common::SeekableSubReadStreamEndian sub(dec, 0, dec->size(), false, DisposeAfterUse::YES);

		load(sub, fileName);
		return;
	}

	// File doesn't exist, try to open the big-endian'd alternate file
	Common::String alternateFileName = fileName;
	alternateFileName.setChar('_', 0);

	dec = _vm->_dataIO->getFile(alternateFileName);
	if (dec) {
		Common::SeekableSubReadStreamEndian sub(dec, 0, dec->size(), true, DisposeAfterUse::YES);

		// The big endian version pads a few fields to even size
		_hasPadding = true;

		load(sub, fileName);
		return;
	}

	warning("DECFile::DECFile(): No such file \"%s\"", fileName.c_str());
}

DECFile::~DECFile() {
	delete _backdrop;
}

void DECFile::load(Common::SeekableSubReadStreamEndian &dec, const Common::String &fileName) {
	dec.skip(2); // Unused

	int16 backdropCount = dec.readUint16();
	int16 layerCount    = dec.readUint16();

	// Sanity checks
	if (backdropCount > 1)
		warning("DECFile::load(): More than one backdrop (%d) in file \"%s\"",
		        backdropCount, fileName.c_str());
	if (layerCount < 1)
		warning("DECFile::load(): Less than one layer (%d) in file \"%s\"",
		        layerCount, fileName.c_str());

	// Load the backdrop
	if (backdropCount > 0) {
		loadBackdrop(dec);

		// We only support one backdrop, skip the rest
		dec.skip((backdropCount - 1) * (13 + (_hasPadding ? 1 : 0)));
	}

	// Load the layers
	_layers.resize(MAX(0, layerCount - 1));
	for (LayerArray::iterator l = _layers.begin(); l != _layers.end(); ++l)
		loadLayer(*l, dec);

	// Load the backdrop parts
	if (backdropCount > 0)
		loadParts(dec);
}

void DECFile::loadBackdrop(Common::SeekableSubReadStreamEndian &dec) {
	// Interestingly, DEC files reference "FOO.LBM" instead of "FOO.CMP"
	Common::String file = Util::setExtension(Util::readString(dec, 13), ".CMP");
	if (_hasPadding)
		dec.skip(1);

	if (file.empty() || !_vm->_dataIO->hasFile(file))
		return;

	_vm->_video->drawPackedSprite(file.c_str(), *_backdrop);
}

void DECFile::loadLayer(Layer &layer, Common::SeekableSubReadStreamEndian &dec) {
	Common::String file = Util::readString(dec, 13);
	if (_hasPadding)
		dec.skip(1);

	if (file.empty())
		return;

	Common::String fileRXY = Util::setExtension(file, ".RXY");
	Common::String fileCMP = Util::setExtension(file, ".CMP");
	if (!_vm->_dataIO->hasFile(fileRXY) || !_vm->_dataIO->hasFile(fileCMP))
		return;

	loadLayer(layer, fileRXY, fileCMP);
}

void DECFile::loadLayer(Layer &layer, const Common::String &fileRXY,
                                      const Common::String &fileCMP) {

	Common::SeekableReadStream *dataRXY = _vm->_dataIO->getFile(fileRXY);
	if (!dataRXY)
		return;

	layer.coordinates = new RXYFile(*dataRXY);
	layer.surface     = new Surface(_width, layer.coordinates->getHeight(), _bpp);

	_vm->_video->drawPackedSprite(fileCMP.c_str(), *layer.surface);
}

void DECFile::loadParts(Common::SeekableSubReadStreamEndian &dec) {
	dec.skip(13); // Name
	if (_hasPadding)
		dec.skip(1);

	dec.skip(13); // File?
	if (_hasPadding)
		dec.skip(1);

	uint16 partCount = dec.readUint16();

	_parts.resize(partCount);
	for (PartArray::iterator p = _parts.begin(); p != _parts.end(); ++p)
		loadPart(*p, dec);
}

void DECFile::loadPart(Part &part, Common::SeekableSubReadStreamEndian &dec) {
	part.layer = dec.readByte() - 1;
	part.part  = dec.readByte();

	dec.skip(1); // Unknown

	part.x = dec.readUint16();
	part.y = dec.readUint16();

	part.transp = dec.readByte() != 0;
}

void DECFile::draw(Surface &dest) const {
	drawBackdrop(dest);

	for (PartArray::const_iterator p = _parts.begin(); p != _parts.end(); ++p)
		drawLayer(dest, p->layer, p->part, p->x, p->y, p->transp ? 0 : -1);
}

void DECFile::drawBackdrop(Surface &dest) const {
	dest.blit(*_backdrop);
}

void DECFile::drawLayer(Surface &dest, uint16 layer, uint16 part,
                        uint16 x, uint16 y, int32 transp) const {

	if (layer >= _layers.size())
		return;

	const Layer &l = _layers[layer];
	if (!l.surface || !l.coordinates)
		return;

	if (part >= l.coordinates->size())
		return;

	const RXYFile::Coordinates &c = (*l.coordinates)[part];
	if (c.left == 0xFFFF)
		return;

	dest.blit(*l.surface, c.left, c.top, c.right, c.bottom, x, y, transp);
}

} // End of namespace Gob
