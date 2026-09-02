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

#include "common/archive.h"
#include "common/scummsys.h"
#ifdef USE_BINK
#include "video/bink_decoder.h"
#endif

#include "mohawk/video.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/transition_logo.h"
#include "mohawk/zoombini_sound.h"

namespace Mohawk {

ZoombiniTransitionLogo::ZoombiniTransitionLogo(MohawkEngine_Zoombini *vm) : ZoombiniTransition(vm, ZoombiniPageType::kLogo) {
}

ZoombiniTransitionLogo::~ZoombiniTransitionLogo() {
	if (_cdtoonsVideo)
		_vm->_video->removeEntry(_cdtoonsVideo);

#ifdef USE_BINK
	if (_binkDecoder) {
		delete _binkDecoder;
		_binkDecoder = nullptr;
	}
#endif

	if (_switchedToTrueColor)
		_vm->_gfx->reinitGraphics(false);
}

void ZoombiniTransitionLogo::loadFeatures() {
	_vm->setNextPage(_vm->isDemo() ? ZoombiniPageType::kNet : ZoombiniPageType::kPicker);

	// 1.x: LOGO025.MOV (CDToons)
	// 2.0 retail: LOGO025.BIK (Bink)
	// 2.0 demo: LOGODEMO.BIK (Bink)
	if (_vm->isVersionFamilyTlcV2()) {
		const char *videoFile = _vm->isDemo() ? ZMB_VIDEO_BINK_DEMO : ZMB_VIDEO_BINK;
		const Common::Path videoPath = Common::Path(_vm->getArchiveRoot()).append(videoFile);
		if (!Common::File::exists(videoPath)) {
			warning("Bink video file [%s] does not exist, skip", videoPath.toString().c_str());
			close();
			return;
		}

#ifdef USE_BINK
		// Bink requires a true-color pixel format (2 or 4 bpp).
		// Switch OSystem and internal buffers to true-color for the duration
		// of logo playback; reinitGraphics(false) in the destructor restores CLUT8.
		_vm->_gfx->reinitGraphics(true);
		_switchedToTrueColor = true;
		_vm->_gfx->clearScreens();

		_binkDecoder = new Video::BinkDecoder();
		_binkDecoder->setSoundType(Audio::Mixer::kSFXSoundType);
		if (_binkDecoder->loadFile(videoPath)) {
			_binkDecoder->setOutputPixelFormat(_vm->_system->getScreenFormat());
			_binkDecoder->start();
			_binkFrame = nullptr;
			_demoStartupRevealActive = _vm->consumeDemoStartupLogoReveal();
			_demoStartupRevealStartFrame = 0;
		} else {
			delete _binkDecoder;
			_binkDecoder = nullptr;

			warning("Failed to load bink video [%s], skip", videoPath.toString().c_str());
			close();
			return;
		}
#else
		warning("Bink video support is not available, cannot load bink video [%s], skip", videoPath.toString().c_str());
		close();
		return;
#endif
	} else {
		_vm->_gfx->clearScreens();

		const Common::Path videoPath = Common::Path(_vm->getArchiveRoot()).append(ZMB_VIDEO_CDTOONS);
		if (!Common::File::exists(videoPath)) {
			warning("CDToons video file [%s] does not exist, skip", videoPath.toString().c_str());
			close();
			return;
		}
		_cdtoonsVideo = _vm->_video->playMovie(videoPath.toString().c_str(), Audio::Mixer::kSFXSoundType);
		if (!_cdtoonsVideo) {
			warning("Failed to open the CDToons video [%s], skip", videoPath.toString().c_str());
			close();
			return;
		}

		_cdtoonsVideo->center();
	}
}

void ZoombiniTransitionLogo::onEveryFrame() {
	if (_vm->isVersionFamilyTlcV2()) {
#ifdef USE_BINK
		if (!_binkDecoder || _binkDecoder->endOfVideo()) {
			close();
			return;
		}

		bool decodedFrameChanged = false;
		if (_binkDecoder->needsUpdate()) {
			const Graphics::Surface *frame = _binkDecoder->decodeNextFrame();
			if (frame) {
				const bool firstDecodedFrame = !_binkFrame;
				_binkFrame = frame;
				decodedFrameChanged = true;
				if (firstDecodedFrame && _demoStartupRevealActive)
					_demoStartupRevealStartFrame = _currentFrameCounter;
			}
		}

		if (_binkFrame && (decodedFrameChanged || _demoStartupRevealActive)) {
			// Write the decoded frame into the graphics compositor's shape
			// screen so that the normal flushScreens() pipeline delivers it
			// to the display. The video is 640x480, matching the game screen.
			_vm->_gfx->getShapeScreen()->copyRectToSurface(*_binkFrame, 0, 0, Common::Rect(_binkFrame->w, _binkFrame->h));
			applyDemoStartupRevealMask();
			_vm->_gfx->setDirty();
		}
#endif
	} else {
		if (!_cdtoonsVideo || _cdtoonsVideo->endOfVideo()) {
			close();
			return;
		}

		_vm->_video->updateMovies();
	}
}

void ZoombiniTransitionLogo::onAnimFrame() {
}

void ZoombiniTransitionLogo::applyDemoStartupRevealMask() {
	if (!_demoStartupRevealActive)
		return;

	// The demo reveals its first logo through a fixed sequence of 16x16 regions.
	// ScummVM recreates that presentation over the decoder's complete surface.
	// kRevealStateRows is a 30 by 40 temporal mask for the 640 by 480 screen, not ASCII art.
	// Its character at [tileY][tileX] gives the first reveal state at which that tile remains visible.
	// Codes are a base-24 state index: '0' through '9' mean states 0 through 9, and 'A' through 'N' mean states 10 through 23.
	// kRevealStateStartFrames maps each state index to its first animation frame.
	// Until the current state reaches a tile's code, that tile is filled black over the decoded video frame.
	static constexpr byte kRevealStateStartFrames[] = {
		0, 1, 2, 5, 6, 7, 10, 11, 12, 15, 16, 19,
		20, 21, 25, 26, 30, 31, 35, 39, 40, 44, 45, 50
	};
	static constexpr char kRevealStateRows[][41] = {
		"0J06E3B0I300G0E30I630390B033300303300300",
		"BI3360063333093060003E3300I9300000000003",
		"9E063EC6090030300CGN6933000K006000030036",
		"03GKGN99E960300090LL0KC9903CNE0333060090",
		"CELLI33K0003060006G0KLG00009E60630003030",
		"93M30006000036000996KC003G33C0300C630303",
		"033003C90900000GCC0000003360C03060360000",
		"003003600600C6003606E300000060300000C000",
		"0339009C9693C33C3C0006066F300900303I0C00",
		"0000000C930630000003303C0369000030000000",
		"00300300C0933600033300000003330600003633",
		"0300000000C00060300600900960303003030000",
		"0300003000003333004440000444600644400000",
		"000011111111144111111116C4161496411C6446",
		"1141111111111111441117911411119474111411",
		"4711111441414411111471714111147171711111",
		"1111111A14111114741117441447141174414111",
		"11171111FC111771A1414417744AAA1714A41714",
		"1111111111441111144444141744114441471411",
		"1141111111111441741144117171114111114111",
		"1111111111111711141111141111141111411111",
		"111111111111111441117A174711744111111111",
		"111111111111411111414DA41171444141417111",
		"11111111111144111117A71711114A4114111141",
		"111114111111111111144H1A1117111111111111",
		"1114111144111141441A11141111111111111111",
		"114411111171D411711411141AA1111111111151",
		"11555151115277557A5222552A25255255522225",
		"2222858222228285228222522522228282552252",
		"22828522222225852825555D5585582222222522"
	};

	const uint32 revealFrame = _currentFrameCounter - _demoStartupRevealStartFrame;
	byte revealState = 0;
	for (byte stateIdx = 1; stateIdx < ARRAYSIZE(kRevealStateStartFrames); stateIdx++) {
		if (revealFrame < kRevealStateStartFrames[stateIdx])
			break;

		revealState = stateIdx;
	}

	Graphics::Surface *shapeScreen = _vm->_gfx->getShapeScreen();
	const uint32 black = shapeScreen->format.RGBToColor(0, 0, 0);

	for (byte tileY = 0; tileY < ARRAYSIZE(kRevealStateRows); tileY++) {
		for (byte tileX = 0; tileX < 40; tileX++) {
			const char revealCode = kRevealStateRows[tileY][tileX];
			byte tileRevealState;
			if (revealCode <= '9')
				tileRevealState = static_cast<byte>(revealCode - '0');
			else
				tileRevealState = static_cast<byte>(revealCode - 'A' + 10);
			if (revealState < tileRevealState) {
				const int left = tileX * 16;
				const int top = tileY * 16;
				shapeScreen->fillRect(Common::Rect(left, top, MIN<int>(left + 16, shapeScreen->w), MIN<int>(top + 16, shapeScreen->h)), black);
			}
		}
	}

	if (kRevealStateStartFrames[ARRAYSIZE(kRevealStateStartFrames) - 1] < revealFrame)
		_demoStartupRevealActive = false;
}

} // End of namespace Mohawk
