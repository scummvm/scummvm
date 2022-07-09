/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __dgRef__
#define __dgRef__

#include "dgStdafx.h"
#include "dgCRC.h"
#include "dgRtti.h"
#include "dgDebug.h"
#include "dgMemory.h"


struct dgRefFlags
{
	dgRefFlags ();
	inline dgInt32 operator = (dgInt32 val);

	dgUnsigned8 m_alive; 
	dgUnsigned8 m_userFlag0;
	dgUnsigned8 m_userFlag1;
	dgUnsigned8 m_userFlag2;
//	dgUnsigned32 m_userFlag3	: 1;
//	dgUnsigned32 m_userFlag4	: 1;
//	dgUnsigned32 m_userFlag5	: 1;
//	dgUnsigned32 m_userFlag6	: 1;

	dgUnsigned32 m_ref;
};


class dgRef: public dgRefFlags
{
   public:
	dgRef ();
	dgRef (const char *name);
	dgRef (dgUnsigned32 idArg);
	dgRef(const dgRef &Clone);
	dgRef *AddRef () ;
	dgInt32 Release ();
	dgInt32 GetRefCount() const;

	DG_CLASS_ALLOCATOR(allocator)

	virtual dgRef *CreateClone ()	const;
	virtual dgUnsigned32 GetTypeId () const;
	virtual bool IsType (dgUnsigned32 typeId) const;

	bool GetUserFlag0 () const;
	bool GetUserFlag1 () const;
	void SetUserFlag0 (bool flags);
	void SetUserFlag1 (bool flags);

	bool IsAlive() const;
	virtual void Kill(); 
	virtual void Unkill();

	const char* GetName () const;
	dgUnsigned32 GetNameID () const;
	inline void SetNameID (dgUnsigned32 newID);
	virtual inline void SetName (const char *name);

	void AttachRef (dgRef **oldRef, dgRef *newRef);

	
	bool IsTypeByName (const char *typeName) const;
	static dgUnsigned32 GetRttiType();

	protected:
	virtual ~dgRef (); 

	private:
	dgUnsigned32 m_id;
	static dgRtti m_rtti;
};



inline dgRefFlags::dgRefFlags ()
{
	*this = 0;
	m_alive = true;
	m_ref = 1;
}

inline dgInt32 dgRefFlags::operator = (dgInt32 val)
{
	dgInt32* ptr;
	ptr = &(*(dgInt32*)this);
	*ptr = val;
	return val;
}



inline dgRef::dgRef ()
{
	m_id = 0;
}

inline dgRef::dgRef (const char *name)
{
	SetName (name);
}

inline dgRef::dgRef (dgUnsigned32 idArg)
{
	SetNameID (idArg);
}

inline dgRef::dgRef(const dgRef &Clone)
{
	m_id = Clone.m_id;
}

inline dgRef::~dgRef () 
{
}

inline dgRef *dgRef::AddRef () 
{
	m_ref ++;
	_ASSERTE (m_ref < ((1<<24) - 1));
	return this;
}

inline dgInt32 dgRef::Release ()
{
	m_ref --;
	if (m_ref) {
		return dgInt32 (m_ref);
	}
	delete this;
	return 0;
}

inline dgRef *dgRef::CreateClone () const
{
	_ASSERTE (0);
	return NULL;
}


inline dgUnsigned32 dgRef::GetTypeId () const
{
	return m_rtti.GetTypeId ();
}

inline bool dgRef::IsType (dgUnsigned32 typeId) const
{
	return m_rtti.IsTypeID (typeId);
}

inline dgUnsigned32 dgRef::GetRttiType()
{
	return m_rtti.GetTypeId();
}

inline bool dgRef::IsTypeByName (const char *typeName) const
{
	return IsType (dgCRC (typeName,  (dgInt32) strlen (typeName)));
}


inline bool dgRef::GetUserFlag0 () const
{
	return m_userFlag0 ? true : false;
}

inline bool dgRef::GetUserFlag1 () const
{
	return m_userFlag1 ? true : false;
}


inline void dgRef::SetUserFlag0 (bool flags)
{
	m_userFlag0 = dgUnsigned8 (flags);
}

inline void dgRef::SetUserFlag1 (bool flags)
{
	m_userFlag1 = dgUnsigned8 (flags);
}


inline bool dgRef::IsAlive() const
{
	return m_alive ? true : false;
}

inline void dgRef::Kill() 
{
	m_alive = false;
}

inline void dgRef::Unkill() 
{
	m_alive = true;
}

inline void dgRef::SetNameID (dgUnsigned32 newID)
{
	m_id = newID;
}

inline dgUnsigned32 dgRef::GetNameID () const
{
	return m_id;
}

inline void dgRef::SetName (const char *name)
{
	SetNameID (0);
	if (name) {
		SetNameID (dgCRC (name));
	}
}

inline const char* dgRef::GetName () const
{
	return dgInverseCRC (GetNameID ());
}

inline dgInt32 dgRef::GetRefCount() const
{
	return dgInt32 (m_ref);
}

#endif

