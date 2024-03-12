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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/audio/speech_flex.h"
#include "ultima/ultima8/audio/audio_sample.h"
#include "ultima/ultima8/misc/util.h"

namespace Ultima {
namespace Ultima8 {

SpeechFlex::SpeechFlex(Common::SeekableReadStream *rs) : SoundFlex(rs) {
	uint32 size = getRawSize(0);
	const uint8 *buf = getRawObject(0);

	const char *cbuf = reinterpret_cast<const char *>(buf);

	// Note: SplitString doesn't work here because Std::string can't
	// hold multiple null-terminated strings.
	unsigned int off = 0;
	while (off < size) {
		unsigned int slen = 0;
		while (off + slen < size && cbuf[off + slen])
			slen++;
		Std::string str(cbuf + off, slen);
		off += slen + 1;

		TabsToSpaces(str, 1);
		TrimSpaces(str);

		//debug(MM_INFO, "Found string: \"%s\"", str.c_str());

		_phrases.push_back(str);
	}

	delete [] buf;

}

SpeechFlex::~SpeechFlex(void) {
}

int SpeechFlex::getIndexForPhrase(const Std::string &phrase,
								  uint32 start, uint32 &end) const {
	Std::vector<Std::string>::const_iterator it;
	int i = 1;

	Std::string text = phrase.substr(start);
	TabsToSpaces(text, 1);

	Std::string::size_type pos1 = text.findFirstNotOf(' ');
	if (pos1 == Std::string::npos) return 0;

	Std::string::size_type pos2 = text.findLastNotOf(' ');
	text = text.substr(pos1, pos2 - pos1 + 1);

	//debug(MM_INFO, "Looking for string: \"%s\"", text.c_str());

	for (it = _phrases.begin(); it != _phrases.end(); ++it) {
		if (text.hasPrefixIgnoreCase(*it)) {
			//debug(MM_INFO, "Found: %d", i);
			end = (*it).size() + start + pos1;
			if (end >= start + pos2)
				end = phrase.size();
			return i;
		}
		i++;
	}

	//debug(MM_INFO, "Not found");

	return 0;
}

uint32 SpeechFlex::getSpeechLength(const Std::string &phrase) {
	uint32 start = 0, end = 0;
	uint32 length = 0;

	while (end < phrase.size()) {
		start = end;
		int index = getIndexForPhrase(phrase, start, end);
		if (!index) break;

		const AudioSample *sample = getSample(index);
		if (!sample) break;

		uint32 samples = sample->getLength();
		uint32 rate = sample->getRate();
		bool stereo = sample->isStereo();
		if (stereo) rate *= 2;

		length += (samples * 1000) / rate;
	}

	return length;
}

} // End of namespace Ultima8
} // End of namespace Ultima
