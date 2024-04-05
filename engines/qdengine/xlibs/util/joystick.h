#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include "Serialization\LibraryWrapper.h"

const int JOYSTICK_BUTTONS_MAX = 16;
const int JOYSTICK_AXIS_MAX = 2;

enum JoystickControlID 
{
	JOY_AXIS_X = 0,
	JOY_AXIS_Y,
	JOY_AXIS_Z,
	JOY_AXIS_X_ROT,
	JOY_AXIS_Y_ROT,
	JOY_AXIS_Z_ROT,
	JOY_BUTTON_01,
	JOY_BUTTON_02,
	JOY_BUTTON_03,
	JOY_BUTTON_04,
	JOY_BUTTON_05,
	JOY_BUTTON_06,
	JOY_BUTTON_07,
	JOY_BUTTON_08,
	JOY_BUTTON_09,
	JOY_BUTTON_10,
	JOY_BUTTON_11,
	JOY_BUTTON_12,
	JOY_BUTTON_13,
	JOY_BUTTON_14,
	JOY_BUTTON_15,
	JOY_BUTTON_16,
	JOY_BUTTON_POV1,

	JOY_CONTROL_ID_MAX
};

enum JoystickGameControlID
{
	JOY_MOVEMENT_X,
	JOY_MOVEMENT_Y,
	JOY_CAMERA_X,
	JOY_CAMERA_Y,

	JOY_PRIMARY_WEAPON,
	JOY_SECONDARY_WEAPON,

	JOY_PREV_WEAPON,
	JOY_NEXT_WEAPON,

	JOY_GAME_CONTROL_MAX
};

enum JoystickAxisMode
{
	JOY_AXIS_NORMAL,
	JOY_AXIS_NEGATIVE,
	JOY_AXIS_POSITIVE
};

class JoystickControlSetup
{
public:
	JoystickControlSetup(JoystickControlID id = JOY_AXIS_X, JoystickAxisMode mode = JOY_AXIS_NORMAL) : controlID_(id), pressMode_(mode) { }

	void serialize(Archive& ar);

	JoystickControlID controlID() const { return controlID_; }
	JoystickAxisMode pressMode() const { return pressMode_; }

private:
	JoystickControlID controlID_;
	JoystickAxisMode pressMode_;
};


class JoystickState
{
public:
	JoystickState();

	bool isControlPressed(JoystickControlID control_id) const;
	bool isControlPressed(const JoystickControlSetup& control_setup) const;

	int controlState(JoystickControlID control_id) const { return controlState_[control_id]; }
	void setControlState(JoystickControlID control_id, int state){ controlState_[control_id] = state; }

private:

	int controlState_[JOY_CONTROL_ID_MAX];
};

class JoystickSetup : public LibraryWrapper<JoystickSetup>
{
public:
	JoystickSetup();

	bool isEnabled() const { return enable_; }

	JoystickControlID controlID(JoystickGameControlID id) const { return controls_[id].controlID(); }
	const JoystickControlSetup& controlSetup(JoystickGameControlID id) const { return controls_[id]; }

	int axisDeltaMin() const { return axisDeltaMin_; }
	float cameraTurnSpeed() const { return cameraTurnSpeed_; }

	void serialize(Archive& ar);

	static bool isAxisControl(JoystickControlID control_id){
		switch(control_id){
		case JOY_AXIS_X:
		case JOY_AXIS_Y:
		case JOY_AXIS_Z:
		case JOY_AXIS_X_ROT:
		case JOY_AXIS_Y_ROT:
		case JOY_AXIS_Z_ROT:
			return true;
		}
		return false;
	}

private:

	bool enable_;

	JoystickControlSetup controls_[JOY_GAME_CONTROL_MAX];

	int axisDeltaMin_;
	float cameraTurnSpeed_;
};

extern JoystickState joystickState;

bool InitDirectInput(HWND hDlg);
void FreeDirectInput();
bool UpdateDirectInputState();

#endif /* __JOYSTICK_H__ */