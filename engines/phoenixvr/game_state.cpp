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

#include "phoenixvr/game_state.h"
#include "common/debug.h"
#include "graphics/surface.h"

namespace PhoenixVR {

static Common::String readSaveText(Common::SeekableReadStream &stream) {
	const uint32 size = stream.readUint32LE();
	Common::String result;

	for (uint32 i = 0; i < size; ++i) {
		const byte c = stream.readByte();
		if (c == 0) {
			result += '\n';
		} else {
			result += static_cast<char>(c);
		}
	}

	return result;
}

GameState GameState::load(Common::SeekableReadStream &stream) {
	GameState state;

	state.script = readSaveText(stream);
	debug("save.script: %s", state.script.c_str());
	state.game = readSaveText(stream);
	debug("save.game: %s", state.game.c_str());
	state.info = readSaveText(stream);
	debug("save.datetime: %s", state.info.c_str());
	uint dibHeaderSize = stream.readUint32LE();
	stream.seek(-4, SEEK_CUR);
	state.dibHeader.resize(dibHeaderSize + 3 * 4); // rmask/gmask/bmask
	stream.read(state.dibHeader.data(), state.dibHeader.size());
	state.thumbWidth = READ_LE_UINT32(state.dibHeader.data() + 0x04);
	state.thumbHeight = READ_LE_UINT32(state.dibHeader.data() + 0x08);
	auto imageSize = READ_LE_UINT32(state.dibHeader.data() + 0x14);
	debug("save.image %dx%d, %u", state.thumbWidth, state.thumbHeight, imageSize);
	state.thumbnail.resize(imageSize);
	stream.read(state.thumbnail.data(), state.thumbnail.size());
	auto gameStateSize = stream.readUint32LE();
	debug("save.state %u bytes", gameStateSize);
	state.state.resize(gameStateSize);
	stream.read(state.state.data(), state.state.size());
	return state;
}

void GameState::save(Common::WriteStream &stream) const {
	auto writeString = [&](const Common::String &str) {
		stream.writeUint32LE(str.size() + 1);
		stream.writeString(str);
		stream.writeByte(0);
	};

	writeString(script);
	writeString(game);
	writeString(info);

	assert(dibHeader.size() == 0x28 + 3 * 4);
	stream.write(dibHeader.data(), dibHeader.size());

	stream.write(thumbnail.data(), thumbnail.size());

	stream.writeUint32LE(state.size());
	stream.write(state.data(), state.size());
}

Graphics::Surface *GameState::getThumbnail(const Graphics::PixelFormat &fmt, int newWidth) {
	Graphics::PixelFormat rgb565(2, 5, 6, 5, 0, 11, 5, 0, 0);
	Graphics::Surface th;
	th.init(thumbWidth, thumbHeight, thumbnail.size() / thumbHeight, thumbnail.data(), rgb565);
	Graphics::Surface *src = th.convertTo(fmt);
	if (newWidth > 0) {
		int newHeight = newWidth * src->h / src->w;
		auto *scaled = src->scale(newWidth, newHeight, true, Graphics::FLIP_V);
		src->free();
		delete src;
		return scaled;
	} else {
		src->flipVertical(src->getRect());
	}
	return src;
}

} // namespace PhoenixVR
