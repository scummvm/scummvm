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

#include "mohawk/myst_card.h"

#include "mohawk/myst_areas.h"
#include "mohawk/myst_graphics.h"

#include "mohawk/resource.h"

namespace Mohawk {

MystCard::MystCard(MohawkEngine_Myst *vm, uint16 id) :
		_vm(vm),
		_id(id),
		_hoverResource(nullptr),
		_activeResource(nullptr),
		_clickedResource(nullptr) {

	loadView();
	loadResources();
	loadCursorHints();
}

void MystCard::enter() {
	// Handle images
	drawBackground();

	// Handle sound
	_vm->applySoundBlock(_soundBlock);

	if (_flags & kMystZipDestination)
		_vm->_gameState->addZipDest(_vm->_stack->getStackId(), _id);

	// Run the entrance script (if present)
	runInitScript();

	// Update the images of each area too
	drawResourceImages();

	for (uint16 i = 0; i < _resources.size(); i++)
		_resources[i]->handleCardChange();
}

void MystCard::leave() {
	runExitScript();
}

MystCard::~MystCard() {
	for (uint32 i = 0; i < _resources.size(); i++)
		delete _resources[i];
}

uint16 MystCard::getId() const {
	return _id;
}

void MystCard::loadView() {
	debugC(kDebugView, "Loading Card View: %d", _id);

	Common::SeekableReadStream *viewStream = _vm->getResource(ID_VIEW, _id);

	// Card Flags
	_flags = viewStream->readUint16LE();
	debugC(kDebugView, "Flags: 0x%04X", _flags);

	// The Image Block (Reminiscent of Riven PLST resources)
	uint16 conditionalImageCount = viewStream->readUint16LE();
	debugC(kDebugView, "Conditional Image Count: %d", conditionalImageCount);
	if (conditionalImageCount != 0) {
		for (uint16 i = 0; i < conditionalImageCount; i++) {
			MystCondition conditionalImage;

			debugC(kDebugView, "\tImage %d:", i);
			conditionalImage.var = viewStream->readUint16LE();
			debugC(kDebugView, "\t\tVar: %d", conditionalImage.var);
			uint16 numStates = viewStream->readUint16LE();
			debugC(kDebugView, "\t\tNumber of States: %d", numStates);
			for (uint16 j = 0; j < numStates; j++) {
				conditionalImage.values.push_back(viewStream->readUint16LE());
				debugC(kDebugView, "\t\tState %d -> Value %d", j, conditionalImage.values[j]);
			}

			_conditionalImages.push_back(conditionalImage);
		}
		_mainImage = 0;
	} else {
		_mainImage = viewStream->readUint16LE();
		debugC(kDebugView, "Main Image: %d", _mainImage);
	}

	// The Sound Block (Reminiscent of Riven SLST resources)
	_soundBlock = _vm->readSoundBlock(viewStream);

	// Resources that scripts can call upon
	uint16 scriptResCount = viewStream->readUint16LE();
	debugC(kDebugView, "Script Resource Count: %d", scriptResCount);
	for (uint16 i = 0; i < scriptResCount; i++) {
		ScriptResource scriptResource;

		debugC(kDebugView, "\tResource %d:", i);
		scriptResource.type = (ScriptResourceType) viewStream->readUint16LE();
		debugC(kDebugView, "\t\t Type: %d", scriptResource.type);

		switch (scriptResource.type) {
			case kResourceImage:
				debugC(kDebugView, "\t\t\t\t= Image");
				break;
			case kResourceSound:
				debugC(kDebugView, "\t\t\t\t= Sound");
				break;
			case kResourceSwitch:
				debugC(kDebugView, "\t\t\t\t= Resource Switch");
				break;
			case kResourceImageNoCache:
				debugC(kDebugView, "\t\t\t\t= Image - Caching disabled");
				break;
			case kResourceSoundNoCache:
				debugC(kDebugView, "\t\t\t\t= Sound - Caching disabled");
				break;
			default:
				debugC(kDebugView, "\t\t\t\t= Unknown");
				warning("Unknown script resource type '%d' in card '%d'", scriptResource.type, _id);
				break;
		}

		if (scriptResource.type == kResourceSwitch) {
			scriptResource.switchVar = viewStream->readUint16LE();
			debugC(kDebugView, "\t\t Var: %d", scriptResource.switchVar);
			uint16 count = viewStream->readUint16LE();
			debugC(kDebugView, "\t\t Resource List Count: %d", count);
			scriptResource.switchResourceType = (ScriptResourceType) viewStream->readUint16LE();
			debugC(kDebugView, "\t\t u0: %d", scriptResource.switchResourceType);

			for (uint16 j = 0; j < count; j++) {
				scriptResource.switchResourceIds.push_back(viewStream->readSint16LE());
				debugC(kDebugView, "\t\t Resource List %d: %d", j, scriptResource.switchResourceIds[j]);
			}
		} else {
			scriptResource.id = viewStream->readUint16LE();
			debugC(kDebugView, "\t\t Id: %d", scriptResource.id);
		}

		_scriptResources.push_back(scriptResource);
	}

	// Identifiers for other resources. 0 if non existent. There is always an RLST.
	_resourceListId = viewStream->readUint16LE();
	if (!_resourceListId)
		error("RLST Index missing");

	_hintResourceId = viewStream->readUint16LE();
	_initScriptId = viewStream->readUint16LE();
	_exitScriptId = viewStream->readUint16LE();

	delete viewStream;

	// Precache Card Resources
	uint32 cacheImageType;
	if (_vm->isGameVariant(GF_ME))
		cacheImageType = ID_PICT;
	else
		cacheImageType = ID_WDIB;

	// Precache Image Block data
	if (!_conditionalImages.empty()) {
		for (uint16 i = 0; i < _conditionalImages.size(); i++) {
			uint16 value = _vm->_stack->getVar(_conditionalImages[i].var);
			_vm->cachePreload(cacheImageType, _conditionalImages[i].values[value]);
		}
	} else {
		_vm->cachePreload(cacheImageType, _mainImage);
	}

	// Precache Sound Block data
	if (_soundBlock.sound > 0)
		_vm->cachePreload(ID_MSND, _soundBlock.sound);
	else if (_soundBlock.sound == kMystSoundActionConditional) {
		uint16 value = _vm->_stack->getVar(_soundBlock.soundVar);
		if (_soundBlock.soundList[value].action > 0) {
			_vm->cachePreload(ID_MSND, _soundBlock.soundList[value].action);
		}
	}

	// Precache Script Resources
	for (uint16 i = 0; i < _scriptResources.size(); i++) {
		ScriptResourceType type;
		int16 id;
		if (_scriptResources[i].type == kResourceSwitch) {
			type = _scriptResources[i].switchResourceType;
			uint16 value = _vm->_stack->getVar(_scriptResources[i].switchVar);
			id = _scriptResources[i].switchResourceIds[value];
		} else {
			type = _scriptResources[i].type;
			id = _scriptResources[i].id;
		}

		if (id < 0) continue;

		switch (type) {
			case kResourceImage:
				_vm->cachePreload(cacheImageType, id);
				break;
			case kResourceSound:
				_vm->cachePreload(ID_MSND, id);
				break;
			default:
				// The other resource types should not be cached
				break;
		}
	}
}

void MystCard::loadCursorHints() {
	if (!_hintResourceId) {
		debugC(kDebugHint, "No HINT Present");
		return;
	}

	debugC(kDebugHint, "Loading Cursor Hints:");

	Common::SeekableReadStream *hintStream = _vm->getResource(ID_HINT, _id);
	uint16 cursorHintCount = hintStream->readUint16LE();
	debugC(kDebugHint, "Cursor Hint Count: %d", cursorHintCount);

	for (uint16 i = 0; i < cursorHintCount; i++) {
		MystCursorHint hint;

		debugC(kDebugHint, "Cursor Hint %d:", i);
		hint.id = hintStream->readUint16LE();
		debugC(kDebugHint, "\tId: %d", hint.id);
		hint.cursor = hintStream->readSint16LE();
		debugC(kDebugHint, "\tCursor: %d", hint.cursor);

		if (hint.cursor == -1) {
			debugC(kDebugHint, "\tConditional Cursor Hints:");
			hint.variableHint.var = hintStream->readUint16LE();
			debugC(kDebugHint, "\tVar: %d", hint.variableHint.var);
			uint16 numStates = hintStream->readUint16LE();
			debugC(kDebugHint, "\tNumber of States: %d", numStates);
			for (uint16 j = 0; j < numStates; j++) {
				hint.variableHint.values.push_back(hintStream->readUint16LE());
				debugC(kDebugHint, "\t\t State %d: Cursor %d", j, hint.variableHint.values[j]);
			}
		} else {
			hint.variableHint.var = 0;
		}

		_cursorHints.push_back(hint);
	}

	delete hintStream;
}

void MystCard::loadResources() {
	if (!_resourceListId) {
		debugC(kDebugResource, "No RLST present");
		return;
	}

	Common::SeekableReadStream *rlstStream = _vm->getResource(ID_RLST, _resourceListId);
	uint16 resourceCount = rlstStream->readUint16LE();
	debugC(kDebugResource, "RLST Resource Count: %d", resourceCount);

	for (uint16 i = 0; i < resourceCount; i++) {
		debugC(kDebugResource, "Resource #%d:", i);
		_resources.push_back(_vm->loadResource(rlstStream, nullptr));
	}

	delete rlstStream;
}

uint16 MystCard::getBackgroundImageId() {
	uint16 imageToDraw = 0;

	if (_conditionalImages.empty())
		imageToDraw = _mainImage;
	else {
		for (uint16 i = 0; i < _conditionalImages.size(); i++) {
			uint16 varValue = _vm->_stack->getVar(_conditionalImages[i].var);
			if (varValue < _conditionalImages[i].values.size())
				imageToDraw = _conditionalImages[i].values[varValue];
		}
	}

	return imageToDraw;
}

void MystCard::drawBackground() {
	_vm->_gfx->copyImageToBackBuffer(getBackgroundImageId(), Common::Rect(0, 0, 544, 332));
}

void MystCard::runInitScript() {
	if (!_initScriptId) {
		debugC(kDebugINIT, "No INIT Present");
		return;
	}

	debugC(kDebugINIT, "Running INIT script");

	Common::SeekableReadStream *initStream = _vm->getResource(ID_INIT, _initScriptId);
	MystScript script = _vm->_stack->readScript(initStream, kMystScriptInit);
	delete initStream;

	_vm->_stack->runScript(script);
}

void MystCard::runExitScript() {
	if (!_exitScriptId) {
		debugC(kDebugEXIT, "No EXIT Present");
		return;
	}

	debugC(kDebugEXIT, "Running EXIT script");

	Common::SeekableReadStream *exitStream = _vm->getResource(ID_EXIT, _exitScriptId);
	MystScript script = _vm->_stack->readScript(exitStream, kMystScriptExit);
	delete exitStream;

	_vm->_stack->runScript(script);
}

void MystCard::drawResourceRects() {
	for (uint16 i = 0; i < _resources.size(); i++) {
		_resources[i]->getRect().debugPrint(0);
		_resources[i]->drawBoundingRect();
	}
}

void MystCard::updateActiveResource(const Common::Point &mouse) {
	_activeResource = nullptr;
	for (uint16 i = 0; i < _resources.size(); i++) {
		if (_resources[i]->contains(mouse) && _resources[i]->canBecomeActive()) {
			_activeResource = _resources[i];
			break;
		}
	}
}

MystArea *MystCard::forceUpdateClickedResource(const Common::Point &mouse) {
	updateActiveResource(mouse);

	_clickedResource = _activeResource;

	return _clickedResource;
}

void MystCard::updateResourcesForInput(const Common::Point &mouse, bool mouseClicked, bool mouseMoved) {
	// Tell previous resource the mouse is no longer hovering it
	if (_hoverResource && !_hoverResource->contains(mouse)) {
		_hoverResource->handleMouseLeave();
		_hoverResource = nullptr;
	}

	for (uint16 i = 0; i < _resources.size(); i++) {
		if (_resources[i]->contains(mouse) && _resources[i]->hasType(kMystAreaHover)
		    && _hoverResource != _resources[i]) {
			_hoverResource = static_cast<MystAreaHover *>(_resources[i]);
			_hoverResource->handleMouseEnter();
		}
	}

	if (!mouseClicked && _clickedResource) {
		if (_clickedResource->isEnabled()) {
			_clickedResource->handleMouseUp();
		}
		_clickedResource = nullptr;
	} else if (mouseMoved && _clickedResource) {
		if (_clickedResource->isEnabled()) {
			_clickedResource->handleMouseDrag();
		}
	} else if (mouseClicked && !_clickedResource) {
		if (_activeResource && _activeResource->isEnabled()) {
			_clickedResource = _activeResource;
			_clickedResource->handleMouseDown();
		}
	}
}

int16 MystCard::getActiveResourceCursor() {
	if (!_hintResourceId) {
		// Default to the main cursor when no hints are present
		return -1;
	}

	// Check all the cursor hints to see if we're in a hotspot that contains a hint.
	for (uint16 i = 0; i < _cursorHints.size(); i++) {
		if (_activeResource && _resources[_cursorHints[i].id] == _activeResource && _activeResource->isEnabled()) {
			if (_cursorHints[i].cursor == -1) {
				uint16 var_value = _vm->_stack->getVar(_cursorHints[i].variableHint.var);

				if (var_value >= _cursorHints[i].variableHint.values.size())
					warning("Variable %d Out of Range in variable HINT Resource %d", _cursorHints[i].variableHint.var,
					        i);
				else {
					uint16 cursor = _cursorHints[i].variableHint.values[var_value];
					if (cursor == 0)
						return -1;
					else
						return cursor;
				}
			} else {
				if (_cursorHints[i].cursor == 0)
					return -1;
				else
					return _cursorHints[i].cursor;
			}
		}
	}

	return -1;
}

void MystCard::setResourceEnabled(uint16 resourceIndex, bool enable) {
	if (resourceIndex < _resources.size()) {
		_resources[resourceIndex]->setEnabled(enable);
	} else
		warning("Attempt to change unknown resource enable state");
}

void MystCard::drawResourceImages() {
	for (uint16 i = 0; i < _resources.size(); i++)
		if (_resources[i]->isDrawSubimages())
			_resources[i]->drawDataToScreen();
}

void MystCard::redrawArea(uint16 var, bool updateScreen) {
	for (uint16 i = 0; i < _resources.size(); i++)
		if (_resources[i]->hasType(kMystAreaImageSwitch) && _resources[i]->getImageSwitchVar() == var)
			_vm->redrawResource(static_cast<MystAreaImageSwitch *>(_resources[i]), updateScreen);
}

bool MystCard::isDraggingResource() const {
	return _clickedResource != nullptr;
}

void MystCard::resetClickedResource() {
	_clickedResource = nullptr;
}

} // End of namespace Mohawk
