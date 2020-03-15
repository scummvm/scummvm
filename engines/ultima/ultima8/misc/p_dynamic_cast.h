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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_MISC_P_DYNAMIC_CAST_H
#define ULTIMA8_MISC_P_DYNAMIC_CAST_H

namespace Ultima {
namespace Ultima8 {

// The Pentagram dynamic cast
template<class A, class B> inline A p_dynamic_cast(B *object) {
	if (object && object->IsOfType(static_cast<A>(0)->ClassType)) return static_cast<A>(object);
	return nullptr;
}

// This is just a 'type' used to differentiate each class.
struct RunTimeClassType {
	const char *_className;
	inline bool operator == (const RunTimeClassType &other) const {
		return this == &other;
	}
};

//
// Include this in every class. It sets up the class to be able to use
// p_dynamic_cast. Make sure this is public!
//
#define ENABLE_RUNTIME_CLASSTYPE()                                              \
	static const RunTimeClassType   ClassType;                                  \
	virtual bool IsOfType(const RunTimeClassType & type) const override;        \
	virtual bool IsOfType(const char * type) const override;                    \
	template<class Type> inline bool IsOfType() const { return IsOfType(Type::ClassType); }   \
	virtual const RunTimeClassType & GetClassType() const override { return ClassType; }

#define ENABLE_RUNTIME_CLASSTYPE_BASE()                                         \
	static const RunTimeClassType   ClassType;                                  \
	virtual bool IsOfType(const RunTimeClassType & type) const;                 \
	virtual bool IsOfType(const char * type) const;                             \
	template<class Type> inline bool IsOfType() const { return IsOfType(Type::ClassType); }   \
	virtual const RunTimeClassType & GetClassType() const { return ClassType; }



//
// Define this in the source files of base classes
//
#define DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Classname)         \
	const RunTimeClassType Classname::ClassType = {                 \
	#Classname                                                      \
    };                                                                  \
	\
	bool Classname::IsOfType(const RunTimeClassType &classType) const   \
	{                                                                   \
		if (classType == ClassType) return true;                        \
		return false;                                                   \
	}                                                                   \
	\
	bool Classname::IsOfType(const char *classType) const               \
	{                                                                   \
		if (!Std::strcmp(classType,ClassType._className)) return true;  \
		return false;                                                   \
	}


//
// Define this in the source files of child classes, with 1 parent
//
#define DEFINE_RUNTIME_CLASSTYPE_CODE(Classname,ParentClassname)    \
	const RunTimeClassType Classname::ClassType = {                     \
	                                                                    #Classname                                                      \
	                                              };                                                                  \
	\
	bool Classname::IsOfType(const RunTimeClassType & classType) const  \
	{                                                                   \
		if (classType == ClassType) return true;                        \
		return ParentClassname::IsOfType(classType);                    \
	}                                                                   \
	\
	bool Classname::IsOfType(const char *typeName) const                \
	{                                                                   \
		if (!Std::strcmp(typeName,ClassType._className)) return true;   \
		return ParentClassname::IsOfType(typeName);                     \
	}



//
// Define this in the source files of child classes, with 2 parents
//
#define DEFINE_RUNTIME_CLASSTYPE_CODE_MULTI2(Classname,Parent1,Parent2) \
	const RunTimeClassType Classname::ClassType = {                         \
	                                                                        #Classname                                                          \
	                                              };                                                                      \
	\
	bool Classname::IsOfType(const RunTimeClassType &type) const            \
	{                                                                       \
		typedef Parent1 P1;                                                 \
		typedef Parent2 P2;                                                 \
		if (type == ClassType) return true;                                 \
		bool ret = P1::IsOfType(type);                                      \
		if (ret) return true;                                               \
		return P2::IsOfType(type);                                          \
	}                                                                       \
	\
	bool Classname::IsOfType(const char * type) const                       \
	{                                                                       \
		typedef Parent1 P1;                                                 \
		typedef Parent2 P2;                                                 \
		if (!Std::strcmp(type,ClassType._className)) return true;           \
		bool ret = P1::IsOfType(type);                                      \
		if (ret) return true;                                               \
		return P2::IsOfType(type);                                          \
	}

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
