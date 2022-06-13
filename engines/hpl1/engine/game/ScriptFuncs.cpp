/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "hpl1/engine/game/ScriptFuncs.h"

#include <stdlib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "hpl1/engine/game/Game.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/sound/Sound.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/Light3D.h"
#include "hpl1/engine/scene/World3D.h"
#include "hpl1/engine/scene/SoundEntity.h"
#include "hpl1/engine/scene/PortalContainer.h"
#include "hpl1/engine/input/Input.h"
#include "hpl1/engine/system/System.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include "hpl1/engine/sound/MusicHandler.h"
#include "hpl1/engine/resources/SoundManager.h"
#include "hpl1/engine/resources/SoundEntityManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/sound/SoundData.h"
#include "hpl1/engine/sound/SoundChannel.h"
#include "hpl1/engine/sound/SoundHandler.h"
#include "hpl1/engine/sound/SoundEntityData.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/physics/PhysicsJoint.h"
#include "hpl1/engine/physics/PhysicsJointHinge.h"
#include "hpl1/engine/physics/PhysicsJointScrew.h"
#include "hpl1/engine/physics/PhysicsJointSlider.h"
#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsController.h"
#include "hpl1/engine/system/Script.h"
#include "hpl1/engine/graphics/ParticleSystem3D.h"
#include "hpl1/engine/scene/MeshEntity.h"
#include "hpl1/engine/graphics/BillBoard.h"
#include "hpl1/engine/graphics/Beam.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/libraries/angelscript/angelscript.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// JOINT CALLBACK
	//////////////////////////////////////////////////////////////////////////

	cScriptJointCallback::cScriptJointCallback(cScene *apScene)
	{
		mpScene = apScene;

		msMaxFunc = "";
		msMinFunc = "";
	}

	void cScriptJointCallback::OnMinLimit(iPhysicsJoint *apJoint)
	{
		if(msMinFunc!="")
		{
			iScript *pScript = mpScene->GetWorld3D()->GetScript();

			tString sCommand = msMinFunc + "(\"" + apJoint->GetName() + "\")";
			if(pScript->Run(sCommand)==false){
				Warning("Couldn't run script command '%s'\n",sCommand.c_str());
			}

		}
	}

	void cScriptJointCallback::OnMaxLimit(iPhysicsJoint *apJoint)
	{
		if(msMaxFunc!="")
		{
			iScript *pScript = mpScene->GetWorld3D()->GetScript();

			tString sCommand = msMaxFunc + "(\"" + apJoint->GetName() + "\")";
			if(pScript->Run(sCommand)==false){
				Warning("Couldn't run script command '%s'\n",sCommand.c_str());
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	static cResources *gpResources=NULL;
	static cSystem *gpSystem=NULL;
	static cInput *gpInput=NULL;
	static cGraphics *gpGraphics=NULL;
	static cScene *gpScene=NULL;
	static cSound *gpSound=NULL;
	static cGame *gpGame=NULL;

	//-----------------------------------------------------------------------

	static void __stdcall Print(std::string asText)
	{
		Log(asText.c_str());
	}
	SCRIPT_DEFINE_FUNC_1(void, Print, string)

	static std::string __stdcall FloatToString(float afX)
	{
		char sTemp[30];
		sprintf(sTemp,"%f",afX);
		return (std::string) sTemp;
	}
	SCRIPT_DEFINE_FUNC_1(string, FloatToString, float)

	static std::string __stdcall IntToString(int alX)
	{
		char sTemp[30];
		sprintf(sTemp,"%d",alX);
		return (std::string) sTemp;
	}
	SCRIPT_DEFINE_FUNC_1(string, IntToString, int)

	static float __stdcall RandFloat(float afMin, float afMax)
	{
		return cMath::RandRectf(afMin,afMax);
	}
	SCRIPT_DEFINE_FUNC_2(float, RandFloat, float, float)

	static int __stdcall RandInt(int alMin, int alMax)
	{
		return cMath::RandRectl(alMin,alMax);
	}
	SCRIPT_DEFINE_FUNC_2(int, RandInt, int, int)

	static bool __stdcall StringContains(std::string asString, std::string asSubString)
	{
		return cString::GetLastStringPos(asString,asSubString)>=0;
	}
	SCRIPT_DEFINE_FUNC_2(bool, StringContains, string, string)

	static void __stdcall ResetLogicTimer()
	{
		gpGame->ResetLogicTimer();
	}
	SCRIPT_DEFINE_FUNC(void, ResetLogicTimer)

	/////////////////////////////////////////////////////////////////////////
	/////// RENDERER //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	/**
	* Sets ambient light color-
	**/
	static void __stdcall SetAmbientColor(float afR, float afG, float afB)
	{
		gpGraphics->GetRenderer3D()->SetAmbientColor(cColor(afR,afG,afB,1.0f));
	}
	SCRIPT_DEFINE_FUNC_3(void, SetAmbientColor, float, float, float)

	//-----------------------------------------------------------------------

	/**
	* Sets if the skybox should be active
	**/
	static void __stdcall SetSkyboxActive(bool abX)
	{
		gpGraphics->GetRenderer3D()->SetSkyBoxActive(abX);
	}
	SCRIPT_DEFINE_FUNC_1(void, SetSkyboxActive, bool)

	//-----------------------------------------------------------------------

	/**
	* Sets the skybox color.
	**/
	static void __stdcall SetSkyboxColor(float afR, float afG, float afB, float afA)
	{
		gpGraphics->GetRenderer3D()->SetSkyBoxColor(cColor(afR,afG,afB,afA));
	}
	SCRIPT_DEFINE_FUNC_4(void, SetSkyboxColor, float, float, float, float)

	//-----------------------------------------------------------------------


	/**
	* Sets the skybox
	* \param asTexture Name of the cube map texture to use
	**/
	static void __stdcall SetSkybox(std::string asTexture)
	{
		if(asTexture!="")
		{
			iTexture *pTex = gpResources->GetTextureManager()->CreateCubeMap(asTexture,false);
			gpGraphics->GetRenderer3D()->SetSkyBox(pTex,true);
		}
		else
		{
			gpGraphics->GetRenderer3D()->SetSkyBox(NULL,false);
		}
	}
	SCRIPT_DEFINE_FUNC_1(void, SetSkybox, string)

	//-----------------------------------------------------------------------

	/**
	* Creates a particle system and attaches it to the camera.
	* \param asName Name of particle system
	* \param asType The type of particle system (file)
	**/
	static void __stdcall CreateParticleSystemOnCamera(std::string asName,std::string asType)
	{
		cParticleSystem3D *pPS = gpScene->GetWorld3D()->CreateParticleSystem(asName,asType,
																			1,cMatrixf::Identity);
		if(pPS)
		{
			cCamera3D *pCam = static_cast<cCamera3D*>(gpScene->GetCamera());
			pCam->AttachEntity(pPS);
		}
	}
	SCRIPT_DEFINE_FUNC_2(void, CreateParticleSystemOnCamera, string, string)

	//-----------------------------------------------------------------------

	/**
	* Sets if fog should be active
	* \param abX If the fog is active or not.
	**/
	static void __stdcall SetFogActive(bool abX)
	{
		gpGraphics->GetRenderer3D()->SetFogActive(abX);
	}
	SCRIPT_DEFINE_FUNC_1(void, SetFogActive, bool)

	/**
	* Sets if the fog should be used to cull non-visible objects
	* \param abX If the culling is active or not.
	**/
	static void __stdcall SetFogCulling(bool abX)
	{
		gpGraphics->GetRenderer3D()->SetFogCulling(abX);
	}
	SCRIPT_DEFINE_FUNC_1(void, SetFogCulling, bool)

	/**
	* Creates a particle system and attaches it to the camera.
	* \param afStart Start of fog color
	* \param afStart End of fog fade. After this limit all geometry is full fog color.
	* \param afR, afG, afB Color of Fog.
	**/
	static void __stdcall SetFogProperties(float afStart, float afEnd, float afR,float afG, float afB)
	{
		gpGraphics->GetRenderer3D()->SetFogStart(afStart);
		gpGraphics->GetRenderer3D()->SetFogEnd(afEnd);
		gpGraphics->GetRenderer3D()->SetFogColor(cColor(afR,afG,afB,1.0f));
	}
	SCRIPT_DEFINE_FUNC_5(void, SetFogProperties, float, float, float, float, float)

	//-----------------------------------------------------------------------

	static void __stdcall SetSectorProperties(std::string asSector, float afAmbR,float afAmbG, float afAmbB)
	{
		cPortalContainer *pContainer = gpScene->GetWorld3D()->GetPortalContainer();

		cSector *pSector = pContainer->GetSector(asSector);
		if(pSector == NULL){
			Warning("Could not find sector '%s'\n", asSector.c_str());
			return;
		}

		pSector->SetAmbientColor(cColor(afAmbR, afAmbG, afAmbB,1));
	}
	SCRIPT_DEFINE_FUNC_4(void, SetSectorProperties, string, float, float, float)

	//-----------------------------------------------------------------------

	static void __stdcall SetSectorPortalActive(std::string asSector, int alPortal, bool abActive)
	{
		cPortalContainer *pContainer = gpScene->GetWorld3D()->GetPortalContainer();

		cSector *pSector = pContainer->GetSector(asSector);
		if(pSector == NULL){
			Warning("Could not find sector '%s'\n", asSector.c_str());
			return;
		}

		cPortal *pPortal = pSector->GetPortal(alPortal);
		if(pPortal==NULL)
		{
			Warning("Could not find portal %d in sector '%s'\n",alPortal,asSector.c_str());
			return;
		}

		pPortal->SetActive(abActive);
	}
	SCRIPT_DEFINE_FUNC_3(void, SetSectorPortalActive, string, int, bool)

	/////////////////////////////////////////////////////////////////////////
	/////// RESOURCES //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	/**
	 * Gets a string in the current language.
	 * \param asCat The translation category
	 * \param asName The name of the category entry.
	 **/
	static std::string __stdcall Translate(std::string asCat, std::string asName)
	{
		tWString wsText = gpResources->Translate(asCat,asName);
		return cString::To8Char(wsText);
	}
	SCRIPT_DEFINE_FUNC_2(string, Translate, string, string)

	//-----------------------------------------------------------------------

	/**
	* Preloads the data for a sound.
	* \param asFile This can be a wav, ogg, mp3 or snt file.
	**/
	static void __stdcall PreloadSound(std::string asFile)
	{
		tString sExt = cString::ToLowerCase(cString::GetFileExt(asFile));
		if(sExt=="snt")
		{
			cSoundEntityData *pData = gpResources->GetSoundEntityManager()->CreateSoundEntity(asFile);
			if(pData == NULL) {
				Warning("Couldn't preload sound '%s'\n",asFile.c_str());
				return;
			}

			if(pData->GetMainSoundName() != ""){
				iSoundChannel *pChannel = gpSound->GetSoundHandler()->CreateChannel(pData->GetMainSoundName(),0);
				hplDelete(pChannel);
			}
			if(pData->GetStartSoundName() != ""){
				iSoundChannel *pChannel = gpSound->GetSoundHandler()->CreateChannel(pData->GetStartSoundName(),0);
				hplDelete(pChannel);
			}
			if(pData->GetStopSoundName() != ""){
				iSoundChannel *pChannel = gpSound->GetSoundHandler()->CreateChannel(pData->GetStopSoundName(),0);
				hplDelete(pChannel);
			}
		}
		else
		{
			iSoundData *pSound = gpResources->GetSoundManager()->CreateSoundData(asFile,false);
			if(pSound){
				Warning("Couldn't preload sound '%s'\n",asFile.c_str());
			}
		}
	}
	SCRIPT_DEFINE_FUNC_1(void, PreloadSound, string)

	//-----------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////
	/////// MESH ENTITY //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	static void __stdcall SetMeshActive(std::string asName, bool abActive)
	{
		cMeshEntity *pEnt = gpScene->GetWorld3D()->GetMeshEntity(asName);
		if(pEnt==NULL){
			Warning("Didn't find mesh entity '%s'\n",asName.c_str());
			return;
		}

		pEnt->SetActive(abActive);
		pEnt->SetVisible(abActive);
	}
	SCRIPT_DEFINE_FUNC_2(void, SetMeshActive, string, bool)

	//-----------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////
	/////// PARTICLE SYSTEM //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	/**
	* Create a particle system at the position of an area
	* \param asName The name of the particle system.
	* \param abActive If it should be active or not.
	**/
	static void __stdcall SetParticleSystemActive(std::string asName, bool abActive)
	{
		cParticleSystem3D *pPS = gpScene->GetWorld3D()->GetParticleSystem(asName);
		if(pPS==NULL){
			Warning("Didn't find particle system '%s'\n",asName.c_str());
			return;
		}

		pPS->SetActive(abActive);
		pPS->SetVisible(abActive);
	}
	SCRIPT_DEFINE_FUNC_2(void, SetParticleSystemActive, string, bool)

	//-----------------------------------------------------------------------

	/**
	* Create a particle system at the position of an area
	* \param asName The name of the particle system.
	* \param asType The type of aprticle system
	* \param asArea The name of the area
	* \param X Y and Z the variables of the particle system.
	**/
	static void __stdcall CreateParticleSystem(std::string asName, std::string asType, std::string asArea,
												float afX, float afY, float afZ)
	{
		cAreaEntity* pArea = gpScene->GetWorld3D()->GetAreaEntity(asArea);
		if(pArea==NULL){
			Warning("Couldn't find area '%s'\n",asArea.c_str());
			return;
		}

		cParticleSystem3D *pPS = gpScene->GetWorld3D()->CreateParticleSystem(asName,asType,
														cVector3f(afX,afY,afZ),pArea->m_mtxTransform);
		if(pPS==NULL){
			Warning("No particle system of type '%s'\n",asType.c_str());
			return;
		}
	}
	SCRIPT_DEFINE_FUNC_6(void, CreateParticleSystem, string, string, string, float, float, float)

	//-----------------------------------------------------------------------

	/**
	* Kill a particle system
	* \param asName The name of the particle system.
	**/
	static void __stdcall KillParticleSystem(std::string asName)
	{
		/*cParticleSystem3D *pPS = gpScene->GetWorld3D()->GetParticleSystem(asName);
		if(pPS==NULL){
			Warning("Didn't find particle system '%s'\n",asName.c_str());
			return;
		}
		pPS->Kill();*/

		bool bFound = false;
		cParticleSystem3DIterator it = gpScene->GetWorld3D()->GetParticleSystemIterator();
		while(it.HasNext())
		{
			cParticleSystem3D *pPS = it.Next();

			if(pPS->GetName() == asName)
			{
				pPS->Kill();
				bFound = true;
			}
		}

		if(!bFound) Warning("Didn't find particle system '%s'\n",asName.c_str());
	}
	SCRIPT_DEFINE_FUNC_1(void, KillParticleSystem, string)

	//-----------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////
	/////// BEAM //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------


	/**
	 * Creates an a beam between two areas
	 * \param asName
	 * \param asFile
	 * \param asStartArea
	 * \param asEndArea
	 */
	static void __stdcall CreateBeam(std::string asName, std::string asFile,
									std::string asStartArea, std::string asEndArea)
	{
		cAreaEntity* pStartArea = gpScene->GetWorld3D()->GetAreaEntity(asStartArea);
		if(pStartArea==NULL){
			Warning("Couldn't find area '%s'\n",asStartArea.c_str());
			return;
		}

		cAreaEntity* pEndArea = gpScene->GetWorld3D()->GetAreaEntity(asEndArea);
		if(pEndArea==NULL){
			Warning("Couldn't find area '%s'\n",asEndArea.c_str());
			return;
		}

		cBeam *pBeam = gpScene->GetWorld3D()->CreateBeam(asName);

		if(pBeam->LoadXMLProperties(asFile)==false)
		{
			Error("Couldn't create beam from file '%s'\n",asFile.c_str());
			gpScene->GetWorld3D()->DestroyBeam(pBeam);
			return;
		}

		pBeam->SetPosition(pStartArea->m_mtxTransform.GetTranslation());
		pBeam->GetEnd()->SetPosition(pEndArea->m_mtxTransform.GetTranslation());
	}
	SCRIPT_DEFINE_FUNC_4(void, CreateBeam, string, string, string, string)

	//-----------------------------------------------------------------------

	/**
	 * Destroys a beam
	 * \param asName
	 */
	static void __stdcall DestroyBeam(std::string asName)
	{
		cBeam* pBeam = gpScene->GetWorld3D()->GetBeam(asName);
		if(pBeam==NULL)
		{
			Warning("Couldn't find beam '%s'\n",asName.c_str());
			return;
		}

		gpScene->GetWorld3D()->DestroyBeam(pBeam);
	}
	SCRIPT_DEFINE_FUNC_1(void, DestroyBeam, string)

	//-----------------------------------------------------------------------



	/////////////////////////////////////////////////////////////////////////
	/////// LIGHT //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	/**
	* Fades a sound to a color and a radius
	* \param asName The name of the light
	* \param afR The red channel to fade to.
	* \param afG The green channel to fade to.
	* \param afB The blue channel to fade to.
	* \param afA The alpha channel to fade to.
	* \param afRadius The radius to fade to.
	* \param afTime The amount of seconds the fade should last.
	**/
	static void __stdcall FadeLight3D(std::string asName, float afR,float afG, float afB, float afA,
										float afRadius, float afTime)
	{
		iLight3D *pLight = gpScene->GetWorld3D()->GetLight(asName);
		if(pLight==NULL)
		{
			Warning("Couldn't find light '%s'\n",asName.c_str());
			return;
		}

		pLight->FadeTo(cColor(afR,afG,afB,afA),afRadius, afTime);
		pLight->SetVisible(true);
		pLight->UpdateLight(2.0f/60.0f);
	}
	SCRIPT_DEFINE_FUNC_7(void, FadeLight3D, string, float, float, float, float, float, float)

	//-----------------------------------------------------------------------

	/**
	* Attaches a billboard to a light
	* \param asBillboardName The billbaord name
	* \param asLightName The light name
	* \param abX True if it should be attached, false if you want to remove.
	**/
	static void __stdcall AttachBillboardToLight3D(std::string asBillboardName, std::string asLightName,bool abX)
	{
		iLight3D *pLight = gpScene->GetWorld3D()->GetLight(asLightName);
		if(pLight==NULL)
		{
			Warning("Couldn't find light '%s'\n",asLightName.c_str());
			return;
		}

		cBillboard *pBillboard = gpScene->GetWorld3D()->GetBillboard(asBillboardName);
		if(pBillboard==NULL)
		{
			Warning("Couldn't find billboard '%s'\n",asBillboardName.c_str());
			return;
		}

		if(abX)
			pLight->AttachBillboard(pBillboard);
		else
			pLight->RemoveBillboard(pBillboard);
	}
	SCRIPT_DEFINE_FUNC_3(void, AttachBillboardToLight3D, string, string, bool)

	//-----------------------------------------------------------------------


	/**
	* Sets on/off a light
	* \param asName The light name
	* \param abX if the light should be on or off.
	**/
	static void __stdcall SetLight3DVisible(std::string asName, bool abX)
	{
		iLight3D *pLight = gpScene->GetWorld3D()->GetLight(asName);
		if(pLight==NULL)
		{
			Warning("Couldn't find light '%s'\n",asName.c_str());
			return;
		}

		pLight->SetVisible(abX);
	}
	SCRIPT_DEFINE_FUNC_2(void, SetLight3DVisible, string, bool)

		//-----------------------------------------------------------------------

	/**
	* Sets on/off for affect only in sector where centre is.
	* \param asName The light name
	* \param abX if the light should only affects objects in same sector or not.
	**/
	static void __stdcall SetLight3DOnlyAffectInSector(std::string asName, bool abX)
	{
		iLight3D *pLight = gpScene->GetWorld3D()->GetLight(asName);
		if(pLight==NULL)
		{
			Warning("Couldn't find light '%s'\n",asName.c_str());
			return;
		}

		pLight->SetOnlyAffectInSector(abX);
	}
	SCRIPT_DEFINE_FUNC_2(void, SetLight3DOnlyAffectInSector, string, bool)

	//-----------------------------------------------------------------------

	/**
	* Sets flickering on/off a light
	* \param asName The light name
	* \param abX if the light flicker should be on or off.
	**/
	static void __stdcall SetLight3DFlickerActive(std::string asName, bool abX)
	{
		iLight3D *pLight = gpScene->GetWorld3D()->GetLight(asName);
		if(pLight==NULL)
		{
			Warning("Couldn't find light '%s'\n",asName.c_str());
			return;
		}

		pLight->SetFlickerActive(abX);
	}
	SCRIPT_DEFINE_FUNC_2(void, SetLight3DFlickerActive, string, bool)

	//-----------------------------------------------------------------------

	/**
	* Sets flickering parameters
	* \param asName The light name
	* \param abR, afG, afB, afA The color of the light when off
	* \param afRadius The radius of the light when off.
	* \param afOnMinLength Minimum time before going from off to on.
	* \param afOnMaxLength Maximum time before going from off to on.
	* \param asOnSound Name of the sound played when going from off to on. "" means no sound.
	* \param asOnPS Name of the particle system played when going from off to on. "" means none.
	* \param afOffMinLength Minimum time before going from on to off.
	* \param afOffMaxLength Maximum time before going from on to off.
	* \param asOffSound Name of the sound played when going from on to off. "" means no sound.
	* \param asOffPS Name of the particle system played when going from on to off. "" means none.
	* \param abFade If there should be a fade between off and on.
	* \param afOnFadeLength Fade length from off to on.
	* \param afOffFadeLength Fade length from on to off.
	**/
	static void __stdcall SetLight3DFlicker(std::string asName,
											float afR,float afG, float afB, float afA,
											float afRadius,

											float afOnMinLength, float afOnMaxLength,
											std::string asOnSound,std::string asOnPS,

											float afOffMinLength, float afOffMaxLength,
											std::string asOffSound,std::string asOffPS,

											bool abFade,
											float afOnFadeLength, float afOffFadeLength)
	{
		iLight3D *pLight = gpScene->GetWorld3D()->GetLight(asName);
		if(pLight==NULL)
		{
			Warning("Couldn't find light '%s'\n",asName.c_str());
			return;
		}

		pLight->SetFlicker(cColor(afR,afG,afB,afA),afRadius,
							afOnMinLength, afOnMaxLength,asOnSound,asOnPS,
							afOffMinLength, afOffMaxLength,asOffSound,asOffPS,
							abFade,afOnFadeLength,afOffFadeLength);
	}
	SCRIPT_DEFINE_FUNC_17(void, SetLight3DFlicker, string,
			float, float, float, float,
			float, float, float, string, string, float, float, string, string, bool, float, float)

	//-----------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////
	/////// SOUND //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////


	//-----------------------------------------------------------------------

	/**
	 * Creates a sound entity at the postion of an area.
	 * \param asName Name of the sound area
	 * \param asFile The snt file to load.
	 * \param asArea The area to create at.
	 */
	static void __stdcall CreateSoundEntity(std::string asName, std::string asFile,
												std::string asArea)
	{
		cAreaEntity* pArea = gpScene->GetWorld3D()->GetAreaEntity(asArea);
		if(pArea==NULL){
			Warning("Couldn't find area '%s'\n",asArea.c_str());
			return;
		}

		cSoundEntity *pSound = gpScene->GetWorld3D()->CreateSoundEntity(asName,asFile,true);
		if(pSound==NULL)
		{
			Warning("Couldn't create sound entity '%s'\n",asFile.c_str());
			return;
		}

		pSound->SetPosition(pArea->m_mtxTransform.GetTranslation());
	}
	SCRIPT_DEFINE_FUNC_3(void, CreateSoundEntity, string, string, string)
	//-----------------------------------------------------------------------

	/**
	* Play a sound entity
	* \param asName The entity name
	* \param abPlayStart If the start sound should be played.
	**/
	static void __stdcall PlaySoundEntity(std::string asName, bool abPlayStart)
	{
		cSoundEntity *pSound = gpScene->GetWorld3D()->GetSoundEntity(asName);
		if(pSound==NULL)
		{
			Warning("Couldn't find sound entity '%s'\n",asName.c_str());
			return;
		}

		pSound->Play(abPlayStart);
	}
	SCRIPT_DEFINE_FUNC_2(void, PlaySoundEntity, string, bool)

	/**
	* Stop a sound entity
	* \param asName The entity name
	* \param abPlayEnd If the end sound should be played.
	**/
	static void __stdcall StopSoundEntity(std::string asName, bool abPlayEnd)
	{
		cSoundEntity *pSound = gpScene->GetWorld3D()->GetSoundEntity(asName);
		if(pSound==NULL)
		{
			Warning("Couldn't find sound entity '%s'\n",asName.c_str());
			return;
		}

		pSound->Stop(abPlayEnd);
	}
	SCRIPT_DEFINE_FUNC_2(void, StopSoundEntity, string, bool)

	//-----------------------------------------------------------------------

	/**
	* Play a sound entity fading it
	* \param asName The entity name
	* \param afSpeed Volume increase per second.
	**/
	static void __stdcall FadeInSoundEntity(std::string asName, float afSpeed)
	{
		cSoundEntity *pSound = gpScene->GetWorld3D()->GetSoundEntity(asName);
		if(pSound==NULL)
		{
			Warning("Couldn't find sound entity '%s'\n",asName.c_str());
			return;
		}

		pSound->FadeIn(afSpeed);
	}
	SCRIPT_DEFINE_FUNC_2(void, FadeInSoundEntity, string, float)

	/**
	* Stop a sound entity fading it
	* \param asName The entity name
	* \param afSpeed Volume decrease per second.
	**/
	static void __stdcall FadeOutSoundEntity(std::string asName, float afSpeed)
	{
		cSoundEntity *pSound = gpScene->GetWorld3D()->GetSoundEntity(asName);
		if(pSound==NULL)
		{
			Warning("Couldn't find sound entity '%s'\n",asName.c_str());
			return;
		}

		pSound->FadeOut(afSpeed);
	}
	SCRIPT_DEFINE_FUNC_2(void, FadeOutSoundEntity, string, float)

	//-----------------------------------------------------------------------

	static void __stdcall PlayMusic(std::string asName, float afVol, float afStepSize, bool abLoop)
	{
		gpSound->GetMusicHandler()->Play(asName,afVol,afStepSize,abLoop);
	}
	SCRIPT_DEFINE_FUNC_4(void, PlayMusic, string, float, float, bool)

	//-----------------------------------------------------------------------

	static void __stdcall StopMusic(float afStepSize)
	{
		gpSound->GetMusicHandler()->Stop(afStepSize);
	}
	SCRIPT_DEFINE_FUNC_1(void, StopMusic, float)

	//-----------------------------------------------------------------------

	/**
	* Play a sound gui sound, with out any position.
	* \param asName The sound name
	* \param afVol Volume of the sound
	**/
	static void __stdcall PlayGuiSound(std::string asName, float afVol)
	{
		gpSound->GetSoundHandler()->PlayGui(asName,false,afVol);
	}
	SCRIPT_DEFINE_FUNC_2(void, PlayGuiSound, string, float)

	/////////////////////////////////////////////////////////////////////////
	/////// PHYSICS //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	/**
	* Sets a callback for a joint.
	* The syntax for the function is: void MyFunction(string asJointName)
	* \param asJointName The joint name
	* \param asType The type, can be: "OnMax" or "OnMin".
	* \param asFunc The script function to be called. Must be in the current script file. "" = disabled.
	**/
	static void __stdcall SetJointCallback(std::string asJointName, std::string asType,
											std::string asFunc)
	{
		iPhysicsJoint *pJoint = gpScene->GetWorld3D()->GetPhysicsWorld()->GetJoint(asJointName);
		if(pJoint==NULL){
			Warning("Couldn't find joint '%s'\n",asJointName.c_str());return;
		}

		int lType = 0;
		tString sLowName = cString::ToLowerCase(asType);
		if(sLowName == "onmax") lType = 1;
		if(sLowName == "onmin") lType = 2;

		if(lType ==0){
			Warning("Joint callback type '%s' does not exist\n",asType.c_str()); return;
		}

		cScriptJointCallback *pCallback = static_cast<cScriptJointCallback*>(pJoint->GetCallback());
		if(pCallback==NULL)
		{
			pCallback = hplNew( cScriptJointCallback, (gpScene) );
			pJoint->SetCallback(pCallback,true);
		}

		if(lType==1) pCallback->msMaxFunc = asFunc;
		if(lType==2) pCallback->msMinFunc = asFunc;
	}
	SCRIPT_DEFINE_FUNC_3(void, SetJointCallback, string, string, string)

	//-----------------------------------------------------------------------

	/**
	* Breaks a joint.
	* \param asJointName The joint name
	**/
	static void __stdcall BreakJoint(std::string asJointName)
	{
		iPhysicsJoint *pJoint = gpScene->GetWorld3D()->GetPhysicsWorld()->GetJoint(asJointName);
		if(pJoint==NULL){
			Warning("Couldn't find joint '%s'\n",asJointName.c_str());return;
		}

		pJoint->Break();
	}
	SCRIPT_DEFINE_FUNC_1(void, BreakJoint, string)

	//-----------------------------------------------------------------------

	/**
	* Sets if a joint controller is active or not.
	* \param asJointName The joint name
	* \param asCtrlName The controller name
	* \param abActive If the controller is to be active or not.
	**/
	static void __stdcall SetJointControllerActive(std::string asJointName,std::string asCtrlName, bool abActive)
	{
		iPhysicsJoint *pJoint = gpScene->GetWorld3D()->GetPhysicsWorld()->GetJoint(asJointName);
		if(pJoint==NULL){
			Warning("Couldn't find joint '%s'\n",asJointName.c_str());return;
		}

		iPhysicsController *pCtrl = pJoint->GetController(asCtrlName);
		if(pCtrl==NULL){
			Warning("Couldn't find controller %s in joint '%s'\n",asCtrlName.c_str(),asJointName.c_str());return;
		}

		pCtrl->SetActive(abActive);
	}
	SCRIPT_DEFINE_FUNC_3(void, SetJointControllerActive, string, string, bool)

	//-----------------------------------------------------------------------

	/**
	* Change the active controller. All other controllers are set to false.
	* \param asJointName The joint name
	* \param asCtrlName The controller name
	**/
	static void __stdcall ChangeJointController(std::string asJointName,std::string asCtrlName)
	{
		iPhysicsJoint *pJoint = gpScene->GetWorld3D()->GetPhysicsWorld()->GetJoint(asJointName);
		if(pJoint==NULL){
			Warning("Couldn't find joint '%s'\n",asJointName.c_str());return;
		}

		if(pJoint->ChangeController(asCtrlName)==false)
		{
			Warning("Couldn't find controller %s in joint '%s'\n",asCtrlName.c_str(),asJointName.c_str());
			return;
		}
	}
	SCRIPT_DEFINE_FUNC_2(void, ChangeJointController, string, string)

	//-----------------------------------------------------------------------

	/**
	* Sets if a joint controller is active or not.
	* \param asJointName The joint name
	* \param asCtrlName The controller name
	* \param asProperty Property to change, can be "DestValue"
	* \param afValue Value to set it to.
	**/
	static void __stdcall SetJointControllerPropertyFloat(std::string asJointName,std::string asCtrlName,
													std::string asProperty, float afValue)
	{
		iPhysicsJoint *pJoint = gpScene->GetWorld3D()->GetPhysicsWorld()->GetJoint(asJointName);
		if(pJoint==NULL){
			Warning("Couldn't find joint '%s'\n",asJointName.c_str());return;
		}

		iPhysicsController *pCtrl = pJoint->GetController(asCtrlName);
		if(pCtrl==NULL){
			Warning("Couldn't find controller %s in joint '%s'\n",asCtrlName.c_str(),asJointName.c_str());return;
		}

		if(asProperty == "DestValue")
		{
			pCtrl->SetDestValue(afValue);
		}
	}
	SCRIPT_DEFINE_FUNC_4(void, SetJointControllerPropertyFloat, string, string, string, float)

	//-----------------------------------------------------------------------


	/**
	* Gets a property from the joint.
	* Valid properties are:
	* "Angle" The angle between the bodies (in degrees) (Not working for ball joint)
	* "Distance" The distance between the bodies (in meter)
	* "LinearSpeed" The relative linear speed between the bodies (in m/s)
	* "AngularSpeed" The relative angular speed between the bodies (in m/s)
	* "Force" The size of the force (in newton, kg*m/s^2).
	* "MaxLimit" The max limit (meters or degrees)
	* "MinLimit" The in limit (meters or degrees)
	* \param asJointName The joint name
	* \param asProp The property to get
	**/
	static float __stdcall GetJointProperty(std::string asJointName, std::string asProp)
	{
		iPhysicsJoint *pJoint = gpScene->GetWorld3D()->GetPhysicsWorld()->GetJoint(asJointName);
		if(pJoint==NULL){
			Warning("Couldn't find joint '%s'\n",asJointName.c_str());return 0;
		}

		tString sLowProp = cString::ToLowerCase(asProp);

		if(sLowProp == "angle")
		{
			return cMath::ToDeg(pJoint->GetAngle());
		}
		else if(sLowProp == "distance")
		{
			return pJoint->GetDistance();
		}
		else if(sLowProp == "linearspeed")
		{
			return pJoint->GetVelocity().Length();
		}
		else if(sLowProp == "angularspeed")
		{
			return pJoint->GetAngularVelocity().Length();
		}
		else if(sLowProp == "force")
		{
			return pJoint->GetForce().Length();
		}
		/////////////////////////////
		// Min Limit
		else if(sLowProp == "minlimit")
		{
			switch(pJoint->GetType())
			{
			case ePhysicsJointType_Hinge:
				{
					iPhysicsJointHinge *pHingeJoint = static_cast<iPhysicsJointHinge*>(pJoint);
					return cMath::ToDeg(pHingeJoint->GetMinAngle());
				}
			case ePhysicsJointType_Screw:
				{
					iPhysicsJointScrew *pScrewJoint = static_cast<iPhysicsJointScrew*>(pJoint);
					return pScrewJoint->GetMinDistance();
				}
			case ePhysicsJointType_Slider:
				{
					iPhysicsJointSlider *pSliderJoint = static_cast<iPhysicsJointSlider*>(pJoint);
					return pSliderJoint->GetMinDistance();
				}
			}
		}
		/////////////////////////////
		// Max Limit
		else if(sLowProp == "maxlimit")
		{
			switch(pJoint->GetType())
			{
			case ePhysicsJointType_Hinge:
				{
					iPhysicsJointHinge *pHingeJoint = static_cast<iPhysicsJointHinge*>(pJoint);
					return cMath::ToDeg(pHingeJoint->GetMaxAngle());
				}
			case ePhysicsJointType_Screw:
				{
					iPhysicsJointScrew *pScrewJoint = static_cast<iPhysicsJointScrew*>(pJoint);
					return pScrewJoint->GetMaxDistance();
				}
			case ePhysicsJointType_Slider:
				{
					iPhysicsJointSlider *pSliderJoint = static_cast<iPhysicsJointSlider*>(pJoint);
					return pSliderJoint->GetMaxDistance();
				}
			}

		}

		Warning("Joint property '%s' does not exist!\n",asProp.c_str());
		return 0;
	}
	SCRIPT_DEFINE_FUNC_2(float, GetJointProperty, string, string)

	//-----------------------------------------------------------------------

	/**
	* Gets a property from the body.
	* Valid properties are:
	* "Mass" The mass of the body (in kg)
	* "LinearSpeed" The linear speed the body has (in m/s)
	* "AngularSpeed" The angular speed the body has (in m/s)
	* \param asBodyName The body name
	* \param asProp The property to get
	**/
	static float __stdcall GetBodyProperty(std::string asBodyName, std::string asProp)
	{
		iPhysicsBody *pBody = gpScene->GetWorld3D()->GetPhysicsWorld()->GetBody(asBodyName);
		if(pBody==NULL){
			Warning("Couldn't find Body '%s'\n",asBodyName.c_str());return 0;
		}

		tString sLowProp = cString::ToLowerCase(asProp);

		if(sLowProp == "mass")
		{
			return pBody->GetMass();
		}
		else if(sLowProp == "linearspeed")
		{
			return pBody->GetLinearVelocity().Length();
		}
		else if(sLowProp == "angularspeed")
		{
			return pBody->GetAngularVelocity().Length();
		}

		Warning("Body property '%s' does not exist!\n",asProp.c_str());
		return 0;
	}
	SCRIPT_DEFINE_FUNC_2(float, GetBodyProperty, string, string)

	//-----------------------------------------------------------------------

	/**
	* Sets a property to the body.
	* Valid properties are:
	* "Mass" The mass of the body (in kg)
	* "CollideCharacter"	0 = false 1=true
	* \param asBodyName The body name
	* \param asProp The property to get
	* \param afVal The new value of the property
	**/
	static void __stdcall SetBodyProperty(std::string asBodyName, std::string asProp, float afVal)
	{
		iPhysicsBody *pBody = gpScene->GetWorld3D()->GetPhysicsWorld()->GetBody(asBodyName);
		if(pBody==NULL){
			Warning("Couldn't find Body '%s'\n",asBodyName.c_str());return;
		}

		tString sLowProp = cString::ToLowerCase(asProp);

		if(sLowProp == "mass")
		{
			pBody->SetMass(afVal);
			pBody->SetEnabled(true);

			if(afVal == 0)
			{
				pBody->SetLinearVelocity(0);
				pBody->SetAngularVelocity(0);
			}

			return;
		}
		else if(sLowProp == "collidecharacter")
		{
			pBody->SetCollideCharacter(afVal <0.05 ? false : true);
			return;
		}
		else if(sLowProp == "hasgravity")
		{
			pBody->SetCollideCharacter(afVal <0.05 ? false : true);
			return;
		}


		Warning("Body property '%s' does not exist!\n",asProp.c_str());
	}
	SCRIPT_DEFINE_FUNC_3(void, SetBodyProperty, string, string, float)

	//-----------------------------------------------------------------------

	static void __stdcall AttachBodiesWithJoint(std::string asParentName, std::string asChildName, std::string asJointName)
	{
		iPhysicsBody *pParent = gpScene->GetWorld3D()->GetPhysicsWorld()->GetBody(asParentName);
		if(pParent==NULL){
			Warning("Couldn't find Body '%s'\n",asParentName.c_str());return;
		}

		iPhysicsBody *pChild = gpScene->GetWorld3D()->GetPhysicsWorld()->GetBody(asChildName);
		if(pChild==NULL){
			Warning("Couldn't find Body '%s'\n",asChildName.c_str());return;
		}

		iPhysicsWorld *pPhysicsWorld = gpScene->GetWorld3D()->GetPhysicsWorld();

		cVector3f vPivot = (pParent->GetLocalPosition() + pChild->GetLocalPosition()) * 0.5f;
		cVector3f vDir = cMath::Vector3Normalize(pChild->GetLocalPosition() - pParent->GetLocalPosition());

		iPhysicsJointSlider *pJoint = pPhysicsWorld->CreateJointSlider(asJointName,vPivot,vDir,pParent,pChild);

		pJoint->SetMinDistance(-0.01f);
		pJoint->SetMaxDistance(0.01f);
	}
	SCRIPT_DEFINE_FUNC_3(void, AttachBodiesWithJoint, string, string, string)

	//-----------------------------------------------------------------------

	/**
	* Sets a property to the joint.
	* Valid properties are:
	* "MinLimit" The min limit (depends on joint, does not work on ball)
	* "MaxLimit" The max limit (depends on joint, does not work on ball)
	* \param asJointName The body name
	* \param asProp The property to get
	* \param afVal The new value of the property
	**/
	static void __stdcall SetJointProperty(std::string asJointName, std::string asProp, float afVal)
	{
		iPhysicsJoint *pJoint = gpScene->GetWorld3D()->GetPhysicsWorld()->GetJoint(asJointName);
		if(pJoint==NULL){
			Warning("Couldn't find joint '%s'\n",asJointName.c_str());return;
		}

		tString sLowProp = cString::ToLowerCase(asProp);

		if(pJoint->GetChildBody()) pJoint->GetChildBody()->SetEnabled(true);
		if(pJoint->GetParentBody()) pJoint->GetParentBody()->SetEnabled(true);

		/////////////////////////////
		// Min Limit
		if(sLowProp == "minlimit")
		{
			switch(pJoint->GetType())
			{
			case ePhysicsJointType_Hinge:
				{
					iPhysicsJointHinge *pHingeJoint = static_cast<iPhysicsJointHinge*>(pJoint);
					pHingeJoint->SetMinAngle(cMath::ToRad(afVal));
					break;
				}
			case ePhysicsJointType_Screw:
				{
					iPhysicsJointScrew *pScrewJoint = static_cast<iPhysicsJointScrew*>(pJoint);
					pScrewJoint->SetMinDistance(afVal);
					break;
				}
			case ePhysicsJointType_Slider:
				{
					iPhysicsJointSlider *pSliderJoint = static_cast<iPhysicsJointSlider*>(pJoint);
					pSliderJoint->SetMinDistance(afVal);
					break;
				}
			}
		}
		/////////////////////////////
		// Max Limit
		else if(sLowProp == "maxlimit")
		{
			switch(pJoint->GetType())
			{
			case ePhysicsJointType_Hinge:
				{
					iPhysicsJointHinge *pHingeJoint = static_cast<iPhysicsJointHinge*>(pJoint);
					pHingeJoint->SetMaxAngle(cMath::ToRad(afVal));
					break;
				}
			case ePhysicsJointType_Screw:
				{
					iPhysicsJointScrew *pScrewJoint = static_cast<iPhysicsJointScrew*>(pJoint);
					pScrewJoint->SetMaxDistance(afVal);
					break;
				}
			case ePhysicsJointType_Slider:
				{
					iPhysicsJointSlider *pSliderJoint = static_cast<iPhysicsJointSlider*>(pJoint);
					pSliderJoint->SetMaxDistance(afVal);
					break;
				}
			}

		}
		else
		{
			Warning("Joint property '%s' does not exist!\n",asProp.c_str());
		}
	}
	SCRIPT_DEFINE_FUNC_3(void, SetJointProperty, string, string, float)


	//-----------------------------------------------------------------------

	/**
	* Adds a force to the body. This can either be in the bodies local coord system or the world's.
	* \param asBodyName The body name
	* \param asCoordType The coordinate system type. "World" or "Local".
	* \param afX force in the x direction. (in newton, kg*m/s^2)
	* \param afY force in the y direction. (in newton, kg*m/s^2)
	* \param afZ force in the z direction. (in newton, kg*m/s^2)
	**/
	static void __stdcall AddBodyForce(std::string asBodyName, std::string asCoordType,
										float afX, float afY, float afZ)
	{
		iPhysicsBody *pBody = gpScene->GetWorld3D()->GetPhysicsWorld()->GetBody(asBodyName);
		if(pBody==NULL){
			Warning("Couldn't find Body '%s'\n",asBodyName.c_str());return;
		}

		int lType =0;
		tString sLowType = cString::ToLowerCase(asCoordType);

		if(sLowType == "world") lType = 1;
		else if(sLowType == "local") lType =2;

		if(lType==0){
			Warning("Coord system type '%s' is not valid.\n",asCoordType.c_str());
			return;
		}

		if(lType==1)
		{
			pBody->AddForce(cVector3f(afX,afY,afZ));
		}
		else if(lType==2)
		{
			cVector3f vWorldForce = cMath::MatrixMul(pBody->GetLocalMatrix().GetRotation(),
														cVector3f(afX,afY,afZ));
			pBody->AddForce(vWorldForce);
		}
	}
	SCRIPT_DEFINE_FUNC_5(void, AddBodyForce, string, string, float, float, float)

	//-----------------------------------------------------------------------

	/**
	* Adds an impule (a change in velocity) to the body. This can either be in the bodies local coord system or the world's.
	* \param asBodyName The body name
	* \param asCoordType The coordinate system type. "World" or "Local".
	* \param afX velocity in the x direction. (in m/s)
	* \param afY velocity in the y direction. (in m/s)
	* \param afZ velocity in the z direction. (in m/s)
	**/
	static void __stdcall AddBodyImpulse(std::string asBodyName, std::string asCoordType,
		float afX, float afY, float afZ)
	{
		iPhysicsBody *pBody = gpScene->GetWorld3D()->GetPhysicsWorld()->GetBody(asBodyName);
		if(pBody==NULL){
			Warning("Couldn't find Body '%s'\n",asBodyName.c_str());return;
		}

		int lType =0;
		tString sLowType = cString::ToLowerCase(asCoordType);

		if(sLowType == "world") lType = 1;
		else if(sLowType == "local") lType =2;

		if(lType==0){
			Warning("Coord system type '%s' is not valid.\n",asCoordType.c_str());
			return;
		}

		if(lType==1)
		{
			pBody->AddImpulse(cVector3f(afX,afY,afZ));
		}
		else if(lType==2)
		{
			cVector3f vWorldForce = cMath::MatrixMul(pBody->GetLocalMatrix().GetRotation(),
				cVector3f(afX,afY,afZ));
			pBody->AddImpulse(vWorldForce);
		}
	}
	SCRIPT_DEFINE_FUNC_5(void, AddBodyImpulse, string, string, float, float, float)

	//-----------------------------------------------------------------------


	/////////////////////////////////////////////////////////////////////////
	/////// LOCAL VARS //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	static void __stdcall CreateLocalVar(std::string asName, int alVal)
	{
		if(gpScene->GetLocalVar(asName)==NULL)
		{
			cScriptVar* pVar = gpScene->CreateLocalVar(asName);
			pVar->mlVal = alVal;
		}
	}
	SCRIPT_DEFINE_FUNC_2(void, CreateLocalVar, string, int)

	static void __stdcall SetLocalVar(std::string asName, int alVal)
	{
		cScriptVar* pVar = gpScene->CreateLocalVar(asName);
		pVar->mlVal = alVal;
	}
	SCRIPT_DEFINE_FUNC_2(void, SetLocalVar, string, int)

	static void __stdcall AddLocalVar(std::string asName, int alVal)
	{
		cScriptVar* pVar = gpScene->CreateLocalVar(asName);
		pVar->mlVal += alVal;
	}
	SCRIPT_DEFINE_FUNC_2(void, AddLocalVar, string, int)

	static int __stdcall GetLocalVar(std::string asName)
	{
		cScriptVar* pVar = gpScene->GetLocalVar(asName);
		if(pVar==NULL)
		{
			Error("Couldn't find local var '%s'\n",asName.c_str());
			return 0;
		}
		return pVar->mlVal;
	}
	SCRIPT_DEFINE_FUNC_1(int, GetLocalVar, string)

	//-----------------------------------------------------------------------
	/////////////////////////////////////////////////////////////////////////
	/////// GLOBAL VARS //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	static void __stdcall CreateGlobalVar(std::string asName, int alVal)
	{
		if(gpScene->GetGlobalVar(asName)==NULL)
		{
			cScriptVar* pVar = gpScene->CreateGlobalVar(asName);
			pVar->mlVal = alVal;
		}
	}
	SCRIPT_DEFINE_FUNC_2(void, CreateGlobalVar, string, int)

	static void __stdcall SetGlobalVar(std::string asName, int alVal)
	{
		cScriptVar* pVar = gpScene->CreateGlobalVar(asName);
		pVar->mlVal = alVal;
	}
	SCRIPT_DEFINE_FUNC_2(void, SetGlobalVar, string, int)


	static void __stdcall AddGlobalVar(std::string asName, int alVal)
	{
		cScriptVar* pVar = gpScene->CreateGlobalVar(asName);
		pVar->mlVal += alVal;
	}
	SCRIPT_DEFINE_FUNC_2(void, AddGlobalVar, string, int)


	static int __stdcall GetGlobalVar(std::string asName)
	{
		cScriptVar* pVar = gpScene->GetGlobalVar(asName);
		if(pVar==NULL)
		{
			Error("Couldn't find global var '%s'\n",asName.c_str());
			return 0;
		}
		return pVar->mlVal;
	}
	SCRIPT_DEFINE_FUNC_1(int, GetGlobalVar, string)

	//-----------------------------------------------------------------------



	void cScriptFuncs::Init(	cGraphics* apGraphics,
				cResources *apResources,
				cSystem *apSystem,
				cInput *apInput,
				cScene *apScene,
				cSound *apSound,
				cGame *apGame
			)
	{
		gpGraphics = apGraphics;
		gpResources = apResources;
		gpSystem = apSystem;
		gpInput = apInput;
		gpScene = apScene;
		gpSound = apSound;
		gpGame = apGame;

		//General
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(Print));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(FloatToString));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(IntToString));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(RandFloat));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(RandInt));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(StringContains));

		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(ResetLogicTimer));

		//Renderer
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetAmbientColor));

		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetSkybox));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetSkyboxActive));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetSkyboxColor));

		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(CreateParticleSystemOnCamera));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetFogActive));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetFogCulling));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetFogProperties));

		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetSectorProperties));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetSectorPortalActive));


		//Resources
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(PreloadSound));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(Translate));

		//Mesh Entity
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetMeshActive));

		//Beams
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(CreateBeam));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(DestroyBeam));

		//Particle systems
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetParticleSystemActive));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(CreateParticleSystem));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(KillParticleSystem));

		//Light
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(FadeLight3D));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(AttachBillboardToLight3D));

		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetLight3DVisible));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetLight3DFlickerActive));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetLight3DFlicker));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetLight3DOnlyAffectInSector));

		//Sound
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(PlayMusic));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(StopMusic));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(PlaySoundEntity));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(StopSoundEntity));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(FadeInSoundEntity));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(FadeOutSoundEntity));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(PlayGuiSound));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(CreateSoundEntity));


		//Physics
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetJointCallback));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(BreakJoint));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(GetJointProperty));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(GetBodyProperty));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetBodyProperty));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetJointProperty));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(AttachBodiesWithJoint));


		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetJointControllerActive));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(ChangeJointController));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetJointControllerPropertyFloat));


		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(AddBodyForce));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(AddBodyImpulse));

		//Local vars
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(CreateLocalVar));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetLocalVar));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(AddLocalVar));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(GetLocalVar));

		//Global vars
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(CreateGlobalVar));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(SetGlobalVar));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(AddGlobalVar));
		gpSystem->GetLowLevel()->AddScriptFunc(SCRIPT_REGISTER_FUNC(GetGlobalVar));
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------



	//-----------------------------------------------------------------------

}
