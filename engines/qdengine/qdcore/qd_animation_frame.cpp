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

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_animation_frame.h"


namespace QDEngine {

qdAnimationFrame::qdAnimationFrame() : _start_time(0.0f),
	_length(0.0f) {
}

qdAnimationFrame::qdAnimationFrame(const qdAnimationFrame &frm) : qdSprite(frm),
	_start_time(frm._start_time),
	_length(frm._length) {
}

qdAnimationFrame::~qdAnimationFrame() {
	free();
}

qdAnimationFrame &qdAnimationFrame::operator = (const qdAnimationFrame &frm) {
	if (this == &frm) return *this;

	*static_cast<qdSprite *>(this) = frm;

	_start_time = frm._start_time;
	_length = frm._length;

	return *this;
}

qdAnimationFrame *qdAnimationFrame::clone() const {
	return new qdAnimationFrame(*this);
}

void qdAnimationFrame::qda_load(Common::SeekableReadStream *fh, int version) {
	/*int32 fl = */fh->readSint32LE();
	_start_time = fh->readFloatLE();
	_length = fh->readFloatLE();

	qdSprite::qda_load(fh, version);
}

bool qdAnimationFrame::load_resources() {
	if (!load()) return false;

	return true;
}

void qdAnimationFrame::free_resources() {
	free();
}
} // namespace QDEngine
