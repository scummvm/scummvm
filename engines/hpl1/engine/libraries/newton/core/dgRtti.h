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

#ifndef __dgRtti__
#define __dgRtti__

#include "dgStdafx.h"
#include "dgCRC.h"

class dgRtti
{
	public:
	dgRtti(const char* typeName);
	dgUnsigned32 GetTypeId() const;
	bool IsTypeID(dgUnsigned32 id) const;

	private:
	dgUnsigned32 m_TypeId;
};

inline dgRtti::dgRtti(const char* typeName)
{
	m_TypeId = dgCRC (typeName,  (dgInt32) strlen (typeName));
}

inline dgUnsigned32 dgRtti::GetTypeId() const
{
	return m_TypeId;
}

inline bool dgRtti::IsTypeID (dgUnsigned32 id) const
{
	return m_TypeId == id;
}



#define dgAddRtti(baseClass)								\
	private:												\
	static dgRtti rtti; 									\
	public:													\
	virtual bool IsType (dgUnsigned32 typeId) const			\
	{														\
		if (rtti.IsTypeID (typeId)) {						\
			return true;									\
		}													\
		return baseClass::IsType (typeId);					\
	}														\
	virtual dgUnsigned32 GetTypeId () const					\
	{														\
		return rtti.GetTypeId ();							\
	}														\
	static dgUnsigned32 GetRttiType()						\
	{														\
		return rtti.GetTypeId();							\
	}


#define dgInitRtti(className)								\
	dgRtti className::rtti (#className)

#endif

