/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TETRAEDGE_TE_TE_BUTTON_LAYOUT_H
#define TETRAEDGE_TE_TE_BUTTON_LAYOUT_H

#include "tetraedge/te/te_callback.h"
#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_signal.h"
#include "tetraedge/te/te_timer.h"

namespace Common {
struct Point;
}

namespace Tetraedge {

class TeButtonLayout : public TeLayout {
public:
	TeButtonLayout();

	virtual ~TeButtonLayout();

	enum State {
		BUTTON_STATE_UP = 0,
		BUTTON_STATE_DOWN = 1,
		BUTTON_STATE_DISABLED = 2,
		BUTTON_STATE_ROLLOVER = 3
	};

	long doubleValidationProtectionTimeoutTime() { return 500; }
	long frozenValidationTimeoutTime() { return 500; }

	virtual bool isMouseIn(const TeVector2s32 &mouseloc) override;
	bool onMouseLeftDown(const Common::Point &pt);
	bool onMouseLeftDownMaxPriority(const Common::Point &pt) { return false; }
	bool onMouseLeftUp(const Common::Point &pt);
	bool onMouseLeftUpMaxPriority(const Common::Point &pt) { return false; }
	bool onMousePositionChanged(const Common::Point &pt);
	bool onMousePositionChangedMaxPriority(const Common::Point &pt) {
		_mousePositionChangedCatched = false;
		return false;
	}

	void reset();

	void resetTimeFromLastValidation();
	long timeFromLastValidation();

	void setDisabledLayout(TeLayout *disabledLayout);
	void setHitZone(TeLayout *hitZoneLayout);
	void setDownLayout(TeLayout *downLayout);
	void setRollOverLayout(TeLayout *rollOverLayout);
	void setUpLayout(TeLayout *upLayout);

	void setDoubleValidationProtectionEnabled(bool enable);
	void setEnable(bool enable);
	virtual void setPosition(const TeVector3f32 &pos) override;

	void setClickPassThrough(bool val) {
		_clickPassThrough = val;
	}
	void setValidationSound(const Common::String &val) {
		_validationSound = val;
	}
	void setValidationSoundVolume(float val) {
		_validationSoundVolume = val;
	}

	void setState(State newState);

	TeSignal0Param &onMouseClickValidated() { return _onMouseClickValidatedSignal; };

	bool _someClickFlag;
	TeLayout *_upLayout;
	TeLayout *_downLayout;

private:
	static bool _mousePositionChangedCatched;
	bool _doubleValidationProtectionEnabled;
	static TeTimer *getDoubleValidationProtectionTimer();
	static TeTimer *_doubleValidationProtectionTimer;

	State _currentState;
	bool _clickPassThrough;
	Common::String _validationSound;
	float _validationSoundVolume;

	Common::Array<unsigned int> _intArr;

	TeICallback1ParamPtr<const Common::Point &> _onMousePositionChangedMaxPriorityCallback;
	TeICallback1ParamPtr<const Common::Point &> _onMousePositionChangedCallback;
	TeICallback1ParamPtr<const Common::Point &> _onMouseLeftDownCallback;
	TeICallback1ParamPtr<const Common::Point &> _onMouseLeftUpMaxPriorityCallback;
	TeICallback1ParamPtr<const Common::Point &> _onMouseLeftUpCallback;

	TeLayout *_rolloverLayout;
	TeLayout *_disabledLayout;
	TeLayout *_hitZoneLayout;

	TeSignal0Param _onMouseClickValidatedSignal;
	TeSignal0Param _onButtonChangedToStateUpSignal;
	TeSignal0Param _onButtonChangedToStateDownSignal;
	TeSignal0Param _onButtonChangedToStateRolloverSignal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_BUTTON_LAYOUT_H
