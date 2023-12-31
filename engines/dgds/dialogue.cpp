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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"

#include "dgds/dialogue.h"

namespace Dgds {

void Dialogue::parseSDS(Common::SeekableReadStream *stream) {
	// Debug
	/*uint32 pos = stream->pos();
					byte *tmp = new byte[stream->size()];
					stream->read(tmp, stream->size());
					Common::hexdump(tmp, stream->size());
					stream->seek(pos, SEEK_SET);*/

	uint32 mark = stream->readUint32LE();
	debug("    0x%X", mark);

	char version[7];
	stream->read(version, sizeof(version));
	debug("    %s", version);

	uint16 idx;
	idx = stream->readUint16LE();
	debug("    S%d.SDS", idx);

	// gross hack to grep the strings.
	//_bubbles.clear();

	bool inside = false;
	Common::String txt;
	{
		int64 start = stream->pos();
		Common::DumpFile dump;
		dump.open("/tmp/sds.dat");
		dump.writeStream(stream);
		stream->seek(start);
	}
	
	
	while (1) {
		char buf[4];
		stream->read(buf, sizeof(buf));
		if (stream->pos() >= stream->size())
			break;
		if (Common::isPrint(buf[0]) && Common::isPrint(buf[1]) && Common::isPrint(buf[2]) && Common::isPrint(buf[3])) {
			inside = true;
		}
		stream->seek(-3, SEEK_CUR);

		if (inside) {
			if (buf[0] == '\0') {
				// here's where we do a clever thing. we want Pascal like strings.
				uint16 pos = txt.size() + 1;
				stream->seek(-pos - 2, SEEK_CUR);
				uint16 len = stream->readUint16LE();
				stream->seek(pos, SEEK_CUR);

				// gotcha!
				if (len == pos) {
					//if (resource == 0)
						//_bubbles.push_back(txt);
					debug("    \"%s\"", txt.c_str());
				}
				// let's hope the string wasn't shorter than 4 chars...
				txt.clear();
				inside = false;
			} else {
				txt += buf[0];
			}
		}
	}
#if 0
						idx = stream->readUint16LE();
						debug("    %d", idx);

						idx = stream->readUint16LE();
						debug("    %d", idx);

						uint16 count;
						while (1) {
							uint16 code;
							code = stream->readUint16LE();
							count = stream->readUint16LE();
							idx = stream->readUint16LE();

							debugN("\tOP: 0x%8.8x %2u %2u\n", code, count, idx);

							uint16 pitch = (count+1)&(~1); // align to word.
							if ((stream->pos()+pitch) >= stream->size()) break;

							if (code == 0 && count == 0) break;

							stream->skip(pitch);
						}

						Common::String sval;
						byte ch;

						do {
							ch = stream->readByte();
							sval += ch;
						} while (ch != 0);

						debug("\"%s\"", sval.c_str());
#endif
#if 0
						// probe for the .ADS name. are these shorts?
						uint count;
						count = 0;
						while (1) {
							uint16 x;
							x = stream->readUint16LE();
							if ((x & 0xFF00) != 0)
								break;
							debug("      %u: %u|0x%4.4X", count++, x, x);
						}
						stream->seek(-2, SEEK_CUR);

						// .ADS name.
						Common::String ads;
						byte ch;
						while ((ch = stream->readByte()))
							ads += ch;
						debug("    %s", ads.c_str());

						stream->hexdump(6);
						stream->skip(6);

						int w, h;

						w = stream->readSint16LE();
						h = stream->readSint16LE();
						debug("    %dx%d", w, h);

						// probe for the strings. are these shorts?
						count = 0;
						while (1) {
							uint16 x;
							x = stream->readUint16LE();
							if ((x & 0xFF00) != 0)
								break;
							if (stream->pos() >= stream->size()) break;
							debug("      %u: %u|0x%4.4X", count++, x, x);
						}
						stream->seek(-4, SEEK_CUR);
						// here we are.

						uint16 len;
						len = stream->readSint16LE();
						Common::String txt;
						for (uint16 j=0; j<len; j++) {
							ch = stream->readByte();
							txt += ch;
							debug("      \"%s\"", txt.c_str());
						}
#endif
}

} // End of namespace Dgds

