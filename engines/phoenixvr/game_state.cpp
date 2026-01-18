#include "phoenixvr/game_state.h"
#include "common/debug.h"
#include "graphics/surface.h"

namespace PhoenixVR {
GameState GameState::load(Common::SeekableReadStream &stream) {
	GameState state;

	auto readString = [&]() {
		auto size = stream.readUint32LE();
		return stream.readString(0, size);
	};

	state.script = readString();
	debug("save.script: %s", state.script.c_str());
	state.game = readString();
	debug("save.game: %s", state.game.c_str());
	state.info = readString();
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
	src->flipVertical(src->getRect());
	if (newWidth > 0) {
		int newHeight = newWidth * src->h / src->w;
		auto *scaled = src->scale(newWidth, newHeight, true);
		src->free();
		delete src;
		return scaled;
	}
	return src;
}

} // namespace PhoenixVR
