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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/scripting/sidefx/animation_node.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"

#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace ZVision {

AnimationNode::AnimationNode(ZVision *engine, uint32 controlKey, const Common::String &fileName, int32 mask, int32 frate, bool DisposeAfterUse)
	: SideFX(engine, controlKey, SIDEFX_ANIM),
	  _DisposeAfterUse(DisposeAfterUse),
	  _mask(mask),
	  _animation(NULL) {

	_animation = engine->loadAnimation(fileName);
	_frmDelay = 1000.0 / _animation->getDuration().framerate();

	if (frate > 0)
		_frmDelay = 1000.0 / frate;

	// WORKAROUND: We do not allow the engine to delay more than 66 msec
	// per frame (15fps max)
	if (_frmDelay > 66)
		_frmDelay = 66;
}

AnimationNode::~AnimationNode() {
	if (_animation)
		delete _animation;

	_engine->getScriptManager()->setStateValue(_key, 2);

	PlayNodes::iterator it = _playList.begin();
	if (it != _playList.end()) {
		_engine->getScriptManager()->setStateValue((*it).slot, 2);

		if ((*it)._scaled)
			delete(*it)._scaled;
	}

	_playList.clear();
}

bool AnimationNode::process(uint32 deltaTimeInMillis) {
	PlayNodes::iterator it = _playList.begin();
	if (it != _playList.end()) {
		playnode *nod = &(*it);

		nod->_delay -= deltaTimeInMillis;
		if (nod->_delay <= 0) {
			nod->_delay += _frmDelay;

			const Graphics::Surface *frame = NULL;

			if (nod->_curFrame == -1) { // Start of new playlist node
				nod->_curFrame = nod->start;

				_animation->seekToFrame(nod->_curFrame);
				frame = _animation->decodeNextFrame();

				nod->_delay = _frmDelay;
				if (nod->slot)
					_engine->getScriptManager()->setStateValue(nod->slot, 1);
			} else {
				nod->_curFrame++;

				if (nod->_curFrame > nod->stop) {
					nod->loop--;

					if (nod->loop == 0) {
						if (nod->slot >= 0)
							_engine->getScriptManager()->setStateValue(nod->slot, 2);
						if (nod->_scaled)
							delete nod->_scaled;
						_playList.erase(it);
						return _DisposeAfterUse;
					}

					nod->_curFrame = nod->start;
					_animation->seekToFrame(nod->_curFrame);
				}

				frame = _animation->decodeNextFrame();
			}

			if (frame) {

				uint32 dstw;
				uint32 dsth;
				if (_engine->getRenderManager()->getRenderTable()->getRenderState() == RenderTable::PANORAMA) {
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
							delete nod->_scaled;
							nod->_scaled = NULL;
						}

					if (!nod->_scaled) {
						nod->_scaled = new Graphics::Surface;
						nod->_scaled->create(dstw, dsth, frame->format);
					}

					_engine->getRenderManager()->scaleBuffer(frame->getPixels(), nod->_scaled->getPixels(), frame->w, frame->h, frame->format.bytesPerPixel, dstw, dsth);
					frame = nod->_scaled;
				}

				if (_engine->getRenderManager()->getRenderTable()->getRenderState() == RenderTable::PANORAMA) {
					Graphics::Surface *transposed = RenderManager::tranposeSurface(frame);
					if (_mask > 0)
						_engine->getRenderManager()->blitSurfaceToBkg(*transposed, nod->pos.left, nod->pos.top, _mask);
					else
						_engine->getRenderManager()->blitSurfaceToBkg(*transposed, nod->pos.left, nod->pos.top);
					delete transposed;
				} else {
					if (_mask > 0)
						_engine->getRenderManager()->blitSurfaceToBkg(*frame, nod->pos.left, nod->pos.top, _mask);
					else
						_engine->getRenderManager()->blitSurfaceToBkg(*frame, nod->pos.left, nod->pos.top);
				}
			}
		}
	}

	return false;
}

void AnimationNode::addPlayNode(int32 slot, int x, int y, int x2, int y2, int startFrame, int endFrame, int loops) {
	playnode nod;
	nod.loop = loops;
	nod.pos = Common::Rect(x, y, x2 + 1, y2 + 1);
	nod.start = startFrame;
	nod.stop = endFrame;

	if (nod.stop >= (int)_animation->getFrameCount())
		nod.stop = _animation->getFrameCount() - 1;

	nod.slot = slot;
	nod._curFrame = -1;
	nod._delay = 0;
	nod._scaled = NULL;
	_playList.push_back(nod);
}

bool AnimationNode::stop() {
	PlayNodes::iterator it = _playList.begin();
	if (it != _playList.end()) {
		_engine->getScriptManager()->setStateValue((*it).slot, 2);
		if ((*it)._scaled)
			delete(*it)._scaled;
	}

	_playList.clear();

	// We don't need to delete, it's may be reused
	return false;
}

int32 AnimationNode::getFrameDelay() {
	return _frmDelay;
}

} // End of namespace ZVision
