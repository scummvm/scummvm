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

#ifndef BS_ANIMATION_H
#define BS_ANIMATION_H

// Includes
#include "kernel/common.h"
#include "timedrenderobject.h"

#include "kernel/memlog_off.h"
#include <vector>
#include "kernel/memlog_on.h"

// Forward declarations
class BS_Kernel;
class BS_PackageManager;
class BS_AnimationResource;
class BS_AnimationTemplate;
class BS_AnimationDescription;
class BS_InputPersistenceBlock;

class BS_Animation : public BS_TimedRenderObject
{
friend BS_RenderObject;

private:
	BS_Animation(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, const std::string & FileName);
	BS_Animation(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, const BS_AnimationTemplate & Template);
	BS_Animation(BS_InputPersistenceBlock & Reader, BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Handle);

public:
	enum ANIMATION_TYPES
	{
		AT_ONESHOT,
		AT_LOOP,
		AT_JOJO,
	};
	
	virtual ~BS_Animation();

	void Play();
	void Pause();
	void Stop();
	void SetFrame(unsigned int Nr);

	virtual void SetPos(int X, int Y);
	virtual void SetX(int X);
	virtual void SetY(int Y);
	
	virtual int GetX() const;
	virtual int GetY() const;
	virtual int GetAbsoluteX() const;
	virtual int GetAbsoluteY() const;

	/**
		@brief Setzt den Alphawert der Animation.
		@param Alpha der neue Alphawert der Animation (0 = keine Deckung, 255 = volle Deckung).
		@remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsAlphaAllowed() true zurückgibt.
	*/
	void SetAlpha(int Alpha);

	/**
		@brief Setzt die Modulationfarbe der Animation.
		@param Color eine 24-Bit Farbe, die die Modulationsfarbe der Animation festlegt.
		@remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsColorModulationAllowed() true zurückgibt.
	*/
	void SetModulationColor(unsigned int ModulationColor);

	/**
		@brief Setzt den Skalierungsfaktor der Animation.
		@param ScaleFactor der Faktor um den die Animation in beide Richtungen gestreckt werden soll.
		@remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void SetScaleFactor(float ScaleFactor);

	/**
		@brief Setzt den Skalierungsfaktor der Animation auf der X-Achse.
		@param ScaleFactor der Faktor um den die Animation in Richtungen der X-Achse gestreckt werden soll.
		@remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void SetScaleFactorX(float ScaleFactorX);

	/**
		@brief Setzt den Skalierungsfaktor der Animation auf der Y-Achse.
		@param ScaleFactor der Faktor um den die Animation in Richtungen der Y-Achse gestreckt werden soll.
		@remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void SetScaleFactorY(float ScaleFactorY);

	/**
	@brief Gibt den Skalierungsfakter der Animation auf der X-Achse zurück.
	@remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	float GetScaleFactorX() const { return m_ScaleFactorX; }

	/**
	@brief Gibt den Skalierungsfakter der Animation auf der Y-Achse zurück.
	@remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	float GetScaleFactorY() const { return m_ScaleFactorY; }
	
	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

	virtual void FrameNotification(int TimeElapsed);
	
	ANIMATION_TYPES		GetAnimationType() const;
	int					GetFPS() const;
	int					GetFrameCount() const;
	bool				IsScalingAllowed() const;
	bool				IsAlphaAllowed() const;
	bool				IsColorModulationAllowed() const;
	unsigned int		GetCurrentFrame() const { return m_CurrentFrame; }
	const std::string &	GetCurrentAction() const ;
	bool				IsRunning() const { return m_Running; }

	typedef bool (*ANIMATION_CALLBACK)(unsigned int);

	void RegisterLoopPointCallback(ANIMATION_CALLBACK Callback, unsigned int Data = 0);
	void RegisterActionCallback(ANIMATION_CALLBACK Callback, unsigned int Data = 0);
	void RegisterDeleteCallback(ANIMATION_CALLBACK Callback, unsigned int Data = 0);

protected:
	virtual bool DoRender();

private:
	enum DIRECTION
	{
		FORWARD,
		BACKWARD
	};

	int							m_RelX;
	int							m_RelY;
	float						m_ScaleFactorX;
	float						m_ScaleFactorY;
	unsigned int				m_ModulationColor;
	unsigned int				m_CurrentFrame;
	int							m_CurrentFrameTime;
	bool						m_Running;
	bool						m_Finished;
	DIRECTION					m_Direction;
	BS_AnimationResource *		m_AnimationResourcePtr;
	unsigned int				m_AnimationTemplateHandle;
	bool						m_FramesLocked;

	struct ANIMATION_CALLBACK_DATA
	{
		ANIMATION_CALLBACK	Callback;
		unsigned int		Data;
	};
	std::vector<ANIMATION_CALLBACK_DATA> m_LoopPointCallbacks;
	std::vector<ANIMATION_CALLBACK_DATA> m_ActionCallbacks;
	std::vector<ANIMATION_CALLBACK_DATA> m_DeleteCallbacks;

	/**
		@brief Lockt alle Frames.
		@return Gibt false zurück, falls nicht alle Frames gelockt werden konnten.
	*/
	bool LockAllFrames();

	/**
		@brief Unlockt alle Frames.
		@return Gibt false zurück, falls nicht alles Frames freigegeben werden konnten.
	*/
	bool UnlockAllFrames();

	/**
		@brief Diese Methode aktualisiert die Parameter (Größe, Position) der Animation anhand des aktuellen Frames.

		Diese Methode muss bei jedem Framewechsel aufgerufen werden damit der RenderObject-Manager immer aktuelle Daten hat.
	*/
	void ComputeCurrentCharacteristics();

	/**
		@brief Berechnet den Abstand zwischen dem linken Rand und dem Hotspot auf X-Achse in der aktuellen Darstellung.
	*/
	int ComputeXModifier() const;

	/**
		@brief Berechnet den Abstand zwischen dem linken Rand und dem Hotspot auf X-Achse in der aktuellen Darstellung.
	*/
	int ComputeYModifier() const;

	void InitMembers();
	void PersistCallbackVector(BS_OutputPersistenceBlock & Writer, const std::vector<ANIMATION_CALLBACK_DATA> & Vector);
	void UnpersistCallbackVector(BS_InputPersistenceBlock & Reader, std::vector<ANIMATION_CALLBACK_DATA> & Vector);
	BS_AnimationDescription * GetAnimationDescription() const;
	void InitializeAnimationResource(const std::string &FileName);
};

#endif
