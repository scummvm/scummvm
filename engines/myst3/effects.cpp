/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/database.h"
#include "engines/myst3/effects.h"
#include "engines/myst3/gfx.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"
#include "engines/myst3/sound.h"

#include "graphics/surface.h"

namespace Myst3 {

Effect::FaceMask::FaceMask() :
		surface(nullptr) {

	for (uint i = 0; i < 10; i++) {
		for (uint j = 0; j < 10; j++) {
			block[i][j] = false;
		}
	}
}

Effect::FaceMask::~FaceMask() {
	if (surface) {
		surface->free();
	}

	delete surface;
}

Common::Rect Effect::FaceMask::getBlockRect(uint x, uint y) {
	Common::Rect rect = Common::Rect(64, 64);
	rect.translate(x * 64, y * 64);
	return rect;
}

Effect::Effect(Myst3Engine *vm) :
		_vm(vm) {
}

Effect::~Effect() {
	for (FaceMaskMap::iterator it = _facesMasks.begin(); it != _facesMasks.end(); it++) {
		delete it->_value;
	}
}

bool Effect::loadMasks(const Common::String &room, uint32 id, Archive::ResourceType type) {
	bool isFrame = _vm->_state->getViewType() == kFrame;

	// Load the mask of each face
	for (uint i = 0; i < 6; i++) {
		ResourceDescription desc = _vm->getFileDescription(room, id, i + 1, type);

		if (desc.isValid()) {
			Common::SeekableReadStream *data = desc.getData();

			// Check if we are overriding an existing mask
			delete _facesMasks[i];
			_facesMasks[i] = loadMask(data);

			// Frame masks are vertically flipped for some reason
			if (isFrame) {
				_vm->_gfx->flipVertical(_facesMasks[i]->surface);
			}

			delete data;
		}
	}

	if (_facesMasks.empty())
		return false;

	return true;
}

Effect::FaceMask *Effect::loadMask(Common::SeekableReadStream *maskStream) {
	FaceMask *mask = new FaceMask();
	mask->surface = new Graphics::Surface();
	mask->surface->create(640, 640, Graphics::PixelFormat::createFormatCLUT8());

	uint32 headerOffset = 0;
	uint32 dataOffset = 0;

	while (headerOffset < 400) {
		int blockX = (headerOffset / sizeof(dataOffset)) % 10;
		int blockY = (headerOffset / sizeof(dataOffset)) / 10;

		maskStream->seek(headerOffset, SEEK_SET);
		dataOffset = maskStream->readUint32LE();
		headerOffset = maskStream->pos();

		if (dataOffset != 0) {
			maskStream->seek(dataOffset, SEEK_SET);

			for(int i = 63; i >= 0; i--) {
				int x = 0;
				byte numValues = maskStream->readByte();
				for (int j = 0; j < numValues; j++) {
					byte repeat = maskStream->readByte();
					byte value = maskStream->readByte();
					for (int k = 0; k < repeat; k++) {
						((uint8*)mask->surface->getPixels())[((blockY * 64) + i) * 640 + blockX * 64 + x] = value;
						x++;
					}

					// If a block has at least one non zero value, mark it as active
					if (value != 0) {
						mask->block[blockX][blockY] = true;
					}
				}
			}
		}
	}

	return mask;
}

Common::Rect Effect::getUpdateRectForFace(uint face) {
	FaceMask *mask = _facesMasks.getVal(face);
	if (!mask)
		error("No mask for face %d", face);

	Common::Rect rect;

	// Build a rectangle containing all the active effect blocks
	for (uint i = 0; i < 10; i++) {
		for (uint j = 0; j < 10; j++) {
			if (mask->block[i][j]) {
				if (rect.isEmpty()) {
					rect = FaceMask::getBlockRect(i, j);
				} else {
					rect.extend(FaceMask::getBlockRect(i, j));
				}
			}
		}
	}

	return rect;
}

WaterEffect::WaterEffect(Myst3Engine *vm) :
		Effect(vm),
		_lastUpdate(0),
		_step(0) {
}

WaterEffect::~WaterEffect() {
}

WaterEffect *WaterEffect::create(Myst3Engine *vm, uint32 id) {
	WaterEffect *s = new WaterEffect(vm);

	if (!s->loadMasks("", id, Archive::kWaterEffectMask)) {
		delete s;
		return 0;
	}

	return s;
}

bool WaterEffect::isRunning() {
	return _vm->_state->getWaterEffectActive()
			&& _vm->_state->getWaterEffectRunning();
}

bool WaterEffect::update() {
	if (!isRunning()) {
		return false;
	}

	if (g_system->getMillis() - _lastUpdate >= 1000 / (uint32)_vm->_state->getWaterEffectSpeed()) {
		_lastUpdate = g_system->getMillis();

		_step++;
		if (_step > _vm->_state->getWaterEffectMaxStep())
			_step = 0;

		float position = _step / (float)_vm->_state->getWaterEffectMaxStep();

		doStep(position, _vm->_state->getViewType() == kFrame);

		return true;
	}

	return false;
}

void WaterEffect::doStep(float position, bool isFrame) {
	double timeOffset;
	double frequency;
	double ampl;

	timeOffset = position * 2 * M_PI;
	frequency = _vm->_state->getWaterEffectFrequency() * 0.1;

	ampl = _vm->_state->getWaterEffectAmpl() / 10.0 / 2.0;
	for (uint i = 0; i < 640; i++) {
		double ampl1;
		if (i < 320)
			ampl1 = i / 320 + 1.0;
		else
			ampl1 = (640 - i) / 320 + 1.0;

		_bottomDisplacement[i] = sin(i / 640.0 * frequency * 2 * M_PI + timeOffset) / 2 * ampl1 * ampl;
	}

	// FIXME: The original sets this to WaterEffectAttenuation, which causes
	// glitches here
	uint32 attenuation = 640;
	for (uint i = 0; i < attenuation; i++) {
		double ampl2 = attenuation / (attenuation - i + 1.0);

		int8 value = sin(i / 640.0 * frequency * 2 * M_PI * ampl2 + timeOffset) / 2 * 1.0 / ampl2 * ampl;

		if (!isFrame) {
			_verticalDisplacement[i] = value;
		} else {
			_verticalDisplacement[attenuation - 1 - i] = value;
		}
	}

	for (uint i = 0; i < 640; i++) {
		double ampl3 = sin(i / 640.0 * frequency * 2 * M_PI + timeOffset) / 2.0;

		_horizontalDisplacements[0][i] = ampl3 * 1.25 * ampl + 0.5;
		_horizontalDisplacements[1][i] = ampl3 * 1.00 * ampl + 0.5;
		_horizontalDisplacements[2][i] = ampl3 * 0.75 * ampl + 0.5;
		_horizontalDisplacements[3][i] = ampl3 * 0.50 * ampl + 0.5;
		_horizontalDisplacements[4][i] = ampl3 * 0.25 * ampl + 0.5;
	}
}

void WaterEffect::applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst) {
	if (!isRunning()) {
		return;
	}

	FaceMask *mask = _facesMasks.getVal(face);

	if (!mask)
		error("No mask for face %d", face);

	apply(src, dst, mask->surface, face == 1, _vm->_state->getWaterEffectAmpl());
}

void WaterEffect::apply(Graphics::Surface *src, Graphics::Surface *dst, Graphics::Surface *mask, bool bottomFace, int32 waterEffectAmpl) {
	int32 waterEffectAttenuation = _vm->_state->getWaterEffectAttenuation();
	int32 waterEffectAmplOffset = _vm->_state->getWaterEffectAmplOffset();

	int8 *hDisplacement = nullptr;
	int8 *vDisplacement = nullptr;

	if (bottomFace) {
		hDisplacement = _bottomDisplacement;
		vDisplacement = _bottomDisplacement;
	} else {
		vDisplacement = _verticalDisplacement;
	}

	uint32 *dstPtr = (uint32 *)dst->getPixels();
	byte *maskPtr = (byte *)mask->getPixels();

	for (uint y = 0; y < dst->h; y++) {
		if (!bottomFace) {
			uint32 strength = (320 * (9 - y / 64)) / waterEffectAttenuation;
			if (strength > 4)
				strength = 4;
			hDisplacement = _horizontalDisplacements[strength];
		}

		for (uint x = 0; x < dst->w; x++) {
			int8 maskValue = *maskPtr;

			if (maskValue != 0) {
				int8 xOffset = hDisplacement[x];
				int8 yOffset = vDisplacement[y];

				if (maskValue < 8) {
					maskValue -= waterEffectAmplOffset;
					if (maskValue < 0) {
						maskValue = 0;
					}

					if (xOffset >= 0) {
						if (xOffset > maskValue)
							xOffset = maskValue;
					} else {
						if (-xOffset > maskValue)
							xOffset = -maskValue;
					}
					if (yOffset >= 0) {
						if (yOffset > maskValue)
							yOffset = maskValue;
					} else {
						if (-yOffset > maskValue)
							yOffset = -maskValue;
					}
				}

				uint32 srcValue1 = *(uint32 *) src->getBasePtr(x + xOffset, y + yOffset);
				uint32 srcValue2 = *(uint32 *) src->getBasePtr(x, y);

#ifdef SCUMM_BIG_ENDIAN
				*dstPtr = 0x000000FF | ((0x7F7F7F00 & (srcValue1 >> 1)) + (0x7F7F7F00 & (srcValue2 >> 1)));
#else
				*dstPtr = 0xFF000000 | ((0x007F7F7F & (srcValue1 >> 1)) + (0x007F7F7F & (srcValue2 >> 1)));
#endif
			}

			maskPtr++;
			dstPtr++;
		}
	}
}

LavaEffect::LavaEffect(Myst3Engine *vm) :
		Effect(vm),
		_lastUpdate(0),
		_step(0) {
}

LavaEffect::~LavaEffect() {

}

LavaEffect *LavaEffect::create(Myst3Engine *vm, uint32 id) {
	LavaEffect *s = new LavaEffect(vm);

	if (!s->loadMasks("", id, Archive::kLavaEffectMask)) {
		delete s;
		return 0;
	}

	return s;
}

bool LavaEffect::update() {
	if (!_vm->_state->getLavaEffectActive()) {
		return false;
	}

	if (g_system->getMillis() - _lastUpdate >= 1000 / (uint32)_vm->_state->getLavaEffectSpeed()) {
		_lastUpdate = g_system->getMillis();

		_step += _vm->_state->getLavaEffectStepSize();

		doStep(_step, _vm->_state->getLavaEffectAmpl() / 10);

		if (_step > 256)
			_step -= 256;

		return true;
	}

	return false;
}

void LavaEffect::doStep(int32 position, float ampl) {
	for (uint i = 0; i < 256; i++) {
		_displacement[i] = (sin((i + position) * 2 * M_PI / 256.0) + 1.0) * ampl;
	}
}

void LavaEffect::applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst) {
	if (!_vm->_state->getLavaEffectActive()) {
		return;
	}

	FaceMask *mask = _facesMasks.getVal(face);

	if (!mask)
		error("No mask for face %d", face);

	uint32 *dstPtr = (uint32 *)dst->getPixels();
	byte *maskPtr = (byte *)mask->surface->getPixels();

	for (uint y = 0; y < dst->h; y++) {
		for (uint x = 0; x < dst->w; x++) {
			uint8 maskValue = *maskPtr;

			if (maskValue != 0) {
				int32 xOffset= _displacement[(maskValue + y) % 256];
				int32 yOffset = _displacement[maskValue % 256];
				int32 maxOffset = (maskValue >> 6) & 0x3;

				if (yOffset > maxOffset) {
					yOffset = maxOffset;
				}
				if (xOffset > maxOffset) {
					xOffset = maxOffset;
				}

//				uint32 srcValue1 = *(uint32 *)src->getBasePtr(x + xOffset, y + yOffset);
//				uint32 srcValue2 = *(uint32 *)src->getBasePtr(x, y);
//
//				*dstPtr = 0xFF000000 | ((0x007F7F7F & (srcValue1 >> 1)) + (0x007F7F7F & (srcValue2 >> 1)));

				// TODO: The original does "blending" as above, but strangely
				// this looks more like the original rendering
				*dstPtr = *(uint32 *)src->getBasePtr(x + xOffset, y + yOffset);
			}

			maskPtr++;
			dstPtr++;
		}
	}
}

MagnetEffect::MagnetEffect(Myst3Engine *vm) :
		Effect(vm),
		_lastSoundId(0),
		_lastTime(0),
		_position(0),
		_lastAmpl(0),
		_shakeStrength(nullptr) {
}

MagnetEffect::~MagnetEffect() {
	delete _shakeStrength;
}

MagnetEffect *MagnetEffect::create(Myst3Engine *vm, uint32 id) {
	if (!vm->_state->getMagnetEffectSound()) {
		return nullptr;
	}

	MagnetEffect *s = new MagnetEffect(vm);
	s->loadMasks("", id, Archive::kMagneticEffectMask);
	return s;
}

bool MagnetEffect::update() {
	int32 soundId = _vm->_state->getMagnetEffectSound();
	if (!soundId) {
		// The effect is no longer active
		_lastSoundId = 0;
		_vm->_state->setMagnetEffectUnk3(0);

		delete _shakeStrength;
		_shakeStrength = nullptr;

		return false;
	}

	if (soundId != _lastSoundId) {
		// The sound changed since last update
		_lastSoundId = soundId;

		ResourceDescription desc = _vm->getFileDescription("", _vm->_state->getMagnetEffectNode(), 0, Archive::kRawData);
		if (!desc.isValid())
			error("Magnet effect support file %d does not exist", _vm->_state->getMagnetEffectNode());

		delete _shakeStrength;
		_shakeStrength = desc.getData();
	}

	int32 soundPosition = _vm->_sound->playedFrames(soundId);
	if (_shakeStrength && soundPosition >= 0) {
		// Update the shake amplitude according to the position in the playing sound.
		// This has no in-game effect (same as original) due to var 122 being 0.
		_shakeStrength->seek(soundPosition, SEEK_SET);
		_vm->_state->setMagnetEffectUnk3(_shakeStrength->readByte());

		// Update the vertical displacements
		float ampl = (_vm->_state->getMagnetEffectUnk1() + _vm->_state->getMagnetEffectUnk3())
				/ (float)_vm->_state->getMagnetEffectUnk2();

		if (ampl != _lastAmpl) {
			for (uint i = 0; i < 256; i++) {
				_verticalDisplacement[i] = sin(i * 2 * M_PI / 255.0) * ampl;
			}

			_lastAmpl = ampl;
		}

		// Update the position in the effect cycle
		uint32 time = g_system->getMillis();
		if (_lastTime) {
			_position += (float)_vm->_state->getMagnetEffectSpeed() * (time - _lastTime) / 1000 / 10;

			while (_position > 1.0) {
				_position -= 1.0;
			}
		}
		_lastTime = time;
	} else {
		_vm->_state->setMagnetEffectUnk3(0);
	}

	return true;
}

void MagnetEffect::applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst) {
	FaceMask *mask = _facesMasks.getVal(face);

	if (!mask)
		error("No mask for face %d", face);

	apply(src, dst, mask->surface, _position * 256.0);
}

void MagnetEffect::apply(Graphics::Surface *src, Graphics::Surface *dst, Graphics::Surface *mask, int32 position) {
	uint32 *dstPtr = (uint32 *)dst->getPixels();
	byte *maskPtr = (byte *)mask->getPixels();

	for (uint y = 0; y < dst->h; y++) {
		for (uint x = 0; x < dst->w; x++) {
			uint8 maskValue = *maskPtr;

			if (maskValue != 0) {
				int32 displacement = _verticalDisplacement[(maskValue + position) % 256];
				int32 displacedY = CLIP<int32>(y + displacement, 0, src->h - 1);

				uint32 srcValue1 = *(uint32 *) src->getBasePtr(x, displacedY);
				uint32 srcValue2 = *(uint32 *) src->getBasePtr(x, y);

#ifdef SCUMM_BIG_ENDIAN
				*dstPtr = 0x000000FF | ((0x7F7F7F00 & (srcValue1 >> 1)) + (0x7F7F7F00 & (srcValue2 >> 1)));
#else
				*dstPtr = 0xFF000000 | ((0x007F7F7F & (srcValue1 >> 1)) + (0x007F7F7F & (srcValue2 >> 1)));
#endif
			}

			maskPtr++;
			dstPtr++;
		}
	}
}

ShakeEffect::ShakeEffect(Myst3Engine *vm) :
		Effect(vm),
		_lastTick(0),
		_magnetEffectShakeStep(0),
		_pitchOffset(0),
		_headingOffset(0) {
}

ShakeEffect::~ShakeEffect() {
}

ShakeEffect *ShakeEffect::create(Myst3Engine *vm) {
	if (vm->_state->getShakeEffectAmpl() == 0) {
		return nullptr;
	}

	return new ShakeEffect(vm);
}

bool ShakeEffect::update() {
	// Check if the effect is active
	int32 ampl = _vm->_state->getShakeEffectAmpl();
	if (ampl == 0) {
		return false;
	}

	// Check if the effect needs to be updated
	uint tick = _vm->_state->getTickCount();
	if (tick < _lastTick + _vm->_state->getShakeEffectTickPeriod()) {
		return false;
	}

	if (_vm->_state->getMagnetEffectUnk3()) {
		// If the magnet effect is also active, use its parameters
		float magnetEffectAmpl = (_vm->_state->getMagnetEffectUnk1() + _vm->_state->getMagnetEffectUnk3()) / 32.0;

		float shakeEffectAmpl;
		if (_magnetEffectShakeStep >= 2) {
			shakeEffectAmpl = ampl;
		} else {
			shakeEffectAmpl = -ampl;
		}
		_pitchOffset = shakeEffectAmpl / 200.0 * magnetEffectAmpl;

		if (_magnetEffectShakeStep >= 1 && _magnetEffectShakeStep <= 2) {
			shakeEffectAmpl = ampl;
		} else {
			shakeEffectAmpl = -ampl;
		}
		_headingOffset = shakeEffectAmpl / 200.0 * magnetEffectAmpl;

		_magnetEffectShakeStep++;
		_magnetEffectShakeStep %= 3;
	} else {
		// Shake effect only
		uint randomAmpl;

		randomAmpl = _vm->_rnd->getRandomNumberRng(0, ampl);
		_pitchOffset = (randomAmpl - ampl / 2.0) / 100.0;

		randomAmpl = _vm->_rnd->getRandomNumberRng(0, ampl);
		_headingOffset = (randomAmpl - ampl / 2.0) / 100.0;
	}

	_lastTick = tick;

	return true;
}

void ShakeEffect::applyForFace(uint face, Graphics::Surface* src, Graphics::Surface* dst) {
}

RotationEffect::RotationEffect(Myst3Engine *vm) :
		Effect(vm),
		_lastUpdate(0),
		_headingOffset(0) {
}

RotationEffect::~RotationEffect() {
}

RotationEffect *RotationEffect::create(Myst3Engine *vm) {
	if (vm->_state->getRotationEffectSpeed() == 0) {
		return nullptr;
	}

	return new RotationEffect(vm);
}

bool RotationEffect::update() {
	// Check if the effect is active
	int32 speed = _vm->_state->getRotationEffectSpeed();
	if (speed == 0) {
		return false;
	}

	if (_lastUpdate != 0) {
		_headingOffset = speed * (g_system->getMillis() - _lastUpdate) / 1000.0;
	}

	_lastUpdate = g_system->getMillis();

	return true;
}

void RotationEffect::applyForFace(uint face, Graphics::Surface* src, Graphics::Surface* dst) {
}

bool ShieldEffect::loadPattern() {
	// Read the shield effect support data
	ResourceDescription desc = _vm->getFileDescription("NARA", 10000, 0, Archive::kRawData);
	if (!desc.isValid()) {
		return false;
	}

	Common::SeekableReadStream *stream = desc.getData();
	if (stream->size() != 4096) {
		error("Incorrect shield effect support file size %d", stream->size());
	}

	stream->read(_pattern, 4096);

	delete stream;

	return true;
}

ShieldEffect::ShieldEffect(Myst3Engine *vm):
	Effect(vm),
	_lastTick(0),
	_amplitude(1.0),
	_amplitudeIncrement(1.0 / 64.0) {
}

ShieldEffect::~ShieldEffect() {

}

ShieldEffect *ShieldEffect::create(Myst3Engine *vm, uint32 id) {
	uint32 room = vm->_state->getLocationRoom();
	uint32 node = vm->_state->getLocationNode();

	// This effect can only be found on Narayan cube nodes
	if (room != kRoomNarayan || node >= 100)
		return nullptr;

	ShieldEffect *s = new ShieldEffect(vm);

	if (!s->loadPattern()) {
		delete s;
		return nullptr; // We don't have the effect file
	}

	bool outerShieldUp = vm->_state->getOuterShieldUp();
	bool innerShieldUp = vm->_state->getInnerShieldUp();
	int32 saavedroStatus = vm->_state->getSaavedroStatus();

	bool hasMasks = false;

	int32 innerShieldMaskNode = 0;
	if (innerShieldUp) {
		innerShieldMaskNode = node + 100;
	}

	if (outerShieldUp) {
		hasMasks |= s->loadMasks("NARA", node + 300, Archive::kShieldEffectMask);
		if (saavedroStatus == 2) {
			innerShieldMaskNode = node + 200;
		}
	}

	if (innerShieldMaskNode) {
		hasMasks |= s->loadMasks("NARA", innerShieldMaskNode, Archive::kShieldEffectMask);
	}

	if (innerShieldMaskNode && innerShieldUp && node > 6) {
		hasMasks |= s->loadMasks("NARA", node + 100, Archive::kShieldEffectMask);
	}

	if (!hasMasks) {
		delete s;
		return nullptr;
	}

	return s;
}

bool ShieldEffect::update() {
	if (_vm->_state->getTickCount() == _lastTick)
		return false;

	_lastTick = _vm->_state->getTickCount();

	// Update the amplitude, varying between 1.0 and 4.0
	_amplitude += _amplitudeIncrement;

	if (_amplitude >= 4.0) {
		_amplitude = 4.0;
		_amplitudeIncrement = -1.0 / 64.0;
	} else if (_amplitude <= 1.0) {
		_amplitude = 1.0;
		_amplitudeIncrement = 1.0 / 64.0;
	}

	// Update the support data
	for (uint i = 0; i < ARRAYSIZE(_pattern); i++) {
		_pattern[i] += 2; // Intentional overflow
	}

	// Update the displacement offsets
	for (uint i = 0; i < 256; i++) {
		_displacement[i] = (sin(i * 2 * M_PI / 255.0) + 1.0) * _amplitude;
	}

	return true;
}

void ShieldEffect::applyForFace(uint face, Graphics::Surface *src, Graphics::Surface *dst) {
	if (!_vm->_state->getShieldEffectActive()) {
		return;
	}

	FaceMask *mask = _facesMasks.getVal(face);

	if (!mask)
		error("No mask for face %d", face);

	uint32 *dstPtr = (uint32 *)dst->getPixels();
	byte *maskPtr = (byte *)mask->surface->getPixels();

	for (uint y = 0; y < dst->h; y++) {
		for (uint x = 0; x < dst->w; x++) {
			uint8 maskValue = *maskPtr;

			if (maskValue != 0) {
				int32 yOffset = _displacement[_pattern[(y % 64) * 64 + (x % 64)]];

				if (yOffset > maskValue) {
					yOffset = maskValue;
				}

				*dstPtr = *(uint32 *)src->getBasePtr(x, y + yOffset);
			}

			maskPtr++;
			dstPtr++;
		}
	}
}

} // End of namespace Myst3
