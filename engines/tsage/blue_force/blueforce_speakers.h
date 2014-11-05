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

#ifndef TSAGE_BLUEFORCE_SPEAKERS_H
#define TSAGE_BLUEFORCE_SPEAKERS_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/blue_force/blueforce_logic.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class VisualSpeaker: public Speaker {
public:
	NamedObject _object1;
	CountdownObject _object2;
	bool _removeObject1, _removeObject2;
	int _xp;
	int _numFrames;
	Common::Point _offsetPos;
public:
	VisualSpeaker();

	virtual Common::String getClassName() override { return "VisualSpeaker"; }
	virtual void synchronize(Serializer &s) override;
	virtual void remove() override;
	virtual void startSpeaking(Action *action) override;
	virtual void setText(const Common::String &msg) override;
};

class SpeakerGameText: public VisualSpeaker {
public:
	SpeakerGameText();

	virtual Common::String getClassName() override { return "SpeakerGameText"; }
};

class SpeakerSutter: public VisualSpeaker {
public:
	SpeakerSutter();

	virtual Common::String getClassName() override { return "SpeakerSutter"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerDoug: public VisualSpeaker {
public:
	SpeakerDoug();

	virtual Common::String getClassName() override { return "SpeakerDoug"; }
};

class SpeakerJakeNoHead: public VisualSpeaker {
public:
	SpeakerJakeNoHead();

	virtual Common::String getClassName() override { return "SpeakerJakeNoHead"; }
};

class SpeakerHarrison: public VisualSpeaker {
public:
	SpeakerHarrison();

	virtual Common::String getClassName() override { return "SpeakerHarrison"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerJake: public VisualSpeaker {
public:
	SpeakerJake();

	virtual Common::String getClassName() override { return "SpeakerJake"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerJakeJacket: public VisualSpeaker {
public:
	SpeakerJakeJacket();

	virtual Common::String getClassName() override { return "SpeakerJakeJacket"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerJakeUniform: public VisualSpeaker {
public:
	SpeakerJakeUniform();

	virtual Common::String getClassName() override { return "SpeakerJakeUniform"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerJailer: public VisualSpeaker {
public:
	SpeakerJailer();

	virtual Common::String getClassName() override { return "SpeakerJailer"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerGreen: public VisualSpeaker {
public:
	SpeakerGreen();

	virtual Common::String getClassName() override { return "SpeakerGreen"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerPSutter: public VisualSpeaker {
public:
	SpeakerPSutter();

	virtual Common::String getClassName() override { return "SpeakerPSutter"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerJakeRadio: public VisualSpeaker {
public:
	SpeakerJakeRadio();

	virtual Common::String getClassName() override { return "SpeakerJakeRadio"; }
};

class SpeakerLyleHat: public VisualSpeaker {
public:
	SpeakerLyleHat();

	virtual Common::String getClassName() override { return "SpeakerLyleHat"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerJordan: public VisualSpeaker {
public:
	SpeakerJordan();

	virtual Common::String getClassName() override { return "SpeakerJordan"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerSkip: public VisualSpeaker {
public:
	SpeakerSkip();

	virtual Common::String getClassName() override { return "SpeakerSkip"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerSkipB: public VisualSpeaker {
public:
	SpeakerSkipB();

	virtual Common::String getClassName() override { return "SpeakerSkipB"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerCarter: public VisualSpeaker {
public:
	SpeakerCarter();

	virtual Common::String getClassName() override { return "SpeakerCarter"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerDriver: public VisualSpeaker {
public:
	SpeakerDriver();

	virtual Common::String getClassName() override { return "SpeakerDriver"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerShooter: public VisualSpeaker {
public:
	SpeakerShooter();

	virtual Common::String getClassName() override { return "SpeakerShooter"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerJim: public VisualSpeaker {
public:
	SpeakerJim();

	virtual Common::String getClassName() override { return "SpeakerJim"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerDezi: public VisualSpeaker {
public:
	SpeakerDezi();

	virtual Common::String getClassName() override { return "SpeakerDezi"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerJake385: public VisualSpeaker {
public:
	SpeakerJake385();

	virtual Common::String getClassName() override { return "SpeakerJake385"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerLaura: public VisualSpeaker {
public:
	SpeakerLaura();

	virtual Common::String getClassName() override { return "SpeakerLaura"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerLauraHeld: public VisualSpeaker {
public:
	SpeakerLauraHeld();

	virtual Common::String getClassName() override { return "SpeakerLaura"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerEugene: public VisualSpeaker {
public:
	SpeakerEugene();

	virtual Common::String getClassName() override { return "SpeakerEugene"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerWeasel: public VisualSpeaker {
public:
	SpeakerWeasel();

	virtual Common::String getClassName() override { return "SpeakerWeasel"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerBilly: public VisualSpeaker {
public:
	SpeakerBilly();

	virtual Common::String getClassName() override { return "SpeakerBilly"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerDrunk: public VisualSpeaker {
public:
	SpeakerDrunk();

	virtual Common::String getClassName() override { return "SpeakerDrunk"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerGiggles: public VisualSpeaker {
public:
	SpeakerGiggles();

	virtual Common::String getClassName() override { return "SpeakerGiggles"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerFBI: public VisualSpeaker {
public:
	SpeakerFBI();

	virtual Common::String getClassName() override { return "FBI"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerNico: public VisualSpeaker {
public:
	SpeakerNico();

	virtual Common::String getClassName() override { return "NICO"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerDA: public VisualSpeaker {
public:
	SpeakerDA();

	virtual Common::String getClassName() override { return "DA"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerGrandma: public VisualSpeaker {
public:
	SpeakerGrandma();

	virtual Common::String getClassName() override { return "SpeakerGrandma"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerLyle: public VisualSpeaker {
public:
	SpeakerLyle();

	virtual Common::String getClassName() override { return "SpeakerLyle"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerGranText: public VisualSpeaker {
public:
	SpeakerGranText();

	virtual Common::String getClassName() override { return "SpeakerGranText"; }
};

class SpeakerLyleText: public VisualSpeaker {
public:
	SpeakerLyleText();

	virtual Common::String getClassName() override { return "SpeakerLyleText"; }
};

class SpeakerKate: public VisualSpeaker {
public:
	SpeakerKate();

	virtual Common::String getClassName() override { return "SpeakerKate"; }
	virtual void setText(const Common::String &msg) override;
};

class SpeakerTony: public VisualSpeaker {
public:
	SpeakerTony();

	virtual Common::String getClassName() override { return "SpeakerTony"; }
	virtual void setText(const Common::String &msg) override;
};
} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
