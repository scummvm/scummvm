#ifndef __FT2_1_3_OBJECT_H__
#define __FT2_1_3_OBJECT_H__

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"

FT2_1_3_BEGIN_HEADER

/**************************************************************
 *
 * @type: FT2_1_3_Object
 *
 * @description:
 *   handle to a FreeType Object. See @FT2_1_3_ObjectRec
 */
typedef struct FT2_1_3_ObjectRec_*        FT2_1_3_Object;


/**************************************************************
 *
 * @type: FT2_1_3_Class
 *
 * @description:
 *   handle to a constant class handle to a FreeType Object.
 *
 *   Note that a class is itself a @FT2_1_3_Object and are dynamically
 *   allocated on the heap.
 *
 * @also:
 *  @FT2_1_3_ClassRec, @FT2_1_3_Object, @FT2_1_3_ObjectRec, @FT2_1_3_Type, @FT2_1_3_TypeRec
 */
typedef const struct FT2_1_3_ClassRec_*   FT2_1_3_Class;


/**************************************************************
 *
 * @type: FT2_1_3_Type
 *
 * @description:
 *   handle to a constant structure (of type @FT2_1_3_TypeRec) used
 *   to describe a given @FT2_1_3_Class type to the FreeType object
 *   sub-system.
 */
typedef const struct FT2_1_3_TypeRec_*    FT2_1_3_Type;



/**************************************************************
 *
 * @struct: FT2_1_3_ObjectRec
 *
 * @description:
 *   a structure describing the root fields of all @FT2_1_3_Object
 *   class instances in FreeType
 *
 * @fields:
 *   clazz     :: handle to the object's class
 *   ref_count :: object's reference count. Starts at 1
 */
typedef struct FT2_1_3_ObjectRec_ {
	FT2_1_3_Class  clazz;
	FT2_1_3_Int    ref_count;

} FT2_1_3_ObjectRec;


/**************************************************************
 *
 * @macro: FT2_1_3_OBJECT (x)
 *
 * @description:
 *   a useful macro to type-cast anything to a @FT2_1_3_Object
 *   handle. No check performed..
 */
#define  FT2_1_3_OBJECT(x)    ((FT2_1_3_Object)(x))


/**************************************************************
 *
 * @macro: FT2_1_3_OBJECT_P (x)
 *
 * @description:
 *   a useful macro to type-cast anything to a pointer to
 *   @FT2_1_3_Object handle.
 */
#define  FT2_1_3_OBJECT_P(x)  ((FT2_1_3_Object*)(x))


/**************************************************************
 *
 * @macro: FT2_1_3_OBJECT__CLASS (obj)
 *
 * @description:
 *   a useful macro to return the class of any object
 */
#define  FT2_1_3_OBJECT__CLASS(x)      FT2_1_3_OBJECT(x)->clazz


/**************************************************************
 *
 * @macro: FT2_1_3_OBJECT__REF_COUNT (obj)
 *
 * @description:
 *   a useful macro to return the reference count of any object
 */
#define  FT2_1_3_OBJECT__REF_COUNT(x)  FT2_1_3_OBJECT(x)->ref_count


/**************************************************************
 *
 * @macro: FT2_1_3_OBJECT__MEMORY (obj)
 *
 * @description:
 *   a useful macro to return a handle to the memory manager
 *   used to allocate a given object
 */
#define  FT2_1_3_OBJECT__MEMORY(x)     FT2_1_3_CLASS__MEMORY(FT2_1_3_OBJECT(x)->clazz)


/**************************************************************
 *
 * @macro: FT2_1_3_OBJECT__LIBRARY (obj)
 *
 * @description:
 *   a useful macro to return a handle to the library handle
 *   that owns the object
 */
#define  FT2_1_3_OBJECT__LIBRARY(x)    FT2_1_3_CLASS__LIBRARY(FT2_1_3_OBJECT(x)->clazz)


/**************************************************************
 *
 * @functype: FT2_1_3_Object_InitFunc
 *
 * @description:
 *   a function used to initialize a new object
 *
 * @input:
 *   object    :: target object handle
 *   init_data :: optional pointer to initialization data
 *
 * @return:
 *   error code. 0 means success
 */
typedef FT2_1_3_Error  (*FT2_1_3_Object_InitFunc)( FT2_1_3_Object   object,
        FT2_1_3_Pointer  init_data );

/**************************************************************
 *
 * @functype: FT2_1_3_Object_DoneFunc
 *
 * @description:
 *   a function used to finalize a given object
 *
 * @input:
 *   object    :: handle to target object
 */
typedef void  (*FT2_1_3_Object_DoneFunc)( FT2_1_3_Object   object );


/**************************************************************
 *
 * @struct: FT2_1_3_ClassRec
 *
 * @description:
 *   a structure used to describe a given object class within
 *   FreeType
 *
 * @fields:
 *   object   :: root @FT2_1_3_ObjectRec fields, since each class is
 *               itself an object. (it's an instance of the
 *               "metaclass", a special object of the FreeType
 *               object sub-system.)
 *
 *   magic    :: a 32-bit magic number used for decoding
 *   super    :: pointer to super class
 *   type     :: the @FT2_1_3_Type descriptor of this class
 *   memory   :: the current memory manager handle
 *   library  :: the current library handle
 *   info     :: an opaque pointer to class-specific information
 *               managed by the FreeType object sub-system
 *
 *   class_done :: the class destructor function
 *
 *   obj_size :: size of class instances in bytes
 *   obj_init :: class instance constructor
 *   obj_done :: class instance destructor
 */
typedef struct FT2_1_3_ClassRec_ {
	FT2_1_3_ObjectRec        object;
	FT2_1_3_UInt32           magic;
	FT2_1_3_Class            super;
	FT2_1_3_Type             type;
	FT2_1_3_Memory           memory;
	FT2_1_3_Library          library;
	FT2_1_3_Pointer          info;

	FT2_1_3_Object_DoneFunc  class_done;

	FT2_1_3_UInt             obj_size;
	FT2_1_3_Object_InitFunc  obj_init;
	FT2_1_3_Object_DoneFunc  obj_done;

} FT2_1_3_ClassRec;


/**************************************************************
 *
 * @macro: FT2_1_3_CLASS (x)
 *
 * @description:
 *   a useful macro to convert anything to a class handle
 *   without checks
 */
#define  FT2_1_3_CLASS(x)    ((FT2_1_3_Class)(x))


/**************************************************************
 *
 * @macro: FT2_1_3_CLASS_P (x)
 *
 * @description:
 *   a useful macro to convert anything to a pointer to a class
 *   handle without checks
 */
#define  FT2_1_3_CLASS_P(x)  ((FT2_1_3_Class*)(x))


/**************************************************************
 *
 * @macro: FT2_1_3_CLASS__MEMORY (clazz)
 *
 * @description:
 *   a useful macro to return the memory manager handle of a
 *   given class
 */
#define  FT2_1_3_CLASS__MEMORY(x)   FT2_1_3_CLASS(x)->memory


/**************************************************************
 *
 * @macro: FT2_1_3_CLASS__LIBRARY (clazz)
 *
 * @description:
 *   a useful macro to return the library handle of a
 *   given class
 */
#define  FT2_1_3_CLASS__LIBRARY(x)  FT2_1_3_CLASS(x)->library



/**************************************************************
 *
 * @macro: FT2_1_3_CLASS__TYPE (clazz)
 *
 * @description:
 *   a useful macro to return the type of a given class
 *   given class
 */
#define  FT2_1_3_CLASS__TYPE(x)     FT2_1_3_CLASS(x)->type

/* */
#define  FT2_1_3_CLASS__INFO(x)     FT2_1_3_CLASS(x)->info
#define  FT2_1_3_CLASS__MAGIC(x)    FT2_1_3_CLASS(x)->magic


/**************************************************************
 *
 * @struct: FT2_1_3_TypeRec
 *
 * @description:
 *   a structure used to describe a given class to the FreeType
 *   object sub-system.
 *
 * @fields:
 *   name       :: class name. only used for debugging
 *   super      :: type of super-class. NULL if none
 *
 *   class_size :: size of class structure in bytes
 *   class_init :: class constructor
 *   class_done :: class finalizer
 *
 *   obj_size   :: instance size in bytes
 *   obj_init   :: instance constructor. can be NULL
 *   obj_done   :: instance destructor. can be NULL
 *
 * @note:
 *   if 'obj_init' is NULL, the class will use it's parent
 *   constructor, if any
 *
 *   if 'obj_done' is NULL, the class will use it's parent
 *   finalizer, if any
 *
 *   the object sub-system allocates a new class, copies
 *   the content of its super-class into the new structure,
 *   _then_ calls 'clazz_init'.
 *
 *   'class_init' and 'class_done' can be NULL, in which case
 *   the parent's class constructor and destructor wil be used
 */
typedef struct FT2_1_3_TypeRec_ {
	const char*         name;
	FT2_1_3_Type             super;

	FT2_1_3_UInt             class_size;
	FT2_1_3_Object_InitFunc  class_init;
	FT2_1_3_Object_DoneFunc  class_done;

	FT2_1_3_UInt             obj_size;
	FT2_1_3_Object_InitFunc  obj_init;
	FT2_1_3_Object_DoneFunc  obj_done;

} FT2_1_3_TypeRec;


/**************************************************************
 *
 * @macro: FT2_1_3_TYPE (x)
 *
 * @description:
 *   a useful macro to convert anything to a class type handle
 *   without checks
 */
#define  FT2_1_3_TYPE(x)  ((FT2_1_3_Type)(x))


/**************************************************************
 *
 * @function: ft_object_check
 *
 * @description:
 *   checks that a handle points to a valid @FT2_1_3_Object
 *
 * @input:
 *   obj :: handle/pointer
 *
 * @return:
 *   1 iff the handle points to a valid object. 0 otherwise
 */
FT2_1_3_BASE( FT2_1_3_Int )
ft_object_check( FT2_1_3_Pointer  obj );


/**************************************************************
 *
 * @function: ft_object_is_a
 *
 * @description:
 *   checks that a handle points to a valid @FT2_1_3_Object that
 *   is an instance of a given class (or of any of its sub-classes)
 *
 * @input:
 *   obj   :: handle/pointer
 *   clazz :: class handle to check
 *
 * @return:
 *   1 iff the handle points to a valid 'clazz' instance. 0
 *   otherwise.
 */
FT2_1_3_BASE( FT2_1_3_Int )
ft_object_is_a( FT2_1_3_Pointer  obj,
                FT2_1_3_Class    clazz );


/**************************************************************
 *
 * @function: ft_object_create
 *
 * @description:
 *   create a new object (class instance)
 *
 * @output:
 *   aobject   :: new object handle. NULL in case of error
 *
 * @input:
 *   clazz     :: object's class pointer
 *   init_data :: optional pointer to initialization data
 *
 * @return:
 *   error code. 0 means success
 */
FT2_1_3_BASE( FT2_1_3_Error )
ft_object_create( FT2_1_3_Object  *aobject,
                  FT2_1_3_Class    clazz,
                  FT2_1_3_Pointer  init_data );


/**************************************************************
 *
 * @function: ft_object_create_from_type
 *
 * @description:
 *   create a new object (class instance) from a @FT2_1_3_Type
 *
 * @output:
 *   aobject   :: new object handle. NULL in case of error
 *
 * @input:
 *   type      :: object's type descriptor
 *   init_data :: optional pointer to initialization data
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   this function is slower than @ft_object_create
 *
 *   this is equivalent to calling @ft_class_from_type followed by
 *   @ft_object_create
 */
FT2_1_3_BASE( FT2_1_3_Error )
ft_object_create_from_type( FT2_1_3_Object  *aobject,
                            FT2_1_3_Type     type,
                            FT2_1_3_Pointer  init_data,
                            FT2_1_3_Library  library );



/**************************************************************
 *
 * @macro FT2_1_3_OBJ_CREATE (object,class,init)
 *
 * @description:
 *   a convenient macro used to create new objects. see
 *   @ft_object_create for details
 */
#define  FT2_1_3_OBJ_CREATE( _obj, _clazz, _init )   \
               ft_object_create( FT2_1_3_OBJECT_P(&(_obj)), _clazz, _init )


/**************************************************************
 *
 * @macro FT2_1_3_CREATE (object,class,init)
 *
 * @description:
 *   a convenient macro used to create new objects. It also
 *   sets an _implicit_ local variable named "error" to the error
 *   code returned by the object constructor.
 */
#define  FT2_1_3_CREATE( _obj, _clazz, _init )  \
             FT2_1_3_SET_ERROR( FT2_1_3_OBJ_CREATE( _obj, _clazz, _init ) )

/**************************************************************
 *
 * @macro FT2_1_3_OBJ_CREATE_FROM_TYPE (object,type,init)
 *
 * @description:
 *   a convenient macro used to create new objects. see
 *   @ft_object_create_from_type for details
 */
#define  FT2_1_3_OBJ_CREATE_FROM_TYPE( _obj, _type, _init, _lib )   \
               ft_object_create_from_type( FT2_1_3_OBJECT_P(&(_obj)), _type, _init, _lib )


/**************************************************************
 *
 * @macro FT2_1_3_CREATE_FROM_TYPE (object,type,init)
 *
 * @description:
 *   a convenient macro used to create new objects. It also
 *   sets an _implicit_ local variable named "error" to the error
 *   code returned by the object constructor.
 */
#define  FT2_1_3_CREATE_FROM_TYPE( _obj, _type, _init, _lib )  \
             FT2_1_3_SET_ERROR( FT2_1_3_OBJ_CREATE_FROM_TYPE( _obj, _type, _init, _lib ) )


/* */

/**************************************************************
 *
 * @function: ft_class_from_type
 *
 * @description:
 *   retrieves the class object corresponding to a given type
 *   descriptor. The class is created when needed
 *
 * @output:
 *   aclass  :: handle to corresponding class object. NULL in
 *              case of error
 *
 * @input:
 *   type    :: type descriptor handle
 *   library :: library handle
 *
 * @return:
 *   error code. 0 means success
 */
FT2_1_3_BASE( FT2_1_3_Error )
ft_class_from_type( FT2_1_3_Class   *aclass,
                    FT2_1_3_Type     type,
                    FT2_1_3_Library  library );


/* */

#include FT2_1_3_INTERNAL_HASH_H

typedef struct FT2_1_3_ClassHNodeRec_*  FT2_1_3_ClassHNode;

typedef struct FT2_1_3_ClassHNodeRec_ {
	FT2_1_3_HashNodeRec  hnode;
	FT2_1_3_Type         type;
	FT2_1_3_Class        clazz;

} FT2_1_3_ClassHNodeRec;

typedef struct FT2_1_3_MetaClassRec_ {
	FT2_1_3_ClassRec   clazz;         /* the meta-class is a class itself */
	FT2_1_3_HashRec    type_to_class; /* the type => class hash table */

} FT2_1_3_MetaClassRec, *FT2_1_3_MetaClass;


/* initialize meta class */
FT2_1_3_BASE( FT2_1_3_Error )
ft_metaclass_init( FT2_1_3_MetaClass  meta,
                   FT2_1_3_Library    library );

/* finalize meta class - destroy all registered class objects */
FT2_1_3_BASE( void )
ft_metaclass_done( FT2_1_3_MetaClass  meta );

/* */

FT2_1_3_END_HEADER

#endif /* __FT2_1_3_OBJECT_H__ */
