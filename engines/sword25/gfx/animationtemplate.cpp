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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#define BS_LOG_PREFIX "ANIMATIONTEMPLATE"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resource.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

#include "sword25/gfx/animationresource.h"
#include "sword25/gfx/animationtemplate.h"
#include "sword25/gfx/animationtemplateregistry.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

uint AnimationTemplate::Create(const Common::String &SourceAnimation) {
	AnimationTemplate *AnimationTemplatePtr = new AnimationTemplate(SourceAnimation);

	if (AnimationTemplatePtr->isValid()) {
		return AnimationTemplateRegistry::getInstance().resolvePtr(AnimationTemplatePtr);
	} else {
		delete AnimationTemplatePtr;
		return 0;
	}
}

// -----------------------------------------------------------------------------

uint AnimationTemplate::Create(const AnimationTemplate &Other) {
	AnimationTemplate *AnimationTemplatePtr = new AnimationTemplate(Other);

	if (AnimationTemplatePtr->isValid()) {
		return AnimationTemplateRegistry::getInstance().resolvePtr(AnimationTemplatePtr);
	} else {
		delete AnimationTemplatePtr;
		return 0;
	}
}

// -----------------------------------------------------------------------------

uint AnimationTemplate::Create(InputPersistenceBlock &Reader, uint Handle) {
	AnimationTemplate *AnimationTemplatePtr = new AnimationTemplate(Reader, Handle);

	if (AnimationTemplatePtr->isValid()) {
		return AnimationTemplateRegistry::getInstance().resolvePtr(AnimationTemplatePtr);
	} else {
		delete AnimationTemplatePtr;
		return 0;
	}
}

// -----------------------------------------------------------------------------

AnimationTemplate::AnimationTemplate(const Common::String &SourceAnimation) {
	// Objekt registrieren.
	AnimationTemplateRegistry::getInstance().registerObject(this);

	_valid = false;

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt
	_sourceAnimationPtr = RequestSourceAnimation(SourceAnimation);

	// Erfolg signalisieren
	_valid = (_sourceAnimationPtr != 0);
}

// -----------------------------------------------------------------------------

AnimationTemplate::AnimationTemplate(const AnimationTemplate &Other) : AnimationDescription(){
	// Objekt registrieren.
	AnimationTemplateRegistry::getInstance().registerObject(this);

	_valid = false;

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt.
	if (!Other._sourceAnimationPtr) return;
	_sourceAnimationPtr = RequestSourceAnimation(Other._sourceAnimationPtr->getFileName());

	// Alle Member kopieren.
	_animationType = Other._animationType;
	_FPS = Other._FPS;
	_millisPerFrame = Other._millisPerFrame;
	_scalingAllowed = Other._scalingAllowed;
	_alphaAllowed = Other._alphaAllowed;
	_colorModulationAllowed = Other._colorModulationAllowed;
	_frames = Other._frames;
	_sourceAnimationPtr = Other._sourceAnimationPtr;
	_valid = Other._valid;

	_valid &= (_sourceAnimationPtr != 0);
}

// -----------------------------------------------------------------------------

AnimationTemplate::AnimationTemplate(InputPersistenceBlock &Reader, uint Handle) {
	// Objekt registrieren.
	AnimationTemplateRegistry::getInstance().registerObject(this, Handle);

	// Objekt laden.
	_valid = unpersist(Reader);
}

// -----------------------------------------------------------------------------

AnimationResource *AnimationTemplate::RequestSourceAnimation(const Common::String &SourceAnimation) const {
	ResourceManager *RMPtr = Kernel::GetInstance()->GetResourceManager();
	Resource *ResourcePtr;
	if (NULL == (ResourcePtr = RMPtr->RequestResource(SourceAnimation)) || ResourcePtr->GetType() != Resource::TYPE_ANIMATION) {
		BS_LOG_ERRORLN("The resource \"%s\" could not be requested or is has an invalid type. The animation template can't be created.", SourceAnimation.c_str());
		return 0;
	}
	return static_cast<AnimationResource *>(ResourcePtr);
}

// -----------------------------------------------------------------------------

AnimationTemplate::~AnimationTemplate() {
	// Animations-Resource freigeben
	if (_sourceAnimationPtr) {
		_sourceAnimationPtr->release();
	}

	// Objekt deregistrieren
	AnimationTemplateRegistry::getInstance().deregisterObject(this);
}

// -----------------------------------------------------------------------------

void AnimationTemplate::AddFrame(int Index) {
	if (ValidateSourceIndex(Index)) {
		_frames.push_back(_sourceAnimationPtr->getFrame(Index));
	}
}

// -----------------------------------------------------------------------------

void AnimationTemplate::SetFrame(int DestIndex, int SrcIndex) {
	if (ValidateDestIndex(DestIndex) && ValidateSourceIndex(SrcIndex)) {
		_frames[DestIndex] = _sourceAnimationPtr->getFrame(SrcIndex);
	}
}

// -----------------------------------------------------------------------------

bool AnimationTemplate::ValidateSourceIndex(uint Index) const {
	if (Index > _sourceAnimationPtr->getFrameCount()) {
		BS_LOG_WARNINGLN("Tried to insert a frame (\"%d\") that does not exist in the source animation (\"%s\"). Ignoring call.",
		                 Index, _sourceAnimationPtr->getFileName().c_str());
		return false;
	} else
		return true;
}

// -----------------------------------------------------------------------------

bool AnimationTemplate::ValidateDestIndex(uint Index) const {
	if (Index > _frames.size()) {
		BS_LOG_WARNINGLN("Tried to change a nonexistent frame (\"%d\") in a template animation. Ignoring call.",
		                 Index);
		return false;
	} else
		return true;
}

// -----------------------------------------------------------------------------

void AnimationTemplate::SetFPS(int FPS) {
	_FPS = FPS;
	_millisPerFrame = 1000000 / _FPS;
}

// -----------------------------------------------------------------------------

bool AnimationTemplate::persist(OutputPersistenceBlock &writer) {
	bool Result = true;

	// Parent persistieren.
	Result &= AnimationDescription::persist(writer);

	// Frameanzahl schreiben.
	writer.write(_frames.size());

	// Frames einzeln persistieren.
	Common::Array<const Frame>::const_iterator Iter = _frames.begin();
	while (Iter != _frames.end()) {
		writer.write(Iter->HotspotX);
		writer.write(Iter->HotspotY);
		writer.write(Iter->FlipV);
		writer.write(Iter->FlipH);
		writer.write(Iter->FileName);
		writer.write(Iter->Action);
		++Iter;
	}

	// Restliche Member persistieren.
	writer.write(_sourceAnimationPtr->getFileName());
	writer.write(_valid);

	return Result;
}

// -----------------------------------------------------------------------------

bool AnimationTemplate::unpersist(InputPersistenceBlock &reader) {
	bool Result = true;

	// Parent wieder herstellen.
	Result &= AnimationDescription::unpersist(reader);

	// Frameanzahl lesen.
	uint FrameCount;
	reader.read(FrameCount);

	// Frames einzeln wieder herstellen.
	for (uint i = 0; i < FrameCount; ++i) {
		Frame frame;
		reader.read(frame.HotspotX);
		reader.read(frame.HotspotY);
		reader.read(frame.FlipV);
		reader.read(frame.FlipH);
		reader.read(frame.FileName);
		reader.read(frame.Action);

		_frames.push_back(frame);
	}

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt
	Common::String SourceAnimation;
	reader.read(SourceAnimation);
	_sourceAnimationPtr = RequestSourceAnimation(SourceAnimation);

	reader.read(_valid);

	return _sourceAnimationPtr && reader.isGood() && Result;
}

} // End of namespace Sword25
