#include "made/pmvplayer.h"

namespace Made {

PmvPlayer::PmvPlayer(OSystem *system, Audio::Mixer *mixer) : _fd(NULL), _system(system), _mixer(mixer) {
}

PmvPlayer::~PmvPlayer() {
}

void PmvPlayer::play(const char *filename) {

	_abort = false;
	_surface = NULL;

	_fd = new Common::File();
	_fd->open(filename);

	uint32 chunkType, chunkSize;

	readChunk(chunkType, chunkSize);	// "MOVE"
	readChunk(chunkType, chunkSize);	// "MHED"

	// TODO: Evaluate header
	//_fd->skip(0x3A);

	uint frameDelay = _fd->readUint16LE();
	_fd->skip(10);
	uint soundFreq = _fd->readUint16LE();
	// FIXME: weird frequencies... (11127 or 22254)
	//if (soundFreq == 11127) soundFreq = 11025;
	//if (soundFreq == 22254) soundFreq = 22050;

	int unk;

	for (int i = 0; i < 22; i++) {
		unk = _fd->readUint16LE();
		debug(2, "%i ", unk);
	}
	debug(2, "\n");

	_mixer->stopAll();

	_audioStream = Audio::makeAppendableAudioStream(soundFreq, Audio::Mixer::FLAG_UNSIGNED);
	_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle, _audioStream);

	// Read palette
	_fd->read(_palette, 768);
	updatePalette();

	//FILE *raw = fopen("track.raw", "wb");

	while (!_abort && !_fd->eof()) {

		readChunk(chunkType, chunkSize);

		if (_fd->eof())
			break;

		byte *frameData = new byte[chunkSize];
		_fd->read(frameData, chunkSize);

		byte *audioData = frameData + READ_LE_UINT32(frameData + 8) - 8;
		chunkSize = READ_LE_UINT16(audioData + 4);
		uint16 chunkCount = READ_LE_UINT16(audioData + 6);

		if (chunkCount > 50) break;	// FIXME: this is a hack

		debug(2, "chunkCount = %d; chunkSize = %d\n", chunkCount, chunkSize);

		uint32 soundSize = chunkCount * chunkSize;
		byte *soundData = new byte[soundSize];
		decompressSound(audioData + 8, soundData, chunkSize, chunkCount);
		_audioStream->queueBuffer(soundData, soundSize);
		
		//fwrite(soundData, soundSize, 1, raw);

		byte *imageData = frameData + READ_LE_UINT32(frameData + 12) - 8;

		uint32 frameNum = READ_LE_UINT32(frameData);
		uint16 width = READ_LE_UINT16(imageData + 8);
		uint16 height = READ_LE_UINT16(imageData + 10);
		uint16 cmdOffs = READ_LE_UINT16(imageData + 12);
		uint16 pixelOffs = READ_LE_UINT16(imageData + 16);
		uint16 maskOffs = READ_LE_UINT16(imageData + 20);
		uint16 lineSize = READ_LE_UINT16(imageData + 24);

		debug(2, "width = %d; height = %d; cmdOffs = %04X; pixelOffs = %04X; maskOffs = %04X; lineSize = %d\n",
			width, height, cmdOffs, pixelOffs, maskOffs, lineSize);

		if (!_surface) {
			_surface = new Graphics::Surface();
			_surface->create(width, height, 1);
		}

		decompressImage(imageData, *_surface, cmdOffs, pixelOffs, maskOffs, lineSize, frameNum > 0);

		handleEvents();
		updateScreen();

		delete[] frameData;

		_system->delayMillis(frameDelay);
	}

	_audioStream->finish();
	_mixer->stopAll();
	
	//delete _audioStream;
	delete _fd;
	delete _surface;

	//fclose(raw);

}

void PmvPlayer::readChunk(uint32 &chunkType, uint32 &chunkSize) {
	chunkType = _fd->readUint32BE();
	chunkSize = _fd->readUint32LE();

	debug(2, "chunkType = %c%c%c%c; chunkSize = %d\n",
		(chunkType >> 24) & 0xFF, (chunkType >> 16) & 0xFF, (chunkType >> 8) & 0xFF, chunkType & 0xFF,
		chunkSize);

}

void PmvPlayer::handleEvents() {
	Common::Event event;
	while (_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
				_abort = true;
			break;
		case Common::EVENT_QUIT:
			// TODO: Exit more gracefully
			g_system->quit();
			break;
		default:
			break;
		}
	}
}

void PmvPlayer::updatePalette() {
	byte colors[1024];
	for (int i = 0; i < 256; i++) {
		colors[i * 4 + 0] = _palette[i * 3 + 0];
		colors[i * 4 + 1] = _palette[i * 3 + 1];
		colors[i * 4 + 2] = _palette[i * 3 + 2];
		colors[i * 4 + 3] = 0;
	}
	_system->setPalette(colors, 0, 256);
}

void PmvPlayer::updateScreen() {
	_system->copyRectToScreen((const byte*)_surface->pixels, _surface->pitch, 0, 0, _surface->w, _surface->h);
	_system->updateScreen();
}

}
