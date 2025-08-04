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

#ifndef MEDIASTATION_GRAPHICS_H
#define MEDIASTATION_GRAPHICS_H

#include "common/rect.h"
#include "common/array.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"

#include "mediastation/clients.h"
#include "mediastation/mediascript/scriptvalue.h"

namespace MediaStation {

class MediaStationEngine;
struct DissolvePattern;
class Bitmap;

enum BlitMode {
	kUncompressedBlit = 0x00,
	kRle8Blit = 0x01,
	kClipEnabled = 0x04,
	kUncompressedTransparentBlit = 0x08,
	kPartialDissolve = 0x10,
	kFullDissolve = 0x20,
	kCccBlit = 0x40,
	kCccTransparentBlit = 0x80
};

enum TransitionType {
	kTransitionFadeToBlack = 300,
	kTransitionFadeToPalette = 301,
	kTransitionSetToPalette = 302,
	kTransitionSetToBlack = 303,
	kTransitionFadeToColor = 304,
	kTransitionSetToColor = 305,
	kTransitionSetToPercentOfPalette = 306,
	kTransitionFadeToPaletteObject = 307,
	kTransitionSetToPaletteObject = 308,
	kTransitionSetToPercentOfPaletteObject = 309,
	kTransitionColorShiftCurrentPalette = 310,
	kTransitionCircleOut = 328
};

enum VideoDisplayManagerSectionType {
	kVideoDisplayManagerUpdateDirty = 0x578,
	kVideoDisplayManagerUpdateAll = 0x579,
	kVideoDisplayManagerEffectTransition = 0x57a,
	kVideoDisplayManagerSetTime = 0x57b,
	kVideoDisplayManagerLoadPalette = 0x5aa,
};

class VideoDisplayManager : public ParameterClient {
public:
	VideoDisplayManager(MediaStationEngine *vm);
	~VideoDisplayManager();

	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) override;

	void updateScreen() { _screen->update(); }
	Graphics::Palette *getRegisteredPalette() { return _registeredPalette; }
	void setRegisteredPalette(Graphics::Palette *palette) { _registeredPalette = palette; }

	void imageBlit(
		const Common::Point &destinationPoint,
		const Bitmap *image,
		double dissolveFactor,
		const Common::Array<Common::Rect> &dirtyRegion,
		Graphics::ManagedSurface *destinationImage = nullptr);

	void imageDeltaBlit(
		const Common::Point &deltaFramePos,
		const Common::Point &keyFrameOffset,
		const Bitmap *deltaFrame,
		const Bitmap *keyFrame,
		const double dissolveFactor,
		const Common::Array<Common::Rect> &dirtyRegion);

	void effectTransition(Common::Array<ScriptValue> &args);
	void setTransitionOnSync(Common::Array<ScriptValue> &args) { _scheduledTransitionOnSync = args; }
	void doTransitionOnSync();

	void performUpdateDirty();
	void performUpdateAll();

private:
	MediaStationEngine *_vm = nullptr;
	Graphics::Screen *_screen = nullptr;
	Graphics::Palette *_registeredPalette = nullptr;
	Common::Array<ScriptValue> _scheduledTransitionOnSync;
	double _defaultTransitionTime = 0.0;

	void readAndEffectTransition(Chunk &chunk);
	void readAndRegisterPalette(Chunk &chunk);

	// Blitting methods.
	// blitRectsClip encompasses the functionality of both opaqueBlitRectsClip
	// and transparentBlitRectsClip in the disasm.
	void blitRectsClip(
		Graphics::ManagedSurface *dest,
		const Common::Point &destLocation,
		const Graphics::ManagedSurface &source,
		const Common::Array<Common::Rect> &dirtyRegion);
	void rleBlitRectsClip(
		Graphics::ManagedSurface *dest,
		const Common::Point &destLocation,
		const Bitmap *source,
		const Common::Array<Common::Rect> &dirtyRegion);
	Graphics::ManagedSurface decompressRle8Bitmap(
		const Bitmap *source,
		const Graphics::ManagedSurface *keyFrame = nullptr,
		const Common::Point *keyFrameOffset = nullptr);
	void dissolveBlitRectsClip(
		Graphics::ManagedSurface *dest,
		const Common::Point &destPos,
		const Bitmap *source,
		const Common::Array<Common::Rect> &dirtyRegion,
		const uint dissolveFactor);
	void dissolveBlit1Rect(
		Graphics::ManagedSurface *dest,
		const Common::Rect &areaToRedraw,
		const Common::Point &originOnScreen,
		const Bitmap *source,
		const Common::Rect &dirtyRegion,
		const DissolvePattern &pattern);
	void fullDeltaRleBlitRectsClip(
		Graphics::ManagedSurface *destinationImage,
		const Common::Point &deltaFramePos,
		const Common::Point &keyFrameOffset,
		const Bitmap *deltaFrame,
		const Bitmap *keyFrame,
		const Common::Array<Common::Rect> &dirtyRegion);
	void deltaRleBlitRectsClip(
		Graphics::ManagedSurface *destinationImage,
		const Common::Point &deltaFramePos,
		const Bitmap *deltaFrame,
		const Bitmap *keyFrame,
		const Common::Array<Common::Rect> &dirtyRegion);
	void deltaRleBlit1Rect(
		Graphics::ManagedSurface *destinationImage,
		const Common::Point &destinationPoint,
		const Bitmap *sourceImage,
		const Bitmap *deltaImage,
		const Common::Rect &dirtyRect);

	// Transition methods.
	const double DEFAULT_FADE_TRANSITION_TIME_IN_SECONDS = 0.5;
	const byte DEFAULT_PALETTE_TRANSITION_START_INDEX = 0x01;
	const byte DEFAULT_PALETTE_TRANSITION_COLOR_COUNT = 0xFE;
	void fadeToBlack(Common::Array<ScriptValue> &args);
	void fadeToRegisteredPalette(Common::Array<ScriptValue> &args);
	void setToRegisteredPalette(Common::Array<ScriptValue> &args);
	void setToBlack(Common::Array<ScriptValue> &args);
	void fadeToColor(Common::Array<ScriptValue> &args);
	void setToColor(Common::Array<ScriptValue> &args);
	void setToPercentOfPalette(Common::Array<ScriptValue> &args);
	void fadeToPaletteObject(Common::Array<ScriptValue> &args);
	void setToPaletteObject(Common::Array<ScriptValue> &args);
	void setToPercentOfPaletteObject(Common::Array<ScriptValue> &args);
	void colorShiftCurrentPalette(Common::Array<ScriptValue> &args);
	void circleOut(Common::Array<ScriptValue> &args);

	void _setPalette(Graphics::Palette &palette, uint startIndex, uint colorCount);
	void _setPaletteToColor(Graphics::Palette &targetPalette, byte r, byte g, byte b);
	uint _limitColorRange(uint &startIndex, uint &colorCount);
	byte _interpolateColorComponent(byte source, byte target, double progress);

	void _fadeToColor(byte r, byte g, byte b, double fadeTime, uint startIndex, uint colorCount);
	void _setToColor(byte r, byte g, byte b, uint startIndex, uint colorCount);
	void _setPercentToColor(double percent, byte r, byte g, byte b, uint startIndex, uint colorCount);

	void _fadeToPalette(double fadeTime, Graphics::Palette &targetPalette, uint startIndex, uint colorCount);
	void _setToPercentPalette(double percent, Graphics::Palette &currentPalette, Graphics::Palette &targetPalette,
		uint startIndex, uint colorCount);
	void _fadeToRegisteredPalette(double fadeTime, uint startIndex, uint colorCount);
	void _setToRegisteredPalette(uint startIndex, uint colorCount);
	void _colorShiftCurrentPalette(uint startIndex, uint shiftAmount, uint colorCount);
	void _fadeToPaletteObject(uint paletteId, double fadeTime, uint startIndex, uint colorCount);
	void _setToPaletteObject(uint paletteId, uint startIndex, uint colorCount);
	void _setPercentToPaletteObject(double percent, uint paletteId, uint startIndex, uint colorCount);
};

} // End of namespace MediaStation

#endif
