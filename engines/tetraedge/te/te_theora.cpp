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

#include "common/file.h"

#include "video/theora_decoder.h"

#include "tetraedge/te/te_theora.h"

namespace Tetraedge {

TeTheora::TeTheora() : _hitEnd(false) {
	_decoder = new Video::TheoraDecoder();
}

TeTheora::~TeTheora() {
	delete _decoder;
}

/*static*/
bool TeTheora::matchExtension(const Common::String &extn) {
	return extn == "ogv";
}

bool TeTheora::load(const TetraedgeFSNode &node) {
	_loadedNode = node;
	if (!_decoder->loadStream(node.createReadStream()))
		return false;
	_decoder->setOutputPixelFormat(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
	return true;
}

uint TeTheora::width() {
	return _decoder->getWidth();
}

uint TeTheora::height() {
	return _decoder->getHeight();
}

int TeTheora::nbFrames() {
	return _decoder->getFrameCount();
}

TeImage::Format TeTheora::imageFormat() {
	//const Graphics::PixelFormat format = _decoder->getPixelFormat();
	// TODO: use format?
	return TeImage::RGBA8;
}

void TeTheora::setLeftBorderSize(uint val) {
	error("TODO: Implement TeTheora::setLeftBorderSize");
}

uint TeTheora::leftBorderSize() {
	error("TODO: Implement TeTheora::leftBorderSize");
}

void TeTheora::setRightBorderSize(uint val) {
	error("TODO: Implement TeTheora::setRightBorderSize");
}

uint TeTheora::rightBorderSize() {
	error("TODO: Implement TeTheora::rightBorderSize");
}

void TeTheora::setBottomBorderSize(uint val) {
	error("TODO: Implement TeTheora::setBottomBorderSize");
}

uint TeTheora::bottomBorderSize() {
	error("TODO: Implement TeTheora::bottomBorderSize");
}

void TeTheora::setTopBorderSize(uint val) {
	error("TODO: Implement TeTheora::setTopBorderSize");
}

uint TeTheora::topBorderSize() {
	error("TODO: Implement TeTheora::topBorderSize");
}

float TeTheora::frameRate() {
	return _decoder->getFrameRate().toDouble();
}

bool TeTheora::update(uint i, TeImage &imgout) {
	if (!_decoder->isPlaying())
		_decoder->start();

	if (_decoder->getCurFrame() > (int)i && _loadedNode.exists()) {
		// rewind.. no good way to do that, but it should
		// only happen on loop.
		load(_loadedNode);
		_decoder->start();
	}

	const Graphics::Surface *frame = nullptr;
	while (_decoder->getCurFrame() <= (int)i && !_decoder->endOfVideo())
		frame = _decoder->decodeNextFrame();

	_hitEnd = _decoder->endOfVideo();

	if (frame && frame->getPixels()) {
		//debug("TeTheora: %s %ld", _path.toString().c_str(), i);
		imgout.copyFrom(*frame);
		return true;
	} else if (_hitEnd && _loadedNode.exists()) {
		// Loop to the start.
		load(_loadedNode);
		frame = _decoder->decodeNextFrame();
		if (frame) {
			imgout.copyFrom(*frame);
			return true;
		}
	}
	return false;
}

bool TeTheora::isAtEnd() {
	return _hitEnd;
}

void TeTheora::setColorKeyActivated(bool val) {
	error("TODO: Implement TeTheora::setColorKeyActivated");
}

void TeTheora::setColorKey(const TeColor &col) {
	error("TODO: Implement TeTheora::setColorKey");
}

void TeTheora::setColorKeyTolerence(float val) {
	error("TODO: Implement TeTheora::setColorKeyTolerence");
}

} // end namespace Tetraedge
