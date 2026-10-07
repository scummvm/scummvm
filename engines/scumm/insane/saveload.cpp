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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scumm/insane/saveload.h"

#include "common/endian.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/thumbnail.h"
#include "scumm/scumm.h"

namespace Scumm {

const uint32 kInsaneScummSaveVersion = 124;

bool writeInsaneSavegameHeader(Common::WriteStream *out, const Common::String &desc, uint32 playTime) {
	char name[32] = {};
	Common::strlcpy(name, desc.c_str(), sizeof(name));
	out->writeUint32BE(MKTAG('S', 'C', 'V', 'M'));
	out->writeUint32LE(0);
	out->writeUint32LE(kInsaneScummSaveVersion);
	out->write(name, sizeof(name));
	if (!Graphics::saveThumbnail(*out))
		return false;

	TimeDate now;
	g_system->getTimeAndDate(now);
	out->writeUint32BE(MKTAG('I', 'N', 'F', 'O'));
	out->writeUint32BE(2);
	out->writeUint32BE(26);
	out->writeUint32BE(0);
	out->writeUint32BE(playTime / 1000);
	out->writeUint32BE(((now.tm_mday & 0xff) << 24) | (((now.tm_mon + 1) & 0xff) << 16) | ((now.tm_year + 1900) & 0xffff));
	out->writeUint16BE(((now.tm_hour & 0xff) << 8) | (now.tm_min & 0xff));
	return !out->err();
}

bool readInsaneSavegameHeader(Common::SeekableReadStream *in, Common::String *desc, SaveStateMetaInfos &infos) {
	memset(&infos, 0, sizeof(infos));
	if (in->readUint32BE() != MKTAG('S', 'C', 'V', 'M'))
		return false;
	in->readUint32LE();
	uint32 version = in->readUint32LE();
	if (version > 0xffffff)
		version = SWAP_BYTES_32(version);
	if (version < 52 || version > kInsaneScummSaveVersion)
		return false;
	char name[32];
	if (in->read(name, sizeof(name)) != sizeof(name))
		return false;
	name[sizeof(name) - 1] = 0;
	if (desc)
		*desc = name;
	if (!Graphics::skipThumbnail(*in))
		return false;
	if (version >= 56) {
		const int64 infoStart = in->pos();
		if (in->readUint32BE() != MKTAG('I', 'N', 'F', 'O'))
			return false;
		const uint32 infoVersion = in->readUint32BE(), size = in->readUint32BE();
		if (!infoVersion || size < (infoVersion == 1 ? 20U : 26U) || size > in->size() - infoStart)
			return false;
		in->readUint32BE();
		infos.playtime = in->readUint32BE();
		if (infoVersion >= 2) {
			infos.date = in->readUint32BE();
			infos.time = in->readUint16BE();
		}
		if (!in->seek(infoStart + size))
			return false;
	}
	return !in->err() && !in->eos();
}

} // End of namespace Scumm
