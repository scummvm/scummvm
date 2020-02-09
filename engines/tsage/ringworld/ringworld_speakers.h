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

#ifndef TSAGE_RINGWORLD_SPEAKERS_H
#define TSAGE_RINGWORLD_SPEAKERS_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/ringworld/ringworld_logic.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class SpeakerGText : public Speaker {
public:
	SceneObject _sceneObject;
public:
	SpeakerGText();

	Common::String getClassName() override { return "SpeakerGText"; }
	void setText(const Common::String &msg) override;
	void removeText() override;
};

class SpeakerPOR : public AnimatedSpeaker {
	class SpeakerAction1 : public SpeakerAction {
	public:
		void signal() override;
	};

public:
	SceneObject _object3;
	SpeakerAction1 _action2;
public:
	SpeakerPOR();
	Common::String getClassName() override { return "SpeakerPOR"; }
	void setText(const Common::String &msg) override;
};

class SpeakerOR : public AnimatedSpeaker {
public:
	SpeakerOR();
	Common::String getClassName() override { return "SpeakerOR"; }
	void setText(const Common::String &msg) override;
};

class SpeakerOText : public SpeakerGText {
public:
	SpeakerOText();

	Common::String getClassName() override { return "SpeakerOText"; }
};

class SpeakerPOText : public ScreenSpeaker {
public:
	SpeakerPOText();

	Common::String getClassName() override { return "SpeakerPOText"; }
};

class SpeakerSText : public ScreenSpeaker {
public:
	SpeakerSText();

	Common::String getClassName() override { return "SpeakerSText"; }
};

class SpeakerQText : public ScreenSpeaker {
public:
	SpeakerQText();

	Common::String getClassName() override { return "SpeakerQText"; }
};

class SpeakerMText : public ScreenSpeaker {
public:
	SpeakerMText();

	Common::String getClassName() override { return "SpeakerMText"; }
};

class SpeakerCText : public ScreenSpeaker {
public:
	SpeakerCText();

	Common::String getClassName() override { return "SpeakerCText"; }
};

class SpeakerEText : public ScreenSpeaker {
public:
	SpeakerEText();

	Common::String getClassName() override { return "SpeakerEText"; }
};

class SpeakerGR : public AnimatedSpeaker {
public:
	SpeakerGR();

	Common::String getClassName() override { return "SpeakerGR"; }
	void setText(const Common::String &msg) override;
};

class SpeakerHText : public ScreenSpeaker {
public:
	SpeakerHText();

	Common::String getClassName() override { return "SpeakerHText"; }
};

class SpeakerPText : public ScreenSpeaker {
public:
	SpeakerPText();

	Common::String getClassName() override { return "SpeakerPText"; }
};

class SpeakerCHFText : public ScreenSpeaker {
public:
	SpeakerCHFText();

	Common::String getClassName() override { return "SpeakerCHFText"; }
};

class SpeakerSKText : public ScreenSpeaker {
public:
	SpeakerSKText();

	Common::String getClassName() override { return "SpeakerSKText"; }
};

class SpeakerCDRText : public ScreenSpeaker {
public:
	SpeakerCDRText();

	Common::String getClassName() override { return "SpeakerCDRText"; }
};

class SpeakerFLText : public ScreenSpeaker {
public:
	SpeakerFLText();

	Common::String getClassName() override { return "SpeakerFLText"; }
};

class SpeakerBatText : public ScreenSpeaker {
public:
	SpeakerBatText();

	Common::String getClassName() override { return "SpeakerFLText"; }
};

class SpeakerQR : public AnimatedSpeaker {
public:
	SpeakerQR();

	Common::String getClassName() override { return "SpeakerQR"; }
	void setText(const Common::String &msg) override;
};

class SpeakerQU : public AnimatedSpeaker {
public:
	SpeakerQU();

	Common::String getClassName() override { return "SpeakerQU"; }
	void setText(const Common::String &msg) override;
};

class SpeakerSKL : public AnimatedSpeaker {
public:
	SpeakerSKL();

	Common::String getClassName() override { return "SpeakerQL"; }
	void setText(const Common::String &msg) override;
};

class SpeakerQL : public AnimatedSpeaker {
public:
	SpeakerQL();

	Common::String getClassName() override { return "SpeakerQL"; }
	void setText(const Common::String &msg) override;
};

class SpeakerSR : public AnimatedSpeaker {
public:
	SceneObject _object3;
public:
	SpeakerSR();

	Common::String getClassName() override { return "SpeakerSR"; }
	void setText(const Common::String &msg) override;
};

class SpeakerSL : public AnimatedSpeaker {
public:
	SpeakerSL();

	Common::String getClassName() override { return "SpeakerSL"; }
	void setText(const Common::String &msg) override;
};

class SpeakerCR : public AnimatedSpeaker {
public:
	SpeakerCR();

	Common::String getClassName() override { return "SpeakerCR"; }
	void setText(const Common::String &msg) override;
};

class SpeakerMR : public AnimatedSpeaker {
public:
	SpeakerMR();

	Common::String getClassName() override { return "SpeakerMR"; }
	void setText(const Common::String &msg) override;
};

class SpeakerSAL : public AnimatedSpeaker {
public:
	SpeakerSAL();

	Common::String getClassName() override { return "SpeakerSAL"; }
	void setText(const Common::String &msg) override;
};

class SpeakerML : public AnimatedSpeaker {
public:
	SpeakerML();

	Common::String getClassName() override { return "SpeakerML"; }
	void setText(const Common::String &msg) override;
};

class SpeakerCHFL : public AnimatedSpeaker {
public:
	SpeakerCHFL();

	Common::String getClassName() override { return "SpeakerCHFL"; }
	void setText(const Common::String &msg) override;
};

class SpeakerCHFR : public AnimatedSpeaker {
public:
	SpeakerCHFR();

	Common::String getClassName() override { return "SpeakerCHFR"; }
	void setText(const Common::String &msg) override;
};

class SpeakerPL : public AnimatedSpeaker {
public:
	SceneObject _object3;
	SpeakerAction _speakerAction2;

	SpeakerPL();

	Common::String getClassName() override { return "SpeakerPL"; }
	void setText(const Common::String &msg) override;
	void removeText() override;
};

class SpeakerPR : public AnimatedSpeaker {
public:
	SceneObject _object3;
	SpeakerAction _speakerAction2;

	SpeakerPR();

	Common::String getClassName() override { return "SpeakerPR"; }
	void setText(const Common::String &msg) override;
	void removeText() override;
};

class SpeakerCDR : public AnimatedSpeaker {
public:
	SpeakerCDR();

	Common::String getClassName() override { return "SpeakerCDR"; }
	void setText(const Common::String &msg) override;
};

class SpeakerCDL : public AnimatedSpeaker {
public:
	SpeakerCDL();

	Common::String getClassName() override { return "SpeakerCDL"; }
	void setText(const Common::String &msg) override;
};

class SpeakerFLL : public AnimatedSpeaker {
public:
	SpeakerFLL();

	Common::String getClassName() override { return "SpeakerFLL"; }
	void setText(const Common::String &msg) override;
};

class SpeakerBatR : public AnimatedSpeaker {
public:
	SpeakerBatR();

	Common::String getClassName() override { return "SpeakerBatR"; }
	void setText(const Common::String &msg) override;
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
