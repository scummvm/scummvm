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

static bool acquireFullFrame(RexNebularEngine &engine) {
	if (engine.isMacintoshFullFrameActive())
		return false;
	engine.setMacintoshFullFrameActive(true);
	return true;
}

static void releaseFullFrame(RexNebularEngine &engine, bool acquired) {
	if (acquired)
		engine.setMacintoshFullFrameActive(false);
}

static void runAnimViewContent(RexNebularEngine &engine,
		const char *resource) {
	AnimView::Presentation presentation;
	presentation.bufferHeight = 200;
	presentation.drawBoundaryLines = false;
	presentation.serviceFramesInline = true;

	engine.getScreen()->clear();
	AnimView::animview_main(resource, presentation);
}

static void runTextViewContent(const char *resource) {
	TextView::Presentation presentation;
	presentation.bufferHeight = 210;
	presentation.visibleHeight = 200;
	presentation.matteHeight = 156;
	presentation.drawBoundaryLines = false;
	presentation.macintoshFullFrame = true;
	// CODE 133 initializes both TextView drawing colors to yellow.
	presentation.textColor.r = 63;
	presentation.textColor.g = 63;
	presentation.textColor.b = 0;
	presentation.shadowColor = presentation.textColor;

	TextView::textview_main(resource, presentation);
}

void runAnimView(RexNebularEngine &engine, const char *resource) {
	const bool acquired = acquireFullFrame(engine);
	runAnimViewContent(engine, resource);
	releaseFullFrame(engine, acquired);
}

void runTextView(RexNebularEngine &engine, const char *resource) {
	const bool acquired = acquireFullFrame(engine);
	runTextViewContent(resource);
	releaseFullFrame(engine, acquired);
}

void runEndingSequence(RexNebularEngine &engine, const char *animation,
		const char *ending, bool showCredits) {
	// CODE 133 retains its frontend window and palette ownership while it
	// advances from an ending resource to CRED 1000.
	const bool acquired = acquireFullFrame(engine);
	if (animation)
		runAnimViewContent(engine, animation);
	if (ending)
		runTextViewContent(ending);
	if (showCredits)
		runTextViewContent("credits");
	releaseFullFrame(engine, acquired);
}

} // namespace MacFrontend
} // namespace RexNebular
} // namespace MADS
