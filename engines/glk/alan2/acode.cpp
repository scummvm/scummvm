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

#include "glk/alan2/acode.h"

namespace Glk {
namespace Alan2 {

AcdHdr::AcdHdr() : size(0), pack(0), paglen(0), pagwidth(0), debug(0), dict(0), oatrs(0),
		latrs(0), aatrs(0), acts(0), objs(0), locs(0), stxs(0), vrbs(0), evts(0),
		cnts(0), ruls(0), init(0), start(0), msgs(0), objmin(0), objmax(0), actmin(0),
		actmax(0), cntmin(0), cntmax(0), locmin(0), locmax(0), dirmin(0), dirmax(0),
		evtmin(0), evtmax(0), rulmin(0), rulmax(0), maxscore(0), scores(0),
		freq(0), acdcrc(0), txtcrc(0) {
	vers[0] = vers[1] = vers[2] = vers[3] = 0;
}

void AcdHdr::load(Common::SeekableReadStream &s) {
	s.read(vers, 4);
	size = s.readUint32LE();
	pack = s.readUint32LE();
	paglen = s.readUint32LE();
	pagwidth = s.readUint32LE();
	debug = s.readUint32LE();
	dict = s.readUint32LE();
	oatrs = s.readUint32LE();
	latrs = s.readUint32LE();
	aatrs = s.readUint32LE();
	acts = s.readUint32LE();
	objs = s.readUint32LE();
	locs = s.readUint32LE();
	stxs = s.readUint32LE();
	vrbs = s.readUint32LE();
	evts = s.readUint32LE();
	cnts = s.readUint32LE();
	ruls = s.readUint32LE();
	init = s.readUint32LE();
	start = s.readUint32LE();
	msgs = s.readUint32LE();
	objmin = s.readUint32LE();
	objmax = s.readUint32LE();
	actmin = s.readUint32LE();
	actmax = s.readUint32LE();
	cntmin = s.readUint32LE();
	cntmax = s.readUint32LE();
	locmin = s.readUint32LE();
	locmax = s.readUint32LE();
	dirmin = s.readUint32LE();
	dirmax = s.readUint32LE();
	evtmin = s.readUint32LE();
	evtmax = s.readUint32LE();
	rulmin = s.readUint32LE();
	rulmax = s.readUint32LE();
	maxscore = s.readUint32LE();
	scores = s.readUint32LE();
	freq = s.readUint32LE();
	acdcrc = s.readUint32LE();
	txtcrc = s.readUint32LE();
}

} // End of namespace Alan2
} // End of namespace Glk
