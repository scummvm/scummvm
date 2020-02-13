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

	Common::String getClassName() override { return "VisualSpeaker"; }
	void synchronize(Serializer &s) override;
	void remove() override;
	void startSpeaking(Action *action) override;
	void setText(const Common::String &msg) override;
};

class SpeakerGameText: public VisualSpeaker {
public:
	SpeakerGameText();

	Common::String getClassName() override { return "SpeakerGameText"; }
};

class SpeakerSutter: public VisualSpeaker {
public:
	SpeakerSutter();

	Common::String getClassName() override { return "SpeakerSutter"; }
	void setText(const Common::String &msg) override;
};

class SpeakerDoug: public VisualSpeaker {
public:
	SpeakerDoug();

	Common::String getClassName() override { return "SpeakerDoug"; }
};

class SpeakerJakeNoHead: public VisualSpeaker {
public:
	SpeakerJakeNoHead();

	Common::String getClassName() override { return "SpeakerJakeNoHead"; }
};

class SpeakerHarrison: public VisualSpeaker {
public:
	SpeakerHarrison();

	Common::String getClassName() override { return "SpeakerHarrison"; }
	void setText(const Common::String &msg) override;
};

class SpeakerJake: public VisualSpeaker {
public:
	SpeakerJake();

	Common::String getClassName() override { return "SpeakerJake"; }
	void setText(const Common::String &msg) override;
};

class SpeakerJakeJacket: public VisualSpeaker {
public:
	SpeakerJakeJacket();

	Common::String getClassName() override { return "SpeakerJakeJacket"; }
	void setText(const Common::String &msg) override;
};

class SpeakerJakeUniform: public VisualSpeaker {
public:
	SpeakerJakeUniform();

	Common::String getClassName() override { return "SpeakerJakeUniform"; }
	void setText(const Common::String &msg) override;
};

class SpeakerJailer: public VisualSpeaker {
public:
	SpeakerJailer();

	Common::String getClassName() override { return "SpeakerJailer"; }
	void setText(const Common::String &msg) override;
};

class SpeakerGreen: public VisualSpeaker {
public:
	SpeakerGreen();

	Common::String getClassName() override { return "SpeakerGreen"; }
	void setText(const Common::String &msg) override;
};

class SpeakerPSutter: public VisualSpeaker {
public:
	SpeakerPSutter();

	Common::String getClassName() override { return "SpeakerPSutter"; }
	void setText(const Common::String &msg) override;
};

class SpeakerJakeRadio: public VisualSpeaker {
public:
	SpeakerJakeRadio();

	Common::String getClassName() override { return "SpeakerJakeRadio"; }
};

class SpeakerLyleHat: public VisualSpeaker {
public:
	SpeakerLyleHat();

	Common::String getClassName() override { return "SpeakerLyleHat"; }
	void setText(const Common::String &msg) override;
};

class SpeakerJordan: public VisualSpeaker {
public:
	SpeakerJordan();

	Common::String getClassName() override { return "SpeakerJordan"; }
	void setText(const Common::String &msg) override;
};

class SpeakerSkip: public VisualSpeaker {
public:
	SpeakerSkip();

	Common::String getClassName() override { return "SpeakerSkip"; }
	void setText(const Common::String &msg) override;
};

class SpeakerSkipB: public VisualSpeaker {
public:
	SpeakerSkipB();

	Common::String getClassName() override { return "SpeakerSkipB"; }
	void setText(const Common::String &msg) override;
};

class SpeakerCarter: public VisualSpeaker {
public:
	SpeakerCarter();

	Common::String getClassName() override { return "SpeakerCarter"; }
	void setText(const Common::String &msg) override;
};

class SpeakerDriver: public VisualSpeaker {
public:
	SpeakerDriver();

	Common::String getClassName() override { return "SpeakerDriver"; }
	void setText(const Common::String &msg) override;
};

class SpeakerShooter: public VisualSpeaker {
public:
	SpeakerShooter();

	Common::String getClassName() override { return "SpeakerShooter"; }
	void setText(const Common::String &msg) override;
};

class SpeakerJim: public VisualSpeaker {
public:
	SpeakerJim();

	Common::String getClassName() override { return "SpeakerJim"; }
	void setText(const Common::String &msg) override;
};

class SpeakerDezi: public VisualSpeaker {
public:
	SpeakerDezi();

	Common::String getClassName() override { return "SpeakerDezi"; }
	void setText(const Common::String &msg) override;
};

class SpeakerJake385: public VisualSpeaker {
public:
	SpeakerJake385();

	Common::String getClassName() override { return "SpeakerJake385"; }
	void setText(const Common::String &msg) override;
};

class SpeakerLaura: public VisualSpeaker {
public:
	SpeakerLaura();

	Common::String getClassName() override { return "SpeakerLaura"; }
	void setText(const Common::String &msg) override;
};

class SpeakerLauraHeld: public VisualSpeaker {
public:
	SpeakerLauraHeld();

	Common::String getClassName() override { return "SpeakerLaura"; }
	void setText(const Common::String &msg) override;
};

class SpeakerEugene: public VisualSpeaker {
public:
	SpeakerEugene();

	Common::String getClassName() override { return "SpeakerEugene"; }
	void setText(const Common::String &msg) override;
};

class SpeakerWeasel: public VisualSpeaker {
public:
	SpeakerWeasel();

	Common::String getClassName() override { return "SpeakerWeasel"; }
	void setText(const Common::String &msg) override;
};

class SpeakerBilly: public VisualSpeaker {
public:
	SpeakerBilly();

	Common::String getClassName() override { return "SpeakerBilly"; }
	void setText(const Common::String &msg) override;
};

class SpeakerDrunk: public VisualSpeaker {
public:
	SpeakerDrunk();

	Common::String getClassName() override { return "SpeakerDrunk"; }
	void setText(const Common::String &msg) override;
};

class SpeakerGiggles: public VisualSpeaker {
public:
	SpeakerGiggles();

	Common::String getClassName() override { return "SpeakerGiggles"; }
	void setText(const Common::String &msg) override;
};

class SpeakerFBI: public VisualSpeaker {
public:
	SpeakerFBI();

	Common::String getClassName() override { return "FBI"; }
	void setText(const Common::String &msg) override;
};

class SpeakerNico: public VisualSpeaker {
public:
	SpeakerNico();

	Common::String getClassName() override { return "NICO"; }
	void setText(const Common::String &msg) override;
};

class SpeakerDA: public VisualSpeaker {
public:
	SpeakerDA();

	Common::String getClassName() override { return "DA"; }
	void setText(const Common::String &msg) override;
};

class SpeakerGrandma: public VisualSpeaker {
public:
	SpeakerGrandma();

	Common::String getClassName() override { return "SpeakerGrandma"; }
	void setText(const Common::String &msg) override;
};

class SpeakerLyle: public VisualSpeaker {
public:
	SpeakerLyle();

	Common::String getClassName() override { return "SpeakerLyle"; }
	void setText(const Common::String &msg) override;
};

class SpeakerGranText: public VisualSpeaker {
public:
	SpeakerGranText();

	Common::String getClassName() override { return "SpeakerGranText"; }
};

class SpeakerLyleText: public VisualSpeaker {
public:
	SpeakerLyleText();

	Common::String getClassName() override { return "SpeakerLyleText"; }
};

class SpeakerKate: public VisualSpeaker {
public:
	SpeakerKate();

	Common::String getClassName() override { return "SpeakerKate"; }
	void setText(const Common::String &msg) override;
};

class SpeakerTony: public VisualSpeaker {
public:
	SpeakerTony();

	Common::String getClassName() override { return "SpeakerTony"; }
	void setText(const Common::String &msg) override;
};
} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
