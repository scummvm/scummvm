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

#ifndef TETRAEDGE_TE_TE_CHECKBOX_LAYOUT_H
#define TETRAEDGE_TE_TE_CHECKBOX_LAYOUT_H

#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_vector2s32.h"

namespace Tetraedge {

class TeCheckboxLayout : public TeLayout {
public:
	TeCheckboxLayout();
	virtual ~TeCheckboxLayout();

	enum State {
		CheckboxStateActive,
		CheckboxStateUnactive,
		CheckboxStateActiveDisabled,
		CheckboxStateUnactiveDisabled,
		CheckboxStateActiveRollover,
		CheckboxStateUnactiveRollover,
		CheckboxState6
	};

	void setActiveLayout(TeLayout *layout);
	void setUnactiveLayout(TeLayout *layout);
	void setActiveDisabledLayout(TeLayout *layout);
	void setUnactiveDisabledLayout(TeLayout *layout);
	void setActiveRollOverLayout(TeLayout *layout);
	void setUnactiveRollOverLayout(TeLayout *layout);
	void setHitZone(TeLayout *layout);
	void setClickPassThrough(bool val);
	void setActivationSound(const Common::String &sound);
	void setUnactivationSound(const Common::String &sound);
	bool isMouseIn(const TeVector2s32 &pt) override;
	void setState(State state);

	TeSignal1Param<State> &onStateChangedSignal() { return _onStateChangedSignal; }

	bool onMouseLeftUp(const Common::Point &pt);
	bool onMouseLeftUpMaxPriority(const Common::Point &pt);
	bool onMouseLeftDown(const Common::Point &pt);
	bool onMousePositionChanged(const Common::Point &pt);

private:
	TeLayout *_activeLayout;
	TeLayout *_unactiveLayout;
	TeLayout *_activeDisabledLayout;
	TeLayout *_unactiveDisabledLayout;
	TeLayout *_activeRollOverLayout;
	TeLayout *_unactiveRollOverLayout;
	TeLayout *_hitZone;

	bool _clickPassThrough;
	Common::String _activationSound;
	Common::String _unactivationSound;
	State _state;
	TeSignal1Param<State> _onStateChangedSignal;

	TeICallback1ParamPtr<const Common::Point &> _onMouseLeftUpCallback;
	TeICallback1ParamPtr<const Common::Point &> _onMouseLeftUpMaxPriorityCallback;
	TeICallback1ParamPtr<const Common::Point &> _onMouseLeftDownCallback;
	TeICallback1ParamPtr<const Common::Point &> _onMousePositionChangedCallback;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_CHECKBOX_LAYOUT_H
