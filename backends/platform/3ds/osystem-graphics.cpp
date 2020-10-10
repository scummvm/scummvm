/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This _program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This _program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this _program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 *
 */

#include "backends/platform/3ds/osystem.h"
#include "backends/platform/3ds/shader_shbin.h"
#include "backends/platform/3ds/options-dialog.h"
#include "backends/platform/3ds/config.h"
#include "common/rect.h"
#include "graphics/fontman.h"
#include "gui/gui-manager.h"

// Used to transfer the final rendered display to the framebuffer
#define DISPLAY_TRANSFER_FLAGS                                                    \
        (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) |                    \
         GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
         GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |                           \
         GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
#define TEXTURE_TRANSFER_FLAGS(fmt)                             \
        (GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) |  \
         GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(fmt) | \
         GX_TRANSFER_OUT_FORMAT(fmt) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
#define DEFAULT_MODE _modeRGBA8

namespace _3DS {
/* Group the various enums, values, etc. needed for
 * each graphics mode into instaces of GfxMode3DS */
static const GfxMode3DS _modeRGBA8 = { Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0),
                                       GPU_RGBA8, TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGBA8) };
static const GfxMode3DS _modeRGB565 = { Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),
                                        GPU_RGB565, TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB565) };
static const GfxMode3DS _modeRGB555 = { Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0),
                                        GPU_RGBA5551, TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB5A1) };
static const GfxMode3DS _modeRGB5A1 = { Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0),
                                        GPU_RGBA5551, TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB5A1) };
static const GfxMode3DS _modeCLUT8 = _modeRGBA8;

static const GfxMode3DS *gfxModes[] = { &_modeRGBA8, &_modeRGB565, &_modeRGB555, &_modeRGB5A1, &_modeCLUT8 };


void OSystem_3DS::init3DSGraphics() {
	_gfxState.gfxMode = gfxModes[CLUT8];
	_pfGame = Graphics::PixelFormat::createFormatCLUT8();
	_pfDefaultTexture = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	// Initialize the render targets

	int topScreenWidth = gfxIsWide() ? 800 : 400;

	_renderTargetTop =
	    C3D_RenderTargetCreate(240, topScreenWidth, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetClear(_renderTargetTop, C3D_CLEAR_ALL, 0x0000000, 0);
	C3D_RenderTargetSetOutput(_renderTargetTop, GFX_TOP, GFX_LEFT,
	                          DISPLAY_TRANSFER_FLAGS);

	_renderTargetBottom =
	    C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetClear(_renderTargetBottom, C3D_CLEAR_ALL, 0x00000000, 0);
	C3D_RenderTargetSetOutput(_renderTargetBottom, GFX_BOTTOM, GFX_LEFT,
	                          DISPLAY_TRANSFER_FLAGS);

	// Load and bind simple default shader (shader.v.pica)
	_dvlb = DVLB_ParseFile((u32*)shader_shbin, shader_shbin_size);
	shaderProgramInit(&_program);
	shaderProgramSetVsh(&_program, &_dvlb->DVLE[0]);
	C3D_BindProgram(&_program);

	_projectionLocation = shaderInstanceGetUniformLocation(_program.vertexShader, "projection");
	_modelviewLocation = shaderInstanceGetUniformLocation(_program.vertexShader, "modelView");

	C3D_AttrInfo *attrInfo = C3D_GetAttrInfo();
	AttrInfo_Init(attrInfo);
	AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
	AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord

	Mtx_OrthoTilt(&_projectionTop, 0.0, 400.0, 240.0, 0.0, 0.0, 1.0, true);
	Mtx_OrthoTilt(&_projectionBottom, 0.0, 320.0, 240.0, 0.0, 0.0, 1.0, true);

	C3D_TexEnv *env = C3D_GetTexEnv(0);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
	C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

	C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_ALL);
	C3D_CullFace(GPU_CULL_NONE);
}

void OSystem_3DS::destroy3DSGraphics() {
	_gameScreen.free();
	_gameTopTexture.free();
	_gameBottomTexture.free();
	_cursor.free();
	_cursorTexture.free();
	_overlay.free();
	_activityIcon.free();

	shaderProgramFree(&_program);
	DVLB_Free(_dvlb);

	C3D_RenderTargetDelete(_renderTargetTop);
	C3D_RenderTargetDelete(_renderTargetBottom);

	C3D_Fini();
}

bool OSystem_3DS::hasFeature(OSystem::Feature f) {
	return (f == OSystem::kFeatureCursorPalette ||
	        f == OSystem::kFeatureFilteringMode ||
	        f == OSystem::kFeatureOverlaySupportsAlpha ||
	        f == OSystem::kFeatureKbdMouseSpeed ||
	        f == OSystem::kFeatureJoystickDeadzone);
}

void OSystem_3DS::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureCursorPalette:
		_cursorPaletteEnabled = enable;
		flushCursor();
		break;
	case OSystem::kFeatureFilteringMode:
		_filteringEnabled = enable;
		break;
	default:
		break;
	}
}

bool OSystem_3DS::getFeatureState(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureCursorPalette:
		return _cursorPaletteEnabled;
	case OSystem::kFeatureFilteringMode:
		return _filteringEnabled;
	default:
		return false;
	}
}

GraphicsModeID OSystem_3DS::chooseMode(Graphics::PixelFormat *format) {
	if (format->bytesPerPixel > 2) {
		return RGBA8;
	} else if (format->bytesPerPixel > 1) {
		if (format->gBits() > 5) {
			return RGB565;
		} else if (format->aBits() == 0) {
			return RGB555;
		} else {
			return RGB5A1;
		}
	}
	return CLUT8;
}

bool OSystem_3DS::setGraphicsMode(GraphicsModeID modeID) {
	switch (modeID) {
	case RGBA8:
	case RGB565:
	case RGB555:
	case RGB5A1:
	case CLUT8:
		_gfxState.gfxMode = gfxModes[modeID];
		return true;
	default:
		return false;
	}
}

void OSystem_3DS::initSize(uint width, uint height,
                           const Graphics::PixelFormat *format) {
	debug("3ds initsize w:%d h:%d", width, height);
	int oldScreen = config.screen;
	loadConfig();
	if (config.screen != oldScreen) {
		_screenChangeId++;
	}

	_gameWidth = width;
	_gameHeight = height;
	_magCenterX = _magWidth / 2;
	_magCenterY = _magHeight / 2;

	_oldPfGame = _pfGame;
	if (!format) {
		_pfGame = Graphics::PixelFormat::createFormatCLUT8();
	} else {
		debug("pixelformat: %d %d %d %d %d", format->bytesPerPixel, format->rBits(), format->gBits(), format->bBits(), format->aBits());
		_pfGame = *format;
	}

	/* If the current graphics mode does not fit with the pixel
	 * format being requested, choose one that does and switch to it */
	assert(_pfGame.bytesPerPixel > 0);
	if (_pfGame != _oldPfGame) {
		assert(_transactionState == kTransactionActive);
		_gfxState.gfxModeID = chooseMode(&_pfGame);
		_transactionDetails.formatChanged = true;
	}

	_gameTopTexture.create(width, height, _gfxState.gfxMode);
	_overlay.create(400, 320, &DEFAULT_MODE);
	_gameScreen.create(width, height, _pfGame);

	_focusDirty = true;
	_focusRect = Common::Rect(_gameWidth, _gameHeight);

	updateSize();
}

void OSystem_3DS::updateSize() {
	if (config.stretchToFit) {
		_gameTopX = _gameTopY = _gameBottomX = _gameBottomY = 0;
		_gameTopTexture.setScale(400.f / _gameWidth, 240.f / _gameHeight);
		_gameBottomTexture.setScale(320.f / _gameWidth, 240.f / _gameHeight);
	} else {
		float ratio = static_cast<float>(_gameWidth) / _gameHeight;

		if (ratio > 400.f / 240.f) {
			float r = 400.f / _gameWidth;
			_gameTopTexture.setScale(r, r);
			_gameTopX = 0;
			_gameTopY = (240.f / r - _gameHeight) / 2.f;
		} else {
			float r = 240.f / _gameHeight;
			_gameTopTexture.setScale(r, r);
			_gameTopY = 0;
			_gameTopX = (400.f / r - _gameWidth) / 2.f;
		}
		if (ratio > 320.f / 240.f) {
			float r = 320.f / _gameWidth;
			_gameBottomTexture.setScale(r, r);
			_gameBottomX = 0;
			_gameBottomY = (240.f / r - _gameHeight) / 2.f;
		} else {
			float r = 240.f / _gameHeight;
			_gameBottomTexture.setScale(r, r);
			_gameBottomY = 0;
			_gameBottomX = (320.f / r - _gameWidth) / 2.f;
		}
	}
	_gameTopTexture.setPosition(_gameTopX, _gameTopY);
	_gameBottomTexture.setPosition(_gameBottomX, _gameBottomY);
	_gameTopTexture.setOffset(0, 0);
	_gameBottomTexture.setOffset(0, 0);
	if (_overlayVisible) {
		_cursorTexture.setScale(1.f, 1.f);
	} else if (config.screen == kScreenTop) {
		_cursorTexture.setScale(_gameTopTexture.getScaleX(), _gameTopTexture.getScaleY());
	} else {
		_cursorTexture.setScale(_gameBottomTexture.getScaleX(), _gameBottomTexture.getScaleY());
	}
}

Common::List<Graphics::PixelFormat> OSystem_3DS::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> list;
	list.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)); // GPU_RGBA8
	list.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)); // GPU_RGB565
	list.push_back(Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0)); // RGB555 (needed for FMTOWNS?)
	list.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0)); // GPU_RGBA5551
	list.push_back(Graphics::PixelFormat::createFormatCLUT8());
	return list;
}

void OSystem_3DS::beginGFXTransaction() {
	assert(_transactionState == kTransactionNone);
	_transactionState = kTransactionActive;
	_transactionDetails.formatChanged = false;
	_oldGfxState = _gfxState;
}

OSystem::TransactionError OSystem_3DS::endGFXTransaction() {
	int errors = OSystem::kTransactionSuccess;

	assert(_transactionState != kTransactionNone);
	if (_transactionState == kTransactionRollback) {
		if (_gfxState.gfxModeID != _oldGfxState.gfxModeID) {
			errors |= OSystem::kTransactionModeSwitchFailed;
			_gfxState = _oldGfxState;
		} else if ((_gfxState.gfxMode != _oldGfxState.gfxMode) |
		           (_gfxState.gfxMode != gfxModes[_gfxState.gfxModeID])) {
			errors |= OSystem::kTransactionFormatNotSupported;
			_gfxState = _oldGfxState;
		}

		_oldGfxState.setup = false;
	}
	if (_transactionDetails.formatChanged) {
		if (!setGraphicsMode(_gfxState.gfxModeID)) {
			if (_oldGfxState.setup) {
				_transactionState = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else if (_gfxState.gfxMode != gfxModes[_gfxState.gfxModeID]) {
			if (_oldGfxState.setup) {
				_transactionState = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			initSize(_gameWidth, _gameHeight, &_pfGame);
			clearOverlay();
			_gfxState.setup = true;
			_screenChangeId++;
		}
	}

	_transactionState = kTransactionNone;
	return (OSystem::TransactionError)errors;
}

float OSystem_3DS::getScaleRatio() const {
	if (_overlayVisible) {
		return 1.0;
	} else if (config.screen == kScreenTop) {
		return _gameTopTexture.getScaleX();
	} else {
		return _gameBottomTexture.getScaleX();
	}
}

void OSystem_3DS::setPalette(const byte *colors, uint start, uint num) {
	assert(start + num <= 256);
	memcpy(_palette + 3 * start, colors, 3 * num);
	_gameTextureDirty = true;
}

void OSystem_3DS::grabPalette(byte *colors, uint start, uint num) const {
	assert(start + num <= 256);
	memcpy(colors, _palette + 3 * start, 3 * num);
}

static void copyRect555To5551(const Graphics::Surface &srcSurface, Graphics::Surface &destSurface, uint16 destX, uint16 destY, const Common::Rect &srcRect) {
	const uint16 *src = (const uint16 *)srcSurface.getBasePtr(srcRect.left, srcRect.top);
	uint16 *dst = (uint16 *)destSurface.getBasePtr(destX, destY);
	for (int i = 0; i < srcRect.height(); i++) {
		for (int j = 0; j < srcRect.width(); j++) {
			*dst++ = (*src++ << 1) | 1;
		}
		src += srcSurface.pitch / 2 - srcRect.width();
		dst += destSurface.pitch / 2 - srcRect.width();
	}
}

void OSystem_3DS::copyRectToScreen(const void *buf, int pitch, int x,
                                   int y, int w, int h) {
	Common::Rect rect(x, y, x+w, y+h);
	_gameScreen.copyRectToSurface(buf, pitch, x, y, w, h);
	Graphics::Surface subSurface = _gameScreen.getSubArea(rect);

	if (_pfGame == _gameTopTexture.format) {
		_gameTopTexture.copyRectToSurface(subSurface, x, y, Common::Rect(w, h));
	} else if (_gfxState.gfxMode == &_modeRGB555) {
		copyRect555To5551(subSurface, _gameTopTexture, x, y, Common::Rect(w, h));
	} else {
		Graphics::Surface *convertedSubSurface = subSurface.convertTo(_gameTopTexture.format, _palette);
		_gameTopTexture.copyRectToSurface(*convertedSubSurface, x, y, Common::Rect(w, h));
		convertedSubSurface->free();
		delete convertedSubSurface;
	}

	_gameTopTexture.markDirty();
}

void OSystem_3DS::flushGameScreen() {
	if (_pfGame == _gameTopTexture.format) {
		_gameTopTexture.copyRectToSurface(_gameScreen, 0, 0, Common::Rect(_gameScreen.w, _gameScreen.h));
	} else if (_gfxState.gfxMode == &_modeRGB555) {
		copyRect555To5551(_gameScreen, _gameTopTexture, 0, 0, Common::Rect(_gameScreen.w, _gameScreen.h));
	} else {
		Graphics::Surface *converted = _gameScreen.convertTo(_gameTopTexture.format, _palette);
		_gameTopTexture.copyRectToSurface(*converted, 0, 0, Common::Rect(converted->w, converted->h));
		converted->free();
		delete converted;
	}

	_gameTopTexture.markDirty();
}

Graphics::Surface *OSystem_3DS::lockScreen() {
	return &_gameScreen;
}
void OSystem_3DS::unlockScreen() {
	_gameTextureDirty = true;
}

void OSystem_3DS::updateScreen() {
	if (sleeping || exiting) {
		return;
	}

	if (_gameTextureDirty) {
		flushGameScreen();
		_gameTextureDirty = false;
	}

// 	updateFocus();
	updateMagnify();

	if (_osdMessage.getPixels() && _osdMessageEndTime <= getMillis(true)) {
		_osdMessage.free();
	}

	C3D_FrameBegin(0);
		_gameTopTexture.transfer();
		if (_overlayVisible) {
			_overlay.transfer();
		}
		if (_cursorVisible && config.showCursor) {
			_cursorTexture.transfer();
		}
		_osdMessage.transfer();
		_activityIcon.transfer();
	C3D_FrameEnd(0);

	C3D_FrameBegin(0);
		// Render top screen
		C3D_RenderTargetClear(_renderTargetTop, C3D_CLEAR_ALL, 0x00000000, 0);
		C3D_FrameDrawOn(_renderTargetTop);
		if (config.screen == kScreenTop || config.screen == kScreenBoth) {
			C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _projectionLocation, &_projectionTop);
			C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _gameTopTexture.getMatrix());
			_gameTopTexture.setFilteringMode(_magnifyMode != MODE_MAGON && _filteringEnabled);
			_gameTopTexture.render();
			if (_overlayVisible && config.screen == kScreenTop) {
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _overlay.getMatrix());
				_overlay.render();
			}
			if (_activityIcon.getPixels() && config.screen == kScreenTop) {
				_activityIcon.setPosition(400 - _activityIcon.actualWidth, 0);
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _activityIcon.getMatrix());
				_activityIcon.render();
			}
			if (_osdMessage.getPixels() && config.screen == kScreenTop) {
				_osdMessage.setPosition((400 - _osdMessage.actualWidth) / 2, (240 - _osdMessage.actualHeight) / 2);
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _osdMessage.getMatrix());
				_osdMessage.render();
			}
			if (_cursorVisible && config.showCursor && config.screen == kScreenTop) {
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _cursorTexture.getMatrix());
				_cursorTexture.setFilteringMode(!_overlayVisible && _filteringEnabled);
				_cursorTexture.render();
			}
		}

		// Render bottom screen
		C3D_RenderTargetClear(_renderTargetBottom, C3D_CLEAR_ALL, 0x00000000, 0);
		C3D_FrameDrawOn(_renderTargetBottom);
		if (config.screen == kScreenBottom || config.screen == kScreenBoth) {
			C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _projectionLocation, &_projectionBottom);
			C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _gameBottomTexture.getMatrix());
			_gameTopTexture.setFilteringMode(_filteringEnabled);
			_gameTopTexture.render();
			if (_overlayVisible) {
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _overlay.getMatrix());
				_overlay.render();
			}
			if (_activityIcon.getPixels()) {
				_activityIcon.setPosition(320 - _activityIcon.actualWidth, 0);
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _activityIcon.getMatrix());
				_activityIcon.render();
			}
			if (_osdMessage.getPixels()) {
				_osdMessage.setPosition((320 - _osdMessage.actualWidth) / 2, (240 - _osdMessage.actualHeight) / 2);
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _osdMessage.getMatrix());
				_osdMessage.render();
			}
			if (_cursorVisible && config.showCursor) {
				C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, _modelviewLocation, _cursorTexture.getMatrix());
				_cursorTexture.setFilteringMode(!_overlayVisible && _filteringEnabled);
				_cursorTexture.render();
			}
		}
	C3D_FrameEnd(0);
}

void OSystem_3DS::setShakePos(int shakeXOffset, int shakeYOffset) {
	// TODO: implement this in overlay, top screen, and mouse too
	_screenShakeXOffset = shakeXOffset;
	_screenShakeYOffset = shakeYOffset;
	int topX = _gameTopX + (_gameTopTexture.getScaleX() * shakeXOffset);
	int topY = _gameTopY + (_gameTopTexture.getScaleY() * shakeYOffset);
	_gameTopTexture.setPosition(topX, topY);
	int bottomX = _gameBottomX + (_gameBottomTexture.getScaleX() * shakeXOffset);
	int bottomY = _gameBottomY + (_gameBottomTexture.getScaleY() * shakeYOffset);
	_gameBottomTexture.setPosition(bottomX, bottomY);
}

void OSystem_3DS::setFocusRectangle(const Common::Rect &rect) {
	debug("setfocus: %d %d %d %d", rect.left, rect.top, rect.width(), rect.height());
	_focusRect = rect;
	_focusDirty = true;
	_focusClearTime = 0;
}

void OSystem_3DS::clearFocusRectangle() {
	_focusClearTime = getMillis();
}

void OSystem_3DS::updateFocus() {

	if (_focusClearTime && getMillis() - _focusClearTime > 5000) {
		_focusClearTime = 0;
		_focusDirty = true;
		_focusRect = Common::Rect(_gameWidth, _gameHeight);
	}

	if (_focusDirty) {
		float duration = 1.f / 20.f; // Focus animation in frame duration
		float w = 400.f;
		float h = 240.f;
		float ratio = _focusRect.width() / _focusRect.height();
		if (ratio > w/h) {
			_focusTargetScaleX = w / _focusRect.width();
			float newHeight = (float)_focusRect.width() / w/h;
			_focusTargetScaleY = h / newHeight;
			_focusTargetPosX = _focusTargetScaleX * _focusRect.left;
			_focusTargetPosY = _focusTargetScaleY * ((float)_focusRect.top - (newHeight - _focusRect.height())/2.f);
		} else {
			_focusTargetScaleY = h / _focusRect.height();
			float newWidth = (float)_focusRect.height() * w/h;
			_focusTargetScaleX = w / newWidth;
			_focusTargetPosY = _focusTargetScaleY * _focusRect.top;
			_focusTargetPosX = _focusTargetScaleX * ((float)_focusRect.left - (newWidth - _focusRect.width())/2.f);
		}
		if (_focusTargetPosX < 0 && _focusTargetScaleY != 240.f / _gameHeight) {
			_focusTargetPosX = 0;
		}
		if (_focusTargetPosY < 0 && _focusTargetScaleX != 400.f / _gameWidth) {
			_focusTargetPosY = 0;
		}
		_focusStepPosX = duration * (_focusTargetPosX - _focusPosX);
		_focusStepPosY = duration * (_focusTargetPosY - _focusPosY);
		_focusStepScaleX = duration * (_focusTargetScaleX - _focusScaleX);
		_focusStepScaleY = duration * (_focusTargetScaleY - _focusScaleY);
	}

	if (_focusDirty || _focusPosX != _focusTargetPosX || _focusPosY != _focusTargetPosY ||
	    _focusScaleX != _focusTargetScaleX || _focusScaleY != _focusTargetScaleY) {
		_focusDirty = false;

		if ((_focusStepPosX > 0 && _focusPosX > _focusTargetPosX) || (_focusStepPosX < 0 && _focusPosX < _focusTargetPosX)) {
			_focusPosX = _focusTargetPosX;
		} else if (_focusPosX != _focusTargetPosX) {
			_focusPosX += _focusStepPosX;
		}

		if ((_focusStepPosY > 0 && _focusPosY > _focusTargetPosY) || (_focusStepPosY < 0 && _focusPosY < _focusTargetPosY)) {
			_focusPosY = _focusTargetPosY;
		} else if (_focusPosY != _focusTargetPosY) {
			_focusPosY += _focusStepPosY;
		}

		if ((_focusStepScaleX > 0 && _focusScaleX > _focusTargetScaleX) || (_focusStepScaleX < 0 && _focusScaleX < _focusTargetScaleX)) {
			_focusScaleX = _focusTargetScaleX;
		} else if (_focusScaleX != _focusTargetScaleX) {
			_focusScaleX += _focusStepScaleX;
		}

		if ((_focusStepScaleY > 0 && _focusScaleY > _focusTargetScaleY) || (_focusStepScaleY < 0 && _focusScaleY < _focusTargetScaleY)) {
			_focusScaleY = _focusTargetScaleY;
		} else if (_focusScaleY != _focusTargetScaleY) {
			_focusScaleY += _focusStepScaleY;
		}

		Mtx_Identity(&_focusMatrix);
		Mtx_Translate(&_focusMatrix, -_focusPosX, -_focusPosY, 0, true);
		Mtx_Scale(&_focusMatrix, _focusScaleX, _focusScaleY, 1.f);
	}
}

void OSystem_3DS::updateMagnify() {
	if (_magnifyMode == MODE_MAGON && config.screen != kScreenBoth) {
		// Only allow to magnify when both screens are enabled
		_magnifyMode = MODE_MAGOFF;
	}

	if (_magnifyMode == MODE_MAGON) {
		if (!_overlayVisible) {
			_magX = (_cursorScreenX < _magCenterX) ?
			         0 : ((_cursorScreenX < (_gameWidth - _magCenterX)) ?
			         _cursorScreenX - _magCenterX : _gameWidth - _magWidth);
			_magY = (_cursorScreenY < _magCenterY) ?
			         0 : ((_cursorScreenY < _gameHeight - _magCenterY) ?
			         _cursorScreenY - _magCenterY : _gameHeight - _magHeight);
		}
		_gameTopTexture.setScale(1.f,1.f);
		_gameTopTexture.setPosition(0,0);
		_gameTopTexture.setOffset(_magX, _magY);
	}
}

void OSystem_3DS::showOverlay() {
	_overlayVisible = true;
	updateSize();
}

void OSystem_3DS::hideOverlay() {
	_overlayVisible = false;
	updateSize();
}

Graphics::PixelFormat OSystem_3DS::getOverlayFormat() const {
	return _overlay.format;
}

void OSystem_3DS::clearOverlay() {
	_overlay.clear();
}

void OSystem_3DS::grabOverlay(void *buf, int pitch) {
	byte *dst = (byte *)buf;

	for (int y = 0; y < getOverlayHeight(); ++y) {
		memcpy(dst, _overlay.getBasePtr(0, y), getOverlayWidth() * _overlay.format.bytesPerPixel);
		dst += pitch;
	}
}

void OSystem_3DS::copyRectToOverlay(const void *buf, int pitch, int x,
                                    int y, int w, int h) {
	_overlay.copyRectToSurface(buf, pitch, x, y, w, h);
	_overlay.markDirty();
}

void OSystem_3DS::displayMessageOnOSD(const Common::U32String &msg) {
	// The font we are going to use:
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);
	if (!font) {
		warning("No available font to render OSD messages");
		return;
	}

	// Split the message into separate lines.
	Common::Array<Common::U32String> lines;
	Common::U32String::const_iterator strLineItrBegin = msg.begin();

	for (Common::U32String::const_iterator itr = msg.begin(); itr != msg.end(); itr++) {
		if (*itr == '\n') {
			lines.push_back(Common::U32String(strLineItrBegin, itr));
			strLineItrBegin = itr + 1;
		}
	}
	if (strLineItrBegin != msg.end())
		lines.push_back(Common::U32String(strLineItrBegin, msg.end()));

	// Determine a rect which would contain the message string (clipped to the
	// screen dimensions).
	const int vOffset = 6;
	const int lineSpacing = 1;
	const int lineHeight = font->getFontHeight() + 2 * lineSpacing;
	int width = 0;
	int height = lineHeight * lines.size() + 2 * vOffset;
	uint i;
	for (i = 0; i < lines.size(); i++) {
		width = MAX(width, font->getStringWidth(lines[i]) + 14);
	}

	// Clip the rect
	if (width > getOverlayWidth()) {
		width = getOverlayWidth();
	}
	if (height > getOverlayHeight()) {
		height = getOverlayHeight();
	}

	_osdMessage.create(width, height, &DEFAULT_MODE);
	_osdMessage.fillRect(Common::Rect(width, height), _pfDefaultTexture.ARGBToColor(200, 0, 0, 0));

	// Render the message, centered, and in white
	for (i = 0; i < lines.size(); i++) {
		font->drawString(&_osdMessage, lines[i],
		                 0, 0 + i * lineHeight + vOffset + lineSpacing, width,
		                 _pfDefaultTexture.RGBToColor(255, 255, 255),
		                 Graphics::kTextAlignCenter);
	}

	_osdMessageEndTime = getMillis(true) + kOSDMessageDuration;
}

void OSystem_3DS::displayActivityIconOnOSD(const Graphics::Surface *icon) {
	if (!icon) {
		_activityIcon.free();
	} else {
		if (!_activityIcon.getPixels() || icon->w != _activityIcon.w || icon->h != _activityIcon.h) {
			_activityIcon.create(icon->w, icon->h, &DEFAULT_MODE);
		}

		if (icon->format == _activityIcon.format) {
			_activityIcon.copyRectToSurface(*icon, 0, 0, Common::Rect(icon->w, icon->h));
		} else {
			Graphics::Surface *converted = icon->convertTo(_activityIcon.format);
			_activityIcon.copyRectToSurface(*converted, 0, 0, Common::Rect(converted->w, converted->h));
			converted->free();
			delete converted;
		}

		_activityIcon.markDirty();
	}
}

int16 OSystem_3DS::getOverlayHeight() {
	return 240;
}

int16 OSystem_3DS::getOverlayWidth() {
	return config.screen == kScreenTop ? 400 : 320;
}

bool OSystem_3DS::showMouse(bool visible) {
	_cursorVisible = visible;
	flushCursor();
	return !visible;
}

void OSystem_3DS::warpMouse(int x, int y) {
	if (!_overlayVisible) {
		_cursorScreenX = x;
		_cursorScreenY = y;
	} else {
		_cursorOverlayX = x;
		_cursorOverlayY = y;
	}

	// TODO: adjust for _cursorScalable ?
	x -= _cursorHotspotX;
	y -= _cursorHotspotY;

	int offsetx = 0;
	int offsety = 0;
	if (!_overlayVisible) {
		offsetx = config.screen == kScreenTop ? _gameTopTexture.getPosX() : _gameBottomTexture.getPosX();
		offsety = config.screen == kScreenTop ? _gameTopTexture.getPosY() : _gameBottomTexture.getPosY();
	}

	_cursorTexture.setPosition(x + offsetx, y + offsety);
}

void OSystem_3DS::setCursorDelta(float deltaX, float deltaY) {
	_cursorDeltaX = deltaX;
	_cursorDeltaY = deltaY;
}

void OSystem_3DS::setMouseCursor(const void *buf, uint w, uint h,
                                 int hotspotX, int hotspotY,
                                 uint32 keycolor, bool dontScale,
                                 const Graphics::PixelFormat *format) {
	_cursorScalable = !dontScale;
	_cursorHotspotX = hotspotX;
	_cursorHotspotY = hotspotY;
	_cursorKeyColor = keycolor;
	_pfCursor = !format ? Graphics::PixelFormat::createFormatCLUT8() : *format;

	if (w != _cursor.w || h != _cursor.h || _cursor.format != _pfCursor) {
		_cursor.create(w, h, _pfCursor);
		_cursorTexture.create(w, h, &DEFAULT_MODE);
	}

	if ( w != 0 && h != 0 ) {
		_cursor.copyRectToSurface(buf, w * _pfCursor.bytesPerPixel, 0, 0, w, h);
	}

	flushCursor();

	if (!_overlayVisible) {
		warpMouse(_cursorScreenX, _cursorScreenY);
	} else {
		warpMouse(_cursorOverlayX, _cursorOverlayY);
	}
}

void OSystem_3DS::setCursorPalette(const byte *colors, uint start, uint num) {
	assert(start + num <= 256);
	memcpy(_cursorPalette + 3 * start, colors, 3 * num);
	_cursorPaletteEnabled = true;
	flushCursor();
}

namespace {
template<typename SrcColor>
void applyKeyColor(Graphics::Surface *src, Graphics::Surface *dst, const SrcColor keyColor) {
	assert(dst->format.bytesPerPixel == 4);
	assert((dst->w >= src->w) && (dst->h >= src->h));

	for (uint y = 0; y < src->h; ++y) {
		SrcColor *srcPtr = (SrcColor *)src->getBasePtr(0, y);
		uint32 *dstPtr = (uint32 *)dst->getBasePtr(0, y);

		for (uint x = 0; x < src->w; ++x) {
			const SrcColor color = *srcPtr++;

			if (color == keyColor) {
				*dstPtr = 0;
			}

			dstPtr++;
		}
	}
}
} // End of anonymous namespace

void OSystem_3DS::flushCursor() {
	if (_cursor.getPixels()) {
		Graphics::Surface *converted = _cursor.convertTo(_cursorTexture.format, _cursorPaletteEnabled ? _cursorPalette : _palette);
		_cursorTexture.copyRectToSurface(*converted, 0, 0, Common::Rect(converted->w, converted->h));
		_cursorTexture.markDirty();
		converted->free();
		delete converted;

		if (_pfCursor.bytesPerPixel == 1) {
			applyKeyColor<byte>(&_cursor, &_cursorTexture, _cursorKeyColor);
		} else if (_pfCursor.bytesPerPixel == 2) {
			applyKeyColor<uint16>(&_cursor, &_cursorTexture, _cursorKeyColor);
		} else if (_pfCursor.bytesPerPixel == 4) {
			applyKeyColor<uint32>(&_cursor, &_cursorTexture, _cursorKeyColor);
		}
	}
}

} // namespace _3DS
