#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobject.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"

#define  FT2_1_3_MAGIC_DEATH   0xDEADdead
#define  FT2_1_3_MAGIC_CLASS   0x12345678

#define  FT2_1_3_TYPE_HASH(x)  (( (FT_UInt32)(x) >> 2 )^( (FT_UInt32)(x) >> 10 ))

#define  FT2_1_3_OBJECT_CHECK(o)                                  \
           ( FT2_1_3_OBJECT(o)               != NULL           && \
             FT2_1_3_OBJECT(o)->clazz        != NULL           && \
             FT2_1_3_OBJECT(o)->ref_count    >= 1              && \
             FT2_1_3_OBJECT(o)->clazz->magic == FT2_1_3_MAGIC_CLASS )

#define  FT2_1_3_CLASS_CHECK(c)  \
           ( FT2_1_3_CLASS(c) != NULL && FT2_1_3_CLASS(c)->magic == FT2_1_3_MAGIC_CLASS )

#define  FT2_1_3_ASSERT_IS_CLASS(c)  FT2_1_3_ASSERT( FT2_1_3_CLASS_CHECK(c) )

/*******************************************************************/
/*******************************************************************/
/*****                                                         *****/
/*****                                                         *****/
/*****                  M E T A - C L A S S                    *****/
/*****                                                         *****/
/*****                                                         *****/
/*******************************************************************/
/*******************************************************************/

/* forward declaration */
FT2_1_3_BASE_DEF( FT_Error )
ft_metaclass_init( FT2_1_3_MetaClass  meta,
                   FT2_1_3_Library    library );

/* forward declaration */
FT2_1_3_BASE_DEF( void )
ft_metaclass_done( FT2_1_3_MetaClass  meta );


/* class type for the meta-class itself */
static const FT2_1_3_TypeRec  ft_meta_class_type = {
	"FT2.MetaClass",
	NULL,

	sizeof( FT2_1_3_MetaClassRec ),
	(FT2_1_3_Object_InitFunc)  ft_metaclass_init,
	(FT2_1_3_Object_DoneFunc)  ft_metaclass_done,

	sizeof( FT2_1_3_ClassRec ),
	(FT2_1_3_Object_InitFunc)  NULL,
	(FT2_1_3_Object_DoneFunc)  NULL
};




/* destroy a given class */
static void
ft_class_hnode_destroy( FT2_1_3_ClassHNode  node ) {
	FT2_1_3_Class   clazz  = node->clazz;
	FT2_1_3_Memory  memory = clazz->memory;

	if ( clazz->class_done )
		clazz->class_done( (FT2_1_3_Object) clazz );

	FT2_1_3_FREE( clazz );

	node->clazz = NULL;
	node->type  = NULL;

	FT2_1_3_FREE( node );
}


static FT_Int
ft_type_equal( FT2_1_3_Type  type1,
               FT2_1_3_Type  type2 ) {
	if ( type1 == type2 )
		goto Ok;

	if ( type1 == NULL || type2 == NULL )
		goto Fail;

	/* compare parent types */
	if ( type1->super != type2->super ) {
		if ( type1->super == NULL           ||
		        type2->super == NULL           ||
		        !ft_type_equal( type1, type2 ) )
			goto Fail;
	}

	/* compare type names */
	if ( type1->name != type2->name ) {
		if ( type1->name == NULL                        ||
		        type2->name == NULL                        ||
		        ft_strcmp( type1->name, type2->name ) != 0 )
			goto Fail;
	}

	/* compare the other type fields */
	if ( type1->class_size != type2->class_size ||
	        type1->class_init != type2->class_init ||
	        type1->class_done != type2->class_done ||
	        type1->obj_size   != type2->obj_size   ||
	        type1->obj_init   != type2->obj_init   ||
	        type1->obj_done   != type2->obj_done   )
		goto Fail;

Ok:
	return 1;

Fail:
	return 0;
}


static FT_Int
ft_class_hnode_equal( const FT2_1_3_ClassHNode  node1,
                      const FT2_1_3_ClassHNode  node2 ) {
	FT2_1_3_Type  type1 = node1->type;
	FT2_1_3_Type  type2 = node2->type;

	/* comparing the pointers should work in 99% of cases */
	return ( type1 == type2 ) ? 1 : ft_type_equal( type1, type2 );
}


FT2_1_3_BASE_DEF( void )
ft_metaclass_done( FT2_1_3_MetaClass  meta ) {
	/* clear all classes */
	ft_hash_done( &meta->type_to_class,
	              (FT2_1_3_Hash_ForeachFunc) ft_class_hnode_destroy,
	              NULL );

	meta->clazz.object.clazz     = NULL;
	meta->clazz.object.ref_count = 0;
	meta->clazz.magic            = FT2_1_3_MAGIC_DEATH;
}


FT2_1_3_BASE_DEF( FT_Error )
ft_metaclass_init( FT2_1_3_MetaClass  meta,
                   FT2_1_3_Library    library ) {
	FT2_1_3_ClassRec*  clazz = (FT2_1_3_ClassRec*) &meta->clazz;

	/* the meta-class is its OWN class !! */
	clazz->object.clazz     = (FT2_1_3_Class) clazz;
	clazz->object.ref_count = 1;
	clazz->magic            = FT2_1_3_MAGIC_CLASS;
	clazz->library          = library;
	clazz->memory           = library->memory;
	clazz->type             = &ft_meta_class_type;
	clazz->info             = NULL;

	clazz->class_done       = (FT2_1_3_Object_DoneFunc) ft_metaclass_done;

	clazz->obj_size         = sizeof( FT2_1_3_ClassRec );
	clazz->obj_init         = NULL;
	clazz->obj_done         = NULL;

	return ft_hash_init( &meta->type_to_class,
	                     (FT2_1_3_Hash_EqualFunc) ft_class_hnode_equal,
	                     library->memory );
}


/* find or create the class corresponding to a given type */
/* note that this function will retunr NULL in case of    */
/* memory overflow                                        */
/*                                                        */
static FT2_1_3_Class
ft_metaclass_get_class( FT2_1_3_MetaClass  meta,
                        FT2_1_3_Type       ctype ) {
	FT2_1_3_ClassHNodeRec   keynode, *node, **pnode;
	FT2_1_3_Memory          memory;
	FT2_1_3_ClassRec*       clazz;
	FT2_1_3_Class           parent;
	FT_Error           error;

	keynode.hnode.hash = FT2_1_3_TYPE_HASH( ctype );
	keynode.type       = ctype;

	pnode = (FT2_1_3_ClassHNode*) ft_hash_lookup( &meta->type_to_class,
	        (FT2_1_3_HashNode) &keynode );
	node  = *pnode;
	if ( node != NULL ) {
		clazz = (FT2_1_3_ClassRec*) node->clazz;
		goto Exit;
	}

	memory = FT2_1_3_CLASS__MEMORY(meta);
	clazz  = NULL;
	parent = NULL;
	if ( ctype->super != NULL ) {
		FT2_1_3_ASSERT( ctype->super->class_size <= ctype->class_size );
		FT2_1_3_ASSERT( ctype->super->obj_size   <= ctype->obj_size   );

		parent = ft_metaclass_get_class( meta, ctype->super );
	}

	if ( !FT2_1_3_NEW( node ) ) {
		if ( !FT2_1_3_ALLOC( clazz, ctype->class_size ) ) {
			if ( parent )
				FT2_1_3_MEM_COPY( (FT2_1_3_ClassRec*)clazz, parent, parent->type->class_size );

			clazz->object.clazz     = (FT2_1_3_Class) meta;
			clazz->object.ref_count = 1;

			clazz->memory  = memory;
			clazz->library = FT2_1_3_CLASS__LIBRARY(meta);
			clazz->super   = parent;
			clazz->type    = ctype;
			clazz->info    = NULL;
			clazz->magic   = FT2_1_3_MAGIC_CLASS;

			clazz->class_done = ctype->class_done;
			clazz->obj_size   = ctype->obj_size;
			clazz->obj_init   = ctype->obj_init;
			clazz->obj_done   = ctype->obj_done;

			if ( parent ) {
				if ( clazz->class_done == NULL )
					clazz->class_done = parent->class_done;

				if ( clazz->obj_init == NULL )
					clazz->obj_init = parent->obj_init;

				if ( clazz->obj_done == NULL )
					clazz->obj_done = parent->obj_done;
			}

			/* find class initializer, if any */
			{
				FT2_1_3_Type             ztype = ctype;
				FT2_1_3_Object_InitFunc  cinit = NULL;

				do {
					cinit = ztype->class_init;
					if ( cinit != NULL )
						break;

					ztype = ztype->super;
				} while ( ztype != NULL );

				/* then call it when needed */
				if ( cinit != NULL )
					error = cinit( (FT2_1_3_Object) clazz, NULL );
			}
		}

		if (error) {
			if ( clazz ) {
				/* we always call the class destructor when    */
				/* an error was detected in the constructor !! */
				if ( clazz->class_done )
					clazz->class_done( (FT2_1_3_Object) clazz );

				FT2_1_3_FREE( clazz );
			}
			FT2_1_3_FREE( node );
		}
	}

Exit:
	return  (FT2_1_3_Class) clazz;
}














FT2_1_3_BASE_DEF( FT_Int )
ft_object_check( FT_Pointer  obj ) {
	return FT2_1_3_OBJECT_CHECK(obj);
}


FT2_1_3_BASE_DEF( FT_Int )
ft_object_is_a( FT_Pointer  obj,
                FT2_1_3_Class    clazz ) {
	if ( FT2_1_3_OBJECT_CHECK(obj) ) {
		FT2_1_3_Class   c = FT2_1_3_OBJECT__CLASS(obj);

		do {
			if ( c == clazz )
				return 1;

			c = c->super;
		} while ( c == NULL );

		return (clazz == NULL);
	}
	return 0;
}


FT2_1_3_BASE_DEF( FT_Error )
ft_object_create( FT2_1_3_Object  *pobject,
                  FT2_1_3_Class    clazz,
                  FT_Pointer  init_data ) {
	FT2_1_3_Memory  memory;
	FT_Error   error;
	FT2_1_3_Object  obj;

	FT2_1_3_ASSERT_IS_CLASS(clazz);

	memory = FT2_1_3_CLASS__MEMORY(clazz);
	if ( !FT2_1_3_ALLOC( obj, clazz->obj_size ) ) {
		obj->clazz     = clazz;
		obj->ref_count = 1;

		if ( clazz->obj_init ) {
			error = clazz->obj_init( obj, init_data );
			if ( error ) {
				/* IMPORTANT: call the destructor when an error  */
				/*            was detected in the constructor !! */
				if ( clazz->obj_done )
					clazz->obj_done( obj );

				FT2_1_3_FREE( obj );
			}
		}
	}
	*pobject = obj;
	return error;
}


FT2_1_3_BASE_DEF( FT2_1_3_Class )
ft_class_find_by_type( FT2_1_3_Type     type,
                       FT2_1_3_Library  library ) {
	FT2_1_3_MetaClass  meta = &library->meta_class;

	return ft_metaclass_get_class( meta, type );
}


FT2_1_3_BASE_DEF( FT_Error )
ft_object_create_from_type( FT2_1_3_Object  *pobject,
                            FT2_1_3_Type     type,
                            FT_Pointer  init_data,
                            FT2_1_3_Library  library ) {
	FT2_1_3_Class  clazz;
	FT_Error  error;

	clazz = ft_class_find_by_type( type, library );
	if ( clazz )
		error = ft_object_create( pobject, clazz, init_data );
	else {
		*pobject = NULL;
		error    = FT2_1_3_Err_Out_Of_Memory;
	}

	return error;
}
