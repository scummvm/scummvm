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

#ifndef AGS_LIB_FREETYPE_FTOBJECT_H
#define AGS_LIB_FREETYPE_FTOBJECT_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/fthash.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


typedef struct FT_ObjectRec_ *FT_Object;
typedef const struct FT_ClassRec_ *FT_Class;
typedef const struct FT_TypeRec_ *FT_Type;

typedef struct FT_ObjectRec_ {
	FT_Class  clazz;
	FT_Int    ref_count;
} FT_ObjectRec;

#define FT_OBJECT(x) ((FT_Object)(x))
#define FT_OBJECT_P(x) ((FT_Object *)(x))
#define FT_OBJECT__CLASS(x) FT_OBJECT(x)->clazz
#define FT_OBJECT__REF_COUNT(x) FT_OBJECT(x)->ref_count
#define FT_OBJECT__MEMORY(x) FT_CLASS__MEMORY(FT_OBJECT(x)->clazz)
#define FT_OBJECT__LIBRARY(x) FT_CLASS__LIBRARY(FT_OBJECT(x)->clazz)

typedef FT_Error (*FT_Object_InitFunc)(FT_Object object, FT_Pointer init_data);

typedef void (*FT_Object_DoneFunc)(FT_Object object);

typedef struct FT_ClassRec_ {
	FT_ObjectRec        object;
	FT_UInt32           magic;
	FT_Class            super;
	FT_Type             type;
	FT_Memory           memory;
	FT_Library          library;
	FT_Pointer          info;

	FT_Object_DoneFunc  class_done;

	FT_UInt             obj_size;
	FT_Object_InitFunc  obj_init;
	FT_Object_DoneFunc  obj_done;
} FT_ClassRec;


#define  FT_CLASS(x)    ((FT_Class)(x))
#define  FT_CLASS_P(x)  ((FT_Class*)(x))
#define  FT_CLASS__MEMORY(x)   FT_CLASS(x)->memory
#define  FT_CLASS__LIBRARY(x)  FT_CLASS(x)->library
#define  FT_CLASS__TYPE(x)     FT_CLASS(x)->type
#define  FT_CLASS__INFO(x)     FT_CLASS(x)->info
#define  FT_CLASS__MAGIC(x)    FT_CLASS(x)->magic

typedef struct FT_TypeRec_ {
	const char*         name;
	FT_Type             super;

	FT_UInt             class_size;
	FT_Object_InitFunc  class_init;
	FT_Object_DoneFunc  class_done;

	FT_UInt             obj_size;
	FT_Object_InitFunc  obj_init;
	FT_Object_DoneFunc  obj_done;
} FT_TypeRec;

// TODO: ftobject.cpp is not compiled - remove functions
#define  FT_TYPE(x)  ((FT_Type)(x))

FT_BASE(FT_Int)
ft_object_check(FT_Pointer obj);

FT_BASE(FT_Int)
ft_object_is_a(FT_Pointer obj, FT_Class clazz);

FT_BASE(FT_Error)
ft_object_create(FT_Object *aobject, FT_Class clazz, FT_Pointer init_data);

FT_BASE(FT_Error)
ft_object_create_from_type(FT_Object *aobject, FT_Type type, FT_Pointer init_data, FT_Library library);

#define FT_OBJ_CREATE(_obj, _clazz, _init) ft_object_create(FT_OBJECT_P(&(_obj)), _clazz, _init)

#define FT_CREATE(_obj, _clazz, _init) FT_SET_ERROR(FT_OBJ_CREATE(_obj, _clazz, _init))

#define FT_OBJ_CREATE_FROM_TYPE(_obj, _type, _init, _lib) ft_object_create_from_type(FT_OBJECT_P(&(_obj)), _type, _init, _lib)

#define FT_CREATE_FROM_TYPE(_obj, _type, _init, _lib) FT_SET_ERROR(FT_OBJ_CREATE_FROM_TYPE(_obj, _type, _init, _lib))

FT_BASE(FT_Error)
ft_class_from_type(FT_Class *aclass, FT_Type type, FT_Library library);

typedef struct FT_ClassHNodeRec_ *FT_ClassHNode;

typedef struct FT_ClassHNodeRec_ {
	FT_HashNodeRec  hnode;
	FT_Type         type;
	FT_Class        clazz;
} FT_ClassHNodeRec;

typedef struct FT_MetaClassRec_ {
	FT_ClassRec   clazz;         /* the meta-class is a class itself */
	FT_HashRec    type_to_class; /* the type => class hash table */
} FT_MetaClassRec, *FT_MetaClass;

/* initialize meta class */
FT_BASE(FT_Error)
ft_metaclass_init(FT_MetaClass meta, FT_Library library);

/* finalize meta class - destroy all registered class objects */
FT_BASE(void)
ft_metaclass_done(FT_MetaClass meta);


FT_END_HEADER


} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTOBJECT_H */
