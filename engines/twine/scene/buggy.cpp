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

#include "twine/scene/buggy.h"
#include "twine/audio/sound.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/scene/collision.h"
#include "twine/scene/movements.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
#include "twine/twine.h"

#define MAX_SAMPLE_PITCH 11000
#define MIN_SAMPLE_PITCH2 5000
#define MAX_SAMPLE_PITCH2 8500
#define MAX_SPEED 3800
#define TEMPO_GEAR 1200 // speed change
#define SAMPLE_BUGGY 109

namespace TwinE {

void Buggy::initBuggy(uint8 numobj, uint32 flaginit) {
	S_BUGGY *ptb = &ListBuggy[0];
	ActorStruct *ptrobj = _engine->_scene->getActor(numobj);

	// So that the objects follow their tracks without being interrupted
	// by the buggy (too bad, it will be pushed)
	ptrobj->_flags.bCanBePushed = true;
	ptrobj->_flags.bCanDrown = true;

	if (flaginit == 2               // we force the repositioning of the buggy
		|| (flaginit && !NumBuggy)) // first initialization
									// because the empty buggy cannot be Twinsen
	{
		ptb->Cube = _engine->_scene->_numCube; // Port-Ludo (Desert)

		ptb->X = ptrobj->_posObj.x;
		ptb->Y = ptrobj->_posObj.y;
		ptb->Z = ptrobj->_posObj.z;

		ptb->Beta = ptrobj->_beta;

		_engine->_actor->initBody(BodyType::btNormal, numobj);

		NumBuggy = (uint8)(numobj | BUGGY_PRESENT);
	} else if (NumBuggy) {
		if (_engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX)->_controlMode != ControlMode::kBuggyManual && _engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX)->_controlMode != ControlMode::kBuggy) {
			int32 x, y;

			if (_engine->_scene->_numCube == ptb->Cube) {
				ptrobj->_posObj.x = ptb->X;
				ptrobj->_posObj.y = ptb->Y;
				ptrobj->_posObj.z = ptb->Z;

				ptrobj->_beta = ptb->Beta;

				_engine->_actor->initBody(BodyType::btNormal, numobj);
			} else if (_engine->_scene->loadSceneCubeXY(ptb->Cube, &x, &y)) {
				x -= _engine->_scene->_currentCubeX;
				y -= _engine->_scene->_currentCubeY;

				ptrobj->_posObj.x = ptb->X + x * 32768;
				ptrobj->_posObj.y = ptb->Y;
				ptrobj->_posObj.z = ptb->Z + y * 32768;

				ptrobj->_beta = ptb->Beta;

				ptrobj->_flags.bNoShadow = 1;
				ptrobj->_flags.bIsBackgrounded = 1;
				ptrobj->_flags.bNoElectricShock = 1;
				ptrobj->_flags.bHasZBuffer = 1;

				_engine->_actor->initBody(BodyType::btNormal, numobj);
			} else {
				_engine->_actor->initBody(BodyType::btNone, numobj);
			}

			_engine->_movements->clearRealAngle(ptrobj);
		} else {
			_engine->_actor->initBody(BodyType::btNone, numobj);
		}

		NumBuggy = (uint8)(numobj | BUGGY_PRESENT);
	} else {
		_engine->_actor->initBody(BodyType::btNone, numobj);
	}
}

void Buggy::resetBuggy() {
	S_BUGGY *ptb = &ListBuggy[0];

	NumBuggy = 0;
	ptb->Cube = 0;
}

void Buggy::takeBuggy() {
#if 0
	int32 sample;
	ActorStruct *ptrobj = _engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX);
	S_BUGGY *ptb = &ListBuggy[0];

	ptb->SpeedRot = 1024;
	ptb->LastTimer = TimerRefHR;

	// TODO: ObjectClear(&ptrobj);

	// Shielding in case the Buggy moved (being pushed, for example).
	ptb->X = _engine->_scene->getActor(NUM_BUGGY)->_pos.x;
	ptb->Y = _engine->_scene->getActor(NUM_BUGGY)->_pos.y;
	ptb->Z = _engine->_scene->getActor(NUM_BUGGY)->_pos.z;

	ptrobj->_pos.x = ptb->X;
	ptrobj->_pos.y = ptb->Y;
	ptrobj->_pos.z = ptb->Z;
	ptrobj->_beta = ptb->Beta;
	_engine->_movements->clearRealAngle(ptrobj); // To avoid crushing the beta.

	ptrobj->_workFlags.bMANUAL_INTER_FRAME = true;
	ptrobj->_flags.bHasZBuffer = true;

	_engine->_actor->setBehaviour(HeroBehaviourType::kBUGGY);

	// Switch Buggy Scenario to NoBody.
	_engine->_actor->initBody(BodyType::btNone, NUM_BUGGY);

	if (ptrobj->SampleAlways) {
		_engine->_sound->stopSample(ptrobj->SampleAlways);
		ptrobj->SampleAlways = 0;
	}

	sample = SAMPLE_BUGGY;

	if (_engine->_sound->isSamplePlaying(sample)) {
		_engine->_sound->stopSample(sample);
	}

	ptrobj->SampleVolume = 20;

	ParmSampleVolume = ptrobj->SampleVolume;

	Gear = 0;
	TimerGear = 0;

	ptrobj->SampleAlways = _engine->_sound->playSample(SAMPLE_BUGGY, 4096, 0, 0,
										   ptrobj->_pos.x, ptrobj->_pos.y, ptrobj->_pos.z);
#endif
}

#if 0
static void ObjectClear(T_OBJ *ptb3d) {
	memset(ptb3d, 0, sizeof(T_OBJ));
	ptb3d.OBJ_3D.Body, -1
	ptb3d.OBJ_3D.NextBody, -1
	ptb3d.OBJ_3D.Texture, -1
	ptb3d.OBJ_3D.NextTexture, -1
	ptb3d.OBJ_3D.Anim, -1
}
#endif

void Buggy::leaveBuggy(HeroBehaviourType behaviour) {
	int32 sample;
	ActorStruct *ptrobj = _engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX);
	S_BUGGY *ptb = &ListBuggy[0];

	sample = SAMPLE_BUGGY;

	if (_engine->_sound->isSamplePlaying(sample)) {
		_engine->_sound->stopSample(sample);
		ptrobj->SampleAlways = 0;
	}

	ptb->X = ptrobj->_posObj.x;
	ptb->Y = ptrobj->_posObj.y;
	ptb->Z = ptrobj->_posObj.z;
	ptb->Beta = ptrobj->_beta;
	ptb->Cube = _engine->_scene->_numCube;

	// TODO: ObjectClear(ptrobj);

	ptrobj->_workFlags.bMANUAL_INTER_FRAME = 0;
	ptrobj->_flags.bHasZBuffer = 0;

	_engine->_actor->initBody(BodyType::btTunic, OWN_ACTOR_SCENE_INDEX);

	_engine->_actor->setBehaviour(behaviour);

	// Restore scenario buggy.

	ptrobj = _engine->_scene->getActor(NUM_BUGGY);

	ptrobj->_posObj.x = ptb->X;
	ptrobj->_posObj.y = ptb->Y;
	ptrobj->_posObj.z = ptb->Z;
	ptrobj->_beta = ptb->Beta;

	ptrobj->_brickSound = _engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX)->_brickSound;

	_engine->_movements->clearRealAngle(ptrobj); // To avoid crushing the beta

	_engine->_actor->initBody(BodyType::btNormal, NUM_BUGGY);

	// Search for a free position for Twinsen nearby.
	_engine->_actor->posObjectAroundAnother(NUM_BUGGY, OWN_ACTOR_SCENE_INDEX);
}

void Buggy::doAnimBuggy(ActorStruct *ptrobj) {
#if 0
	int32 x1, y1, z1, yw;
	S_BUGGY *ptb = &ListBuggy[0];
	T_OBJ_3D *ptb3d = &ptrobj->Obj;

	// wheels rot
	int32 c, d;

	int32 x, y, z;

	// Trick to avoid crushing the groups in AffOneObject().
	ObjectSetInterFrame(ptb3d);

	if (ptrobj->_workFlags.bIsFalling || ptrobj->_workFlags.bUnk1000) {
		return;
	}

	LongRotate(0, ptb->SpeedInc * 1024, ptb3d->Beta);
	Nxw = ptb3d->X + X0 / 1024;
	Nzw = ptb3d->Z + Z0 / 1024;

	// Ideal altitude
	yw = CalculAltitudeObjet(Nxw, Nzw, -1); // Ground Y for XZ

	// test altitude #2: Forbidden triangles

	// Front left wheel direction
	ptb3d->CurrentFrame[3].Beta = (int16)ptb->BetaWheel;
	// Front right wheel direction
	ptb3d->CurrentFrame[6].Beta = (int16)ptb->BetaWheel;

	// Management of 4 separate wheels.

	// front right wheel

	LongRotate(-400, 400, ptb3d->Beta);
	x = Nxw + X0;
	z = Nzw + Z0;
	y = yw;

	if (x >= 0 && x < 32768 && z >= 0 && z < 32768) {
		y += CalculAltitudeObjet(x, z, -1);
	}

	c = (260 * 31415) / 1000; // Circumference * 10
	d = Distance3D(ptb->Wheel[0].X, ptb->Wheel[0].Y, ptb->Wheel[0].Z, x, y, z);

	if (ptb->Speed >= 0) {
		ptb->Wheel[0].Angle += (4096 * 10 * d) / c;
	} else {
		ptb->Wheel[0].Angle -= (4096 * 10 * d) / c;
	}

	ptb->Wheel[0].X = x;
	ptb->Wheel[0].Y = y;
	ptb->Wheel[0].Z = z;

	// front left wheel

	LongRotate(400, 400, ptb3d->Beta);
	x = Nxw + X0;
	z = Nzw + Z0;
	y = yw;

	if (x >= 0 && x < 32768 && z >= 0 && z < 32768) {
		y += CalculAltitudeObjet(x, z, -1);
	}

	c = (260 * 31415) / 1000; // Circumference * 10
	d = Distance3D(ptb->Wheel[1].X, ptb->Wheel[1].Y, ptb->Wheel[1].Z, x, y, z);

	if (ptb->Speed >= 0) {
		ptb->Wheel[1].Angle += (4096 * 10 * d) / c;
	} else {
		ptb->Wheel[1].Angle -= (4096 * 10 * d) / c;
	}

	ptb->Wheel[1].X = x;
	ptb->Wheel[1].Y = y;
	ptb->Wheel[1].Z = z;

	// back left wheel

	LongRotate(400, -350, ptb3d->Beta);
	x = Nxw + X0;
	z = Nzw + Z0;
	y = yw;

	if (x >= 0 && x < 32768 && z >= 0 && z < 32768) {
		y += CalculAltitudeObjet(x, z, -1);
	}

	c = (360 * 31415) / 1000; // Circumference * 10
	d = Distance3D(ptb->Wheel[2].X, ptb->Wheel[2].Y, ptb->Wheel[2].Z, x, y, z);

	if (ptb->Speed >= 0) {
		ptb->Wheel[2].Angle += (4096 * 10 * d) / c;
	} else {
		ptb->Wheel[2].Angle -= (4096 * 10 * d) / c;
	}

	ptb->Wheel[2].X = x;
	ptb->Wheel[2].Y = y;
	ptb->Wheel[2].Z = z;

	// back right wheel

	LongRotate(-400, -350, ptb3d->Beta);
	x = Nxw + X0;
	z = Nzw + Z0;
	y = yw;

	if (x >= 0 && x < 32768 && z >= 0 && z < 32768) {
		y += CalculAltitudeObjet(x, z, -1);
	}

	c = (360 * 31415) / 1000; // Circumference * 10
	d = Distance3D(ptb->Wheel[3].X, ptb->Wheel[3].Y, ptb->Wheel[3].Z, x, y, z);

	if (ptb->Speed >= 0) {
		ptb->Wheel[3].Angle += (4096 * 10 * d) / c;
	} else {
		ptb->Wheel[3].Angle -= (4096 * 10 * d) / c;
	}

	ptb->Wheel[3].X = x;
	ptb->Wheel[3].Y = y;
	ptb->Wheel[3].Z = z;

	// front right wheel
	ptb3d->CurrentFrame[4].Alpha = (int16)ptb->Wheel[1].Angle;
	// front left wheel
	ptb3d->CurrentFrame[7].Alpha = (int16)ptb->Wheel[0].Angle;
	// back left wheel
	ptb3d->CurrentFrame[11].Alpha = (int16)ptb->Wheel[2].Angle;
	// back right wheel
	ptb3d->CurrentFrame[9].Alpha = (int16)ptb->Wheel[3].Angle;

	// Car inclination (pitch)
	ptb3d->CurrentFrame[1].Type = 0;

	LongRotate(0, 400, ptb3d->Beta);
	x1 = X0;
	z1 = Z0;
	LongRotate(0, -400, ptb3d->Beta);

	if (Nxw + x1 >= 0 && Nxw + x1 < 32768 && Nzw + z1 >= 0 && Nzw + z1 < 32768) {
		y = CalculAltitudeObjet(Nxw + x1, Nzw + z1, -1);
	} else {
		y = yw;
	}

	if (Nxw + X0 >= 0 && Nxw + X0 < 32768 && Nzw + Z0 >= 0 && Nzw + Z0 < 32768) {
		y1 = CalculAltitudeObjet(Nxw + X0, Nzw + Z0, -1);
	} else {
		y1 = yw;
	}

	ptb3d->CurrentFrame[1].Alpha = (int16)(1024 - GetAngle2D(0, y, 800, y1));
	ptb->Alpha = ptb3d->CurrentFrame[1].Alpha;

	// Car inclination (roll)
	LongRotate(400, 0, ptb3d->Beta);
	x1 = X0;
	z1 = Z0;
	LongRotate(-400, 0, ptb3d->Beta);

	if (Nxw + X0 >= 0 && Nxw + X0 < 32768 && Nzw + Z0 >= 0 && Nzw + Z0 < 32768) {
		y = CalculAltitudeObjet(Nxw + X0, Nzw + Z0, -1);
	} else {
		y = yw;
	}

	if (Nxw + x1 >= 0 && Nxw + x1 < 32768 && Nzw + z1 >= 0 && Nzw + z1 < 32768) {
		y1 = CalculAltitudeObjet(Nxw + x1, Nzw + z1, -1);
	} else {
		y1 = yw;
	}

	ptb3d->CurrentFrame[1].Gamma = (int16)GetAngle2D(y, 0, y1, 800);

	// Steering wheel
	ptb3d->CurrentFrame[12].Gamma = (int16)-ptb->BetaWheel;

	// Twinsen's head
	ptb3d->CurrentFrame[14].Beta = (int16)ptb->BetaWheel;
#endif
}

void Buggy::moveBuggy(ActorStruct *ptrobj) {
#if 0
	S_BUGGY *ptb = &ListBuggy[0];
	T_OBJ_3D *ptb3d = &ptrobj->Obj;
	int32 pitch = 0;
	int32 flagattack = false;

	int32 speedinc;
	int32 rotlevel;
	int32 timerhr, deltatimer;

	timerhr = TimerRefHR;
	deltatimer = timerhr - ptb->LastTimer;

	if ((Input & I_THROW) && (PtrComportement->Flags & CF_WEAPON)) {
		// Are we in mage?
		if (TabInv[FLAG_TUNIQUE].IdObj3D == 0) {
			_engine->_actor->initBody(BodyType::btTunicTir, OWN_ACTOR_SCENE_INDEX);
		} else {
			_engine->_actor->initBody(BodyType::btMageTir, OWN_ACTOR_SCENE_INDEX);
		}

		_engine->_animations->initAnim(GEN_ANIM_LANCE, AnimType::kAnimationTypeRepeat, OWN_ACTOR_SCENE_INDEX);

		/* control direction pendant Aiming */
		if (!ptrobj->_workFlags.bIsRotationByAnim) {
			ptb3d->Beta += GetDeltaMove(&ptrobj->BoundAngle.Move);
			ptb3d->Beta &= 4095;

			_engine->_movements->initRealAngleConst(ptrobj);
		}

		_engine->_movements->_lastJoyFlag = true;
		flagattack = true;
	} else {
		if (LastInput & I_THROW) {
			// We finished shooting with the buggy,
			// we close the hood
			_engine->_actor->initBody(BodyType::btTunic, OWN_ACTOR_SCENE_INDEX);
			_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, OWN_ACTOR_SCENE_INDEX);
		}
	}

	if (!flagattack && !ptrobj->_workFlags.bIsFalling && !ptrobj->_workFlags.bUnk1000) {
		_engine->_movements->clearRealAngle(ptrobj);

		if (_engine->_movements->_lastJoyFlag && (((Input & I_JOY) != LastMyJoy) || ((Input & I_FIRE) != LastMyFire))) {
			_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, OWN_ACTOR_SCENE_INDEX);
			Pushing = false;
		}

		_engine->_movements->_lastJoyFlag = false;

		// Pushing contains the number of the object being pushed
		// So 1000 is an impossible value used as an initialization flag
		// no animation
		if (Pushing == 1000) {
			_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, OWN_ACTOR_SCENE_INDEX);
			Pushing = false;
		}

		if (Input & I_UP) {
			if (Pushing) {
				_engine->_animations->initAnim(AnimationTypes::kPush, ANIM_TEMPO, OWN_ACTOR_SCENE_INDEX);
				_engine->_movements->_lastJoyFlag = true;
			} else {
				_engine->_animations->initAnim(AnimationTypes::kForward, AnimType::kAnimationTypeRepeat, OWN_ACTOR_SCENE_INDEX);
				_engine->_movements->_lastJoyFlag = true;
			}
		} else if (Input & I_DOWN) {
			_engine->_animations->initAnim(AnimationTypes::kBackward, AnimType::kAnimationTypeRepeat, OWN_ACTOR_SCENE_INDEX);
			_engine->_movements->_lastJoyFlag = true;
		}
	}

	if (!ptrobj->_workFlags.bIsFalling && !ptrobj->_workFlags.bUnk1000) {
		// check speed command
		if ((Input & I_UP) // accelerating
			&& !flagattack) {
			ptb->Speed += deltatimer * 4;

			if (!TimerGear)
				TimerGear = TimerRefHR + TEMPO_GEAR;
			else {
				if (Gear < 0)
					Gear = 0;

				if (TimerRefHR > TimerGear && Gear < 2) {
					Gear++;
					TimerGear = TimerRefHR + TEMPO_GEAR;
				}
			}
		} else if ((Input & I_DOWN) // brake / reverse
				   && !flagattack) {
			ptb->Speed -= deltatimer * 12;
			Gear = -1;
			TimerGear = 0;
		} else // slow down
		{
			if (ptb->Speed > 0) {
				ptb->Speed -= deltatimer * 7;
				if (ptb->Speed < 0) {
					ptb->Speed = 0;
				}
			}
			if (ptb->Speed < 0) {
				ptb->Speed += deltatimer * 7;
				if (ptb->Speed > 0) {
					ptb->Speed = 0;
				}
			}
			Gear = 0;
			TimerGear = 0;
		}

		if (ptb->Speed < -2000)
			ptb->Speed = -2000;
		if (ptb->Speed > MAX_SPEED)
			ptb->Speed = MAX_SPEED;

		speedinc = ptb->Speed * deltatimer / 1000;
	} else {
		speedinc = 0;
	}

	// check dir

	if (!flagattack) {
		if (Input & I_RIGHT) {
			ptb->BetaWheel = -300;
			if (ptb->Speed) {
				rotlevel = -ptb->SpeedRot * speedinc / ptb->Speed;
			} else {
				rotlevel = 0;
			}
		} else if (Input & I_LEFT) {
			ptb->BetaWheel = 300;
			if (ptb->Speed) {
				rotlevel = ptb->SpeedRot * speedinc / ptb->Speed;
			} else {
				rotlevel = 0;
			}
		} else {
			ptb->BetaWheel = 0;
			rotlevel = 0;
		}

		if (ptrobj->_staticFlags & SKATING) {
			ptb->Speed = 3000;
			speedinc = ptb->Speed * deltatimer / 1000;
		} else {
			if (ptb->Speed >= 0) {
				ptb3d->Beta += rotlevel;
			} else {
				ptb3d->Beta -= rotlevel;
			}

			ptb3d->Beta = ClampAngle(ptb3d->Beta);
		}
	} else {
		ptb->BetaWheel = 0;
	}

	LastMyJoy = Input & I_JOY;
	LastMyFire = Input & I_FIRE;
	LastInput = Input;

	ptb->LastTimer = timerhr;
	ptb->SpeedInc = speedinc;

	if (ptrobj->SampleAlways && _engine->_sound->isSamplePlaying(ptrobj->SampleAlways)) {
		int32 pitch;

		switch (Gear) {
		case -1:
			pitch = boundRuleThree(3000, MAX_SAMPLE_PITCH2, MAX_SPEED, ABS(ptb->Speed));
			break;

		case 0:
			pitch = boundRuleThree(3000, MAX_SAMPLE_PITCH, MAX_SPEED, ABS(ptb->Speed));
			if (pitch >= MAX_SAMPLE_PITCH)
				TimerGear = 1;
			break;

		case 1:
			pitch = boundRuleThree(MAX_SAMPLE_PITCH2, MIN_SAMPLE_PITCH2, TEMPO_GEAR, TimerGear - TimerRefHR);
			break;

		default:
			pitch = MAX_SAMPLE_PITCH2;
		}

		_engine->_sound->ChangePitchbendSample(ptrobj->SampleAlways, pitch);
	}
#endif
}

} // namespace TwinE
