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

#include "ags/shared/ac/dialog_topic.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using AGS::Shared::Stream;

void DialogTopic::ReadFromFile(Stream *in) {
	in->ReadArray(optionnames, 150 * sizeof(char), MAXTOPICOPTIONS);
	in->ReadArrayOfInt32(optionflags, MAXTOPICOPTIONS);
	in->ReadInt32(); // optionscripts 32-bit pointer
	in->ReadArrayOfInt16(entrypoints, MAXTOPICOPTIONS);
	startupentrypoint = in->ReadInt16();
	codesize = in->ReadInt16();
	numoptions = in->ReadInt32();
	topicFlags = in->ReadInt32();
}

void DialogTopic::ReadFromSavegame(Shared::Stream *in) {
	in->ReadArrayOfInt32(optionflags, MAXTOPICOPTIONS);
}

void DialogTopic::WriteToSavegame(Shared::Stream *out) const {
	out->WriteArrayOfInt32(optionflags, MAXTOPICOPTIONS);
}

} // namespace AGS3
