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

#ifndef SWORD25_ANIMATIONRESOURCE_H
#define SWORD25_ANIMATIONRESOURCE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/xmlparser.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/resource.h"
#include "sword25/gfx/animationdescription.h"
#include "sword25/gfx/animation.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class Kernel;
class PackageManager;

// -----------------------------------------------------------------------------
// Class Definition
// -----------------------------------------------------------------------------

class AnimationResource : public Resource, public AnimationDescription, public Common::XMLParser {
public:
	AnimationResource(const Common::String &filename);
	virtual ~AnimationResource();

	virtual const Frame    &GetFrame(uint Index) const {
		BS_ASSERT(Index < m_Frames.size());
		return m_Frames[Index];
	}
	virtual uint    GetFrameCount() const {
		return m_Frames.size();
	}
	virtual void            Unlock() {
		Release();
	}

	Animation::ANIMATION_TYPES   GetAnimationType() const {
		return m_AnimationType;
	}
	int                             GetFPS() const {
		return m_FPS;
	}
	int                             GetMillisPerFrame() const {
		return m_MillisPerFrame;
	}
	bool                            IsScalingAllowed() const {
		return m_ScalingAllowed;
	}
	bool                            IsAlphaAllowed() const {
		return m_AlphaAllowed;
	}
	bool                            IsColorModulationAllowed() const {
		return m_ColorModulationAllowed;
	}
	bool                            IsValid() const {
		return m_Valid;
	}

private:
	bool                            m_Valid;

	Common::Array<Frame>            m_Frames;

	PackageManager *				_pPackage;


	bool ComputeFeatures();
	bool PrecacheAllFrames() const;

	// Parser
	CUSTOM_XML_PARSER(AnimationResource) {
		XML_KEY(animation)
			XML_PROP(fps, true)
			XML_PROP(type, true)

			XML_KEY(frame)
				XML_PROP(file, true)
				XML_PROP(hotspotx, true)
				XML_PROP(hotspoty, true)
				XML_PROP(fliph, false)
				XML_PROP(flipv, false)
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parseBooleanKey(Common::String s, bool &result);

	// Parser callback methods
	bool parserCallback_animation(ParserNode *node);
	bool parserCallback_frame(ParserNode *node);
};

} // End of namespace Sword25

#endif
