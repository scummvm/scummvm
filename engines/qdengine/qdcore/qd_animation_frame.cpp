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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/file.h"
#include "qdengine/qd_precomp.h"
#ifndef __QD_SYSLIB__
//#include "qdengine/parser/qdscr_parser.h"
#endif

#include "qdengine/qdcore/qd_animation_frame.h"


namespace QDEngine {

qdAnimationFrame::qdAnimationFrame() : start_time_(0.0f),
	length_(0.0f) {
}

qdAnimationFrame::qdAnimationFrame(const qdAnimationFrame &frm) : qdSprite(frm),
	start_time_(frm.start_time_),
	length_(frm.length_) {
}

qdAnimationFrame::~qdAnimationFrame() {
	free();
}

qdAnimationFrame &qdAnimationFrame::operator = (const qdAnimationFrame &frm) {
	if (this == &frm) return *this;

	*static_cast<qdSprite *>(this) = frm;

	start_time_ = frm.start_time_;
	length_ = frm.length_;

	return *this;
}

qdAnimationFrame *qdAnimationFrame::clone() const {
	return new qdAnimationFrame(*this);
}

void qdAnimationFrame::qda_load(class XStream &fh, int version) {
	warning("STUB: qdAnimationFrame::qda_load(XStream &fh, int version)");
	return;
}

void qdAnimationFrame::qda_load(Common::SeekableReadStream *fh, int version) {
	/*int32 fl = */fh->readSint32LE();
	start_time_ = fh->readFloatLE();
	length_ = fh->readFloatLE();

	qdSprite::qda_load(fh, version);
}
void qdAnimationFrame::qda_load(XZipStream &fh, int version) {
	warning("STUB: qdAnimationFrame::qda_load(XZipStream &fh, int verion)");
	return;
}

bool qdAnimationFrame::load_resources() {
	if (!load()) return false;

	return true;
}

void qdAnimationFrame::free_resources() {
	free();
}
} // namespace QDEngine
