//-----------------------------------------------------------------------------
// File: Joystick.cpp
//
// Desc: Demonstrates an application which receives immediate 
//       joystick data in exclusive mode via a dialog timer.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "StdAfx.h"

#include "joystick.h"
#include <dinput.h>

#include "Serialization\XPrmArchive.h"
#include "Serialization\MultiArchive.h"
#include "Serialization\RangedWrapper.h"
#include "Serialization\EnumDescriptor.h"

WRAP_LIBRARY(JoystickSetup, "JoystickSetup", "JoystickSetup", "Scripts\\Content\\JoystickSetup", 0, 0);

//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
BOOL CALLBACK    EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

LPDIRECTINPUT8       g_pDI              = NULL;         
LPDIRECTINPUTDEVICE8 g_pJoystick        = NULL;     

JoystickState joystickState;

//-----------------------------------------------------------------------------
// Name: InitDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
bool InitDirectInput( HWND hDlg )
{
	if(!JoystickSetup::instance().isEnabled())
		return false;

    HRESULT hr;

    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&g_pDI, NULL ) ) )
        return false;

    // Look for a simple joystick we can use for this sample program.
    if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, 
                                         EnumJoysticksCallback,
                                         NULL, DIEDFL_ATTACHEDONLY ) ) )
        return false;

    // Make sure we got a joystick
    if( NULL == g_pJoystick )
    {
        return false;
    }

    // Set the data format to "simple joystick" - a predefined data format 
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
    if( FAILED( hr = g_pJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
        return false;

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    if( FAILED( hr = g_pJoystick->SetCooperativeLevel( hDlg, DISCL_EXCLUSIVE | DISCL_FOREGROUND ) ) )
        return false;

    // Enumerate the joystick objects. The callback function enabled user
    // interface elements for objects that are found, and sets the min/max
    // values property for discovered axes.
    if( FAILED( hr = g_pJoystick->EnumObjects( EnumObjectsCallback, 
                                                (VOID*)hDlg, DIDFT_ALL ) ) )
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallback()
// Desc: Called once for each enumerated joystick. If we find one, create a
//       device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
    UNREFERENCED_PARAMETER( pContext );
    HRESULT hr;

    // Obtain an interface to the enumerated joystick.
    hr = g_pDI->CreateDevice( pdidInstance->guidInstance, &g_pJoystick, NULL );

    // If it failed, then we can't use this joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if( FAILED(hr) ) 
        return DIENUM_CONTINUE;

    // Stop enumeration. Note: we're just taking the first joystick we get. You
    // could store all the enumerated joysticks and let the user pick.
    return DIENUM_STOP;
}




//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                   VOID* pContext )
{
    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg; 
        diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        diprg.diph.dwHow        = DIPH_BYID; 
        diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin              = -1000; 
        diprg.lMax              = +1000; 
    
        // Set the range for the axis
        if( FAILED( g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
            return DIENUM_STOP;
         
    }
    return DIENUM_CONTINUE;
}




//-----------------------------------------------------------------------------
// Name: UpdateInputState()
// Desc: Get the input device's state and display it.
//-----------------------------------------------------------------------------
bool UpdateDirectInputState()
{
	if(!JoystickSetup::instance().isEnabled())
		return false;

    HRESULT     hr;
    DIJOYSTATE2 js;           // DInput joystick state 

    if( NULL == g_pJoystick ) 
        return true;

    // Poll the device to read the current state
    hr = g_pJoystick->Poll(); 
    if( FAILED(hr) )  
    {
        // DInput is telling us that the input stream has been
        // interrupted. We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done. We
        // just re-acquire and try again.
        hr = g_pJoystick->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = g_pJoystick->Acquire();

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return true; 
    }

    // Get the input's device state
    if( FAILED( hr = g_pJoystick->GetDeviceState( sizeof(DIJOYSTATE2), &js ) ) )
        return false; // The device should have been acquired during the Poll()

	joystickState.setControlState(JOY_AXIS_X, js.lX);
	joystickState.setControlState(JOY_AXIS_Y, js.lY);
	joystickState.setControlState(JOY_AXIS_Z, js.lZ);
	joystickState.setControlState(JOY_AXIS_X_ROT, js.lRx);
	joystickState.setControlState(JOY_AXIS_Y_ROT, js.lRy);
	joystickState.setControlState(JOY_AXIS_Z_ROT, js.lRz);
	joystickState.setControlState(JOY_BUTTON_POV1, js.rgdwPOV[0]);

    for(int i = 0; i < JOYSTICK_BUTTONS_MAX; i++)
		joystickState.setControlState(JoystickControlID(JOY_BUTTON_01 + i), (js.rgbButtons[i] & 0x80) ? 1 : 0);

    return true;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
void FreeDirectInput()
{
	if(!JoystickSetup::instance().isEnabled())
		return;

    // Unacquire the device one last time just in case 
    // the app tried to exit while the device is still acquired.
    if( g_pJoystick ) 
        g_pJoystick->Unacquire();
    
    // Release any DirectInput objects.
    SAFE_RELEASE( g_pJoystick );
    SAFE_RELEASE( g_pDI );
}

void JoystickControlSetup::serialize(Archive& ar)
{
	ar.serialize(controlID_, "controlID", "контрол");
	if(JoystickSetup::isAxisControl(controlID_))
		ar.serialize(pressMode_, "pressMode", "режим обработки");
}

JoystickState::JoystickState()
{
	for(int i = 0; i < JOY_CONTROL_ID_MAX; i++)
		controlState_[i] = 0;
}

bool JoystickState::isControlPressed(JoystickControlID control_id) const
{
	if(JoystickSetup::isAxisControl(control_id))
		return abs(controlState_[control_id]) > JoystickSetup::instance().axisDeltaMin();
	else
		return controlState_[control_id] != 0;
}

bool JoystickState::isControlPressed(const JoystickControlSetup& control_setup) const
{
	if(JoystickSetup::isAxisControl(control_setup.controlID())){
		switch(control_setup.pressMode()){
		case JOY_AXIS_NORMAL:
			return abs(controlState_[control_setup.controlID()]) > JoystickSetup::instance().axisDeltaMin();
		case JOY_AXIS_NEGATIVE:
			return controlState_[control_setup.controlID()] < -JoystickSetup::instance().axisDeltaMin();
		case JOY_AXIS_POSITIVE:
			return controlState_[control_setup.controlID()] > JoystickSetup::instance().axisDeltaMin();
		}
	}

	return controlState_[control_setup.controlID()] != 0;
}

JoystickSetup::JoystickSetup()
{
	enable_ = true;

	controls_[JOY_MOVEMENT_X] = JoystickControlSetup(JOY_AXIS_X);
	controls_[JOY_MOVEMENT_Y] = JoystickControlSetup(JOY_AXIS_Y);

	controls_[JOY_CAMERA_X] = JoystickControlSetup(JOY_AXIS_Z);
	controls_[JOY_CAMERA_Y] = JoystickControlSetup(JOY_AXIS_Z_ROT);

	controls_[JOY_PRIMARY_WEAPON] = JoystickControlSetup(JOY_BUTTON_07);
	controls_[JOY_SECONDARY_WEAPON] = JoystickControlSetup(JOY_BUTTON_08);
	controls_[JOY_PREV_WEAPON] = JoystickControlSetup(JOY_BUTTON_05);
	controls_[JOY_NEXT_WEAPON] = JoystickControlSetup(JOY_BUTTON_06);
/*
	controls_[JOY_MOVEMENT_X] = JoystickControlSetup(JOY_AXIS_X);
	controls_[JOY_MOVEMENT_Y] = JoystickControlSetup(JOY_AXIS_Y);

	controls_[JOY_CAMERA_X] = JoystickControlSetup(JOY_AXIS_X_ROT);
	controls_[JOY_CAMERA_Y] = JoystickControlSetup(JOY_AXIS_Y_ROT);

	controls_[JOY_PRIMARY_WEAPON] = JoystickControlSetup(JOY_AXIS_Z, JOY_AXIS_NEGATIVE);
	controls_[JOY_SECONDARY_WEAPON] = JoystickControlSetup(JOY_AXIS_Z, JOY_AXIS_POSITIVE);
	controls_[JOY_PREV_WEAPON] = JoystickControlSetup(JOY_BUTTON_05);
	controls_[JOY_NEXT_WEAPON] = JoystickControlSetup(JOY_BUTTON_06);
*/
	axisDeltaMin_ = 350;
	cameraTurnSpeed_ = .005f;
}

void JoystickSetup::serialize(Archive& ar)
{
	ar.serialize(enable_, "enable", "Разрешить управление джойстиком");

	if(ar.openBlock("Controls", "Настройки управления")){
		for(int i = 0; i < JOY_GAME_CONTROL_MAX; i++)
			ar.serialize(controls_[i], getEnumName(JoystickGameControlID(i)), getEnumNameAlt(JoystickGameControlID(i)));
		ar.closeBlock();
	}

	ar.serialize(RangedWrapperi(axisDeltaMin_, 0, 1000), "axisDeltaMin", "минимальное фиксируемое отклонение оси");
	ar.serialize(cameraTurnSpeed_, "cameraTurnSpeed", "коэффициент скорости поворота камеры");
}

BEGIN_ENUM_DESCRIPTOR(JoystickControlID, "JoystickControlID")
REGISTER_ENUM(JOY_AXIS_X, "Ось X");
REGISTER_ENUM(JOY_AXIS_Y, "Ось Y");
REGISTER_ENUM(JOY_AXIS_Z, "Ось Z");
REGISTER_ENUM(JOY_AXIS_X_ROT, "Вращение вокруг оси X");
REGISTER_ENUM(JOY_AXIS_Y_ROT, "Вращение вокруг оси Y");
REGISTER_ENUM(JOY_AXIS_Z_ROT, "Вращение вокруг оси Z");
REGISTER_ENUM(JOY_BUTTON_01, "Кнопка 1");
REGISTER_ENUM(JOY_BUTTON_02, "Кнопка 2");
REGISTER_ENUM(JOY_BUTTON_03, "Кнопка 3");
REGISTER_ENUM(JOY_BUTTON_04, "Кнопка 4");
REGISTER_ENUM(JOY_BUTTON_05, "Кнопка 5");
REGISTER_ENUM(JOY_BUTTON_06, "Кнопка 6");
REGISTER_ENUM(JOY_BUTTON_07, "Кнопка 7");
REGISTER_ENUM(JOY_BUTTON_08, "Кнопка 8");
REGISTER_ENUM(JOY_BUTTON_09, "Кнопка 9");
REGISTER_ENUM(JOY_BUTTON_10, "Кнопка 10");
REGISTER_ENUM(JOY_BUTTON_11, "Кнопка 11");
REGISTER_ENUM(JOY_BUTTON_12, "Кнопка 12");
REGISTER_ENUM(JOY_BUTTON_13, "Кнопка 13");
REGISTER_ENUM(JOY_BUTTON_14, "Кнопка 14");
REGISTER_ENUM(JOY_BUTTON_15, "Кнопка 15");
REGISTER_ENUM(JOY_BUTTON_16, "Кнопка 16");
REGISTER_ENUM(JOY_BUTTON_POV1, "POV");
END_ENUM_DESCRIPTOR(JoystickControlID)

BEGIN_ENUM_DESCRIPTOR(JoystickGameControlID, "JoystickGameControls")
REGISTER_ENUM(JOY_MOVEMENT_X, "движение вправо/влево")
REGISTER_ENUM(JOY_MOVEMENT_Y, "движение вперёд/назад")
REGISTER_ENUM(JOY_CAMERA_X, "поворот камеры по горизонтали")
REGISTER_ENUM(JOY_CAMERA_Y, "поворот камеры по вертикали")
REGISTER_ENUM(JOY_PRIMARY_WEAPON, "выстрел основным оружием")
REGISTER_ENUM(JOY_SECONDARY_WEAPON, "выстрел второстепенным оружием")
REGISTER_ENUM(JOY_PREV_WEAPON, "предыдущее оружие")
REGISTER_ENUM(JOY_NEXT_WEAPON, "следующее оружие")
END_ENUM_DESCRIPTOR(JoystickGameControlID)

BEGIN_ENUM_DESCRIPTOR(JoystickAxisMode, "JoystickAxisMode")
REGISTER_ENUM(JOY_AXIS_NORMAL, "отклонение в любую сторону")
REGISTER_ENUM(JOY_AXIS_NEGATIVE, "отклонение в минус")
REGISTER_ENUM(JOY_AXIS_POSITIVE, "отклонение в плюс")
END_ENUM_DESCRIPTOR(JoystickAxisMode)
