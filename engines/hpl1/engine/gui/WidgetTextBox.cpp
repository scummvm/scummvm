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

#include "hpl1/engine/gui/WidgetTextBox.h"

#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/graphics/font_data.h"

#include "hpl1/engine/system/String.h"

#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiGfxElement.h"
#include "hpl1/engine/gui/GuiSet.h"
#include "hpl1/engine/gui/GuiSkin.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cWidgetTextBox::cWidgetTextBox(cGuiSet *apSet, cGuiSkin *apSkin) : iWidget(eWidgetType_TextBox, apSet, apSkin) {
	LoadGraphics();

	mlMarkerCharPos = -1;
	mlSelectedTextEnd = -1;

	mlFirstVisibleChar = 0;
	mlVisibleCharSize = 0;

	mfTextMaxSize = 0;
	mfMaxTextSizeNeg = 0;

	mlMaxCharacters = -1;

	mlVisibleCharSize = 0;

	mbPressed = false;

	mbCanEdit = true;

	mpPointerGfx = mpSkin->GetGfx(eGuiSkinGfx_PointerText);
}

//-----------------------------------------------------------------------

cWidgetTextBox::~cWidgetTextBox() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWidgetTextBox::SetMaxTextLength(int alLength) {
	if (mlMaxCharacters == alLength)
		return;

	mlMaxCharacters = alLength;

	if (mlMaxCharacters >= 0 && (int)msText.size() > mlMaxCharacters) {
		SetText(cString::SubW(msText, 0, mlMaxCharacters));

		if (mlSelectedTextEnd >= mlMaxCharacters)
			mlSelectedTextEnd = mlMaxCharacters - 1;
		if (mlMarkerCharPos >= mlMaxCharacters)
			mlMarkerCharPos = mlMaxCharacters - 1;

		OnChangeText();
	}
}

//-----------------------------------------------------------------------

cVector2f cWidgetTextBox::GetBackgroundSize() {
	return cVector2f(mvSize.x - mvGfxCorners[0]->GetActiveSize().x -
						 mvGfxCorners[1]->GetActiveSize().x,
					 mvSize.y - mvGfxCorners[0]->GetActiveSize().y -
						 mvGfxCorners[2]->GetActiveSize().y);
}

//-----------------------------------------------------------------------

void cWidgetTextBox::SetMaxTextSizeNeg(float afX) {
	mfMaxTextSizeNeg = afX;

	OnChangeSize();
}

//-----------------------------------------------------------------------

void cWidgetTextBox::SetCanEdit(bool abX) {
	mbCanEdit = abX;

	if (mbCanEdit)
		mpPointerGfx = mpSkin->GetGfx(eGuiSkinGfx_PointerText);
	else
		mpPointerGfx = mpSkin->GetGfx(eGuiSkinGfx_PointerNormal);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

int cWidgetTextBox::GetLastCharInSize(int alStartPos, float afMaxSize, float afLengthAdd) {
	int lCharPos = (int)msText.size();
	float fLength = 0;
	int lFirst = mpDefaultFontType->getFirstChar();
	int lLast = mpDefaultFontType->getLastChar();
	for (int i = alStartPos; i < (int)msText.size(); ++i) {
		if (i < lFirst || i > lLast)
			continue;

		Glyph *pGlyph = mpDefaultFontType->getGlyph(msText[i] - lFirst);
		if (pGlyph == NULL)
			continue;

		fLength += pGlyph->_advance * mvDefaultFontSize.x;
		if (fLength + afLengthAdd >= afMaxSize) {
			lCharPos = i;
			break;
		}
	}

	return lCharPos;
}

//-----------------------------------------------------------------------

int cWidgetTextBox::GetFirstCharInSize(int alStartPos, float afMaxSize, float afLengthAdd) {
	int lCharPos = 0;
	float fLength = 0;
	int lFirst = mpDefaultFontType->getFirstChar();
	int lLast = mpDefaultFontType->getLastChar();
	for (int i = alStartPos; i >= 0; --i) {
		if (i < lFirst || i > lLast)
			continue;

		Glyph *pGlyph = mpDefaultFontType->getGlyph(msText[i] - lFirst);
		if (pGlyph == NULL)
			continue;

		fLength += pGlyph->_advance * mvDefaultFontSize.x;
		if (fLength + afLengthAdd >= afMaxSize) {
			lCharPos = i;
			break;
		}
	}

	return lCharPos;
}

//-----------------------------------------------------------------------

int cWidgetTextBox::WorldToCharPos(const cVector2f &avWorldPos) {
	float fTextPos = WorldToLocalPosition(avWorldPos).x -
					 mvGfxCorners[0]->GetActiveSize().x + 3;

	int lMarkerCharPos;
	if (fTextPos > 0) {
		lMarkerCharPos = GetLastCharInSize(mlFirstVisibleChar, fTextPos, 3.0f);
	} else {
		lMarkerCharPos = mlFirstVisibleChar;
	}

	return lMarkerCharPos;
}

//-----------------------------------------------------------------------

float cWidgetTextBox::CharToLocalPos(int alChar) {
	float fMarkerPos = -2;
	if (alChar > 0 && alChar - mlFirstVisibleChar > 0) {
		fMarkerPos = mpDefaultFontType->getLength(mvDefaultFontSize,
												  cString::SubW(msText, mlFirstVisibleChar, alChar - mlFirstVisibleChar).c_str());
	}
	return fMarkerPos;
}

//-----------------------------------------------------------------------

void cWidgetTextBox::SetMarkerPos(int alPos) {
	mlMarkerCharPos = alPos;
	if (mlMarkerCharPos < 0)
		mlMarkerCharPos = 0;
	if (mlMarkerCharPos > (int)msText.size() && msText.size() > 0)
		mlMarkerCharPos = (int)msText.size();

	if (mlMarkerCharPos > mlFirstVisibleChar + mlVisibleCharSize) {
		mlFirstVisibleChar = GetFirstCharInSize(mlMarkerCharPos, mfTextMaxSize, 0) + 1;
		if (msText.size() <= 1)
			mlFirstVisibleChar = 0;
		OnChangeText();
	} else if (mlMarkerCharPos < mlFirstVisibleChar) {
		mlFirstVisibleChar = mlMarkerCharPos;
		OnChangeText();
	}
}

//-----------------------------------------------------------------------

void cWidgetTextBox::OnChangeSize() {
	mvSize.y = mvGfxBorders[0]->GetActiveSize().y +
			   mvGfxBorders[2]->GetActiveSize().y +
			   mvDefaultFontSize.y + 2 * 2;

	mfTextMaxSize = mvSize.x - mvGfxBorders[0]->GetActiveSize().x -
					mvGfxBorders[1]->GetActiveSize().x - 3 * 2 -
					mfMaxTextSizeNeg;

	OnChangeText();
}

//-----------------------------------------------------------------------

void cWidgetTextBox::OnChangeText() {
	if (msText == _W(""))
		mlVisibleCharSize = 0;
	else
		mlVisibleCharSize = GetLastCharInSize(mlFirstVisibleChar, mfTextMaxSize, 0) -
							mlFirstVisibleChar;

	if (mlMaxCharacters >= 0 && (int)msText.size() > mlMaxCharacters) {
		SetText(cString::SubW(msText, 0, mlMaxCharacters));

		if (mlSelectedTextEnd >= mlMaxCharacters)
			mlSelectedTextEnd = mlMaxCharacters - 1;
		if (mlMarkerCharPos >= mlMaxCharacters)
			mlMarkerCharPos = mlMaxCharacters - 1;
	}
}

//-----------------------------------------------------------------------

void cWidgetTextBox::OnLoadGraphics() {
	mpGfxMarker = mpSkin->GetGfx(eGuiSkinGfx_TextBoxMarker);
	mpGfxSelectedTextBack = mpSkin->GetGfx(eGuiSkinGfx_TextBoxSelectedTextBack);

	mpGfxBackground = mpSkin->GetGfx(eGuiSkinGfx_TextBoxBackground);

	mvGfxBorders[0] = mpSkin->GetGfx(eGuiSkinGfx_FrameBorderRight);
	mvGfxBorders[1] = mpSkin->GetGfx(eGuiSkinGfx_FrameBorderLeft);
	mvGfxBorders[2] = mpSkin->GetGfx(eGuiSkinGfx_FrameBorderUp);
	mvGfxBorders[3] = mpSkin->GetGfx(eGuiSkinGfx_FrameBorderDown);

	mvGfxCorners[0] = mpSkin->GetGfx(eGuiSkinGfx_FrameCornerLU);
	mvGfxCorners[1] = mpSkin->GetGfx(eGuiSkinGfx_FrameCornerRU);
	mvGfxCorners[2] = mpSkin->GetGfx(eGuiSkinGfx_FrameCornerRD);
	mvGfxCorners[3] = mpSkin->GetGfx(eGuiSkinGfx_FrameCornerLD);

	OnChangeSize();
}

//-----------------------------------------------------------------------

void cWidgetTextBox::OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion) {
	////////////////////////////////
	// Text
	cVector3f vTextAdd = cVector3f(3, 2, 0.2f) + mvGfxCorners[0]->GetActiveSize();
	DrawDefaultText(cString::SubW(msText, mlFirstVisibleChar, mlVisibleCharSize),
					GetGlobalPosition() + vTextAdd,
					eFontAlign_Left);

	// Marker
	if (mlMarkerCharPos >= 0) {
		float fMarkerPos = CharToLocalPos(mlMarkerCharPos);
		mpSet->DrawGfx(mpGfxMarker, GetGlobalPosition() + vTextAdd + cVector3f(fMarkerPos, 0, 0.1f),
					   cVector2f(2, mvDefaultFontSize.y));

		// Selected text
		if (mlSelectedTextEnd >= 0) {
			float fSelectEnd = CharToLocalPos(mlSelectedTextEnd);

			float fPos = fSelectEnd < fMarkerPos ? fSelectEnd : fMarkerPos;
			float fEnd = fSelectEnd > fMarkerPos ? fSelectEnd : fMarkerPos;

			if (fPos < 0)
				fPos = 0;
			if (fEnd > mfTextMaxSize)
				fEnd = mfTextMaxSize;

			float fSize = fEnd - fPos;

			mpSet->DrawGfx(mpGfxSelectedTextBack, GetGlobalPosition() + vTextAdd + cVector3f(fPos, 0, 0.2f),
						   cVector2f(fSize, mvDefaultFontSize.y));
		}
	}

	////////////////////////////////
	// Background and Borders
	DrawBordersAndCorners(mpGfxBackground, mvGfxBorders, mvGfxCorners,
						  GetGlobalPosition(), mvSize);
}

//-----------------------------------------------------------------------

bool cWidgetTextBox::OnMouseMove(cGuiMessageData &aData) {
	if (mbPressed) {
		int lPos = WorldToCharPos(aData.mvPos);
		if (lPos != mlMarkerCharPos) {
			if (mlSelectedTextEnd == -1)
				mlSelectedTextEnd = mlMarkerCharPos;
			SetMarkerPos(lPos);
		}
	}

	return true;
}

//-----------------------------------------------------------------------

bool cWidgetTextBox::OnMouseDown(cGuiMessageData &aData) {
	if ((aData.mlVal & eGuiMouseButton_Left) == 0)
		return true;
	if (mbCanEdit == false)
		return true;

	SetMarkerPos(WorldToCharPos(aData.mvPos));
	mlSelectedTextEnd = -1;
	mbPressed = true;
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetTextBox::OnMouseUp(cGuiMessageData &aData) {
	if ((aData.mlVal & eGuiMouseButton_Left) == 0)
		return true;
	if (mbCanEdit == false)
		return true;

	mbPressed = false;
	return true;
}

//-----------------------------------------------------------------------

bool cWidgetTextBox::OnMouseDoubleClick(cGuiMessageData &aData) {
	if ((aData.mlVal & eGuiMouseButton_Left) == 0)
		return true;
	if (mbCanEdit == false)
		return true;

	SetMarkerPos(WorldToCharPos(aData.mvPos));

	if (msText[mlMarkerCharPos] == ' ')
		return true;

	/////////////////////////////
	// Get space to the right.
	mlSelectedTextEnd = 0;
	for (size_t i = mlMarkerCharPos; i > 0; --i) {
		if (msText[i] == ' ') {
			mlSelectedTextEnd = (int)i + 1;
			break;
		}
	}

	/////////////////////////////
	// Get space to the left
	for (size_t i = mlMarkerCharPos + 1; i < (size_t)msText.size(); ++i) {
		if (msText[i] == ' ' || i == (size_t)msText.size() - 1) {
			if (i == (size_t)msText.size() - 1)
				SetMarkerPos((int)msText.size() - 1);
			else
				SetMarkerPos((int)i);
			break;
		}
	}

	mbPressed = false;

	return true;
}

//-----------------------------------------------------------------------

bool cWidgetTextBox::OnMouseEnter(cGuiMessageData &aData) {
	mpSet->SetCurrentPointer(mpSkin->GetGfx(eGuiSkinGfx_PointerText));

	return true;
}

//-----------------------------------------------------------------------

bool cWidgetTextBox::OnMouseLeave(cGuiMessageData &aData) {
	mpSet->SetCurrentPointer(mpSkin->GetGfx(eGuiSkinGfx_PointerNormal));
	mbPressed = false;

	return false;
}

//-----------------------------------------------------------------------

bool cWidgetTextBox::OnLostFocus(cGuiMessageData &aData) {
	mlMarkerCharPos = -1;
	mlSelectedTextEnd = -1;
	return false;
}

//-----------------------------------------------------------------------

bool cWidgetTextBox::OnKeyPress(cGuiMessageData &aData) {
	if (mbCanEdit == false)
		return true;
	if (mlMarkerCharPos < 0)
		return false;

	auto key = aData.mKeyPress.keycode;
	int mod = aData.mKeyPress.flags;

	if (mpGfxMarker)
		mpGfxMarker->SetAnimationTime(0);

	//////////////////////////////
	// Copy / Pase / Cut
	if ((mod & Common::KBD_CTRL)) {
		int lStart = mlMarkerCharPos < mlSelectedTextEnd ? mlMarkerCharPos : mlSelectedTextEnd;
		int lEnd = mlMarkerCharPos > mlSelectedTextEnd ? mlMarkerCharPos : mlSelectedTextEnd;
		int lSelectSize = lEnd - lStart;

		/////////////////////////////
		// Select all
		if (key == Common::KEYCODE_a) {
			mlSelectedTextEnd = 0;
			mlMarkerCharPos = (int)msText.size() - 1;
		}
		/////////////////////////////
		// Copy
		else if (key == Common::KEYCODE_c) {
			if (mlSelectedTextEnd >= 0)
				CopyTextToClipboard(cString::SubW(msText, lStart, lSelectSize));
		}
		/////////////////////////////
		// Cut
		else if (key == Common::KEYCODE_x) {
			if (mlSelectedTextEnd >= 0) {
				CopyTextToClipboard(cString::SubW(msText, lStart, lSelectSize));
				SetText(cString::SubW(msText, 0, lStart) + cString::SubW(msText, lEnd));
				mlSelectedTextEnd = -1;
			}
		}
		/////////////////////////////
		// Paste
		else if (key == Common::KEYCODE_v) {
			tWString sExtra = LoadTextFromClipboard();

			if (mlSelectedTextEnd < 0) {
				if (mlMaxCharacters == -1 ||
					(int)msText.size() + (int)sExtra.size() <= mlMaxCharacters) {
					SetText(cString::SubW(msText, 0, mlMarkerCharPos) + sExtra +
							cString::SubW(msText, mlMarkerCharPos));

					SetMarkerPos(mlMarkerCharPos + (int)sExtra.size());
				}
			} else {
				if (mlMaxCharacters < 0 ||
					(int)sExtra.size() <= lSelectSize ||
					(int)sExtra.size() + (int)msText.size() - lSelectSize <= mlMaxCharacters) {
					SetText(cString::SubW(msText, 0, lStart) + sExtra +
							cString::SubW(msText, lEnd));

					mlSelectedTextEnd = -1;
					SetMarkerPos(lStart + (int)sExtra.size());
				}
			}
		}
	}
	//////////////////////////////
	// Arrow keys
	else if (key == Common::KEYCODE_LEFT || key == Common::KEYCODE_RIGHT) {
		if (mod & Common::KBD_SHIFT) {
			if (mlSelectedTextEnd == -1)
				mlSelectedTextEnd = mlMarkerCharPos;

			if (key == Common::KEYCODE_LEFT)
				SetMarkerPos(mlMarkerCharPos - 1);
			else
				SetMarkerPos(mlMarkerCharPos + 1);
		} else {
			if (key == Common::KEYCODE_LEFT)
				SetMarkerPos(mlMarkerCharPos - 1);
			else
				SetMarkerPos(mlMarkerCharPos + 1);

			mlSelectedTextEnd = -1;
		}
	}
	//////////////////////////////
	// Delete and backspace
	else if (key == Common::KEYCODE_DELETE || key == Common::KEYCODE_BACKSPACE) {
		if (mlSelectedTextEnd >= 0) {
			int lStart = mlMarkerCharPos < mlSelectedTextEnd ? mlMarkerCharPos : mlSelectedTextEnd;
			int lEnd = mlMarkerCharPos > mlSelectedTextEnd ? mlMarkerCharPos : mlSelectedTextEnd;

			SetText(cString::SubW(msText, 0, lStart) + cString::SubW(msText, lEnd));

			mlSelectedTextEnd = -1;
			SetMarkerPos(lStart);
		} else {
			if (key == Common::KEYCODE_DELETE) {
				SetText(cString::SubW(msText, 0, mlMarkerCharPos) +
						cString::SubW(msText, mlMarkerCharPos + 1));
			} else {
				SetText(cString::SubW(msText, 0, mlMarkerCharPos - 1) +
						cString::SubW(msText, mlMarkerCharPos));
				SetMarkerPos(mlMarkerCharPos - 1);
			}
		}
	}
	//////////////////////////////
	// Home
	else if (key == Common::KEYCODE_HOME) {
		if (mod & Common::KBD_SHIFT) {
			if (mlSelectedTextEnd == -1)
				mlSelectedTextEnd = mlMarkerCharPos;
		} else {
			mlSelectedTextEnd = -1;
		}
		SetMarkerPos(0);
	}
	//////////////////////////////
	// End
	else if (key == Common::KEYCODE_END) {
		if (mod & Common::KBD_SHIFT) {
			if (mlSelectedTextEnd == -1)
				mlSelectedTextEnd = mlMarkerCharPos;
		} else {
			mlSelectedTextEnd = -1;
		}
		SetMarkerPos((int)msText.size());
	}
	//////////////////////////////////
	// Character
	else {
		int lFirstFontChar = mpDefaultFontType->getFirstChar();
		int lLastFontChar = mpDefaultFontType->getLastChar();
		auto textInput = aData.mKeyPress.ascii;

		// Check so press is valid
		if (textInput >= lFirstFontChar && textInput <= lLastFontChar &&
			mpDefaultFontType->getGlyph(textInput - lFirstFontChar)) {
			if (mlSelectedTextEnd < 0) {
				if (mlMaxCharacters == -1 || (int)msText.size() < mlMaxCharacters) {
					SetText(cString::SubW(msText, 0, mlMarkerCharPos) + textInput +
							cString::SubW(msText, mlMarkerCharPos));

					SetMarkerPos(mlMarkerCharPos + 1);
				}
			} else {
				int lStart = mlMarkerCharPos < mlSelectedTextEnd ? mlMarkerCharPos : mlSelectedTextEnd;
				int lEnd = mlMarkerCharPos > mlSelectedTextEnd ? mlMarkerCharPos : mlSelectedTextEnd;

				SetText(cString::SubW(msText, 0, lStart) + textInput +
						cString::SubW(msText, lEnd));

				mlSelectedTextEnd = -1;

				SetMarkerPos(lStart + 1);
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------

} // namespace hpl
