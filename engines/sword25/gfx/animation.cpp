/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include <memory>

#include "sword25/gfx/animation.h"

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resmanager.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/callbackregistry.h"
#include "sword25/package/packagemanager.h"
#include <tinyxml.h>
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/animationtemplate.h"
#include "sword25/gfx/animationtemplateregistry.h"
#include "sword25/gfx/animationresource.h"
#include "sword25/gfx/bitmapresource.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

#define BS_LOG_PREFIX "ANIMATION"

// Konstruktion / Destruktion
// --------------------------

BS_Animation::BS_Animation(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, const Common::String & FileName) :
	BS_TimedRenderObject(ParentPtr, BS_RenderObject::TYPE_ANIMATION)
{
	// Das BS_RenderObject konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!m_InitSuccess) return;

	InitMembers();

	// Vom negativen Fall ausgehen.
	m_InitSuccess = false;

	InitializeAnimationResource(FileName);
	
	// Erfolg signalisieren.
	m_InitSuccess = true;
}

// -----------------------------------------------------------------------------

BS_Animation::BS_Animation(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, const BS_AnimationTemplate & Template) :
	BS_TimedRenderObject(ParentPtr, BS_RenderObject::TYPE_ANIMATION)
{
	// Das BS_RenderObject konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!m_InitSuccess) return;

	InitMembers();

	// Vom negativen Fall ausgehen.
	m_InitSuccess = false;

	m_AnimationTemplateHandle = BS_AnimationTemplate::Create(Template);

	// Erfolg signalisieren.
	m_InitSuccess = true;
}

// -----------------------------------------------------------------------------

BS_Animation::BS_Animation(BS_InputPersistenceBlock & Reader, BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Handle) :
	BS_TimedRenderObject(ParentPtr, BS_RenderObject::TYPE_ANIMATION, Handle)
{
	// Das BS_RenderObject konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!m_InitSuccess) return;

	InitMembers();

	// Objekt vom Stream laden.
	m_InitSuccess = Unpersist(Reader);
}

// -----------------------------------------------------------------------------

void BS_Animation::InitializeAnimationResource(const Common::String &FileName)
{
	// Die Resource wird für die gesamte Lebensdauer des Animations-Objektes gelockt.
	BS_Resource * ResourcePtr = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(FileName);
	if (ResourcePtr && ResourcePtr->GetType() == BS_Resource::TYPE_ANIMATION)
		m_AnimationResourcePtr = static_cast<BS_AnimationResource *>(ResourcePtr);
	else
	{
		BS_LOG_ERRORLN("The resource \"%s\" could not be requested. The Animation can't be created.", FileName.c_str());
		return;
	}

	// Größe und Position der Animation anhand des aktuellen Frames bestimmen.
	ComputeCurrentCharacteristics();
}

// -----------------------------------------------------------------------------

void BS_Animation::InitMembers()
{
	m_CurrentFrame = 0;
	m_CurrentFrameTime = 0;
	m_Direction = FORWARD;
	m_Running = false;
	m_Finished = false;
	m_RelX = 0;
	m_RelY = 0;
	m_ScaleFactorX = 1.0f;
	m_ScaleFactorY = 1.0f;
	m_ModulationColor = 0xffffffff;
	m_AnimationResourcePtr = 0;
	m_AnimationTemplateHandle = 0;
	m_FramesLocked = false;
}

// -----------------------------------------------------------------------------

BS_Animation::~BS_Animation()
{
	if (GetAnimationDescription())
	{
		Stop();
		GetAnimationDescription()->Unlock();
	}

	// Delete Callbacks
	std::vector<ANIMATION_CALLBACK_DATA>::iterator it = m_DeleteCallbacks.begin();
	for (; it != m_DeleteCallbacks.end(); it++) ((*it).Callback)((*it).Data);

}

// -----------------------------------------------------------------------------
// Steuermethoden
// -----------------------------------------------------------------------------

void BS_Animation::Play()
{
	// Wenn die Animation zuvor komplett durchgelaufen ist, wird sie wieder von Anfang abgespielt
	if (m_Finished) Stop();

	m_Running = true;
	LockAllFrames();
}

// -----------------------------------------------------------------------------

void BS_Animation::Pause()
{
	m_Running = false;
	UnlockAllFrames();
}

// -----------------------------------------------------------------------------

void BS_Animation::Stop()
{
	m_CurrentFrame = 0;
	m_CurrentFrameTime = 0;
	m_Direction = FORWARD;
	Pause();
}

// -----------------------------------------------------------------------------

void BS_Animation::SetFrame(unsigned int Nr)
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);

	if (Nr >= animationDescriptionPtr->GetFrameCount())
	{
		BS_LOG_ERRORLN("Tried to set animation to illegal frame (%d). Value must be between 0 and %d.",
					    Nr, animationDescriptionPtr->GetFrameCount());
		return;
	}

	m_CurrentFrame = Nr;
	m_CurrentFrameTime = 0;
	ComputeCurrentCharacteristics();
	ForceRefresh();
}

// -----------------------------------------------------------------------------
// Rendern
// -----------------------------------------------------------------------------

bool BS_Animation::DoRender()
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	BS_ASSERT(m_CurrentFrame < animationDescriptionPtr->GetFrameCount());

	// Bitmap des aktuellen Frames holen
	BS_Resource* pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(animationDescriptionPtr->GetFrame(m_CurrentFrame).FileName);
	BS_ASSERT(pResource);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	BS_BitmapResource * pBitmapResource = static_cast<BS_BitmapResource*>(pResource);

	// Framebufferobjekt holen
	BS_GraphicEngine * pGfx = static_cast<BS_GraphicEngine *>(BS_Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(pGfx);

	// Bitmap zeichnen
	bool Result;
	if (IsScalingAllowed() && (m_Width != pBitmapResource->GetWidth() || m_Height != pBitmapResource->GetHeight()))
	{
		Result = pBitmapResource->Blit(m_AbsoluteX, m_AbsoluteY,
									   (animationDescriptionPtr->GetFrame(m_CurrentFrame).FlipV ? BS_BitmapResource::FLIP_V : 0) |
									   (animationDescriptionPtr->GetFrame(m_CurrentFrame).FlipH ? BS_BitmapResource::FLIP_H : 0),
									   0, m_ModulationColor, m_Width, m_Height);
	}
	else
	{
		Result = pBitmapResource->Blit(m_AbsoluteX, m_AbsoluteY,
									   (animationDescriptionPtr->GetFrame(m_CurrentFrame).FlipV ? BS_BitmapResource::FLIP_V : 0) |
									   (animationDescriptionPtr->GetFrame(m_CurrentFrame).FlipH ? BS_BitmapResource::FLIP_H : 0),
									   0, m_ModulationColor, -1, -1);
	}

	// Resource freigeben
	pBitmapResource->Release();

	return Result;
}

// -----------------------------------------------------------------------------
// Frame Notifikation
// -----------------------------------------------------------------------------

void BS_Animation::FrameNotification(int TimeElapsed)
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	BS_ASSERT(TimeElapsed >= 0);

	// Nur wenn die Animation läuft wird sie auch weiterbewegt
	if (m_Running)
	{
		int OldFrame = m_CurrentFrame;

		// Gesamte vergangene Zeit bestimmen (inkl. Restzeit des aktuellen Frames)
		m_CurrentFrameTime += TimeElapsed;

		// Anzahl an zu überpringenden Frames bestimmen
		int SkipFrames = animationDescriptionPtr->GetMillisPerFrame() == 0 ? 0 : m_CurrentFrameTime / animationDescriptionPtr->GetMillisPerFrame();

		// Neue Frame-Restzeit bestimmen
		m_CurrentFrameTime -= animationDescriptionPtr->GetMillisPerFrame() * SkipFrames;

		// Neuen Frame bestimmen (je nach aktuellener Abspielrichtung wird addiert oder subtrahiert)
		int TmpCurFrame = m_CurrentFrame;
		switch (m_Direction)
		{
		case FORWARD:
			TmpCurFrame += SkipFrames;
			break;

		case BACKWARD:
			TmpCurFrame -= SkipFrames;
			break;

		default:
			BS_ASSERT(0);
		}	

		// Überläufe behandeln
		if (TmpCurFrame < 0)
		{
			// Loop-Point Callbacks
			std::vector<ANIMATION_CALLBACK_DATA>::iterator it = m_LoopPointCallbacks.begin();
			while (it != m_LoopPointCallbacks.end())
			{
				if (((*it).Callback)((*it).Data) == false)
				{
					it = m_LoopPointCallbacks.erase(it);
				}
				else
					it++;
			}

			// Ein Unterlauf darf nur auftreten, wenn der Animationstyp JOJO ist.
			BS_ASSERT(animationDescriptionPtr->GetAnimationType() == AT_JOJO);
			TmpCurFrame = - TmpCurFrame;
			m_Direction = FORWARD;
		}
		else if (static_cast<unsigned int>(TmpCurFrame) >= animationDescriptionPtr->GetFrameCount())
		{
			// Loop-Point Callbacks
			std::vector<ANIMATION_CALLBACK_DATA>::iterator it = m_LoopPointCallbacks.begin();
			while (it != m_LoopPointCallbacks.end())
			{
				if (((*it).Callback)((*it).Data) == false)
					it = m_LoopPointCallbacks.erase(it);
				else
					it++;
			}

			switch (animationDescriptionPtr->GetAnimationType())
			{
			case AT_ONESHOT:
				TmpCurFrame = animationDescriptionPtr->GetFrameCount() - 1;
				m_Finished = true;
				Pause();
				break;

			case AT_LOOP:
				TmpCurFrame = TmpCurFrame % animationDescriptionPtr->GetFrameCount();
				break;

			case AT_JOJO:
				TmpCurFrame = animationDescriptionPtr->GetFrameCount() - (TmpCurFrame % animationDescriptionPtr->GetFrameCount()) - 1;
				m_Direction = BACKWARD;
				break;

			default:
				BS_ASSERT(0);
			}
		}

		if (m_CurrentFrame != TmpCurFrame)
		{
			ForceRefresh();
			
			if (animationDescriptionPtr->GetFrame(m_CurrentFrame).Action != "")
			{
				// Action Callbacks
				std::vector<ANIMATION_CALLBACK_DATA>::iterator it = m_ActionCallbacks.begin();
				while (it != m_ActionCallbacks.end())
				{
					if (((*it).Callback)((*it).Data) == false)
						it = m_ActionCallbacks.erase(it);
					else
						it++;
				}
			}
		}

		m_CurrentFrame = static_cast<unsigned int>(TmpCurFrame);
	}

	// Größe und Position der Animation anhand des aktuellen Frames bestimmen
	ComputeCurrentCharacteristics();

	BS_ASSERT(m_CurrentFrame < animationDescriptionPtr->GetFrameCount());
	BS_ASSERT(m_CurrentFrameTime >= 0);
}

// -----------------------------------------------------------------------------

void BS_Animation::ComputeCurrentCharacteristics()
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	const BS_AnimationResource::Frame & CurFrame = animationDescriptionPtr->GetFrame(m_CurrentFrame);

	BS_Resource* pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(CurFrame.FileName);
	BS_ASSERT(pResource);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	BS_BitmapResource* pBitmap = static_cast<BS_BitmapResource *>(pResource);

	// Größe des Bitmaps auf die Animation übertragen
	m_Width = static_cast<int>(pBitmap->GetWidth() * m_ScaleFactorX);
	m_Height = static_cast<int>(pBitmap->GetHeight() * m_ScaleFactorY);

	// Position anhand des Hotspots berechnen und setzen
	int PosX = m_RelX + ComputeXModifier();				
	int PosY = m_RelY + ComputeYModifier();

	BS_RenderObject::SetPos(PosX, PosY);

	pBitmap->Release();
}

// -----------------------------------------------------------------------------

bool BS_Animation::LockAllFrames()
{
	if (!m_FramesLocked)
	{
		BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
		BS_ASSERT(animationDescriptionPtr);
		for (unsigned int i = 0; i < animationDescriptionPtr->GetFrameCount(); ++i)
		{
			if (!BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(animationDescriptionPtr->GetFrame(i).FileName))
			{
				BS_LOG_ERRORLN("Could not lock all animation frames.");
				return false;
			}
		}

		m_FramesLocked = true;
	}

	return true;
}

// -----------------------------------------------------------------------------

bool BS_Animation::UnlockAllFrames()
{
	if (m_FramesLocked)
	{
		BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
		BS_ASSERT(animationDescriptionPtr);
		for (unsigned int i = 0; i < animationDescriptionPtr->GetFrameCount(); ++i)
		{
			BS_Resource* pResource;
			if (!(pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(animationDescriptionPtr->GetFrame(i).FileName)))
			{
				BS_LOG_ERRORLN("Could not unlock all animation frames.");
				return false;
			}

			// Zwei mal freigeben um den Request von LockAllFrames() und den jetzigen Request aufzuheben
			pResource->Release();
			if (pResource->GetLockCount()) pResource->Release();
		}

		m_FramesLocked = false;
	}

	return true;
}

// -----------------------------------------------------------------------------
// Getter
// -----------------------------------------------------------------------------

BS_Animation::ANIMATION_TYPES BS_Animation::GetAnimationType() const
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->GetAnimationType();
}

// -----------------------------------------------------------------------------

int BS_Animation::GetFPS() const
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->GetFPS();
}

// -----------------------------------------------------------------------------

int	BS_Animation::GetFrameCount() const
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->GetFrameCount();
}

// -----------------------------------------------------------------------------

bool BS_Animation::IsScalingAllowed() const
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->IsScalingAllowed();
}

// -----------------------------------------------------------------------------

bool BS_Animation::IsAlphaAllowed() const
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->IsAlphaAllowed();
}

// -----------------------------------------------------------------------------

bool BS_Animation::IsColorModulationAllowed() const
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->IsColorModulationAllowed();
}

// -----------------------------------------------------------------------------
// Positionieren
// -----------------------------------------------------------------------------

void BS_Animation::SetPos(int RelX, int RelY)
{
	m_RelX = RelX;
	m_RelY = RelY;

	ComputeCurrentCharacteristics();
}

// -----------------------------------------------------------------------------

void BS_Animation::SetX(int RelX)
{
	m_RelX = RelX;

	ComputeCurrentCharacteristics();
}

// -----------------------------------------------------------------------------

void BS_Animation::SetY(int RelY)
{
	m_RelY = RelY;

	ComputeCurrentCharacteristics();
}

// -----------------------------------------------------------------------------
// Darstellungsart festlegen
// -----------------------------------------------------------------------------

void BS_Animation::SetAlpha(int Alpha)
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	if (!animationDescriptionPtr->IsAlphaAllowed())
	{
		BS_LOG_WARNINGLN("Tried to set alpha value on an animation that does not support alpha. Call was ignored.");
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

void BS_Animation::SetModulationColor(unsigned int ModulationColor)
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	if (!animationDescriptionPtr->IsColorModulationAllowed())
	{
		BS_LOG_WARNINGLN("Tried to set modulation color on an animation that does not support color modulation. Call was ignored");
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

void BS_Animation::SetScaleFactor(float ScaleFactor)
{
	SetScaleFactorX(ScaleFactor);
	SetScaleFactorY(ScaleFactor);
}

// -----------------------------------------------------------------------------

void BS_Animation::SetScaleFactorX(float ScaleFactorX)
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	if (!animationDescriptionPtr->IsScalingAllowed())
	{
		BS_LOG_WARNINGLN("Tried to set x scale factor on an animation that does not support scaling. Call was ignored");
		return;
	}

	if (ScaleFactorX != m_ScaleFactorX)
	{
		m_ScaleFactorX = ScaleFactorX;
		if (m_ScaleFactorX <= 0.0f) m_ScaleFactorX = 0.001f;
		ForceRefresh();
		ComputeCurrentCharacteristics();
	}
}

// -----------------------------------------------------------------------------

void BS_Animation::SetScaleFactorY(float ScaleFactorY)
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	if (!animationDescriptionPtr->IsScalingAllowed())
	{
		BS_LOG_WARNINGLN("Tried to set y scale factor on an animation that does not support scaling. Call was ignored");
		return;
	}

	if (ScaleFactorY != m_ScaleFactorY)
	{
		m_ScaleFactorY = ScaleFactorY;
		if (m_ScaleFactorY <= 0.0f) m_ScaleFactorY = 0.001f;
		ForceRefresh();
		ComputeCurrentCharacteristics();
	}
}

// -----------------------------------------------------------------------------

const Common::String & BS_Animation::GetCurrentAction() const
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->GetFrame(m_CurrentFrame).Action;
}

// -----------------------------------------------------------------------------

int BS_Animation::GetX() const
{
	return m_RelX;
}

// -----------------------------------------------------------------------------

int BS_Animation::GetY() const
{
	return m_RelY;
}

// -----------------------------------------------------------------------------

int BS_Animation::GetAbsoluteX() const
{
	return m_AbsoluteX + (m_RelX - m_X);
}

// -----------------------------------------------------------------------------

int BS_Animation::GetAbsoluteY() const
{
	return m_AbsoluteY + (m_RelY - m_Y);
}

// -----------------------------------------------------------------------------

int BS_Animation::ComputeXModifier() const
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	const BS_AnimationResource::Frame & CurFrame = animationDescriptionPtr->GetFrame(m_CurrentFrame);

	BS_Resource* pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(CurFrame.FileName);
	BS_ASSERT(pResource);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	BS_BitmapResource* pBitmap = static_cast<BS_BitmapResource *>(pResource);

	int Result = CurFrame.FlipV ? - static_cast<int>((pBitmap->GetWidth() - 1 - CurFrame.HotspotX) * m_ScaleFactorX) :
								  - static_cast<int>(CurFrame.HotspotX * m_ScaleFactorX);

	pBitmap->Release();

	return Result;
}

// -----------------------------------------------------------------------------

int BS_Animation::ComputeYModifier() const
{
	BS_AnimationDescription * animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	const BS_AnimationResource::Frame & CurFrame = animationDescriptionPtr->GetFrame(m_CurrentFrame);

	BS_Resource* pResource = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(CurFrame.FileName);
	BS_ASSERT(pResource);
	BS_ASSERT(pResource->GetType() == BS_Resource::TYPE_BITMAP);
	BS_BitmapResource* pBitmap = static_cast<BS_BitmapResource *>(pResource);

	int Result = CurFrame.FlipH ? - static_cast<int>((pBitmap->GetHeight() - 1 - CurFrame.HotspotY) * m_ScaleFactorY) :
								  - static_cast<int>(CurFrame.HotspotY * m_ScaleFactorY);

	pBitmap->Release();

	return Result;
}

// -----------------------------------------------------------------------------

void BS_Animation::RegisterActionCallback(ANIMATION_CALLBACK Callback, unsigned int Data)
{
	ANIMATION_CALLBACK_DATA CD;
	CD.Callback = Callback;
	CD.Data = Data;
	m_ActionCallbacks.push_back(CD);
}

// -----------------------------------------------------------------------------

void BS_Animation::RegisterLoopPointCallback(ANIMATION_CALLBACK Callback, unsigned int Data)
{
	ANIMATION_CALLBACK_DATA CD;
	CD.Callback = Callback;
	CD.Data = Data;
	m_LoopPointCallbacks.push_back(CD);
}

// -----------------------------------------------------------------------------

void BS_Animation::RegisterDeleteCallback(ANIMATION_CALLBACK Callback, unsigned int Data)
{
	ANIMATION_CALLBACK_DATA CD;
	CD.Callback = Callback;
	CD.Data = Data;
	m_DeleteCallbacks.push_back(CD);
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

void BS_Animation::PersistCallbackVector(BS_OutputPersistenceBlock & Writer, const std::vector<ANIMATION_CALLBACK_DATA> & Vector)
{
	// Anzahl an Callbacks persistieren.
	Writer.Write(Vector.size());
	
	// Alle Callbacks einzeln persistieren.
	std::vector<ANIMATION_CALLBACK_DATA>::const_iterator It = Vector.begin();
	while (It != Vector.end())
	{
		Writer.Write(BS_CallbackRegistry::GetInstance().ResolveCallbackPointer(It->Callback));
		Writer.Write(It->Data);

		++It;
	}
}

// -----------------------------------------------------------------------------

void BS_Animation::UnpersistCallbackVector(BS_InputPersistenceBlock & Reader, std::vector<ANIMATION_CALLBACK_DATA> & Vector)
{
	// Callbackvector leeren.
	Vector.resize(0);

	// Anzahl an Callbacks einlesen.
	unsigned int CallbackCount;
	Reader.Read(CallbackCount);

	// Alle Callbacks einzeln wieder herstellen.
	for (unsigned int i = 0; i < CallbackCount; ++i)
	{
		ANIMATION_CALLBACK_DATA CallbackData;

		Common::String CallbackFunctionName;
		Reader.Read(CallbackFunctionName);
		CallbackData.Callback = reinterpret_cast<ANIMATION_CALLBACK>(BS_CallbackRegistry::GetInstance().ResolveCallbackFunction(CallbackFunctionName));
		
		Reader.Read(CallbackData.Data);

		Vector.push_back(CallbackData);
	}
}

// -----------------------------------------------------------------------------

bool BS_Animation::Persist(BS_OutputPersistenceBlock & Writer)
{
	bool Result = true;

	Result &= BS_RenderObject::Persist(Writer);

	Writer.Write(m_RelX);
	Writer.Write(m_RelY);
	Writer.Write(m_ScaleFactorX);
	Writer.Write(m_ScaleFactorY);
	Writer.Write(m_ModulationColor);
	Writer.Write(m_CurrentFrame);
	Writer.Write(m_CurrentFrameTime);
	Writer.Write(m_Running);
	Writer.Write(m_Finished);
	Writer.Write(static_cast<unsigned int>(m_Direction));

	// Je nach Animationstyp entweder das Template oder die Ressource speichern.
	if (m_AnimationResourcePtr)
	{
		unsigned int Marker = 0;
		Writer.Write(Marker);
		Writer.Write(m_AnimationResourcePtr->GetFileName());
	}
	else if (m_AnimationTemplateHandle)
	{
		unsigned int Marker = 1;
		Writer.Write(Marker);
		Writer.Write(m_AnimationTemplateHandle);
	}
	else
	{
		BS_ASSERT(false);
	}

	//Writer.Write(m_AnimationDescriptionPtr);

	Writer.Write(m_FramesLocked);
	PersistCallbackVector(Writer, m_LoopPointCallbacks);
	PersistCallbackVector(Writer, m_ActionCallbacks);
	PersistCallbackVector(Writer, m_DeleteCallbacks);

	Result &= BS_RenderObject::PersistChildren(Writer);

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_Animation::Unpersist(BS_InputPersistenceBlock & Reader)
{
	bool Result = true;

	Result &= BS_RenderObject::Unpersist(Reader);

	Reader.Read(m_RelX);
	Reader.Read(m_RelY);
	Reader.Read(m_ScaleFactorX);
	Reader.Read(m_ScaleFactorY);
	Reader.Read(m_ModulationColor);
	Reader.Read(m_CurrentFrame);
	Reader.Read(m_CurrentFrameTime);
	Reader.Read(m_Running);
	Reader.Read(m_Finished);
	unsigned int Direction;
	Reader.Read(Direction);
	m_Direction = static_cast<DIRECTION>(Direction);

	// Animationstyp einlesen.
	unsigned int Marker;
	Reader.Read(Marker);
	if (Marker == 0)
	{
		Common::String ResourceFilename;
		Reader.Read(ResourceFilename);
		InitializeAnimationResource(ResourceFilename);
	}
	else if (Marker == 1)
	{
		Reader.Read(m_AnimationTemplateHandle);
	}
	else
	{
		BS_ASSERT(false);
	}

	Reader.Read(m_FramesLocked);
	if (m_FramesLocked) LockAllFrames();

	UnpersistCallbackVector(Reader, m_LoopPointCallbacks);
	UnpersistCallbackVector(Reader, m_ActionCallbacks);
	UnpersistCallbackVector(Reader, m_DeleteCallbacks);

	Result &= BS_RenderObject::UnpersistChildren(Reader);

	return Reader.IsGood() && Result;
}

// -----------------------------------------------------------------------------

BS_AnimationDescription * BS_Animation::GetAnimationDescription() const
{
	if (m_AnimationResourcePtr) return m_AnimationResourcePtr;
	else return BS_AnimationTemplateRegistry::GetInstance().ResolveHandle(m_AnimationTemplateHandle);
}

} // End of namespace Sword25
