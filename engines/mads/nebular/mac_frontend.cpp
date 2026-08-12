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

#include "mads/nebular/mac_frontend.h"

#include "mads/animview/animview.h"
#include "mads/nebular/nebular.h"
#include "mads/textview/textview.h"

namespace MADS {
namespace RexNebular {
namespace MacFrontend {

void runAnimView(RexNebularEngine &engine, const char *resource) {
	AnimView::Presentation presentation;
	presentation.bufferHeight = 200;
	presentation.drawBoundaryLines = false;
	presentation.serviceFramesInline = true;

	engine.setMacintoshFullFrameActive(true);
	engine.getScreen()->clear();
	AnimView::animview_main(resource, presentation);
	engine.setMacintoshFullFrameActive(false);
}

void runTextView(RexNebularEngine &engine, const char *resource) {
	TextView::Presentation presentation;
	presentation.bufferHeight = 210;
	presentation.visibleHeight = 200;
	presentation.drawBoundaryLines = false;
	presentation.macintoshFullFrame = true;

	engine.setMacintoshFullFrameActive(true);
	TextView::textview_main(resource, presentation);
	engine.setMacintoshFullFrameActive(false);
}

void showCreditsAfterEnding(RexNebularEngine &engine) {
	// CODE 133 follows Endi 49, 50 and 52 with CRED 1000.
	runTextView(engine, "credits");
}

} // namespace MacFrontend
} // namespace RexNebular
} // namespace MADS
