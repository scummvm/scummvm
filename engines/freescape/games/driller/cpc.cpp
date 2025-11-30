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
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/driller/driller.h"
#include "freescape/language/8bitDetokeniser.h"

#include "audio/audiostream.h"
#include "audio/softsynth/ay8912.h"

namespace Freescape {

void DrillerEngine::initCPC() {
	_viewArea = Common::Rect(36, 16, 284, 117);
	_soundIndexShoot = 1;
	_soundIndexCollide = 2;
	_soundIndexStepUp = 3;
	_soundIndexStepDown = 3;
	_soundIndexMenu = 6;
	_soundIndexAreaChange = 10;
	_soundIndexHit = 7;
	_soundIndexFallen = 9;
	_soundIndexMissionComplete = 13;
}

byte kCPCPaletteTitleData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x80, 0xff},
	{0xff, 0x00, 0x00},
	{0xff, 0xff, 0x00},
};

byte kCPCPaletteBorderData[4][3] = {
	{0x00, 0x00, 0x00},
	{0xff, 0x80, 0x00},
	{0x80, 0xff, 0xff},
	{0x00, 0x80, 0x00},
};

byte getCPCPixelMode1(byte cpc_byte, int index) {
	if (index == 0)
		return ((cpc_byte & 0x08) >> 2) | ((cpc_byte & 0x80) >> 7);
	else if (index == 1)
		return ((cpc_byte & 0x04) >> 1) | ((cpc_byte & 0x40) >> 6);
	else if (index == 2)
		return (cpc_byte & 0x02)        | ((cpc_byte & 0x20) >> 5);
	else if (index == 3)
		return ((cpc_byte & 0x01) << 1) | ((cpc_byte & 0x10) >> 4);
	else
		error("Invalid index %d requested", index);
}

byte getCPCPixelMode0(byte cpc_byte, int index) {
    if (index == 0) {
        // Extract Pixel 0 from the byte
        return ((cpc_byte & 0x02) >> 1) |  // Bit 1 -> Bit 3 (MSB)
               ((cpc_byte & 0x20) >> 4) |  // Bit 5 -> Bit 2
               ((cpc_byte & 0x08) >> 1) |  // Bit 3 -> Bit 1
               ((cpc_byte & 0x80) >> 7);   // Bit 7 -> Bit 0 (LSB)
    }
    else if (index == 2) {
        // Extract Pixel 1 from the byte
        return ((cpc_byte & 0x01) << 3) |  // Bit 0 -> Bit 3 (MSB)
               ((cpc_byte & 0x10) >> 2) |  // Bit 4 -> Bit 2
               ((cpc_byte & 0x04) >> 1) |  // Bit 2 -> Bit 1
               ((cpc_byte & 0x40) >> 6);   // Bit 6 -> Bit 0 (LSB)
    }
    else {
        error("Invalid index %d requested", index);
    }
}

byte getCPCPixel(byte cpc_byte, int index, bool mode1) {
	if (mode1)
		return getCPCPixelMode1(cpc_byte, index);
	else
		return getCPCPixelMode0(cpc_byte, index);
}

Graphics::ManagedSurface *readCPCImage(Common::SeekableReadStream *file, bool mode1) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, 320, 200), 0);

	int x, y;
	file->seek(0x80);
	for (int block = 0; block < 8; block++) {
		for (int line = 0; line < 25; line++) {
			for (int offset = 0; offset < 320 / 4; offset++) {
				byte cpc_byte = file->readByte(); // Get CPC byte

				// Process first pixel
				int pixel_0 = getCPCPixel(cpc_byte, 0, mode1); // %Aa
				y = line * 8 + block ; // Coord Y for the pixel
				x = 4 * offset + 0; // Coord X for the pixel
				surface->setPixel(x, y, pixel_0);

				// Process second pixel
				y = line * 8 + block ; // Coord Y for the pixel
				x = 4 * offset + 1; // Coord X for the pixel
				if (mode1) {
					int pixel_1 = getCPCPixel(cpc_byte, 1, mode1); // %Bb
					surface->setPixel(x, y, pixel_1);
				} else
					surface->setPixel(x, y, pixel_0);

				// Process third pixel
				int pixel_2 = getCPCPixel(cpc_byte, 2, mode1); // %Cc
				y = line * 8 + block ; // Coord Y for the pixel
				x = 4 * offset + 2; // Coord X for the pixel
				surface->setPixel(x, y, pixel_2);

				// Process fourth pixel
				y = line * 8 + block ; // Coord Y for the pixel
				x = 4 * offset + 3; // Coord X for the pixel
				if (mode1) {
					int pixel_3 = getCPCPixel(cpc_byte, 3, mode1); // %Dd
					surface->setPixel(x, y, pixel_3);
				} else
					surface->setPixel(x, y, pixel_2);
			}
		}
		// We should skip the next 48 bytes, because they are padding the block to be 2048 bytes
		file->seek(48, SEEK_CUR);
	}
	return surface;
}

void DrillerEngine::loadAssetsCPCFullGame() {
	Common::File file;

	file.open("DSCN1.BIN");
	if (!file.isOpen())
		error("Failed to open DSCN1.BIN");

	_title = readCPCImage(&file, true);
	_title->setPalette((byte*)&kCPCPaletteTitleData, 0, 4);

	file.close();
	file.open("DSCN2.BIN");
	if (!file.isOpen())
		error("Failed to open DSCN2.BIN");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteBorderData, 0, 4);

	file.close();
	file.open("DRILL.BIN");

	if (!file.isOpen())
		error("Failed to open DRILL.BIN");

	loadMessagesFixedSize(&file, 0x214c, 14, 20);
	loadFonts(&file, 0x5b69);
	loadGlobalObjects(&file, 0x1d07, 8);
	load8bitBinary(&file, 0x5ccb, 16);
}

void DrillerEngine::drawCPCUI(Graphics::Surface *surface) {
	uint32 color = _currentArea->_underFireBackgroundColor;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 200, 185, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 151, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 151, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 151, 161, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 54, 161, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 54, 161, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 47, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 44, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 239, 129, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 209, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 232, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 255, 8, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 191, 177, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (_messagesList.size() > 0) {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 191, 177, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (energy >= 0) {
		Common::Rect backBar(25, 184, 89 - energy, 191);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(88 - energy, 184, 88, 191);
		surface->fillRect(energyBar, front);
	}

	if (shield >= 0) {
		Common::Rect backBar(25, 177, 89 - shield, 183);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(88 - shield, 177, 88, 183);
		surface->fillRect(shieldBar, front);
	}

	drawCompass(surface, 87, 156, _yaw - 30, 10, 75, front);
	drawCompass(surface, 230, 156, _pitch - 30, 10, 60, front);
}

class DrillerCPCSfxStream : public Audio::AudioStream {
public:
	DrillerCPCSfxStream(int index, int rate = 44100) : _ay(rate, 1000000), _index(index), _rate(rate) { // 1MHz for CPC AY
		// Initialize sound chip (silence)
		initAY();

		_counter = 0;
		_finished = false;

        // Initialize state based on index
        _var_3a64 = 0;

        if (index == 1) { // Shoot
            _var_3a64 = 0xfffc;

            // Channel data from binary extraction
            uint8_t shoot_data[3][24] = {
                {0x20, 0xa, 0x11, 0xc1, 0x42, 0x5, 0x2, 0x2, 0x1, 0x1, 0x21, 0x1b, 0x77, 0xee, 0x40, 0x85, 0x21, 0x85, 0x24, 0x84, 0x1f, 0x86, 0x9, 0xd},
                {0x0, 0x0, 0x0, 0x1a, 0xf, 0xa, 0x19, 0x1, 0x1b, 0x5, 0x19, 0x0, 0xc, 0x10, 0x1, 0x64, 0xf, 0x0, 0xf, 0x6, 0x16, 0x10, 0x10, 0x1},
                {0x82, 0x20, 0x9c, 0x9, 0x7, 0x20, 0x1, 0x18, 0x8, 0x8, 0x8, 0xc, 0x16, 0x35, 0x35, 0x7, 0x4, 0x4, 0x4, 0x4, 0x85, 0xc, 0x82, 0x20}
            };

            for (int i=0; i<3; i++) {
                memcpy(_channels[i].data, shoot_data[i], 24);
            }
        } else if (index == 10) { // Area Change (Bell)
             _var_3a64 = 0x9408;
        }
	}

	void initAY() {
		// Silence all channels
		writeReg(7, 0xFF); // Disable all tones and noise
		writeReg(8, 0);    // Volume A 0
		writeReg(9, 0);    // Volume B 0
		writeReg(10, 0);   // Volume C 0
	}

	int readBuffer(int16 *buffer, const int numSamples) override {
		if (_finished)
			return 0;

		int samplesPerTick = _rate / 50;
		int samplesGenerated = 0;

		while (samplesGenerated < numSamples && !_finished) {
			int samplesTodo = MIN(numSamples - samplesGenerated, samplesPerTick);

			updateState();

			_ay.readBuffer(buffer + samplesGenerated, samplesTodo);
			samplesGenerated += samplesTodo;

			if (_finished) break;
		}

		return samplesGenerated;
	}

	bool isStereo() const override { return true; }
	bool endOfData() const override { return _finished; }
	bool endOfStream() const override { return _finished; }
	int getRate() const override { return _rate; }

private:
	Audio::AY8912Stream _ay;
	int _index;
	int _rate;
	int _counter;
	bool _finished;
    uint8_t _regs[16];

    void writeReg(int reg, uint8_t val) {
        if (reg >= 0 && reg < 16) {
            _regs[reg] = val;
            _ay.setReg(reg, val);
        }
    }

    uint16_t _var_3a64;

    struct ChannelState {
        uint8_t data[24];
    };

    ChannelState _channels[3];

    void processChannel(int ch_idx) {
        uint8_t *d = _channels[ch_idx].data;

        // Emulate FUN_5a21 logic
        if (d[0x16] == 0) {
            d[4]--;
            if (d[4] == 0) {
                d[4] = d[2]; // Reset counter
                d[5] = (d[5] + d[1]) & 0xf; // Accumulate

                // Write to Env Period (Approx logic based on FUN_5a21)
                // Assuming scaling for audible effect
                int val = d[5] * 20;
                writeReg(11, val & 0xff);
                writeReg(12, (val >> 8) & 0xff);

                // Trigger?
                writeReg(13, 0); // Shape

                // Set Mixer/Volume for this channel
                if (ch_idx == 0) {
                    writeReg(8, 0x10); // Vol A = Env
                    writeReg(7, _regs[7] & ~1); // Enable Tone A
                }
            }
        }
    }

	void updateState() {
		_counter++;

		// Safety timeout
		if (_counter > 200) {
			_finished = true;
			return;
		}

        if (_var_3a64 & 0x10) { // Update channels
            // Process channels using extracted data
            // for (int i=0; i<3; i++) {
            //    processChannel(i);
            // }

            // Manual implementation for Shoot (Index 1) since extracted data seems to be idle/delay state
            if (_index == 1) {
                // Sweep Tone A
                // Period: 10 -> ~130 (Slower sweep)
                // Was: 10 + (_counter * 5)
                int period = 10 + (_counter * 2);
                writeReg(0, period & 0xff);
                writeReg(1, (period >> 8) & 0xf);

                // Enable Tone A, Disable Noise
                writeReg(7, 0x3E); // 0011 1110. Tone A (bit 0) = 0 (On).

                // Volume A: Envelope or Decay
                // Use software volume decay for reliability
                // Was: 15 - (_counter / 2) -> 30 ticks (~0.6s)
                // Now: 15 - (_counter / 4) -> 60 ticks (~1.2s)
                int vol = 15 - (_counter / 4);
                if (vol < 0) vol = 0;
                writeReg(8, vol);

                if (vol == 0) _finished = true;
            }
        }

        // Handle Bell (Index 10) - Using data derived from binary (0x1802, 0xF602)
        if (_index == 10) {
             if (_counter > 200) {
                _finished = true;
             } else {
                // Alternating pitch based on binary values found near 0x4219 (0x0218 and 0x02F6)
                // 0x0218 = 536 (~116Hz)
                // 0x02F6 = 758 (~82Hz)
                // Even slower alternation (was /6 -> 120ms, now /12 -> 240ms)
                int pitch = ((_counter / 12) % 2 == 0) ? 0x218 : 0x2F6;
                writeReg(0, pitch & 0xff);
                writeReg(1, (pitch >> 8) & 0xf);

                // Even slower decay
                int vol = 15 - (_counter / 12);
                if (vol < 0) vol = 0;
                writeReg(8, vol);
                writeReg(7, 0x3E); // Tone A only
             }
        }

        // Handle unhandled sounds
        if (_index != 1 && _index != 10) {
             _finished = true;
        }

        if (_finished) {
             initAY(); // Silence
        }
	}
};

void FreescapeEngine::playSoundDrillerCPC(int index, Audio::SoundHandle &handle) {
	debugC(1, kFreescapeDebugMedia, "Playing Driller CPC sound %d", index);
	// Create a new stream for the sound
	DrillerCPCSfxStream *stream = new DrillerCPCSfxStream(index);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, stream, -1, kFreescapeDefaultVolume, 0, DisposeAfterUse::YES);
}

} // End of namespace Freescape
