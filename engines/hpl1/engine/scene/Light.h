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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_LIGHT_H
#define HPL_LIGHT_H

#include "hpl1/engine/graphics/GraphicsTypes.h"

namespace hpl {

class iLowLevelGraphics;
class cWorld3D;

class iLight {
public:
	iLight();
	virtual ~iLight() = default;

	//////////////////////////
	// Fading
	void FadeTo(const cColor &aCol, float afRadius, float afTime);
	bool IsFading();
	cColor GetDestColor() { return mDestCol; }
	float GetDestRadius() { return mfDestRadius; }

	//////////////////////////
	// FLickering
	void SetFlickerActive(bool abX);
	bool GetFlickerActive() { return mbFlickering; }

	void SetFlicker(const cColor &aOffCol, float afOffRadius,
					float afOnMinLength, float afOnMaxLength, const tString &asOnSound, const tString &asOnPS,
					float afOffMinLength, float afOffMaxLength, const tString &asOffSound, const tString &asOffPS,
					bool abFade, float afOnFadeLength, float afOffFadeLength);

	tString GetFlickerOffSound() { return msFlickerOffSound; }
	tString GetFlickerOnSound() { return msFlickerOnSound; }
	tString GetFlickerOffPS() { return msFlickerOffPS; }
	tString GetFlickerOnPS() { return msFlickerOnPS; }
	float GetFlickerOnMinLength() { return mfFlickerOnMinLength; }
	float GetFlickerOffMinLength() { return mfFlickerOffMinLength; }
	float GetFlickerOnMaxLength() { return mfFlickerOnMaxLength; }
	float GetFlickerOffMaxLength() { return mfFlickerOffMaxLength; }
	cColor GetFlickerOffColor() { return mFlickerOffColor; }
	float GetFlickerOffRadius() { return mfFlickerOffRadius; }
	bool GetFlickerFade() { return mbFlickerFade; }
	float GetFlickerOnFadeLength() { return mfFlickerOnFadeLength; }
	float GetFlickerOffFadeLength() { return mfFlickerOffFadeLength; }

	cColor GetFlickerOnColor() { return mFlickerOnColor; }
	float GetFlickerOnRadius() { return mfFlickerOnRadius; }

	//////////////////////////
	// Properties
	const cColor &GetDiffuseColor() { return mDiffuseColor; }
	void SetDiffuseColor(cColor aColor);
	const cColor &GetSpecularColor() { return mSpecularColor; }
	void SetSpecularColor(cColor aColor) { mSpecularColor = aColor; }

	float GetIntensity() { return mfIntensity; }
	void SetIntensity(float afX) { mfIntensity = afX; }
	bool GetCastShadows() { return mbCastShadows; }
	void SetCastShadows(bool afX) { mbCastShadows = afX; }

	bool GetAffectMaterial() { return mbAffectMaterial; }
	void SetAffectMaterial(bool afX) { mbAffectMaterial = afX; }

	float GetFarAttenuation() { return mfFarAttenuation; }
	float GetNearAttenuation() { return mfNearAttenuation; }

	virtual void SetFarAttenuation(float afX) = 0;
	virtual void SetNearAttenuation(float afX) = 0;

	float GetSourceRadius() { return mfSourceRadius; }
	void SetSourceRadius(float afX) { mfSourceRadius = afX; }

	virtual cVector3f GetLightPosition() = 0;

	void UpdateLight(float afTimeStep);

	void SetWorld3D(cWorld3D *apWorld) { mpWorld3D = apWorld; }

protected:
	virtual void OnFlickerOff() {}
	virtual void OnFlickerOn() {}
	virtual void OnSetDiffuse() {}

	cColor mDiffuseColor;
	cColor mSpecularColor;
	float mfIntensity;
	float mfFarAttenuation;
	float mfNearAttenuation;
	float mfSourceRadius;

	bool mbCastShadows;
	bool mbAffectMaterial;

	cWorld3D *mpWorld3D;

	///////////////////////////
	// Fading.
	cColor mColAdd;
	float mfRadiusAdd;
	cColor mDestCol;
	float mfDestRadius;
	float mfFadeTime;

	///////////////////////////
	// Flicker
	bool mbFlickering;
	tString msFlickerOffSound;
	tString msFlickerOnSound;
	tString msFlickerOffPS;
	tString msFlickerOnPS;
	float mfFlickerOnMinLength;
	float mfFlickerOffMinLength;
	float mfFlickerOnMaxLength;
	float mfFlickerOffMaxLength;
	cColor mFlickerOffColor;
	float mfFlickerOffRadius;
	bool mbFlickerFade;
	float mfFlickerOnFadeLength;
	float mfFlickerOffFadeLength;

	cColor mFlickerOnColor;
	float mfFlickerOnRadius;

	bool mbFlickerOn;
	float mfFlickerTime;
	float mfFlickerStateLength;
};

} // namespace hpl

#endif // HPL_LIGHT_H
