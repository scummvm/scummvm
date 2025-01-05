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

#include "dgds/head.h"
#include "dgds/dgds.h"
#include "dgds/image.h"
#include "dgds/sound_raw.h"

namespace Dgds {

void TalkDataHead::drawHead(Graphics::ManagedSurface *dst, const TalkData &data) const {
	uint drawtype = _drawType ? _drawType : 1;
	// Use specific head shape if available (eg, in Willy Beamish), if not use talk data shape
	Common::SharedPtr<Image> img = _shape;
	if (!img)
		img = data._shape;
	if (!img)
		return;
	switch (drawtype) {
	case 1:
		drawHeadType1(dst, *img);
		break;
	case 2:
		drawHeadType2(dst, *img);
		break;
	case 3:
		if (DgdsEngine::getInstance()->getGameId() == GID_WILLY)
			drawHeadType3Beamish(dst, data);
		else
			drawHeadType3(dst, *img);
		break;
	default:
		error("Unsupported head draw type %d", drawtype);
	}
}

void TalkDataHead::drawHeadType1(Graphics::ManagedSurface *dst, const Image &img) const {
	Common::Rect r = _rect.toCommonRect();
	dst->fillRect(r, _drawCol);
	r.grow(-1);
	dst->fillRect(r, _drawCol == 0 ? 15 : 0);
	r.left += 2;
	r.top += 2;
	const int x = _rect.x;
	const int y = _rect.y;
	if (img.isLoaded()) {
		for (const auto &frame : _headFrames) {
			img.drawBitmap(frame._frameNo & 0xff, x + frame._xoff, y + frame._yoff, r, *dst);
		}
	}
}

void TalkDataHead::drawHeadType2(Graphics::ManagedSurface *dst, const Image &img) const {
	if (!img.isLoaded())
		return;
	const Common::Rect r = _rect.toCommonRect();
	for (const auto &frame : _headFrames) {
		img.drawBitmap(frame._frameNo & 0xff, r.left + frame._xoff, r.top + frame._yoff, r, *dst);
	}
}

void TalkDataHead::drawHeadType3Beamish(Graphics::ManagedSurface *dst, const TalkData &data) const {
	const Common::Rect r = _rect.toCommonRect();

	// Note: only really need the 1px border here but just fill the box.
	dst->fillRect(r, 8);

	Common::Rect fillRect(r);
	fillRect.grow(-1);
	dst->fillRect(fillRect, _drawCol);

	for (const auto &frame : _headFrames) {
		int frameNo = frame._frameNo & 0x7fff;
		bool useHeadShape = frame._frameNo & 0x8000;

		Common::SharedPtr<Image> img = useHeadShape ? _shape : data._shape;
		if (!img || !img->isLoaded() || frameNo >= img->loadedFrameCount())
			continue;

		ImageFlipMode flip = kImageFlipNone;
		// Yes, the numerical values are reversed here (1 -> 2 and 2 -> 1).
		// The head flip flags are reversed from the image draw flags.
		if (frame._flipFlags & 1)
			flip = static_cast<ImageFlipMode>(flip & kImageFlipH);
		if (frame._flipFlags & 2)
			flip = static_cast<ImageFlipMode>(flip & kImageFlipV);

		img->drawBitmap(frameNo, r.left + frame._xoff, r.top + frame._yoff, fillRect, *dst);
	}
}

void TalkDataHead::drawHeadType3(Graphics::ManagedSurface *dst, const Image &img) const {
	Common::Rect r = _rect.toCommonRect();
	dst->fillRect(r, 0);
	if (!img.isLoaded())
		return;
	for (const auto &frame : _headFrames) {
		int frameNo = frame._frameNo;
		if (frameNo < img.loadedFrameCount())
			img.drawBitmap(frameNo, r.left + frame._xoff, r.top + frame._yoff, r, *dst);
		else
			dst->fillRect(r, 4);
	}
}

void TalkDataHead::updateHead() {
	warning("TODO: Update head");
	_flags = static_cast<HeadFlags>(_flags & ~(kHeadFlag1 | kHeadFlag8 | kHeadFlag10 | kHeadFlagVisible));

	/* This seems to just be a "needs redraw" flag, but we always redraw
	for (auto tds : _talkData) {
		for (auto h : tds._heads) {
			if ((h._flags & kHeadFlagVisible) && !(h._flags & (kHeadFlag8 | kHeadFlag10 | kHeadFlag80))) {
				if (h._rect.toCommonRect().intersects(head._rect.toCommonRect())) {
					h._flags = static_cast<HeadFlags>(h._flags | kHeadFlag4);
				}
			}
		}
	}
	*/
}


void TalkData::updateVisibleHeads() {
	for (auto &head : _heads) {
		if (head._flags & kHeadFlagVisible)
			head.updateHead();
	}
}

void TalkData::drawVisibleHeads(Graphics::ManagedSurface *dst) const {
	for (const auto &h : _heads) {
		if ((h._flags & kHeadFlagVisible) && !(h._flags & kHeadFlag40)) {
			h.drawHead(dst, *this);
		}
	}
}

bool TalkData::hasVisibleHead() const {
	for (const auto &h : _heads) {
		if (h._flags & kHeadFlagVisible)
			return true;
	}
	return false;
}

//////

Conversation::~Conversation() {
	unload();
}

void Conversation::unload() {
	if (_sound) {
		_sound->stop();
		_sound.reset();
	}
	_img.reset();
	_ttmScript.reset();
	if (_ttmEnv._soundRaw)
		_ttmEnv._soundRaw->stop();
	_ttmEnv = TTMEnviro();
}

void Conversation::loadData(uint16 dlgFileNum, uint16 dlgNum, int16 sub) {
	unload();

	DgdsEngine *engine = DgdsEngine::getInstance();

	// These files are only present in Willy Beamish CD version
	if (engine->getGameId() != GID_WILLY)
		return;

	ResourceManager *resourceManager = engine->getResourceManager();
	Decompressor *decompressor = engine->getDecompressor();

	Common::String fname;
	if (sub >= 0) {
		assert(sub < 26);
		fname = Common::String::format("F%dB%d%c.CDS", dlgFileNum, dlgNum, 'A' + sub);
	} else {
		fname = Common::String::format("F%dB%d.CDS", dlgFileNum, dlgNum);
	}

	if (!resourceManager->hasResource(fname))
		return;

	debug(10, "CDS: Load CDS resource %s", fname.c_str());

	_sound.reset(new SoundRaw(resourceManager, decompressor));
	_sound->load(fname);
	_img.reset(new Image(resourceManager, decompressor));
	_img->loadBitmap(fname);
	_ttmScript.reset(new TTMInterpreter(engine));
	_ttmScript->load(fname, _ttmEnv);
	_ttmScript->findAndAddSequences(_ttmEnv, _ttmSeqs);

	// The scripts are designed so the resources are patchable, but by default
	// they use the sound and image data from the CDS file.
	_ttmEnv._soundRaw = _sound;
	_ttmEnv._scriptShapes[0] = _img;

	// Always run seq 1 on init.
	_ttmEnv._cdsSeqNum = 1;
	runScript();
}

void Conversation::runScript() {
	if (!_ttmScript)
		return;

	DgdsEngine *engine = DgdsEngine::getInstance();
	if (_nextExec && engine->getThisFrameMs() < _nextExec)
		return;

	_nextExec = 0;
	_ttmEnv._xOff = _drawRect.x;
	_ttmEnv._yOff = _drawRect.y;

	for (auto seq : _ttmSeqs) {
		if (seq->_seqNum == _ttmEnv._cdsSeqNum && seq->_currentFrame < (int)_ttmEnv._frameOffsets.size()) {
			debug(10, "CDS: Running TTM sequence %d frame %d", seq->_seqNum, seq->_currentFrame);
			_ttmEnv.scr->seek(_ttmEnv._frameOffsets[seq->_currentFrame]);

			seq->_drawWin = _drawRect.toCommonRect();
			_ttmScript->run(_ttmEnv, *seq);
			if (_ttmEnv._cdsDelay) {
				_nextExec = engine->getThisFrameMs() + _ttmEnv._cdsDelay;
				_ttmEnv._cdsDelay = 0;
			} else {
				seq->_currentFrame++;
			}
		}
	}
}

} // end namespace Dgds
