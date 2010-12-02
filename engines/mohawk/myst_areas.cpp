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
 * $URL$
 * $Id$
 *
 */

#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

namespace Mohawk {

MystResource::MystResource(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) {
	_vm = vm;
	_parent = parent;

	if (parent == NULL) {
		_flags = rlstStream->readUint16LE();
		_rect.left = rlstStream->readSint16LE();
		_rect.top = rlstStream->readSint16LE();

		if (_rect.top == -1) {
			warning("Invalid _rect.top of -1 found - clamping to 0");
			_rect.top = 0;
		}

		_rect.right = rlstStream->readSint16LE();
		_rect.bottom = rlstStream->readSint16LE();
		_dest = rlstStream->readUint16LE();
	} else {
		_flags = parent->_flags;
		_rect.left = parent->_rect.left;
		_rect.top = parent->_rect.top;
		_rect.right = parent->_rect.right;
		_rect.bottom = parent->_rect.bottom;
		_dest = parent->_dest;
	}

	debugC(kDebugResource, "\tflags: 0x%04X", _flags);
	debugC(kDebugResource, "\tleft: %d", _rect.left);
	debugC(kDebugResource, "\ttop: %d", _rect.top);
	debugC(kDebugResource, "\tright: %d", _rect.right);
	debugC(kDebugResource, "\tbottom: %d", _rect.bottom);
	debugC(kDebugResource, "\tdest: %d", _dest);
}

MystResource::~MystResource() {
}

void MystResource::handleMouseUp(const Common::Point &mouse) {
	if (_dest != 0)
		_vm->changeToCard(_dest, true);
	else
		warning("Movement type resource with null destination at position (%d, %d), (%d, %d)", _rect.left, _rect.top, _rect.right, _rect.bottom);
}

bool MystResource::canBecomeActive() {
	return !unreachableZipDest() && (isEnabled() || (_flags & kMystUnknownFlag));
}

bool MystResource::unreachableZipDest() {
	return (_flags & kMystZipModeEnableFlag) && !_vm->_zipMode;
}

bool MystResource::isEnabled() {
	return _flags & kMystHotspotEnableFlag;
}

void MystResource::setEnabled(bool enabled) {
	if (enabled) {
		_flags |= kMystHotspotEnableFlag;
	} else {
		_flags &= ~kMystHotspotEnableFlag;
	}
}

MystResourceType5::MystResourceType5(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResource(vm, rlstStream, parent) {
	debugC(kDebugResource, "\tResource Type 5 Script:");

	_script = vm->_scriptParser->readScript(rlstStream, kMystScriptNormal);
}

void MystResourceType5::handleMouseUp(const Common::Point &mouse) {
	_vm->_scriptParser->runScript(_script, this);
}

// In Myst/Making of Myst, the paths are hardcoded ala Windows style without extension. Convert them.
Common::String MystResourceType6::convertMystVideoName(Common::String name) {
	Common::String temp;

	for (uint32 i = 1; i < name.size(); i++) {
		if (name[i] == '\\')
			temp += '/';
		else
			temp += name[i];
	}

	return temp + ".mov";
}

MystResourceType6::MystResourceType6(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType5(vm, rlstStream, parent) {
	char c = 0;

	do {
		c = rlstStream->readByte();
		_videoFile += c;
	} while (c);

	rlstStream->skip(_videoFile.size() & 1);

	// Trim method does not remove extra trailing nulls
	while (_videoFile.size() != 0 && _videoFile.lastChar() == 0)
		_videoFile.deleteLastChar();

	_videoFile = convertMystVideoName(_videoFile);

	// Position values require modulus 10000 to keep in sane range.
	_left = rlstStream->readUint16LE() % 10000;
	_top = rlstStream->readUint16LE() % 10000;
	_loop = rlstStream->readUint16LE();
	_direction = rlstStream->readUint16LE();
	_playBlocking = rlstStream->readUint16LE();
	_playOnCardChange = rlstStream->readUint16LE();
	_u3 = rlstStream->readUint16LE();

	if (_direction != 1)
		warning("Type 6 _u0 != 1");
	if (_u3 != 0)
		warning("Type 6 _u3 != 0");

	debugC(kDebugResource, "\tvideoFile: \"%s\"", _videoFile.c_str());
	debugC(kDebugResource, "\tleft: %d", _left);
	debugC(kDebugResource, "\ttop: %d", _top);
	debugC(kDebugResource, "\tloop: %d", _loop);
	debugC(kDebugResource, "\tdirection: %d", _direction);
	debugC(kDebugResource, "\tplayBlocking: %d", _playBlocking);
	debugC(kDebugResource, "\tplayOnCardChange: %d", _playOnCardChange);
	debugC(kDebugResource, "\tu3: %d", _u3);

	_videoRunning = false;
}

void MystResourceType6::handleAnimation() {
	// TODO: Implement Code to allow _playOnCardChange when set
	//       and trigger by Opcode 9 when clear

	if (!_videoRunning) {
		// NOTE: The left and top coordinates are often incorrect and do not make sense.
		// We use the rect coordinates here instead.

		if (_playBlocking)
			_vm->_video->playMovie(_videoFile, _rect.left, _rect.top);
		else
			_vm->_video->playBackgroundMovie(_videoFile, _rect.left, _rect.top, _loop);

		_videoRunning = true;
	}
}

MystResourceType7::MystResourceType7(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResource(vm, rlstStream, parent) {
	_var7 = rlstStream->readUint16LE();
	_numSubResources = rlstStream->readUint16LE();
	debugC(kDebugResource, "\tvar7: %d", _var7);
	debugC(kDebugResource, "\tnumSubResources: %d", _numSubResources);

	for (uint16 i = 0; i < _numSubResources; i++)
		_subResources.push_back(vm->loadResource(rlstStream, this));
}

MystResourceType7::~MystResourceType7() {
	while(!_subResources.empty()) {
		MystResource *temp = _subResources.back();
		_subResources.pop_back();
		delete temp;
	}
}

// TODO: All these functions to switch subresource are very similar.
//       Find way to share code (function pointer pass?)
void MystResourceType7::drawDataToScreen() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->drawDataToScreen();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_scriptParser->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->drawDataToScreen();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->drawDataToScreen();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleAnimation() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleAnimation();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_scriptParser->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleAnimation();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleAnimation();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleMouseUp(const Common::Point &mouse) {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleMouseUp(mouse);
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_scriptParser->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleMouseUp(mouse);
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleMouseUp(mouse);
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleMouseDown(const Common::Point &mouse) {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleMouseDown(mouse);
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_scriptParser->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleMouseDown(mouse);
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleMouseDown(mouse);
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleMouseEnter() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleMouseEnter();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_scriptParser->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleMouseEnter();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleMouseEnter();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

void MystResourceType7::handleMouseLeave() {
	if (_var7 == 0xFFFF) {
		if (_numSubResources == 1)
			_subResources[0]->handleMouseLeave();
		else if (_numSubResources != 0)
			warning("Type 7 Resource with _numSubResources of %d, but no control variable", _numSubResources);
	} else {
		uint16 varValue = _vm->_scriptParser->getVar(_var7);

		if (_numSubResources == 1 && varValue != 0)
			_subResources[0]->handleMouseLeave();
		else if (_numSubResources != 0) {
			if (varValue < _numSubResources)
				_subResources[varValue]->handleMouseLeave();
			else
				warning("Type 7 Resource Var %d: %d exceeds number of sub resources %d", _var7, varValue, _numSubResources);
		}
	}
}

MystResourceType8::MystResourceType8(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType7(vm, rlstStream, parent) {
	_var8 = rlstStream->readUint16LE();
	_numSubImages = rlstStream->readUint16LE();
	debugC(kDebugResource, "\tvar8: %d", _var8);
	debugC(kDebugResource, "\tnumSubImages: %d", _numSubImages);

	_subImages = new MystResourceType8::SubImage[_numSubImages];
	for (uint16 i = 0; i < _numSubImages; i++) {
		debugC(kDebugResource, "\tSubimage %d:", i);

		_subImages[i].wdib = rlstStream->readUint16LE();
		_subImages[i].rect.left = rlstStream->readSint16LE();

		if (_subImages[i].rect.left != -1) {
			_subImages[i].rect.top = rlstStream->readSint16LE();
			_subImages[i].rect.right = rlstStream->readSint16LE();
			_subImages[i].rect.bottom = rlstStream->readSint16LE();
		} else {
			// Use the hotspot rect as the source rect since the subimage is fullscreen
			// Convert to bitmap coordinates (upside down)
			_subImages[i].rect.left = _rect.left;
			_subImages[i].rect.top = 333 - _rect.bottom;
			_subImages[i].rect.right = _rect.right;
			_subImages[i].rect.bottom = 333 - _rect.top;
		}

		debugC(kDebugResource, "\twdib: %d", _subImages[i].wdib);
		debugC(kDebugResource, "\tleft: %d", _subImages[i].rect.left);
		debugC(kDebugResource, "\ttop: %d", _subImages[i].rect.top);
		debugC(kDebugResource, "\tright: %d", _subImages[i].rect.right);
		debugC(kDebugResource, "\tbottom: %d", _subImages[i].rect.bottom);
	}
}

MystResourceType8::~MystResourceType8() {
	delete[] _subImages;
}

void MystResourceType8::drawDataToScreen() {
	// Need to call overidden Type 7 function to ensure
	// switch section is processed correctly.
	MystResourceType7::drawDataToScreen();

	bool drawSubImage = false;
	int16 subImageId = 0;

	if (_var8 == 0xFFFF) {
		if (_numSubImages == 1) {
			subImageId = 0;
			drawSubImage = true;
		} else if (_numSubImages != 0)
			warning("Type 8 Resource with _numSubImages of %d, but no control variable", _numSubImages);
	} else {
		uint16 varValue = _vm->_scriptParser->getVar(_var8);

		if (_numSubImages == 1 && varValue != 0) {
			subImageId = 0;
			drawSubImage = true;
		} else if (_numSubImages != 0) {
			if (varValue < _numSubImages) {
				subImageId = varValue;
				drawSubImage = true;
			} else
				warning("Type 8 Image Var %d: %d exceeds number of subImages %d", _var8, varValue, _numSubImages);
		}
	}

	if (drawSubImage) {
		uint16 imageToDraw = 0;

		if (_subImages[subImageId].wdib == 0xFFFF) {
			// TODO: Think the reason for problematic screen updates in some rects is that they
			//       are these -1 cases.
			// They need to be redrawn i.e. if the Myst marker switches are changed, but I don't think
			// the rects are valid. This does not matter in the original engine as the screen update redraws
			// the VIEW images, followed by the RLST resource images, and -1 for the WDIB is interpreted as
			// "Do Not Draw Image" i.e so the VIEW image is shown through.. We need to fix screen update
			// to do this same behaviour.
			if (_vm->_view.conditionalImageCount == 0)
				imageToDraw = _vm->_view.mainImage;
			else {
				for (uint16 i = 0; i < _vm->_view.conditionalImageCount; i++)
					if (_vm->_scriptParser->getVar(_vm->_view.conditionalImages[i].var) < _vm->_view.conditionalImages[i].numStates)
						imageToDraw = _vm->_view.conditionalImages[i].values[_vm->_scriptParser->getVar(_vm->_view.conditionalImages[i].var)];
			}
		} else
			imageToDraw = _subImages[subImageId].wdib;

		_vm->_gfx->copyImageSectionToScreen(imageToDraw, _subImages[subImageId].rect, _rect);
	}
}

void MystResourceType8::drawConditionalDataToScreen(uint16 state) {
	// Need to call overidden Type 7 function to ensure
	// switch section is processed correctly.
	MystResourceType7::drawDataToScreen();

	bool drawSubImage = false;
	int16 subImageId = 0;


	if (_numSubImages == 1 && state != 0) {
		subImageId = 0;
		drawSubImage = true;
	} else if (_numSubImages != 0) {
		if (state < _numSubImages) {
			subImageId = state;
			drawSubImage = true;
		} else
			warning("Type 8 Image Var %d: %d exceeds number of subImages %d", _var8, state, _numSubImages);
	}


	if (drawSubImage) {
		uint16 imageToDraw = 0;

		if (_subImages[subImageId].wdib == 0xFFFF) {
			// TODO: Think the reason for problematic screen updates in some rects is that they
			//       are these -1 cases.
			// They need to be redrawn i.e. if the Myst marker switches are changed, but I don't think
			// the rects are valid. This does not matter in the original engine as the screen update redraws
			// the VIEW images, followed by the RLST resource images, and -1 for the WDIB is interpreted as
			// "Do Not Draw Image" i.e so the VIEW image is shown through.. We need to fix screen update
			// to do this same behaviour.
			if (_vm->_view.conditionalImageCount == 0)
				imageToDraw = _vm->_view.mainImage;
			else {
				for (uint16 i = 0; i < _vm->_view.conditionalImageCount; i++)
					if (_vm->_scriptParser->getVar(_vm->_view.conditionalImages[i].var) < _vm->_view.conditionalImages[i].numStates)
						imageToDraw = _vm->_view.conditionalImages[i].values[_vm->_scriptParser->getVar(_vm->_view.conditionalImages[i].var)];
			}
		} else
			imageToDraw = _subImages[subImageId].wdib;

		_vm->_gfx->copyImageSectionToScreen(imageToDraw, _subImages[subImageId].rect, _rect);
		_vm->_gfx->updateScreen();
	}
}

uint16 MystResourceType8::getType8Var() {
	return _var8;
}

// No MystResourceType9!

MystResourceType10::MystResourceType10(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType11(vm, rlstStream, parent) {
	_dragSound = rlstStream->readUint16LE();

	debugC(kDebugResource, "\tdrag sound : %d", _dragSound);

	_sliderWidth = _rect.right - _rect.left;
	_sliderHeigth = _rect.bottom - _rect.top;
}

MystResourceType10::~MystResourceType10() {
}

void MystResourceType10::setStep(uint16 step) {
	_rect.top = _minV + _stepV * step - _sliderHeigth / 2;
	_rect.bottom = _rect.top + _sliderHeigth;
	_subImages[0].rect.top = 333 - _rect.bottom - 1;
	_subImages[0].rect.bottom = 333 - _rect.top - 1;
}

Common::Rect MystResourceType10::boundingBox() {
	Common::Rect bb;

	bb.top = _rect.top;
	bb.bottom = _rect.bottom;
	bb.left = _rect.left;
	bb.right = _rect.right;

	if (_flagHV & 1) {
		bb.left = _minH - _sliderWidth / 2;
		bb.right = _maxH + _sliderWidth / 2;
	}

	if (_flagHV & 2) {
		bb.top = _minV - _sliderHeigth / 2;
		bb.bottom = _maxV + _sliderHeigth / 2;
	}

	return bb;
}

void MystResourceType10::restoreBackground() {
	// Restore background
	Common::Rect src = boundingBox();
	Common::Rect dest = boundingBox();
	src.top = 333 - dest.bottom;
	src.bottom = 333 - dest.top;
	_vm->_gfx->copyImageSectionToScreen(_vm->getCardBackgroundId(), src, dest);
}

void MystResourceType10::drawDataToScreen()
{
	// Restore background
	restoreBackground();

	MystResourceType8::drawDataToScreen();
}

void MystResourceType10::handleMouseDown(const Common::Point &mouse) {
	// Tell the engine we are dragging a resource
	_vm->_dragResource = this;

	updatePosition(mouse);

	MystResourceType11::handleMouseDown(mouse);

	// Restore background
	restoreBackground();

	// Draw slider
	drawConditionalDataToScreen(2);
}

void MystResourceType10::handleMouseUp(const Common::Point &mouse) {
	updatePosition(mouse);

	// Restore background
	restoreBackground();

	// Draw slider
	drawConditionalDataToScreen(1);

	// Save slider value
	uint16 value = 0;
	if (_flagHV & 2) {
		if (_stepsV) {
			value = (_pos.y - _minV) / _stepV;
		} else {
			value = _pos.y;
		}
	} else if (_flagHV & 1) {
		if (_stepsH) {
			value = (_pos.x - _minH) / _stepH;
		} else {
			value = _pos.x;
		}
	}
	_vm->_scriptParser->setVarValue(_var8, value);

	MystResourceType11::handleMouseUp(mouse);

	// No longer in drag mode
	_vm->_dragResource = 0;
}

void MystResourceType10::handleMouseDrag(const Common::Point &mouse) {
	updatePosition(mouse);

	MystResourceType11::handleMouseDrag(mouse);

	// Restore background
	restoreBackground();

	// Draw slider
	drawConditionalDataToScreen(2);
}

void MystResourceType10::updatePosition(const Common::Point &mouse) {
	bool positionChanged = false;

	Common::Point mouseClipped;
	setPositionClipping(mouse, mouseClipped);

	if (_flagHV & 2) {
		if (_stepV) {
			uint16 center = _minV + _stepV * (mouseClipped.y - _minV) / _stepV;
			uint16 top = center - _sliderHeigth / 2;
			if (_rect.top != top) {
				positionChanged = true;
				_pos.y = center;
				_rect.top = top;
			}
		} else {
			positionChanged = true;
			_pos.y = mouseClipped.y;
			_rect.top = mouseClipped.y - _sliderHeigth / 2;
		}
		if (positionChanged) {
			_rect.bottom = _rect.top + _sliderHeigth;
			_subImages[0].rect.top = 333 - _rect.bottom - 1;
			_subImages[0].rect.bottom = 333 - _rect.top - 1;
		}
	}

	if (_flagHV & 1) {
		if (_stepH) {
			uint16 center = _minH + _stepH * (mouseClipped.x - _minH) / _stepH;
			uint16 left = center - _sliderWidth / 2;
			if (_rect.left != left) {
				positionChanged = true;
				_pos.x = center;
				_rect.left = left;
			}
		} else {
			positionChanged = true;
			_pos.x = mouseClipped.x;
			_rect.left = mouseClipped.x - _sliderWidth / 2;
		}
		if (positionChanged) {
			_rect.right = _rect.left + _sliderWidth;
		}
	}

	if (positionChanged && _dragSound) {
		_vm->_sound->playSound(_dragSound);
	}
}

MystResourceType11::MystResourceType11(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType8(vm, rlstStream, parent) {
	_flagHV = rlstStream->readUint16LE();
	_minH = rlstStream->readUint16LE();
	_maxH = rlstStream->readUint16LE();
	_minV = rlstStream->readUint16LE();
	_maxV = rlstStream->readUint16LE();
	_stepsH = rlstStream->readUint16LE();
	_stepsV = rlstStream->readUint16LE();
	_mouseDownOpcode = rlstStream->readUint16LE();
	_mouseDragOpcode = rlstStream->readUint16LE();
	_mouseUpOpcode = rlstStream->readUint16LE();

	debugC(kDebugResource, "\tdirection: %d", _flagHV);
	debugC(kDebugResource, "\thorizontal min: %d", _minH);
	debugC(kDebugResource, "\thorizontal max: %d", _maxH);
	debugC(kDebugResource, "\tvertical min: %d", _minV);
	debugC(kDebugResource, "\tvertical max: %d", _maxV);
	debugC(kDebugResource, "\thorizontal steps: %d", _stepsH);
	debugC(kDebugResource, "\tvertical steps: %d", _stepsV);
	debugC(kDebugResource, "\t_mouseDownOpcode: %d", _mouseDownOpcode);
	debugC(kDebugResource, "\t_mouseDragOpcode: %d", _mouseDragOpcode);
	debugC(kDebugResource, "\t_mouseUpOpcode: %d", _mouseUpOpcode);

	debugCN(kDebugResource, "Type 11 _mouseDownOpcode: %d\n", _mouseDownOpcode);
	debugCN(kDebugResource, "Type 11 _mouseDragOpcode: %d\n", _mouseDragOpcode);
	debugCN(kDebugResource, "Type 11 _mouseUpOpcode: %d\n", _mouseUpOpcode);

	for (byte i = 0; i < 3; i++) {
		debugC(kDebugResource, "\tList %d:", i);

		_lists[i].listCount = rlstStream->readUint16LE();
		debugC(kDebugResource, "\t%d values", _lists[i].listCount);

		_lists[i].list = new uint16[_lists[i].listCount];
		for (uint16 j = 0; j < _lists[i].listCount; j++) {
			_lists[i].list[j] = rlstStream->readUint16LE();
			debugC(kDebugResource, "\tValue %d: %d", j, _lists[i].list[j]);
		}
	}

	if (_stepsH) {
		_stepH = (_maxH - _minH) / (_stepsH - 1);
	}

	if (_stepsV) {
		_stepV = (_maxV - _minV) / (_stepsV - 1);
	}
}

MystResourceType11::~MystResourceType11() {
	for (byte i = 0; i < 3; i++)
		delete[] _lists[i].list;
}

void MystResourceType11::handleMouseDown(const Common::Point &mouse) {
	setPositionClipping(mouse, _pos);

	_vm->_scriptParser->setInvokingResource(this);
	_vm->_scriptParser->runOpcode(_mouseDownOpcode, _var8);
}

void MystResourceType11::handleMouseUp(const Common::Point &mouse) {
	setPositionClipping(mouse, _pos);

	_vm->_scriptParser->setInvokingResource(this);
	_vm->_scriptParser->runOpcode(_mouseUpOpcode, _var8);
}

void MystResourceType11::handleMouseDrag(const Common::Point &mouse) {
	setPositionClipping(mouse, _pos);

	_vm->_scriptParser->setInvokingResource(this);
	_vm->_scriptParser->runOpcode(_mouseDragOpcode, _var8);
}

void MystResourceType11::setPositionClipping(const Common::Point &mouse, Common::Point &dest) {
	if (_flagHV & 2) {
		dest.y = CLIP<uint16>(mouse.y, _minV, _maxV);
	}
	if (_flagHV & 1) {
		dest.x = CLIP<uint16>(mouse.x, _minH, _maxH);
	}
}

uint16 MystResourceType11::getList1(uint16 index) {
	if (index < _lists[1].listCount) {
		return _lists[1].list[index];
	}

	return 0;
}

uint16 MystResourceType11::getList2(uint16 index) {
	if (index < _lists[2].listCount) {
		return _lists[2].list[index];
	}

	return 0;
}

uint16 MystResourceType11::getList3(uint16 index) {
	if (index < _lists[3].listCount) {
		return _lists[3].list[index];
	}

	return 0;
}

MystResourceType12::MystResourceType12(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResourceType11(vm, rlstStream, parent) {
	_numFrames = rlstStream->readUint16LE();
	_firstFrame = rlstStream->readUint16LE();
	uint16 frameWidth = rlstStream->readUint16LE();
	uint16 frameHeight = rlstStream->readUint16LE();
	_frameRect.left = rlstStream->readUint16LE();
	_frameRect.top = rlstStream->readUint16LE();

	_frameRect.right = _frameRect.left + frameWidth;
	_frameRect.bottom = _frameRect.top + frameHeight;

	debugC(kDebugResource, "\t_numFrames: %d", _numFrames);
	debugC(kDebugResource, "\t_firstFrame: %d", _firstFrame);
	debugC(kDebugResource, "\tframeWidth: %d", frameWidth);
	debugC(kDebugResource, "\tframeHeight: %d", frameHeight);
	debugC(kDebugResource, "\t_frameRect.left: %d", _frameRect.left);
	debugC(kDebugResource, "\t_frameRect.top: %d", _frameRect.top);
	debugC(kDebugResource, "\t_frameRect.right: %d", _frameRect.right);
	debugC(kDebugResource, "\t_frameRect.bottom: %d", _frameRect.bottom);
}

MystResourceType12::~MystResourceType12() {

}

void MystResourceType12::drawFrame(uint16 frame) {
	_currentFrame = _firstFrame + frame;
	_vm->_gfx->copyImageToScreen(_currentFrame, _frameRect);
	_vm->_gfx->updateScreen();
}

MystResourceType13::MystResourceType13(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent) : MystResource(vm, rlstStream, parent) {
	_enterOpcode = rlstStream->readUint16LE();
	_leaveOpcode = rlstStream->readUint16LE();

	debugC(kDebugResource, "\t_enterOpcode: %d", _enterOpcode);
	debugC(kDebugResource, "\t_leaveOpcode: %d", _leaveOpcode);
}

void MystResourceType13::handleMouseEnter() {
	// Pass along the enter opcode (with no parameters) to the script parser
	_vm->_scriptParser->runOpcode(_enterOpcode);
}

void MystResourceType13::handleMouseLeave() {
	// Pass along the leave opcode (with no parameters) to the script parser
	_vm->_scriptParser->runOpcode(_leaveOpcode);
}

void MystResourceType13::handleMouseUp(const Common::Point &mouse) {
	// Type 13 Resources do nothing on Mouse Clicks.
	// This is required to override the inherited default
	// i.e. MystResource::handleMouseUp
}

} // End of namespace Mohawk
