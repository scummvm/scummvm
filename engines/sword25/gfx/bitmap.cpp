// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "bitmap.h"
#include "kernel/outputpersistenceblock.h"
#include "kernel/inputpersistenceblock.h"

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "BITMAP"

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_Bitmap::BS_Bitmap(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, TYPES Type, unsigned int Handle) :
	BS_RenderObject(ParentPtr, Type, Handle),
	m_ModulationColor(0xffffffff),
	m_ScaleFactorX(1.0f),
	m_ScaleFactorY(1.0f),
	m_FlipH(false),
	m_FlipV(false)
{
}

// -----------------------------------------------------------------------------

BS_Bitmap::~BS_Bitmap()
{
}

// -----------------------------------------------------------------------------
// Darstellungsart festlegen
// -----------------------------------------------------------------------------

void BS_Bitmap::SetAlpha(int Alpha)
{
	if (!IsAlphaAllowed())
	{
		BS_LOG_WARNINGLN("Tried to set alpha value on a bitmap that does not support alpha blending. Call was ignored.");
		return;
	}

	if (Alpha < 0 || Alpha > 255)
	{
		int OldAlpha = Alpha;
		if (Alpha < 0) Alpha = 0;
		if (Alpha > 255) Alpha = 255;
		BS_LOG_WARNINGLN("Tried to set an invalid alpha value (%d) on a bitmap. Value was changed to %d.", OldAlpha, Alpha);
		
		return;
	}

	unsigned int NewModulationColor = (m_ModulationColor & 0x00ffffff) | Alpha << 24;
	if (NewModulationColor != m_ModulationColor)
	{
		m_ModulationColor = NewModulationColor;
		ForceRefresh();
	}
}

// -----------------------------------------------------------------------------

void BS_Bitmap::SetModulationColor(unsigned int ModulationColor)
{
	if (!IsColorModulationAllowed())
	{
		BS_LOG_WARNINGLN("Tried to set modulation color of a bitmap that does not support color modulation. Call was ignored.");
		return;
	}

	unsigned int NewModulationColor = (ModulationColor & 0x00ffffff) | (m_ModulationColor & 0xff000000);
	if (NewModulationColor != m_ModulationColor)
	{
		m_ModulationColor = NewModulationColor;
		ForceRefresh();
	}
}

// -----------------------------------------------------------------------------

void BS_Bitmap::SetScaleFactor(float ScaleFactor)
{
	SetScaleFactorX(ScaleFactor);
	SetScaleFactorY(ScaleFactor);
}

// -----------------------------------------------------------------------------

void BS_Bitmap::SetScaleFactorX(float ScaleFactorX)
{
	if (!IsScalingAllowed())
	{
		BS_LOG_WARNINGLN("Tried to set scale factor of a bitmap that does not support scaling. Call was ignored.");
		return;
	}

	if (ScaleFactorX < 0)
	{
		BS_LOG_WARNINGLN("Tried to set scale factor of a bitmap to a negative value. Call was ignored.");
		return;
	}

	if (ScaleFactorX != m_ScaleFactorX)
	{
		m_ScaleFactorX = ScaleFactorX;
		m_Width = static_cast<int>(m_OriginalWidth * m_ScaleFactorX);
		if (m_ScaleFactorX <= 0.0f) m_ScaleFactorX = 0.001f;
		ForceRefresh();
	}
}

// -----------------------------------------------------------------------------

void BS_Bitmap::SetScaleFactorY(float ScaleFactorY)
{
	if (!IsScalingAllowed())
	{
		BS_LOG_WARNINGLN("Tried to set scale factor of a bitmap that does not support scaling. Call was ignored.");
		return;
	}

	if (ScaleFactorY < 0)
	{
		BS_LOG_WARNINGLN("Tried to set scale factor of a bitmap to a negative value. Call was ignored.");
		return;
	}

	if (ScaleFactorY != m_ScaleFactorY)
	{
		m_ScaleFactorY = ScaleFactorY;
		m_Height = static_cast<int>(m_OriginalHeight * ScaleFactorY);
		if (m_ScaleFactorY <= 0.0f) m_ScaleFactorY = 0.001f;
		ForceRefresh();
	}
}

// -----------------------------------------------------------------------------

void BS_Bitmap::SetFlipH(bool FlipH)
{
	m_FlipH = FlipH;
	ForceRefresh();
}

// -----------------------------------------------------------------------------

void BS_Bitmap::SetFlipV(bool FlipV)
{
	m_FlipV = FlipV;
	ForceRefresh();
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool BS_Bitmap::Persist(BS_OutputPersistenceBlock & Writer)
{
	bool Result = true;

	Result &= BS_RenderObject::Persist(Writer);
	Writer.Write(m_FlipH);
	Writer.Write(m_FlipV);
	Writer.Write(m_ScaleFactorX);
	Writer.Write(m_ScaleFactorY);
	Writer.Write(m_ModulationColor);
	Writer.Write(m_OriginalWidth);
	Writer.Write(m_OriginalHeight);

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_Bitmap::Unpersist(BS_InputPersistenceBlock & Reader)
{
	bool Result = true;

	Result &= BS_RenderObject::Unpersist(Reader);
	Reader.Read(m_FlipH);
	Reader.Read(m_FlipV);
	Reader.Read(m_ScaleFactorX);
	Reader.Read(m_ScaleFactorY);
	Reader.Read(m_ModulationColor);
	Reader.Read(m_OriginalWidth);
	Reader.Read(m_OriginalHeight);

	ForceRefresh();

	return Reader.IsGood() && Result;
}
