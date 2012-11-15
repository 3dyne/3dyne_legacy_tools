/* 
 * 3dyne Legacy Tools GPL Source Code
 * 
 * Copyright (C) 1996-2012 Matthias C. Berger & Simon Berger.
 * 
 * This file is part of the 3dyne Legacy Tools GPL Source Code ("3dyne Legacy
 * Tools Source Code").
 *   
 * 3dyne Legacy Tools Source Code is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * 3dyne Legacy Tools Source Code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * 3dyne Legacy Tools Source Code.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * In addition, the 3dyne Legacy Tools Source Code is also subject to certain
 * additional terms. You should have received a copy of these additional terms
 * immediately following the terms and conditions of the GNU General Public
 * License which accompanied the 3dyne Legacy Tools Source Code.
 * 
 * Contributors:
 *     Matthias C. Berger (mcb77@gmx.de) - initial API and implementation
 *     Simon Berger (simberger@gmail.com) - initial API and implementation
 */ 



// lib_hobj.h

/*
  =============================================================================
  hobjects/pairs, hmanager

  =============================================================================
*/

#ifndef __lib_hobj
#define __lib_hobj

#include <stdio.h>
#include <string.h>

#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
#include "lib_container.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
  ==================================================
  hobject/pairs stuff

  ==================================================
*/

#define HPAIR_TYPE_SIZE		( 8 )
#define HPAIR_KEY_SIZE		( 32 )
#define HPAIR_VALUE_SIZE	( 64 )
#define HOBJ_TYPE_SIZE		( 32 )
#define HOBJ_NAME_SIZE		( 32 )

#define HPAIR_VALUE_FRAG	( 60 )	// max token size of value frags

typedef struct hpair_s
{
	char		type[HPAIR_TYPE_SIZE];
	char		key[HPAIR_KEY_SIZE];
//	char		value[HPAIR_VALUE_SIZE];
	char		*value;

	// special type 'bstring'
//	char		*bytes;

	struct hpair_s	*next;
} hpair_t;

typedef struct hobj_s 
{
	char		type[HOBJ_TYPE_SIZE];
	char		name[HOBJ_NAME_SIZE];
	
	struct hpair_s	*pairs;		// list
	struct hobj_s	*hobjs;		// list

	struct hobj_s	*parent;
	struct hobj_s	*next;

	void		*extra;		// a pointer for user class rep
} hobj_t;

typedef struct hobj_search_iterator_s
{
	char		search_type[HOBJ_TYPE_SIZE];
	struct hobj_s	*hobj;
	struct hobj_s	*current;
} hobj_search_iterator_t;

typedef struct hpair_search_iterator_s
{
	char		search_key[HPAIR_KEY_SIZE];
	struct hobj_s	*hobj;
	struct hpair_s	*current;
} hpair_search_iterator_t;

hobj_t* NewClass( const char *type, const char *name );
void ClassSetName( hobj_t *self, char *name );
void ClassSetType( hobj_t *self, char *type );
char * ClassGetName( hobj_t *self );
char * ClassGetType( hobj_t *self );

void * ClassGetPrimaryKey( const void *obj );
int ClassComparePrimaryKeys( const void *key1, const void *key2 );

bool_t ClassTypeCheck( hobj_t *self, char *type );
bool_t ClassNameCheck( hobj_t *self, char *name );

void FreeClass( hobj_t *obj );
hpair_t* NewHPair( void );
hpair_t* NewHPair2( const char *type, const char *key, const char *value );
//hpair_t* NewHPairBstring( char *key, char *bytes );
void FreeHPair( hpair_t *pair );

void DumpClass( hobj_t *self );
void DumpHPair( hpair_t *self );
void DeepDumpClass( hobj_t *self );

void SetClassExtra( hobj_t *self, void *ptr );
void * GetClassExtra( hobj_t *self );

void InsertHPair( hobj_t *self, hpair_t *pair );
bool_t RemoveHPair( hobj_t *self, hpair_t *pair );
void InsertClass( hobj_t *self, hobj_t *obj );
bool_t RemoveClass( hobj_t *self, hobj_t *obj );
bool_t RemoveClass2( hobj_t *obj ); // now objects know their parents

void RemoveAndDestroyAllHPairsOfKey( hobj_t *self, char *key );

hobj_t* DeepCopyClass( hobj_t *self );
hpair_t* CopyHPair( hpair_t *pair );

hpair_t* FindHPair( hobj_t *self, const char *key );
hobj_t* FindClass( hobj_t *self, char *name );
hobj_t* FindClassType( hobj_t *self, char *type );

void InitClassSearchIterator( hobj_search_iterator_t *iter, hobj_t *obj, const char *type );
hobj_t* SearchGetNextClass( hobj_search_iterator_t *iter );

void InitHPairSearchIterator( hpair_search_iterator_t *iter, hobj_t *obj, const char *key );
hpair_t* SearchGetNextHPair( hpair_search_iterator_t *iter );

void WriteClass( hobj_t *self, FILE * );
void WriteClassWithoutRoot( hobj_t *self, FILE * );

hobj_t* ReadClass( tokenstream_t *ts );

hobj_t * ReadClassFile( char *name );
void WriteClassFile( hobj_t *root, char *name );

void TraverseClasses( hobj_t *root, void (*func)(hobj_t *self) );

/*
  ==================================================
  hpair cast stuff

  ==================================================
*/
void HPairCastToVec3d( vec3d_t v, hpair_t *pair );
void HPairCastToVec2d( vec2d_t v, hpair_t *pair );
void HPairCastToFloat( fp_t *f, hpair_t *pair );
void HPairCastToInt( int *i, hpair_t *pair );
void HPairCastToString( char *t, hpair_t *pair );


// type safe casts
void HPairCastToVec3d_safe( vec3d_t v, hpair_t *pair );
void HPairCastToVec2d_safe( vec2d_t v, hpair_t *pair );
void HPairCastToFloat_safe( fp_t *f, hpair_t *pair );
void HPairCastToInt_safe( int *i, hpair_t *pair );
void HPairCastToString_safe( char *t, hpair_t *pair );
void HPairCastToClsref_safe( char *t, hpair_t *pair );

// easy find and cast
hpair_t * EasyFindVec3d( vec3d_t v, hobj_t *obj, const char *key );
hpair_t * EasyFindVec2d( vec2d_t v, hobj_t *obj, const char *key );
hpair_t * EasyFindFloat( fp_t *f, hobj_t *obj, const char *key );
hpair_t * EasyFindInt( int *i, hobj_t *obj, const char *key );
hpair_t * EasyFindString( char *t, hobj_t *obj, const char *key );

void HPairCastToBstring_safe( void *ptr, int *max_buf_size, hpair_t *pair );
void BstringCastToHPair( void *ptr, int size, hpair_t *pair );

void HPairCastFromVec3d( vec3d_t v, hpair_t *pair );

// easy new and insert
void EasyNewVec3d( hobj_t *obj, char *key, vec3d_t v );
void EasyNewVec2d( hobj_t *obj, char *key, vec2d_t v );
void EasyNewFloat( hobj_t *obj, char *key, fp_t f );
void EasyNewInt( hobj_t *obj, char *key, int i );
void EasyNewString( hobj_t *obj, char *key, char *text );

void EasyNewClsref( hobj_t *obj, char *key, hobj_t *referenced_obj );

/*
  ==================================================
  hmanager stuff

  ==================================================
*/



#define HMANAGER_HASH_SIZE	( 256 )

typedef struct hmanager_listnode_s
{
	struct 	hmanager_listnode_s	*next;
	hobj_t		*obj;
} hmanager_listnode_t;

typedef struct hmanager_s
{
	hobj_t		*root;

	hmanager_listnode_t	*hash[HMANAGER_HASH_SIZE];
} hmanager_t;

typedef struct
{
	int		hash;
	char		type[HOBJ_TYPE_SIZE];
	hmanager_t	*hm;
	struct hmanager_listnode_s	*current;
} hmanager_type_iterator_t;

hmanager_t * NewHManager( void );
hmanager_t * NewHManagerLoadClass( char *class_name  );

void FreeHManager( hmanager_t *hm );
void DumpHManager( hmanager_t *hm, bool_t verbose );
hobj_t* HManagerGetRootClass( hmanager_t *hm );
void HManagerSetRootClass( hmanager_t *hm, hobj_t *obj );
int HManagerCalcHashkey( char *name );
void HManagerRebuildHash( hmanager_t *hm );
void HManagerHashClass( hmanager_t *hm, hobj_t *obj );
void HManagerUnhashClass( hmanager_t *hm, hobj_t *obj );

void HManagerIndexClassesOfType( hmanager_t *hm, int *start, char *type );

hobj_t * HManagerSearchClassName( hmanager_t *hm, char *name );
hobj_t * HManagerSearchClassName_linear( hmanager_t *hm, char *name );

void HManagerInsertClass( hmanager_t *hm, hobj_t *parent, hobj_t *obj );
void HManagerRemoveClass( hmanager_t *hm, hobj_t *obj );
void HManagerDeepDestroyClass( hmanager_t *hm, hobj_t *obj );
void HManagerRemoveAndDestroyAllClassesOfType( hmanager_t *hm, hobj_t *obj, char *type );

bool_t HManagerCheckClassConsistancy( hmanager_t *hm );

unsigned int HManagerGetFreeID( void );
void HManagerSaveID( void );

void HManagerInitTypeSearchIterator( hmanager_type_iterator_t *iter, hmanager_t *hm, char *type );
hobj_t * HManagerGetNextClass( hmanager_type_iterator_t *iter );


hobj_t * EasyLookupClsref( hobj_t *obj, char *key, hmanager_t *hm );
hobj_t * EasyNewClass( char *type );

#if 1
/*
  ==================================================
  ClassMap - the better HManager
  
  ==================================================
*/

void InitClassMap( u_map_t *map );
void ClassMapInsertClassFlat( u_map_t *map, hobj_t *obj );
void ClassMapInsertClassDeep( u_map_t *map, hobj_t *root );

#endif

#ifdef __cplusplus
}
#endif


#endif
