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
#include <tinyxml.h>
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

BS_AnimationResource::BS_AnimationResource(const Common::String &FileName) :
	BS_Resource(FileName, BS_Resource::TYPE_ANIMATION),
	m_Valid(false) {
	// Pointer auf den Package-Manager bekommen
	BS_PackageManager *PackagePtr = BS_Kernel::GetInstance()->GetPackage();
	BS_ASSERT(PackagePtr);

	// Animations-XML laden
	TiXmlDocument Doc;
	{
		// Die Daten werden zunächst über den Package-Manager gelesen und dann in einen um ein Byte größeren Buffer kopiert und
		// NULL-Terminiert, da TinyXML NULL-Terminierte Daten benötigt.
		unsigned int FileSize;
		char *LoadBuffer = (char *) PackagePtr->GetFile(GetFileName(), &FileSize);
		if (!LoadBuffer) {
			BS_LOG_ERRORLN("Could not read \"%s\".", GetFileName().c_str());
			return;
		}
		char *WorkBuffer;
		WorkBuffer = (char *)malloc(FileSize + 1);
		memcpy(&WorkBuffer[0], LoadBuffer, FileSize);
		delete LoadBuffer;
		WorkBuffer[FileSize] = '\0';

		// Datei parsen
		Doc.Parse(&WorkBuffer[0]);
		free(WorkBuffer);
		if (Doc.Error()) {
			BS_LOG_ERRORLN("The following TinyXML-Error occured while parsing \"%s\": %s", GetFileName().c_str(), Doc.ErrorDesc());
			return;
		}
	}

	// Wurzelknoten des Animations-Tags finden, prüfen und Attribute auslesen.
	TiXmlElement *pElement;
	{
		TiXmlNode *pNode = Doc.FirstChild("animation");
		if (!pNode || pNode->Type() != TiXmlNode::ELEMENT) {
			BS_LOG_ERRORLN("No <animation> tag found in \"%s\".", GetFileName().c_str());
			return;
		}
		pElement = pNode->ToElement();

		// Animation-Tag parsen
		if (!ParseAnimationTag(*pElement, m_FPS, m_AnimationType)) {
			BS_LOG_ERRORLN("An error occurred while parsing <animation> tag in \"%s\".", GetFileName().c_str());
			return;
		}
	}

	// Zeit (in Millisekunden) bestimmen für die ein einzelner Frame angezeigt wird
	m_MillisPerFrame = 1000000 / m_FPS;

	// In das Verzeichnis der Eingabedatei wechseln, da die Dateiverweise innerhalb der XML-Datei relativ zu diesem Verzeichnis sind.
	Common::String OldDirectory = PackagePtr->GetCurrentDirectory();
	if (GetFileName().contains('/')) {
		Common::String Dir = Common::String(GetFileName().c_str(), strrchr(GetFileName().c_str(), '/'));
		PackagePtr->ChangeDirectory(Dir);
	}

	// Nacheinander alle Frames-Informationen erstellen.
	TiXmlElement *pFrameElement = pElement->FirstChild("frame")->ToElement();
	while (pFrameElement) {
		Frame CurFrame;

		if (!ParseFrameTag(*pFrameElement, CurFrame, *PackagePtr)) {
			BS_LOG_ERRORLN("An error occurred in \"%s\" while parsing <frame> tag.", GetFileName().c_str());
			return;
		}

		m_Frames.push_back(CurFrame);
		pFrameElement = pFrameElement->NextSiblingElement("frame");
	}

	// Ursprungsverzeichnis wieder herstellen
	PackagePtr->ChangeDirectory(OldDirectory);

	// Sicherstellen, dass die Animation mindestens einen Frame besitzt
	if (m_Frames.empty()) {
		BS_LOG_ERRORLN("\"%s\" does not have any frames.", GetFileName().c_str());
		return;
	}

	// Alle Frame-Dateien werden vorgecached
	if (!PrecacheAllFrames()) {
		BS_LOG_ERRORLN("Could not precache all frames of \"%s\".", GetFileName().c_str());
		return;
	}

	// Feststellen, ob die Animation skalierbar ist
	if (!ComputeFeatures()) {
		BS_LOG_ERRORLN("Could not determine the features of \"%s\".", GetFileName().c_str());
		return;
	}

	m_Valid = true;
}

// -----------------------------------------------------------------------------
// Dokument-Parsermethoden
// -----------------------------------------------------------------------------

bool BS_AnimationResource::ParseAnimationTag(TiXmlElement &AnimationTag, int &FPS, BS_Animation::ANIMATION_TYPES &AnimationType) {
	// FPS einlesen
	const char *FPSString;
	if (FPSString = AnimationTag.Attribute("fps")) {
		int TempFPS;
		if (!BS_String::ToInt(Common::String(FPSString), TempFPS) || TempFPS < MIN_FPS || TempFPS > MAX_FPS) {
			BS_LOG_WARNINGLN("Illegal fps value (\"%s\") in <animation> tag in \"%s\". Assuming default (\"%d\"). "
			                 "The fps value has to be between %d and %d.",
			                 FPSString, GetFileName().c_str(), DEFAULT_FPS, MIN_FPS, MAX_FPS);
		} else
			FPS = TempFPS;
	}

	// Loop-Typ einlesen
	const char *LoopTypeString;
	if (LoopTypeString = AnimationTag.Attribute("type")) {
		if (strcmp(LoopTypeString, "oneshot") == 0)
			AnimationType = BS_Animation::AT_ONESHOT;
		else if (strcmp(LoopTypeString, "loop") == 0)
			AnimationType = BS_Animation::AT_LOOP;
		else if (strcmp(LoopTypeString, "jojo") == 0)
			AnimationType = BS_Animation::AT_JOJO;
		else
			BS_LOG_WARNINGLN("Illegal type value (\"%s\") in <animation> tag in \"%s\". Assuming default (\"loop\").",
			                 LoopTypeString, GetFileName().c_str());
	}

	return true;
}

// -----------------------------------------------------------------------------

bool BS_AnimationResource::ParseFrameTag(TiXmlElement &FrameTag, Frame &Frame_, BS_PackageManager &PackageManager) {
	const char *FileString = FrameTag.Attribute("file");
	if (!FileString) {
		BS_LOG_ERRORLN("<frame> tag without file attribute occurred in \"%s\".", GetFileName().c_str());
		return false;
	}
	Frame_.FileName = PackageManager.GetAbsolutePath(FileString);
	if (Frame_.FileName == "") {
		BS_LOG_ERRORLN("Could not create absolute path for file specified in <frame> tag in \"%s\": \"%s\".", GetFileName().c_str(), FileString);
		return false;
	}

	const char *ActionString = FrameTag.Attribute("action");
	if (ActionString)
		Frame_.Action = ActionString;

	const char *HotspotxString = FrameTag.Attribute("hotspotx");
	const char *HotspotyString = FrameTag.Attribute("hotspoty");
	if ((!HotspotxString && HotspotyString) ||
	        (HotspotxString && !HotspotyString))
		BS_LOG_WARNINGLN("%s attribute occurred without %s attribute in <frame> tag in \"%s\". Assuming default (\"0\").",
		                 HotspotxString ? "hotspotx" : "hotspoty",
		                 !HotspotyString ? "hotspoty" : "hotspotx",
		                 GetFileName().c_str());

	Frame_.HotspotX = 0;
	if (HotspotxString && !BS_String::ToInt(Common::String(HotspotxString), Frame_.HotspotX))
		BS_LOG_WARNINGLN("Illegal hotspotx value (\"%s\") in frame tag in \"%s\". Assuming default (\"%s\").",
		                 HotspotxString, GetFileName().c_str(), Frame_.HotspotX);

	Frame_.HotspotY = 0;
	if (HotspotyString && !BS_String::ToInt(Common::String(HotspotyString), Frame_.HotspotY))
		BS_LOG_WARNINGLN("Illegal hotspoty value (\"%s\") in frame tag in \"%s\". Assuming default (\"%s\").",
		                 HotspotyString, GetFileName().c_str(), Frame_.HotspotY);

	const char *FlipVString = FrameTag.Attribute("flipv");
	if (FlipVString) {
		if (!BS_String::ToBool(Common::String(FlipVString), Frame_.FlipV)) {
			BS_LOG_WARNINGLN("Illegal flipv value (\"%s\") in <frame> tag in \"%s\". Assuming default (\"false\").",
			                 FlipVString, GetFileName().c_str());
			Frame_.FlipV = false;
		}
	} else
		Frame_.FlipV = false;

	const char *FlipHString = FrameTag.Attribute("fliph");
	if (FlipHString) {
		if (!BS_String::ToBool(FlipHString, Frame_.FlipH)) {
			BS_LOG_WARNINGLN("Illegal fliph value (\"%s\") in <frame> tag in \"%s\". Assuming default (\"false\").",
			                 FlipHString, GetFileName().c_str());
			Frame_.FlipH = false;
		}
	} else
		Frame_.FlipH = false;

	return true;
}

// -----------------------------------------------------------------------------

BS_AnimationResource::~BS_AnimationResource() {
}

// -----------------------------------------------------------------------------

bool BS_AnimationResource::PrecacheAllFrames() const {
	Common::Array<Frame>::const_iterator Iter = m_Frames.begin();
	for (; Iter != m_Frames.end(); ++Iter) {
		if (!BS_Kernel::GetInstance()->GetResourceManager()->PrecacheResource((*Iter).FileName)) {
			BS_LOG_ERRORLN("Could not precache \"%s\".", (*Iter).FileName.c_str());
			return false;
		}
	}

	return true;
}

// -----------------------------------------------------------------------------

bool BS_AnimationResource::ComputeFeatures() {
	BS_ASSERT(m_Frames.size());

	// Alle Features werden als vorhanden angenommen
	m_ScalingAllowed = true;
	m_AlphaAllowed = true;
	m_ColorModulationAllowed = true;

	// Alle Frame durchgehen und alle Features deaktivieren, die auch nur von einem Frame nicht unterstützt werden.
	Common::Array<Frame>::const_iterator Iter = m_Frames.begin();
	for (; Iter != m_Frames.end(); ++Iter) {
		BS_BitmapResource *pBitmap;
		if (!(pBitmap = static_cast<BS_BitmapResource *>(BS_Kernel::GetInstance()->GetResourceManager()->RequestResource((*Iter).FileName)))) {
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
