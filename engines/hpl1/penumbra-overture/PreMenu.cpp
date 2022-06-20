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
 * This file is part of Penumbra Overture.
 */

#include "hpl1/penumbra-overture/PreMenu.h"

#include "hpl1/penumbra-overture/ButtonHandler.h"
#include "hpl1/penumbra-overture/GraphicsHelper.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MainMenu.h"
#include "hpl1/penumbra-overture/MapHandler.h"

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPreMenu::cPreMenu(cInit *apInit) : iUpdateable("PreMenu") {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	// Load fonts
	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");
	mpTextFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("font_computer.fnt");

	// Load config file
	LoadConfig();

	// Load text
	tWString sText = kTranslate("MainMenu", "PreMenuText");
	mpTextFont->GetWordWrapRows(750, 19, 17, sText, &mvTextRows);

	mlMaxChars = 0;
	for (size_t i = 0; i < mvTextRows.size(); ++i)
		mlMaxChars += (int)mvTextRows[i].length();

	Reset();
}

//-----------------------------------------------------------------------

cPreMenu::~cPreMenu(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

void cPreMenu::LoadConfig() {
	////////////////////////////////////////////////
	// Load the document
	TiXmlDocument *pXmlDoc = hplNew(TiXmlDocument, ("config/startup.cfg"));
	if (pXmlDoc->LoadFile() == false) {
		Error("Couldn't load XML document 'config/startup.cfg'\n");
		hplDelete(pXmlDoc);
	}

	////////////////////////////////////////////////
	// Load the root
	TiXmlElement *pRootElem = pXmlDoc->FirstChildElement();
	if (pRootElem == NULL) {
		Error("Couldn't load root from XML document 'config/startup.cfg'\n");
		hplDelete(pXmlDoc);
	}

	////////////////////////////////////////////////
	// Load the Main element.
	TiXmlElement *pMainElem = pRootElem->FirstChildElement("Main");
	if (pMainElem == NULL) {
		Error("Couldn't load Main element from XML document 'config/startup.cfg'\n");
		hplDelete(pXmlDoc);
	}

	mbShowText = cString::ToBool(pMainElem->Attribute("ShowText"), false);

	////////////////////////////////////////////////
	// Load the Logos element.
	TiXmlElement *pLogosParentElem = pRootElem->FirstChildElement("Logos");
	if (pLogosParentElem == NULL) {
		Error("Couldn't load Logs element from XML document 'config/startup.cfg'\n");
		hplDelete(pXmlDoc);
	}

	////////////////////////////////////////////////
	// Get logos
	TiXmlElement *pLogoElem = pLogosParentElem->FirstChildElement("Logo");
	for (; pLogoElem != NULL; pLogoElem = pLogoElem->NextSiblingElement("Logo")) {
		tString sFile = cString::ToString(pLogoElem->Attribute("File"), "");
		mvTexNames.push_back(sFile);

		// Log("Texture %s\n",sFile.c_str());
	}

	hplDelete(pXmlDoc);
}

//-----------------------------------------------------------------------

void cPreMenu::Reset() {
	mbActive = false;
	mfAlpha = 0;
	mfAlphaAdd = 0.6f;

	mlState = 0;
	mlCurrentLogo = 0;

	mlCurrentTextChar = 0;
	mfNewCharCount = 0;
	mfClickCount = 0;

	mfFontColor = 1;
	mfLogoSizeFactor = 1.05f;
	mvecLogoSize = cVector3f(1024, 301, 0) * mfLogoSizeFactor;
	mfLogoFade = 1;
	mfEpFade = 0;

	mvecLastTextPos = cVector3f(0);
	mfLastTextSize = 0;
	mfLastTextColor = 1;
	mfLastTextSpeed1 = 0.25f;
	mfLastTextSpeed2 = 0.5f;

	mfAdd = 0.002f;

	mbFadeWindSound = false;
	mbFadeRainSound = false;

	mbPlayingRainSound = false;
	mbPlayingWindSound = false;
	mbPlayingMusic = false;
	mbPlayingThunderSound = false;

	mfRaindropFade = 0;

	mbFlash = true;

	mvRaindropVector.resize(100);

	mpRaindropGfx = mpInit->mpGame->GetGraphics()->GetDrawer()->CreateGfxObject("menu_rain_drop.jpg", "diffadditive2d");
	mpFlashGfx = mpInit->mpGame->GetGraphics()->GetDrawer()->CreateGfxObject("effect_white.jpg", "diffadditive2d");

	for (int i = 0; i < (int)mvRaindropVector.size(); ++i) {
		cRaindrop *pRaindrop = &(mvRaindropVector[i]);
		pRaindrop->vPos = cVector2f(cMath::RandRectf(-20, 800), -50);
		pRaindrop->vDir = cVector2f(cMath::RandRectf(20, 100), 600);
		pRaindrop->vDir.Normalise();
		pRaindrop->fColor = 1;
		pRaindrop->fLength = cMath::RandRectf(10, 40);
		pRaindrop->mpGfx = mpRaindropGfx;
	}

	mpWindSound = NULL;
	mpRainSound = NULL;
}

//-----------------------------------------------------------------------

void cPreMenu::OnPostSceneDraw() {
	mpInit->mpGraphicsHelper->ClearScreen(cColor(0, 0));

	if (mlState == 1 || mlState == 2) {
		mpInit->mpGraphicsHelper->DrawTexture(mvTextures[mlCurrentLogo], cVector3f(0, 0, -10), cVector3f(800, 600, 0),
											  cColor(mfAlpha, 1));
	}
	if (mlState == 4) {
		mpInit->mpGraphicsHelper->DrawTexture(mpLogoTexture, cVector3f(400, 300, -10) - (mvecLogoSize / 2), mvecLogoSize, cColor(mfLogoFade, 1));
		mpInit->mpGraphicsHelper->DrawTexture(mpEpTexture, cVector3f(276, 440, -10), cVector3f(248, 46, 0), cColor(mfEpFade, 1));
		if (mbFlash)
			mpInit->mpGame->GetGraphics()->GetDrawer()->DrawGfxObject(mpFlashGfx, cVector3f(0, 0, 10), cVector2f(800, 600), cColor(1, 0.9f));
	}

	if (mlState == 1 || mlState == 2 || mlState == 4) {
		for (int i = 0; i < (int)mvRaindropVector.size(); ++i) {
			cRaindrop *pRaindrop = &(mvRaindropVector[i]);
			if (pRaindrop->fColor >= 0) {
				mpInit->mpGame->GetGraphics()->GetDrawer()->DrawGfxObject(pRaindrop->mpGfx, cVector3f(pRaindrop->vPos) + cVector3f(0, 0, 5), cVector2f(20, 6 * pRaindrop->fLength), cColor(pRaindrop->fColor, 1), false, false); //, -0.2f - 1.5f*pRaindrop->fAngle );
																																																								 // mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawLine2D(pRaindrop->vPos, pRaindrop->vPos+(pRaindrop->vDir*pRaindrop->fLength),0,cColor(pRaindrop->fColor,pRaindrop->fColor));
			}
		}
	}
}

//-----------------------------------------------------------------------

void cPreMenu::OnDraw() {
	//////////////////////////////////
	// STATE 1 (TEXT)
	if (mlState == 0 || mlState == 3 || mlState == 5) {
		int lPrevStart = 0;
		int lCharCount = 0;
		for (size_t i = 0; i < mvTextRows.size(); ++i) {
			lPrevStart = lCharCount;
			lCharCount += (int)mvTextRows[i].length();
			if (lCharCount >= mlCurrentTextChar) {
				tWString sText = mvTextRows[i].substr(0, mlCurrentTextChar - lPrevStart);
				mpTextFont->Draw(cVector3f(25, 90 + 19 * (float)i, 10) + mvecLastTextPos, 17 + mfLastTextSize, cColor(mfLastTextColor, mfFontColor, mfFontColor, 1), // cColor(0.7f,1,0.7f,1),
								 eFontAlign_Left, sText.c_str());
				if (mlState == 5) {
					// First pair of shadow texts
					mpTextFont->Draw(cVector3f(25 + mfLastTextSpeed1, 90 + 19 * (float)i, 0) + mvecLastTextPos, 17 + (mfLastTextSize), cColor(mfLastTextColor + 0.5f, 0, 0, 0.25f), // cColor(0.7f,1,0.7f,1),
									 eFontAlign_Left, sText.c_str());
					mpTextFont->Draw(cVector3f(25 + 1.5f * mfLastTextSpeed1, 90 + 19 * (float)i, 0) + mvecLastTextPos, 17 + (mfLastTextSize), cColor(mfLastTextColor + 0.5f, 0, 0, 0.20f), // cColor(0.7f,1,0.7f,1),
									 eFontAlign_Left, sText.c_str());
					// Second pair
					mpTextFont->Draw(cVector3f(25 + 3 * mfLastTextSpeed1, 90 + 19 * (float)i, 0) + mvecLastTextPos, 17 + (mfLastTextSize), cColor(mfLastTextColor + 0.25f, 0, 0, 0.15f), // cColor(0.7f,1,0.7f,1),
									 eFontAlign_Left, sText.c_str());
					mpTextFont->Draw(cVector3f(25 + 5 * mfLastTextSpeed1, 90 + 19 * (float)i, 0) + mvecLastTextPos, 17 + (mfLastTextSize), cColor(mfLastTextColor + 0.25f, 0, 0, 0.1f), // cColor(0.7f,1,0.7f,1),
									 eFontAlign_Left, sText.c_str());

					// mpTextFont->Draw(cVector3f(25-mfLastTextPos,90+19*(float)i-mfLastTextPos,0),cVector2f(17,17+(mfLastTextSize)),cColor(mfLastTextColor,mfFontColor,mfFontColor,0.25f),//cColor(0.7f,1,0.7f,1),
					//				eFontAlign_Left,sText.c_str());
				}

				break;
			} else {
				mpTextFont->Draw(cVector3f(25, 90 + 19 * (float)i, 10), 17, cColor(mfFontColor, mfFontColor), // cColor(0.7f,1,0.7f,1),
								 eFontAlign_Left, mvTextRows[i].c_str());
			}
		}
	}
}

//-----------------------------------------------------------------------

void cPreMenu::Update(float afTimeStep) {

	mfStateTimer += afTimeStep;

	// Update wind sound fading
	if (mbFadeWindSound) {
		cSoundEntry *pEntry = mpInit->mpGame->GetSound()->GetSoundHandler()->GetEntryFromSound(mpWindSound);
		if (pEntry) {
			if (pEntry->mfNormalVolumeFadeSpeed > 0) {
				if (pEntry->mfNormalVolume < pEntry->mfNormalVolumeFadeDest) {
					pEntry->mfNormalVolume += pEntry->mfNormalVolumeFadeSpeed * afTimeStep;
					// hpl::Log ("Vol:%f\n",pEntry->mfNormalVolume);
				} else {
					pEntry->mfNormalVolume = pEntry->mfNormalVolumeFadeDest;
					mbFadeWindSound = false;
				}

			} else if (pEntry->mfNormalVolumeFadeSpeed < 0) {
				if (pEntry->mfNormalVolume > pEntry->mfNormalVolumeFadeDest) {
					pEntry->mfNormalVolume += pEntry->mfNormalVolumeFadeSpeed * afTimeStep;
					// hpl::Log ("Vol:%f\n",pEntry->mfNormalVolume);
				} else {
					pEntry->mfNormalVolume = pEntry->mfNormalVolumeFadeDest;
					mbFadeWindSound = false;
				}
			} else
				mbFadeWindSound = false;
		}
	}

	// Update rain sound fading
	if (mbFadeRainSound) {
		cSoundEntry *pEntry = mpInit->mpGame->GetSound()->GetSoundHandler()->GetEntryFromSound(mpRainSound);
		if (pEntry) {
			if (pEntry->mfNormalVolumeFadeSpeed > 0) {
				if (pEntry->mfNormalVolume < pEntry->mfNormalVolumeFadeDest) {
					pEntry->mfNormalVolume += pEntry->mfNormalVolumeFadeSpeed * afTimeStep;
					// hpl::Log ("Vol:%f\n",pEntry->mfNormalVolume);
				} else {
					pEntry->mfNormalVolume = pEntry->mfNormalVolumeFadeDest;
					mbFadeRainSound = false;
				}

			} else if (pEntry->mfNormalVolumeFadeSpeed < 0) {
				if (pEntry->mfNormalVolume > pEntry->mfNormalVolumeFadeDest) {
					pEntry->mfNormalVolume += pEntry->mfNormalVolumeFadeSpeed * afTimeStep;
					// hpl::Log ("Vol:%f\n",pEntry->mfNormalVolume);
				} else {
					pEntry->mfNormalVolume = pEntry->mfNormalVolumeFadeDest;
					mbFadeRainSound = false;
				}
			} else
				mbFadeRainSound = false;

			mfRaindropFade = mpRainSound->GetVolume();
		} else {
			mfRaindropFade = 1;
			mbFadeRainSound = false;
		}
	}

	// Update raindrops
	for (int i = 0; i < (int)mvRaindropVector.size(); ++i) {
		cRaindrop *pRaindrop = &(mvRaindropVector[i]);
		pRaindrop->vPos += pRaindrop->vDir * (1.75f * pRaindrop->fLength);
		pRaindrop->vPos.x += cMath::RandRectf(0, 0.05f);

		if (pRaindrop->vPos.y > 600) {
			pRaindrop->vPos = cVector2f(cMath::RandRectf(-60, 800), -80);
			pRaindrop->vDir = cVector2f(cMath::RandRectf(20, 100), 600);
			pRaindrop->vDir.Normalise();
			pRaindrop->fLength = cMath::RandRectf(30, 70);
			pRaindrop->fColor = cMath::RandRectf(0.5f, 1) * mfRaindropFade;
		}
	}

	//////////////////////////////////
	// STATE 0 (Premenu Text)
	if (mlState == 0) {
		if (mbShowText == false) {
			mlState = 1;
			mfStateTimer = 0;
			return;
		}
		if (mpWindSound == NULL)
			mpWindSound = mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_wind1", true, 0);

		if (mpWindSound && !mbPlayingWindSound) {
			cSoundEntry *pEntry = mpInit->mpGame->GetSound()->GetSoundHandler()->GetEntryFromSound(mpWindSound);
			if (pEntry) {
				pEntry->mfNormalVolumeFadeDest = 1;
				pEntry->mfNormalVolumeFadeSpeed = 0.2f;
				mbFadeWindSound = true;
			}
			mbPlayingWindSound = true;
		}

		if (mlCurrentTextChar < mlMaxChars) {
			mfNewCharCount -= afTimeStep;
			mfClickCount -= afTimeStep;

			if (mfNewCharCount <= 0) {
				if (mfClickCount <= 0) {
					mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_type", false, 1);
					mfClickCount = (1.0f / 8.0f) + cMath::RandRectf(-0.05f, 0.05f);
				}

				mlCurrentTextChar++;
				mfNewCharCount = 1.0f / 19.0f;

				if (mlCurrentTextChar == mlMaxChars) {
					mlState = 3;
					mfStateTimer = 0;
				}

				int lCharCount = 0;
				for (size_t i = 0; i < mvTextRows.size(); ++i) {
					lCharCount += (int)mvTextRows[i].length();
					if (lCharCount == mlCurrentTextChar) {
						if (i + 1 < mvTextRows.size() && mvTextRows[i + 1].size() == 0) {
							mfNewCharCount = 1.0f;
						}
					} else if (lCharCount > mlCurrentTextChar) {
						break;
					}
				}
			}
		}
	}
	/////////////////////
	// STATE 1 (Logos)
	else if (mlState == 1) {
		if (mpRainSound == NULL)
			mpRainSound = mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_rain1", true, 0);

		if (mpWindSound && mbPlayingWindSound) {
			cSoundEntry *pEntry = mpInit->mpGame->GetSound()->GetSoundHandler()->GetEntryFromSound(mpWindSound);
			if (pEntry) {
				pEntry->mfNormalVolumeFadeDest = 0;
				pEntry->mfNormalVolumeFadeSpeed = -0.5f;
				mbFadeWindSound = true;
			}
			mbPlayingWindSound = false;
		}
		if (mpRainSound && !mbPlayingRainSound) {
			cSoundEntry *pEntry = mpInit->mpGame->GetSound()->GetSoundHandler()->GetEntryFromSound(mpRainSound);
			if (pEntry) {
				pEntry->mfNormalVolumeFadeDest = 1;
				pEntry->mfNormalVolumeFadeSpeed = 0.2f;
				mbFadeRainSound = true;
			}
			mbPlayingRainSound = true;
		}

		if (!mbPlayingMusic) {
			mpInit->mpGame->GetSound()->GetMusicHandler()->Play("music_theme.ogg", 1, 0, false);
			mbPlayingMusic = true;
		}

		mfAlpha += afTimeStep * mfAlphaAdd;

		if (mfAlphaAdd > 0) {
			if (mfAlpha > 1) {
				mfAlpha = 1;
				mfAlphaAdd = -mfAlphaAdd;
			}
		} else {
			if (mfAlpha < 0) {
				mfAlpha = 0;
				mfAlphaAdd = -mfAlphaAdd;

				mlCurrentLogo++;
				if (mlCurrentLogo >= (int)mvTextures.size()) {
					mlState = 4;
					mfStateTimer = 0;
				}
			}
		}
	}
	//////////////////////////////////
	// STATE 2 (FAST FADE LOGO)
	else if (mlState == 2) {
		mfAlpha -= cMath::Abs(mfAlphaAdd) * afTimeStep;
		if (mfAlpha <= 0) {
			mfStateTimer = 0;
			mlState = 1;
			mlCurrentLogo++;
			if (mlCurrentLogo >= (int)mvTextures.size())
				mlState = 4;
		}
	}
	//////////////////////////////////
	// STATE 3 (Text Pause)
	else if (mlState == 3) {
		if (mpWindSound && mbPlayingWindSound) {
			cSoundEntry *pEntry = mpInit->mpGame->GetSound()->GetSoundHandler()->GetEntryFromSound(mpWindSound);
			if (pEntry) {
				// pEntry->mfNormalVolumeMul= 0;
				pEntry->mfNormalVolumeFadeDest = 0;
				pEntry->mfNormalVolumeFadeSpeed = -0.2f;
				// mpWindSound->Stop();
				mbFadeWindSound = true;
			}
			mbPlayingWindSound = false;
		}

		mlCurrentTextChar = mlMaxChars;
		mfNewCharCount += afTimeStep;

		if (mfStateTimer > 1) {
			mlState = 5;
			mfStateTimer = 0;
		}

	}
	///////////////////
	// STATE 5 (Text Vanishing)
	else if (mlState == 5) {
		mlCurrentTextChar = mlMaxChars;

		if (mpWindSound && mbPlayingWindSound) {
			cSoundEntry *pEntry = mpInit->mpGame->GetSound()->GetSoundHandler()->GetEntryFromSound(mpWindSound);
			if (pEntry) {
				pEntry->mfNormalVolumeFadeDest = 0;
				pEntry->mfNormalVolumeFadeSpeed = -0.1f;
			}
			mbFadeWindSound = true;
			mbPlayingWindSound = false;
		}

		mfFontColor -= afTimeStep;
		mvecLastTextPos += cVector2f(10 * afTimeStep, -2.5f * afTimeStep);
		mfLastTextSize += 1.05f * afTimeStep;
		mfLastTextSpeed1 += 4 * afTimeStep;
		mfLastTextSpeed2 += 8 * afTimeStep;
		mfLastTextColor -= 0.5f * afTimeStep;

		if (mfStateTimer > 0.5f) {
			if (!mbPlayingMusic) {
				mpInit->mpGame->GetSound()->GetMusicHandler()->Play("music_theme.ogg", 1, 0, false);
				mbPlayingMusic = true;
			}
		}

		if (mfStateTimer > 3.5f) {
			mlState = 1;
			mfStateTimer = 0;
		}

	}
	/////////////////////////////////
	// STATE 4 (PENUMBRA TITLES)
	else if (mlState == 4) {
		if (!mbPlayingThunderSound) {
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_thunder1", false, 1);
			mbPlayingThunderSound = true;
		}

		if (mbFlash && mfStateTimer >= 0.2f)
			mbFlash = false;

		if (mfStateTimer > 0.5f && mfStateTimer < 1.5f)
			if (mfEpFade < 1)
				mfEpFade += afTimeStep;

		if (mfStateTimer < 1) {
			mfAdd = 0.2f * afTimeStep;
		} else if (mfStateTimer < 1.5f) {
			mfAdd = 0.08f * afTimeStep;
		} else if (mfStateTimer < 1.75f) {
			mfAdd = 0.05f * afTimeStep;
		} else if (mfStateTimer >= 1.75f) {
			mfAdd = 0.03f * afTimeStep;

			mfLogoFade -= 0.25f * afTimeStep;
			mfEpFade = mfLogoFade;
		}

		mfLogoSizeFactor -= mfAdd;
		mvecLogoSize = cVector3f(1024, 301, 0) * mfLogoSizeFactor;

		if (mfStateTimer > 5) {
			if (mfRaindropFade > 0)
				mfRaindropFade -= afTimeStep;
			else
				SetActive(false);
		}
	}
}

//-----------------------------------------------------------------------

void cPreMenu::OnMouseDown(eMButton aButton) {
	OnButtonDown();
}

//-----------------------------------------------------------------------

void cPreMenu::OnButtonDown() {
	if (mlState == 0) {
		mfStateTimer = 0;
		mlState = 3;
	}
	if (mlState == 1) {
		mfStateTimer = 0;
		mlState = 2;
	}
	if (mlState == 5) {
		mfStateTimer = 0;
		mlState = 1;
	}
	if (mlState == 4)
		SetActive(false);
}

//-----------------------------------------------------------------------

void cPreMenu::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;

	if (mbActive) {
		mpInit->mpGame->GetUpdater()->SetContainer("PreMenu");
		mpInit->mpGame->GetScene()->SetDrawScene(false);
		mpInit->mpGame->GetScene()->SetUpdateMap(false);
		if (mpInit->mbHasHaptics)
			mpInit->mpGame->GetHaptic()->GetLowLevel()->SetUpdateShapes(false);
		mpInit->mpButtonHandler->ChangeState(eButtonHandlerState_PreMenu);

		for (size_t i = 0; i < mvTexNames.size(); ++i) {
			iTexture *pTex = mpInit->mpGame->GetResources()->GetTextureManager()->Create2D(mvTexNames[i], false);
			if (pTex)
				mvTextures.push_back(pTex);
		}

		mpLogoTexture = mpInit->mpGame->GetResources()->GetTextureManager()->Create2D("title_logo.jpg", false);
		mpEpTexture = mpInit->mpGame->GetResources()->GetTextureManager()->Create2D("title_ep1.jpg", false);

		mfAlpha = 0;
	} else {
		mvRaindropVector.clear();
		if (mpRaindropGfx)
			mpInit->mpGame->GetGraphics()->GetDrawer()->DestroyGfxObject(mpRaindropGfx);
		if (mpFlashGfx)
			mpInit->mpGame->GetGraphics()->GetDrawer()->DestroyGfxObject(mpFlashGfx);

		mpRaindropGfx = NULL;
		mpFlashGfx = NULL;

		if (mpLogoTexture)
			mpInit->mpGame->GetResources()->GetTextureManager()->Destroy(mpLogoTexture);
		if (mpEpTexture)
			mpInit->mpGame->GetResources()->GetTextureManager()->Destroy(mpEpTexture);

		mpLogoTexture = NULL;
		mpEpTexture = NULL;

		for (size_t i = 0; i < mvTextures.size(); ++i) {
			mpInit->mpGame->GetResources()->GetTextureManager()->Destroy(mvTextures[i]);
		}
		mvTextures.clear();

		if (mpInit->mbShowMenu) {
			if (mpInit->mpGame->GetSound()->GetSoundHandler()->IsPlaying("gui_wind1"))
				mpInit->mpGame->GetSound()->GetSoundHandler()->Stop("gui_wind1");

			if (mpRainSound) {
				cSoundEntry *pEntry = mpInit->mpGame->GetSound()->GetSoundHandler()->GetEntryFromSound(mpRainSound);
				if (pEntry)
					pEntry->mfNormalVolume = 1.0f;
			}

			mpInit->mpMainMenu->SetActive(true);
		} else {
			if (mpInit->mpGame->GetSound()->GetSoundHandler()->IsPlaying("gui_rain1"))
				mpInit->mpGame->GetSound()->GetSoundHandler()->Stop("gui_rain1");

			if (mpInit->mpGame->GetSound()->GetSoundHandler()->IsPlaying("gui_wind1"))
				mpInit->mpGame->GetSound()->GetSoundHandler()->Stop("gui_wind1");
			mpInit->mpGame->GetSound()->GetMusicHandler()->Stop(2);

			mpInit->mpGame->GetUpdater()->SetContainer("Default");
			mpInit->mpGame->GetScene()->SetDrawScene(true);
			mpInit->mpGame->GetScene()->SetUpdateMap(true);
			if (mpInit->mbHasHaptics)
				mpInit->mpGame->GetHaptic()->GetLowLevel()->SetUpdateShapes(true);
			mpInit->mpButtonHandler->ChangeState(eButtonHandlerState_Game);

			mpInit->mpMapHandler->Load(mpInit->msStartMap, mpInit->msStartLink);
		}

		mpRainSound = NULL;
		mpWindSound = NULL;
	}
}

//-----------------------------------------------------------------------

void cPreMenu::OnExit() {
	SetActive(false);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
