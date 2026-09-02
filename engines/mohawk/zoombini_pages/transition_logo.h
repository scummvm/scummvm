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

#ifndef MOHAWK_ZOOMBINI_PAGES_TRANSITION_LOGO_H
#define MOHAWK_ZOOMBINI_PAGES_TRANSITION_LOGO_H

#include "mohawk/video.h"
#include "mohawk/zoombini_pages/transition_base.h"

#ifdef USE_BINK
namespace Video {

class BinkDecoder;

} // End of namespace Video
#endif

namespace Mohawk {

/**
 * Startup logo transition page.
 *
 * The page plays the release-specific logo video and advances the startup
 * reveal state on the animation frame clock. When the demo startup sequence is
 * active, @ref ZoombiniTransitionLogo::applyDemoStartupRevealMask() recreates
 * the staged block presentation over the decoder's complete video surface.
 */
class ZoombiniTransitionLogo : public ZoombiniTransition {
public:
	/** Create the startup logo page. */
	ZoombiniTransitionLogo(MohawkEngine_Zoombini *vm);
	/** Stop the logo video and release decoder state. */
	~ZoombiniTransitionLogo() override;

	/** Load the logo feature used by the video transition. */
	void loadFeatures() override;
	/** Update video decoding and the startup reveal presentation. */
	void onEveryFrame() override;
	/** Update the page when the video produces an animation frame. */
	void onAnimFrame() override;

protected:
	/** Apply the demo-only startup mask to the current frame. */
	void applyDemoStartupRevealMask();

	/** QuickTime CDTOONS video used by Broderbund releases. */
	VideoEntryPtr _cdtoonsVideo = nullptr;
#ifdef USE_BINK
	/** Bink decoder used by The Learning Company releases. */
	Video::BinkDecoder *_binkDecoder = nullptr;
	/** Most recently decoded Bink frame used while advancing the startup mask. */
	const Graphics::Surface *_binkFrame = nullptr;
#endif
	/** Whether the startup page has switched from indexed to true-color output. */
	bool _switchedToTrueColor = false;
	/** Whether the demo-only startup reveal mask is active. */
	bool _demoStartupRevealActive = false;
	/** Animation frame at which the demo startup reveal begins. */
	uint32 _demoStartupRevealStartFrame = 0;
};

} // End of namespace Mohawk

#endif
