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

#include "common/scummsys.h"

#include "zvision/scripting/effects/animation_effect.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"

#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace ZVision {

AnimationEffect::AnimationEffect(ZVision *engine, uint32 controlKey, const Common::String &fileName, int32 mask, int32 frate, bool disposeAfterUse)
	: ScriptingEffect(engine, controlKey, SCRIPTING_EFFECT_ANIM),
	  _disposeAfterUse(disposeAfterUse),
	  _mask(mask),
	  _animation(NULL) {

	_animation = engine->loadAnimation(fileName);

	if (frate > 0) {
		_frmDelayOverride = (int32)(1000.0 / frate);

		// WORKAROUND: We do not allow the engine to delay more than 66 msec
		// per frame (15fps max)
		if (_frmDelayOverride > 66)
			_frmDelayOverride = 66;
	} else {
		_frmDelayOverride = 0;
	}
}

AnimationEffect::~AnimationEffect() {
	if (_animation)
		delete _animation;

	_engine->getScriptManager()->setStateValue(_key, 2);

	PlayNodes::iterator it = _playList.begin();
	if (it != _playList.end()) {
		_engine->getScriptManager()->setStateValue((*it).slot, 2);

		if ((*it)._scaled) {
			(*it)._scaled->free();
			delete(*it)._scaled;
		}
	}

	_playList.clear();
}

bool AnimationEffect::process(uint32 deltaTimeInMillis) {
	ScriptManager *scriptManager = _engine->getScriptManager();
	RenderManager *renderManager = _engine->getRenderManager();
	RenderTable::RenderState renderState = renderManager->getRenderTable()->getRenderState();
	bool isPanorama = (renderState == RenderTable::PANORAMA);
	int16 velocity = _engine->getMouseVelocity() + _engine->getKeyboardVelocity();

	// Do not update animation nodes in panoramic mode while turning, if the user
	// has set this option
	if (scriptManager->getStateValue(StateKey_NoTurnAnim) == 1 && isPanorama && velocity)
		return false;

	PlayNodes::iterator it = _playList.begin();
	if (it != _playList.end()) {
		playnode *nod = &(*it);

		if (nod->_curFrame == -1) {
			// The node is just beginning playback
			nod->_curFrame = nod->start;

			_animation->start();
			_animation->seekToFrame(nod->start);
			_animation->setEndFrame(nod->stop);

			nod->_delay = deltaTimeInMillis; // Force the frame to draw
			if (nod->slot)
				scriptManager->setStateValue(nod->slot, 1);
		} else if (_animation->endOfVideo()) {
			// The node has reached the end; check if we need to loop
			nod->loop--;

			if (nod->loop == 0) {
				if (nod->slot >= 0)
					scriptManager->setStateValue(nod->slot, 2);
				if (nod->_scaled) {
					nod->_scaled->free();
					delete nod->_scaled;
				}
				_playList.erase(it);
				return _disposeAfterUse;
			}

			nod->_curFrame = nod->start;
			_animation->seekToFrame(nod->start);
		}

		// Check if we need to draw a frame
		bool needsUpdate = false;
		if (_frmDelayOverride == 0) {
			// If not overridden, use the VideoDecoder's check
			needsUpdate = _animation->needsUpdate();
		} else {
			// Otherwise, implement our own timing
			nod->_delay -= deltaTimeInMillis;

			if (nod->_delay <= 0) {
				nod->_delay += _frmDelayOverride;
				needsUpdate = true;
			}
		}

		if (needsUpdate) {
			const Graphics::Surface *frame = _animation->decodeNextFrame();

			if (frame) {
				uint32 dstw;
				uint32 dsth;
				if (isPanorama) {
					dstw = nod->pos.height();
					dsth = nod->pos.width();
				} else {
					dstw = nod->pos.width();
					dsth = nod->pos.height();
				}

				// We only scale down the animation to fit its frame, not up, otherwise we
				// end up with distorted animations - e.g. the armor visor in location cz1e
				// in Nemesis (one of the armors inside Irondune), or the planet in location
				// aa10 in Nemesis (Juperon, outside the asylum). We do allow scaling up only
				// when a simple 2x filter is requested (e.g. the alchemists and cup sequence
				// in Nemesis)
				if (frame->w > dstw || frame->h > dsth || (frame->w == dstw / 2 && frame->h == dsth / 2)) {
					if (nod->_scaled)
						if (nod->_scaled->w != dstw || nod->_scaled->h != dsth) {
							nod->_scaled->free();
							delete nod->_scaled;
							nod->_scaled = NULL;
						}

					if (!nod->_scaled) {
						nod->_scaled = new Graphics::Surface;
						nod->_scaled->create(dstw, dsth, frame->format);
					}

					renderManager->scaleBuffer(frame->getPixels(), nod->_scaled->getPixels(), frame->w, frame->h, frame->format.bytesPerPixel, dstw, dsth);
					frame = nod->_scaled;
				}

				if (isPanorama) {
					Graphics::Surface *transposed = RenderManager::tranposeSurface(frame);
					renderManager->blitSurfaceToBkg(*transposed, nod->pos.left, nod->pos.top, _mask);
					transposed->free();
					delete transposed;
				} else {
					renderManager->blitSurfaceToBkg(*frame, nod->pos.left, nod->pos.top, _mask);
				}
			}
		}
	}

	return false;
}

void AnimationEffect::addPlayNode(int32 slot, int x, int y, int x2, int y2, int startFrame, int endFrame, int loops) {
	playnode nod;
	nod.loop = loops;
	nod.pos = Common::Rect(x, y, x2 + 1, y2 + 1);
	nod.start = startFrame;
	nod.stop = CLIP<int>(endFrame, 0, _animation->getFrameCount() - 1);
	nod.slot = slot;
	nod._curFrame = -1;
	nod._delay = 0;
	nod._scaled = NULL;
	_playList.push_back(nod);
}

bool AnimationEffect::stop() {
	PlayNodes::iterator it = _playList.begin();
	if (it != _playList.end()) {
		_engine->getScriptManager()->setStateValue((*it).slot, 2);
		if ((*it)._scaled) {
			(*it)._scaled->free();
			delete(*it)._scaled;
		}
	}

	_playList.clear();

	// We don't need to delete, it's may be reused
	return false;
}

} // End of namespace ZVision
