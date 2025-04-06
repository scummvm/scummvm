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

namespace Ultima {
namespace Ultima8 {

SpeechFlex::SpeechFlex(Common::SeekableReadStream *rs) : SoundFlex(rs) {
	uint32 size = getRawSize(0);
	const uint8 *buf = getRawObject(0);

	const char *cbuf = reinterpret_cast<const char *>(buf);

	// Note: stream holds multiple null-terminated strings.
	unsigned int off = 0;
	while (off < size) {
		unsigned int slen = 0;
		while (off + slen < size && cbuf[off + slen])
			slen++;
		Std::string text(cbuf + off, slen);
		text.replace('\t', ' ');
		off += slen + 1;

		Std::string::size_type pos1 = text.findFirstNotOf(' ');
		if (pos1 == Std::string::npos) {
			text = "";
		}
		else {
			Std::string::size_type pos2 = text.findLastNotOf(' ');
			text = text.substr(pos1, pos2 - pos1 + 1);
		}

		debug(6, "Found string: \"%s\"", text.c_str());
		_phrases.push_back(text);
	}

	delete [] buf;

}

SpeechFlex::~SpeechFlex(void) {
}

int SpeechFlex::getIndexForPhrase(const Std::string &phrase,
								  uint32 start, uint32 &end) const {
	int i = 1;

	Std::string text = phrase.substr(start);
	text.replace('\t', ' ');

	Std::string::size_type pos1 = text.findFirstNotOf(' ');
	if (pos1 == Std::string::npos)
		return 0;

	Std::string::size_type pos2 = text.findLastNotOf(' ');
	text = text.substr(pos1, pos2 - pos1 + 1);

	debug(6, "Looking for string: \"%s\"", text.c_str());

	for (const auto &p : _phrases) {
		if (!p.empty() && text.hasPrefixIgnoreCase(p)) {
			debug(6, "Found: %d", i);
			end = p.size() + start + pos1;
			if (end >= start + pos2)
				end = phrase.size();
			return i;
		}
		i++;
	}

	debug(6, "Not found");
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
