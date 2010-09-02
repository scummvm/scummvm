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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/gfx/animationresource.h"

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/string.h"
#include "sword25/package/packagemanager.h"
#include "sword25/gfx/bitmapresource.h"

namespace Sword25 {

// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "ANIMATIONRESOURCE"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

namespace {
const int   DEFAULT_FPS = 10;
const int   MIN_FPS     = 1;
const int   MAX_FPS     = 200;
}

// -----------------------------------------------------------------------------
// Construction / Destruction
// -----------------------------------------------------------------------------

AnimationResource::AnimationResource(const Common::String &filename) :
		Resource(filename, Resource::TYPE_ANIMATION),
		Common::XMLParser(),
		m_Valid(false) {
	// Get a pointer to the package manager
	Kernel *pKernel = Kernel::GetInstance();
	_pPackage = static_cast<PackageManager *>(pKernel->GetService("package"));
	BS_ASSERT(_pPackage);

	// Switch to the folder the specified Xml fiile is in
	Common::String oldDirectory = _pPackage->GetCurrentDirectory();
	if (GetFileName().contains('/')) {
		Common::String dir = Common::String(GetFileName().c_str(), strrchr(GetFileName().c_str(), '/'));
		_pPackage->ChangeDirectory(dir);
	}

	// Load the contents of the file
	uint fileSize;
	char *xmlData = _pPackage->GetXmlFile(GetFileName(), &fileSize);
	if (!xmlData) {
		BS_LOG_ERRORLN("Could not read \"%s\".", GetFileName().c_str());
		return; 
	}

	// Parse the contents
	if (!loadBuffer((const byte *)xmlData, fileSize))
		return;

	m_Valid = parse();
	close();
	free(xmlData);

	// Switch back to the previous folder
	_pPackage->ChangeDirectory(oldDirectory);

	// Give an error message if there weren't any frames specified
	if (m_Frames.empty()) {
		BS_LOG_ERRORLN("\"%s\" does not have any frames.", GetFileName().c_str());
		return;
	}

	// Pre-cache all the frames
	if (!PrecacheAllFrames()) {
		BS_LOG_ERRORLN("Could not precache all frames of \"%s\".", GetFileName().c_str());
		return;
	}

	// Post processing to compute animation features
	if (!ComputeFeatures()) {
		BS_LOG_ERRORLN("Could not determine the features of \"%s\".", GetFileName().c_str());
		return;
	}

	m_Valid = true;
}

// -----------------------------------------------------------------------------

bool AnimationResource::parseBooleanKey(Common::String s, bool &result) {
	s.toLowercase();
	if (!strcmp(s.c_str(), "true"))
		result = true;
	else if (!strcmp(s.c_str(), "false"))
		result = false;
	else
		return false;
	return true;
}

// -----------------------------------------------------------------------------

bool AnimationResource::parserCallback_animation(ParserNode *node) {
	if (!parseIntegerKey(node->values["fps"].c_str(), 1, &m_FPS) || (m_FPS < MIN_FPS) || (m_FPS > MAX_FPS)) {
		return parserError("Illegal or missing fps attribute in <animation> tag in \"%s\". Assuming default (\"%d\").",
		                 GetFileName().c_str(), DEFAULT_FPS);
	}

	// Loop type value
	const char *loopTypeString = node->values["type"].c_str();
	
	if (strcmp(loopTypeString, "oneshot") == 0) {
		m_AnimationType = Animation::AT_ONESHOT;
	} else if (strcmp(loopTypeString, "loop") == 0) {
		m_AnimationType = Animation::AT_LOOP;
	} else if (strcmp(loopTypeString, "jojo") == 0) {
		m_AnimationType = Animation::AT_JOJO;
	} else {
		BS_LOG_WARNINGLN("Illegal type value (\"%s\") in <animation> tag in \"%s\". Assuming default (\"loop\").",
				loopTypeString, GetFileName().c_str());
		m_AnimationType = Animation::AT_LOOP;
	}

	// Calculate the milliseconds required per frame
	// FIXME: Double check variable naming. Based on the constant, it may be microseconds
	m_MillisPerFrame = 1000000 / m_FPS;

	return true;
}

// -----------------------------------------------------------------------------

bool AnimationResource::parserCallback_frame(ParserNode *node) {
	Frame frame;

	const char *fileString = node->values["file"].c_str();
	if (!fileString) {
		BS_LOG_ERRORLN("<frame> tag without file attribute occurred in \"%s\".", GetFileName().c_str());
		return false;
	}
	frame.FileName = _pPackage->GetAbsolutePath(fileString);
	if (frame.FileName.empty()) {
		BS_LOG_ERRORLN("Could not create absolute path for file specified in <frame> tag in \"%s\": \"%s\".", 
			GetFileName().c_str(), fileString);
		return false;
	}

	const char *actionString = node->values["action"].c_str();
	if (actionString)
		frame.Action = actionString;

	const char *hotspotxString = node->values["hotspotx"].c_str();
	const char *hotspotyString = node->values["hotspoty"].c_str();
	if ((!hotspotxString && hotspotyString) ||
	        (hotspotxString && !hotspotyString))
		BS_LOG_WARNINGLN("%s attribute occurred without %s attribute in <frame> tag in \"%s\". Assuming default (\"0\").",
		                 hotspotxString ? "hotspotx" : "hotspoty",
		                 !hotspotyString ? "hotspoty" : "hotspotx",
		                 GetFileName().c_str());

	frame.HotspotX = 0;
	if (hotspotxString && !parseIntegerKey(hotspotxString, 1, &frame.HotspotX))
		BS_LOG_WARNINGLN("Illegal hotspotx value (\"%s\") in frame tag in \"%s\". Assuming default (\"%s\").",
		                 hotspotxString, GetFileName().c_str(), frame.HotspotX);

	frame.HotspotY = 0;
	if (hotspotyString && !parseIntegerKey(hotspotyString, 1, &frame.HotspotY))
		BS_LOG_WARNINGLN("Illegal hotspoty value (\"%s\") in frame tag in \"%s\". Assuming default (\"%s\").",
		                 hotspotyString, GetFileName().c_str(), frame.HotspotY);

	Common::String flipVString = node->values["flipv"];
	if (!flipVString.empty()) {
		if (!parseBooleanKey(flipVString, frame.FlipV)) {
			BS_LOG_WARNINGLN("Illegal flipv value (\"%s\") in <frame> tag in \"%s\". Assuming default (\"false\").",
			                 flipVString.c_str(), GetFileName().c_str());
			frame.FlipV = false;
		}
	} else
		frame.FlipV = false;

	Common::String flipHString = node->values["fliph"];
	if (!flipHString.empty()) {
		if (!parseBooleanKey(flipVString, frame.FlipV)) {
			BS_LOG_WARNINGLN("Illegal fliph value (\"%s\") in <frame> tag in \"%s\". Assuming default (\"false\").",
			                 flipHString.c_str(), GetFileName().c_str());
			frame.FlipH = false;
		}
	} else
		frame.FlipH = false;

	m_Frames.push_back(frame);
	return true;
}

// -----------------------------------------------------------------------------

AnimationResource::~AnimationResource() {
}

// -----------------------------------------------------------------------------

bool AnimationResource::PrecacheAllFrames() const {
	Common::Array<Frame>::const_iterator Iter = m_Frames.begin();
	for (; Iter != m_Frames.end(); ++Iter) {
		if (!Kernel::GetInstance()->GetResourceManager()->PrecacheResource((*Iter).FileName)) {
			BS_LOG_ERRORLN("Could not precache \"%s\".", (*Iter).FileName.c_str());
			return false;
		}
	}

	return true;
}

// -----------------------------------------------------------------------------

bool AnimationResource::ComputeFeatures() {
	BS_ASSERT(m_Frames.size());

	// Alle Features werden als vorhanden angenommen
	m_ScalingAllowed = true;
	m_AlphaAllowed = true;
	m_ColorModulationAllowed = true;

	// Alle Frame durchgehen und alle Features deaktivieren, die auch nur von einem Frame nicht unterstützt werden.
	Common::Array<Frame>::const_iterator Iter = m_Frames.begin();
	for (; Iter != m_Frames.end(); ++Iter) {
		BitmapResource *pBitmap;
		if (!(pBitmap = static_cast<BitmapResource *>(Kernel::GetInstance()->GetResourceManager()->RequestResource((*Iter).FileName)))) {
			BS_LOG_ERRORLN("Could not request \"%s\".", (*Iter).FileName.c_str());
			return false;
		}

		if (!pBitmap->IsScalingAllowed())
			m_ScalingAllowed = false;
		if (!pBitmap->IsAlphaAllowed())
			m_AlphaAllowed = false;
		if (!pBitmap->IsColorModulationAllowed())
			m_ColorModulationAllowed = false;

		pBitmap->Release();
	}

	return true;
}

} // End of namespace Sword25
