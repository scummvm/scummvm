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

#include "common/system.h"

#include "graphics/managed_surface.h"

#include "director/director.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/util.h"

namespace Director {

void Frame::playTransition(Score *score) {
	uint16 duration = _transDuration * 250; // _transDuration in 1/4 of sec
	duration = (duration == 0 ? 250 : duration); // director supports transition duration = 0, but animation play like value = 1, idk.

	if (_transChunkSize == 0)
		_transChunkSize = 1; // equal to 1 step

	uint16 stepDuration = duration / _transChunkSize;
	uint16 steps = duration / stepDuration;

	switch (_transType) {
	case kTransCenterOutHorizontal: // 5
		{
			warning("Frame::playTransition(): Unhandled transition type kTransCenterOutHorizontal %d %d", duration, _transChunkSize);
		}
		break;

	case kTransCenterOutVertical: // 7
		{
			warning("Frame::playTransition(): Unhandled transition type kTransCenterOutVertical %d %d", duration, _transChunkSize);
		}
		break;

	case kTransCoverDown:	// 29
		{
			uint16 stepSize = score->_movieRect.height() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverDownLeft: // 30
		{
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, 0, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverDownRight: // 31
		{
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverLeft:	// 32
		{
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);

				g_system->delayMillis(stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, 0, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverRight:	// 33
		{
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);

				g_system->delayMillis(stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, 0, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverUp:		// 34
		{
			uint16 stepSize = score->_movieRect.height() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, score->_movieRect.height() - stepSize * i, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverUpLeft:	// 35
		{
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, score->_movieRect.width() - stepSize * i, score->_movieRect.height() - stepSize * i, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransCoverUpRight:	// 36
		{
			uint16 stepSize = score->_movieRect.width() / steps;
			Common::Rect r = score->_movieRect;

			for (uint16 i = 1; i < steps; i++) {
				r.setWidth(stepSize * i);
				r.setHeight(stepSize * i);

				g_system->delayMillis(stepDuration);
				processQuitEvent();

				g_system->copyRectToScreen(score->_surface->getPixels(), score->_surface->pitch, 0, score->_movieRect.height() - stepSize * i, r.width(), r.height()); // transition
				g_system->updateScreen();
			}
		}
		break;

	case kTransDissolvePixels: // 51
		{
			warning("Frame::playTransition(): Unhandled transition type kTransDissolvePixels %d %d", duration, _transChunkSize);
		}
		break;

	default:
		warning("Frame::playTransition(): Unhandled transition type %d %d %d", _transType, duration, _transChunkSize);
		break;

	}
}

} // End of namespace Director
