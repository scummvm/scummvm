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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_SOUND_ADLPLAYER_H
#define GOB_SOUND_ADLPLAYER_H

#include "common/array.h"

#include "gob/sound/adlib.h"

namespace Common {
	class SeekableReadStream;
}

namespace Gob {

/** A player for Coktel Vision's ADL music format. */
class ADLPlayer : public AdLib {
public:
	ADLPlayer();
	~ADLPlayer() override;

	bool load(Common::SeekableReadStream &adl);
	bool load(const byte *data, uint32 dataSize, int index = -1);
	void unload();

	int getIndex() const;

protected:
	// AdLib interface
	uint32 pollMusic(bool first) override;
	void rewind() override;

private:
	struct Timbre {
		uint16 startParams[kOperatorsPerVoice * kParamCount];
		uint16 params[kOperatorsPerVoice * kParamCount];
	};

	uint8 _soundMode;

	Common::Array<Timbre> _timbres;

	byte  *_songData;
	uint32 _songDataSize;

	const byte *_playPos;

	int _index;

	uint8  _modifyInstrument;
	uint16 _currentInstruments[kMaxVoiceCount];


	void setInstrument(int voice, int instrument);

	bool readHeader  (Common::SeekableReadStream &adl, int &timbreCount);
	bool readTimbres (Common::SeekableReadStream &adl, int  timbreCount);
	bool readSongData(Common::SeekableReadStream &adl);
};

} // End of namespace Gob

#endif // GOB_SOUND_ADLPLAYER_H
