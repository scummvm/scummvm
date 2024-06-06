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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#ifndef __QD_SYSLIB__
//#include "qdengine/core/parser/qdscr_parser.h"
#endif

#include "qdengine/core/qdcore/qd_animation_frame.h"


namespace QDEngine {


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

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

void qdAnimationFrame::qda_save(class XStream &fh) {
	fh < static_cast<int>(0) < start_time_ < length_;

	qdSprite::qda_save(fh);
}

void qdAnimationFrame::qda_load(class XStream &fh, int version) {
	int fl;
	fh > fl > start_time_ > length_;

	qdSprite::qda_load(fh, version);
}

void qdAnimationFrame::qda_load(XZipStream &fh, int version) {
	int fl;
	fh > fl > start_time_ > length_;

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
