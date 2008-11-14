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
 * $URL$
 * $Id$
 *
 */

#include "groovie/groovie.h"
#include "groovie/roq.h"

#include "sound/mixer.h"

namespace Groovie {

ROQPlayer::ROQPlayer(GroovieEngine *vm) :
	VideoPlayer(vm) {
}

ROQPlayer::~ROQPlayer() {
}

uint16 ROQPlayer::loadInternal() {
	// Begin reading the file
	debugC(1, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Loading video");

	// Read the file header
	ROQBlockHeader blockHeader;
	if (!readBlockHeader(blockHeader)) {
		return 0;
	}
	if (blockHeader.type != 0x1084 || blockHeader.size != 0 || blockHeader.param != 0) {
		return 0;
	}

	// Hardcoded FPS
	return 25;
}

bool ROQPlayer::playFrameInternal() {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Playing frame");

	// Process the needed blocks until the next video frame
	bool endframe = false;
	while (!endframe && !_file->eos()) {
		endframe = processBlock();
	}

	// Wait until the current frame can be shown
	waitFrame();

	// Update the screen
	_syst->updateScreen();

	// Return whether the video has ended
	return _file->eos();
}

bool ROQPlayer::readBlockHeader(ROQBlockHeader &blockHeader) {
	if (_file->eos()) {
		return false;
	} else {
		blockHeader.type = _file->readUint16LE();
		blockHeader.size = _file->readUint32LE();
		blockHeader.param = _file->readUint16LE();

		debugC(10, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Block type = 0x%02X", blockHeader.type);
		debugC(10, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Block size = 0x%08X", blockHeader.size);
		debugC(10, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Block param = 0x%04X", blockHeader.param);

		return true;
	}
}

bool ROQPlayer::processBlock() {
	// Read the header of the block
	ROQBlockHeader blockHeader;
	if (!readBlockHeader(blockHeader)) {
		return true;
	}

	bool ok = true;
	bool endframe = false;
	switch (blockHeader.type) {
	case 0x1001: // Video info
		ok = processBlockInfo(blockHeader);
		break;
	case 0x1002: // Quad codebook definition
		ok = processBlockQuadCodebook(blockHeader);
		break;
	case 0x1011: // Quad vector quantised video frame
		ok = processBlockQuadVector(blockHeader);
		endframe = true;
		break;
	case 0x1012: // Still image (JPEG)
		ok = processBlockStill(blockHeader);
		endframe = true;
		break;
	case 0x1013: // Hang
		//warning("Groovie::ROQ: Hang block (skipped)");
		break;
	case 0x1020: // Mono sound samples
		ok = processBlockSoundMono(blockHeader);
		break;
	case 0x1021: // Stereo sound samples
		ok = processBlockSoundStereo(blockHeader);
		break;
	case 0x1030: // Audio container
		ok = processBlockAudioContainer(blockHeader);
		break;
	default:
		error("Groovie::ROQ: Unknown block type: 0x%04X", blockHeader.type);
		ok = false;
	}

	// End the frame when the graphics have been modified or when there's an error
	return endframe || !ok;
}

bool ROQPlayer::processBlockInfo(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing info block");

	// Verify the block header
	if (blockHeader.type != 0x1001 || blockHeader.size != 8 || blockHeader.param != 0) {
		return false;
	}

	uint16 tmp;
	tmp = _file->readUint16LE();
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "w = %d\n", tmp);
	if (tmp != 640) {
		return false;
	}
	tmp = _file->readUint16LE();
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "h = %d\n", tmp);
	if (tmp != 320) {
		return false;
	}
	tmp = _file->readUint16LE();
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "unk1 = %d\n", tmp);
	if (tmp != 8) {
		return false;
	}
	tmp = _file->readUint16LE();
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "unk2 = %d\n", tmp);
	if (tmp != 4) {
		return false;
	}
	return true;
}

bool ROQPlayer::processBlockQuadCodebook(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing quad codebook block");

	// Get the number of 2x2 pixel blocks
	_num2blocks = blockHeader.param >> 8;
	if (_num2blocks == 0) {
		_num2blocks = 256;
	}

	// Get the number of 4x4 pixel blocks
	_num4blocks = blockHeader.param & 0xFF;
	if (_num4blocks == 0 && (blockHeader.size > (uint32)_num2blocks * 6)) {
		_num4blocks = 256;
	}

	_file->skip(_num2blocks * 6);
	_file->skip(_num4blocks * 4);

	return true;
}

bool ROQPlayer::processBlockQuadVector(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing quad vector block");
	_file->skip(blockHeader.size);
	return true;

	// Get the mean motion vectors
	//byte Mx = blockHeader.param >> 8;
	//byte My = blockHeader.param & 0xFF;

	int32 ends =_file->pos() + blockHeader.size;
	int numblocks = (640 / 8) * (320 / 8);
	for (int j = 0; j < numblocks && ends > _file->pos(); j++) {
		debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "doing block %d/%d\n", j, numblocks);
		uint16 codingType = _file->readUint16LE();
		for (int i = 0; i < 8; i++) {
			switch (codingType >> 14) {
			case 0: // MOT: Skip block
				//printf("coding type 0\n");
				break;
			case 1: { // FCC: Copy an existing block
				//printf("coding type 1\n");
				byte argument;
				argument = _file->readByte();
				//byte Dx = Mx + (argument >> 4);
				//byte Dy = My + (argument & 0x0F);
				// Dx = X + 8 - (argument >> 4) - Mx
				// Dy = Y + 8 - (argument & 0x0F) - My
				break;
			}
			case 2: { // SLD: Quad vector quantisation
				//printf("coding type 2\n");
				byte argument = _file->readByte();
				if (argument > _num4blocks) {
					//error("invalid 4x4 block %d of %d", argument, _num4blocks);
				}
				// Upsample the 4x4 pixel block
				break;
			}
			case 3: // CCC: 
				//printf("coding type 3:\n");
				processBlockQuadVectorSub(blockHeader);
				break;
			}
			codingType <<= 2;
		}
	}
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Should have ended at %d, and has ended at %d\n", ends, _file->pos());
	return true;
}

bool ROQPlayer::processBlockQuadVectorSub(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing quad vector sub block");

	// Get the mean motion vectors
	//byte Mx = blockHeader.param >> 8;
	//byte My = blockHeader.param & 0xFF;

	uint16 codingType = _file->readUint16LE();
	for (int i = 0; i < 4; i++) {
		switch (codingType >> 14) {
		case 0: // MOT: Skip block
			//printf("coding type 0\n");
			break;
		case 1: { // FCC: Copy an existing block
			//printf("coding type 1\n");
			byte argument;
			argument = _file->readByte();
			//byte Dx = Mx + (argument >> 4);
			//byte Dy = My + (argument & 0x0F);
			// Dx = X + 8 - (argument >> 4) - Mx
			// Dy = Y + 8 - (argument & 0x0F) - My
			break;
		}
		case 2: { // SLD: Quad vector quantisation
			//printf("coding type 2\n");
			byte argument = _file->readByte();
			if (argument > _num2blocks) {
				//error("invalid 2x2 block: %d of %d", argument, _num2blocks);
			}
			break;
		}
		case 3:
			//printf("coding type 3\n");
			_file->readByte();
			_file->readByte();
			_file->readByte();
			_file->readByte();
			break;
		}
		codingType <<= 2;
	}
	return true;
}

bool ROQPlayer::processBlockStill(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing still (JPEG) block");
	//Common::ReadStream *jpegData = new Common::SubReadStream(_file, blockHeader.size);
	//Graphics::JPEG jpegFrame;
	//jpegFrame.read(jpegData);
	/*
	Common::File save;
	save.open("dump.jpg", Common::File::kFileWriteMode);
	save.write(data, blockHeader.size);
	save.close();
	*/
	error("JPEG!");
	return true;
}

bool ROQPlayer::processBlockSoundMono(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing mono sound block");

	// Verify the block header
	if (blockHeader.type != 0x1020) {
		return false;
	}

	// Initialize the audio stream if needed
	if (!_audioStream) {
		byte flags = Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_AUTOFREE;
#ifdef SCUMM_LITTLE_ENDIAN
		flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif
		_audioStream = Audio::makeAppendableAudioStream(22050, flags);
		Audio::SoundHandle sound_handle;
		::g_engine->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &sound_handle, _audioStream);
	}

	// Create the audio buffer
	int16 *buffer = new int16[blockHeader.size];

	// Initialize the prediction with the block parameter
	int16 prediction = blockHeader.param ^ 0x8000;

	// Process the data
	for (uint16 i = 0; i < blockHeader.size; i++) {
		int16 data = _file->readByte();
		if (data < 0x80) {
			prediction += data * data;
		} else {
			data -= 0x80;
			prediction -= data * data;
		}
		buffer[i] = prediction;
	}

	// Queue the read buffer
	_audioStream->queueBuffer((byte *)buffer, blockHeader.size * 2);

	return true;
}

bool ROQPlayer::processBlockSoundStereo(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing stereo sound block");

	// Verify the block header
	if (blockHeader.type != 0x1021) {
		return false;
	}

	// Initialize the audio stream if needed
	if (!_audioStream) {
		byte flags = Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_STEREO;
#ifdef SCUMM_LITTLE_ENDIAN
		flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif
		_audioStream = Audio::makeAppendableAudioStream(22050, flags);
		Audio::SoundHandle sound_handle;
		::g_engine->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &sound_handle, _audioStream);
	}

	// Create the audio buffer
	int16 *buffer = new int16[blockHeader.size];

	// Initialize the prediction with the block parameter
	int16 predictionLeft = (blockHeader.param & 0xFF00) ^ 0x8000;
	int16 predictionRight = (blockHeader.param << 8) ^ 0x8000;
	bool left = true;

	// Process the data
	for (uint16 i = 0; i < blockHeader.size; i++) {
		int16 data = _file->readByte();
		if (left) {
			if (data < 0x80) {
				predictionLeft += data * data;
			} else {
				data -= 0x80;
				predictionLeft -= data * data;
			}
			buffer[i] = predictionLeft;
		} else {
			if (data < 0x80) {
				predictionRight += data * data;
			} else {
				data -= 0x80;
				predictionRight -= data * data;
			}
			buffer[i] = predictionRight;
		}
		left = !left;
	}

	// Queue the read buffer
	_audioStream->queueBuffer((byte *)buffer, blockHeader.size * 2);

	return true;
}

bool ROQPlayer::processBlockAudioContainer(ROQBlockHeader &blockHeader) {
	debugC(5, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::ROQ: Processing audio container block: 0x%04X", blockHeader.param);
	return true;
}

} // End of Groovie namespace
