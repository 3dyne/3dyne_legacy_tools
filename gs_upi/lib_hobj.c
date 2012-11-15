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



// lib_hobj.c

#include <math.h>
#include "lib_hobj.h"

/*
  ==================================================
  hobject stuff

  ==================================================
*/



/*
  ====================
  NewClass
  
  ====================
*/
hobj_t* NewClass( const char *type, const char *name )
{
	hobj_t		*obj;

	obj = ( hobj_t * ) malloc( sizeof( hobj_t ) );
	
	ChkPtr( obj );

	memset( obj, 0, sizeof( hobj_t ) );

	strncpy( obj->type, type, HOBJ_TYPE_SIZE-1 );
	strncpy( obj->name, name, HOBJ_NAME_SIZE-1 );

	obj->next = NULL;
	obj->hobjs = NULL;
	obj->pairs = NULL;
	obj->extra = NULL;
	return obj;
}

/*
  ==============================
  ClassSetName

  ==============================
*/
void ClassSetName( hobj_t *self, char *name )
{
	strncpy( self->name, name, HOBJ_NAME_SIZE-1 );
}

/*
  ==============================
  ClassSetType

  ==============================
*/
void ClassSetType( hobj_t *self, char *type )
{
	strncpy( self->type, type, HOBJ_TYPE_SIZE-1 );
}

/*
  ==============================
  ClassGetName
  
  ==============================
*/
char * ClassGetName( hobj_t *self )
{
	return self->name;
}

/*
  ==============================
  ClassGetType

  ==============================
*/
char * ClassGetType( hobj_t *self )
{
	return self->type;
}

/*
  ==============================
  ClassTypeCheck

  ==============================
*/
bool_t ClassTypeCheck( hobj_t *self, char *type )
{
	if ( !strcmp( self->type, type ) )
		return true;

	return false;
}

/*
  ==============================
  ClassNameCheck

  ==============================
*/
bool_t ClassNameCheck( hobj_t *self, char *name )
{
	if ( !strcmp( self->name, name ) )
		return true;

	return false;
}

/*
  ==============================
  ClassGetPrimaryKey

  ==============================
*/
void * ClassGetPrimaryKey( const void *self )
{
	return (void *) ((hobj_t*)(self))->name;
}

/*
  ==============================
  ClassComparePrimaryKeys

  ==============================
*/
int ClassComparePrimaryKeys( const void *key1, const void *key2 )
{
	return strcmp( (char*)(key1), (char*)(key2) );
}

/*
  ====================
  FreeClass

  ====================
*/
void FreeClass( hobj_t *obj )
{
	free( obj );
}



/*
  ====================
  NewHPair

  ====================
*/
hpair_t* NewHPair( void )
{
	hpair_t		*pair;

	pair = ( hpair_t * ) malloc( sizeof( hpair_t ) );
	ChkPtr( pair );
	memset( pair, 0, sizeof( hpair_t ) );

	pair->value = malloc( HPAIR_VALUE_SIZE );
	memset( pair->value, 0, HPAIR_VALUE_SIZE );

	pair->next = NULL;

	return pair;
}

hpair_t* NewHPair2( const char *type, const char *key, const char *value )
{
	hpair_t		*pair;

	pair = ( hpair_t * ) malloc( sizeof( hpair_t ) );
	ChkPtr( pair );
	memset( pair, 0, sizeof( hpair_t ) );

	strcpy( pair->type, type );
	strcpy( pair->key, key );

	pair->value = malloc( strlen( value ) + 1 );
	ChkPtr( pair->value );
	strcpy( pair->value, value );

	pair->next = NULL;

	return pair;	
}

/*
  ====================
  FreeHPair

  ====================
*/
void FreeHPair( hpair_t *pair )
{
	if ( pair->value )
		free( pair->value );

	free( pair );
}

/*
  ====================
  SetClassExtra

  ====================
*/
void SetClassExtra( hobj_t *self, void *ptr )
{
	self->extra = ptr;
}

/*
  ====================
  GetClassExtra

  ====================
*/
void * GetClassExtra( hobj_t *self )
{
	return self->extra;
}

/*
  ====================
  InsertHPair

  ====================
*/
void InsertHPair( hobj_t *self, hpair_t *pair )
{
	pair->next = self->pairs;
	self->pairs = pair;
}



/*
  ====================
  RemoveHPair

  ====================
*/
bool_t RemoveHPair( hobj_t *self, hpair_t *pair )
{
	hpair_t		*p, *next, *head;
	bool_t		success;

	head = NULL;
	success = false;
	for( p = self->pairs; p ; p=next )
	{
		next = p->next;
		if ( p == pair )
		{
			success = true;
			continue;
		}
		p->next = head;
		head = p;
	}
	self->pairs = head;
	return success;
}

/*
  ====================
  RemoveAndDestroyAllHPairsOfKey

  ====================
*/
void RemoveAndDestroyAllHPairsOfKey( hobj_t *self, char *key )
{
	hpair_t	*pair;
	for(;;)
	{
		pair = FindHPair( self, key );
		if ( !pair )
			break;
		RemoveHPair( self, pair );
	}
}

/*
  ==============================
  DeepCopyClass

  ==============================
*/
hobj_t * DeepCopyClassRecursive( hobj_t *obj )
{
	hobj_t	*cpy;
	hobj_t	*o;
	hpair_t	*p;

	cpy = NewClass( obj->type, obj->name );
	
	for ( o = obj->hobjs; o ; o=o->next )
	{
		hobj_t	*tmp;

		tmp = DeepCopyClassRecursive( o );
		InsertClass( cpy, tmp );
	}

	for ( p = obj->pairs; p ; p=p->next )
	{
		hpair_t	*tmp;

		tmp = NewHPair2( p->type, p->key, p->value );
		InsertHPair( cpy, tmp );
	}

	return cpy;
}

hobj_t * DeepCopyClass( hobj_t *self )
{
	hobj_t	*copy;

	copy = DeepCopyClassRecursive( self );
	return copy;
}

/*
  ==============================
  CopyHPair

  ==============================
*/
hpair_t* CopyHPair( hpair_t *pair )
{
	hpair_t		*cpy;

	cpy = NewHPair2( pair->type, pair->key, pair->value );

	return cpy;
}


/*
  ====================
  InsertClass

  ====================
*/
void InsertClass( hobj_t *self, hobj_t *obj )
{
	obj->next = self->hobjs;
	self->hobjs = obj;
	
	obj->parent = self;
}



/*
  ====================
  RemoveClass

  ====================
*/
bool_t RemoveClass( hobj_t *self, hobj_t *obj )
{
	hobj_t		*o, *next, *head;
	bool_t		success;

	head = NULL;
	success = false;
	for( o = self->hobjs; o ; o=next )
	{
		next = o->next;
		if ( o == obj )
		{
			success = true;
			obj->parent = NULL;
			continue;
		}
		o->next = head;
		head = o;
	}
	self->hobjs = head;
	return success;
}

/*
  ====================
  RemoveClass2

  new function, cause objects now now 
  their parent
  ====================
*/
bool_t RemoveClass2( hobj_t *obj )
{
	hobj_t		*parent;
	hobj_t		*o, *next, *head;	
	bool_t		success;
	
	if ( obj->parent == NULL )
		// can't remove root class from anywhere
		return true;

	head = NULL;
	success = false;
	parent = obj->parent;
	for( o = parent->hobjs; o ; o=next )
	{
		next = o->next;
		if ( o == obj )
		{
			success = true;
			obj->parent = NULL;
			continue;
		}
		o->next = head;
		head = o;
	}
	parent->hobjs = head;
	return success;
}


/*
  ====================
  FindHPair

  ====================
*/
hpair_t* FindHPair( hobj_t *self, const char *key )
{
	hpair_t		*p;

	for ( p = self->pairs; p ; p=p->next )
	{
		if ( !strcmp( key, p->key ) )
			return p;
	}
	return NULL;
}


/*
  ====================
  FindClass

  ====================
*/
hobj_t* FindClass( hobj_t *self, char *name )
{
	hobj_t		*o;

	for ( o = self->hobjs; o ; o=o->next )
	{
		if ( !strcmp( name, o->name ) )
			return o;
	}
	return NULL;
}

/*
  ====================
  FindClassType

  ====================
*/
hobj_t* FindClassType( hobj_t *self, char *type )
{
	hobj_t		*o;

	for ( o = self->hobjs; o ; o=o->next )
	{
		if ( !strcmp( type, o->type ) )
			return o;
	}
	return NULL;
}

/*
  ====================
  InitClassSearchIterator

  ===================
*/
void InitClassSearchIterator( hobj_search_iterator_t *iter, hobj_t *obj, const char *type )
{
	iter->hobj = obj;
	iter->current = obj->hobjs;
	strcpy( iter->search_type, type );
}

/*
  ====================
  InitHPairSearchIterator

  ===================
*/
void InitHPairSearchIterator( hpair_search_iterator_t *iter, hobj_t *obj, const char *key )
{
	iter->hobj = obj;
	iter->current = obj->pairs;
	strcpy( iter->search_key, key );
}


/*
  ====================
  SearchGetNextClass

  ====================
*/
hobj_t* SearchGetNextClass( hobj_search_iterator_t *iter )
{
	hobj_t	*cur;

	cur = NULL;

	if ( !iter->current )
		return NULL;

	// search hobj with search_type
	for ( ; iter->current; iter->current=iter->current->next )
	{
		if ( !strcmp( iter->search_type, iter->current->type ) || iter->search_type[0] == '*' )
		{
			cur = iter->current;
			break;
		}
	}
	if ( !iter->current )
		return NULL;
	
	iter->current = iter->current->next;

	return cur;
}

/*
  ====================
  SearchGetNextHPair

  ====================
*/
hpair_t* SearchGetNextHPair( hpair_search_iterator_t *iter )
{
	hpair_t	*cur;

	cur = NULL;

	if ( !iter->current )
		return NULL;

	// search hpair with search_type
	for ( ; iter->current; iter->current=iter->current->next )
	{
		if ( !strcmp( iter->search_key, iter->current->key ) || iter->search_key[0] == '*' )
		{
			cur = iter->current;
			break;
		}
	}
	if ( !iter->current )
		return NULL;
	
	iter->current = iter->current->next;

	return cur;
}

/*
  ====================
  DumpClass
  
  ====================
*/
void DumpClass( hobj_t *self )
{
	hobj_t		*o;
	hpair_t		*p;
	int		num;

	printf( "DumpClass:\n" );
	printf( " type %s, name %s\n", self->type, self->name );
	for ( num = 0, o = self->hobjs; o ; o=o->next, num++ )
	{ }
	printf( " %d hobjs", num );
	for ( num = 0, p = self->pairs; p ; p=p->next, num++ )
	{ }
	printf( " %d pairs\n", num );
}

/*
  ====================
  DumpHPair

  ====================
*/
void DumpHPair( hpair_t *self )
{
	printf( "DumpHPair:\n" );
}



/*
  ====================
  DeepDumpClass

  ====================
*/
static int	cur_objnum;
static int	cur_pairnum;
static int	cur_maxdeep;
static int	cur_deep;
static int	cur_bytes;

void DeepDumpClassRecursive( hobj_t *self )
{
	hpair_t		*p;
	hobj_t		*o;

	cur_bytes += sizeof( hobj_t );

	if ( cur_deep > cur_maxdeep )
		cur_maxdeep = cur_deep;

	for ( p = self->pairs; p ; p=p->next, cur_pairnum++ )
	{
		cur_bytes += sizeof( hpair_t );
		if ( p->value )
			cur_bytes += strlen( p->value );
	}
	for ( o = self->hobjs; o ; o=o->next, cur_objnum++ )
	{
		cur_deep++;
		DeepDumpClassRecursive( o );
		cur_deep--;
	}
}

void DeepDumpClass( hobj_t *self )
{
	printf( "DeepDumpClass:\n" );

	cur_objnum = 0;
	cur_pairnum = 0;
	cur_maxdeep = 0;
	cur_deep = 0;
	cur_bytes = 0;
	DeepDumpClassRecursive( self );

	printf( " total classes %d, total pairs %d, max deep %d\n", cur_objnum, cur_pairnum, cur_maxdeep );
	printf( " %d bytes\n", cur_bytes );
}


/*
  ========================================
  hobj file io

  ========================================
*/
static int		cur_deep;
static FILE		*cur_h;

void WriteIndent( void )
{
	int		i;
	for ( i = 0; i < cur_deep; i++ )
		fprintf( cur_h, "\t" );
}

void WriteClassRecursive( hobj_t *self, int ignore_deep )
{
	hobj_t		*o;
	hpair_t		*p;
	int		len;
	char		*ptr;

	if ( cur_deep != ignore_deep )
	{
		WriteIndent();
		fprintf( cur_h, "obj \"%s\" \"%s\"\n", self->type, self->name );
		WriteIndent();
		fprintf( cur_h, "{\n" );
	}
		
	cur_deep++;


	for ( o = self->hobjs; o ; o=o->next )
	{
		WriteClassRecursive( o, ignore_deep );
	}
	
	for ( p = self->pairs; p ; p=p->next )
	{
		WriteIndent();
		len = strlen( p->value );
		if ( len <= HPAIR_VALUE_FRAG )
		{				
			fprintf( cur_h, "\"%s\" \"%s\" \"%s\"\n", p->type, p->key, p->value );
		}
		else
		{
			cur_deep++;
			fprintf( cur_h, "\"%s\" \"%s\"\n", p->type, p->key );
			WriteIndent();
			fprintf( cur_h, "\"%.*s\"\n", HPAIR_VALUE_FRAG, p->value );
			ptr = p->value+HPAIR_VALUE_FRAG;
			len-=HPAIR_VALUE_FRAG;
			for ( ; len > HPAIR_VALUE_FRAG; len-=HPAIR_VALUE_FRAG, ptr+=HPAIR_VALUE_FRAG )
			{
				WriteIndent();			
				fprintf( cur_h, "\"~%.*s\"\n", HPAIR_VALUE_FRAG, ptr );	
			}
			if ( len )
			{
				WriteIndent();
				fprintf( cur_h, "\"~%s\"\n", ptr );	
			}
			cur_deep--;
		}
	}

	cur_deep--;

	if ( cur_deep != ignore_deep )
	{
		WriteIndent();
		fprintf( cur_h, "}\n" );
	}
}

void WriteClass( hobj_t *self, FILE *h )
{
	cur_deep = 0;
	cur_h = h;
	WriteClassRecursive( self, -1 );
}

void WriteClassWithoutRoot( hobj_t *self, FILE *h )
{
	cur_deep = 0;
	cur_h = h;
	WriteClassRecursive( self, 0 );
	fprintf( h, "end" );
}


/*
  ==============================
  ReadClassFile

  ==============================
*/
hobj_t * ReadClassFile( char *name )
{
	tokenstream_t		*ts;
	hobj_t			*root;

	printf( "ReadClassFile: %s\n", name );

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "can't open class file '%s' for reading\n", name );

	root = ReadClass( ts );
	EndTokenStream( ts );       

	return root;
}

/*
  ==============================
  WriteClassFile

  ==============================
*/
void WriteClassFile( hobj_t *root, char *name )
{
	FILE		*h;

	h = fopen( name, "w" );
	if ( !h )
		Error( "can't open class file '%s' for writing\n", name );

	WriteClass( root, h );
	fclose( h );
}

/*
  ========================================
  ReadClass

  ========================================
*/

#ifndef __WIN32__

//
// this version uses realloc
//

hobj_t* ReadClassRecursive( tokenstream_t *ts )
{
	char		type[HOBJ_TYPE_SIZE];
	char		name[HOBJ_NAME_SIZE];

	hobj_t		*self;
	hobj_t		*hobj;
	hpair_t		*pair;

	int		len;
	char		*ptr;

	// begin new class
	
	GetToken( ts );
	if ( strcmp( ts->token, "obj" ) )
		Error( "ReadClass: expected 'class' token.\n" );
//	printf( "%s\n", ts->token ); getchar();

	// get type
	GetToken( ts );
//	printf( "%s\n", ts->token ); getchar();
	strcpy( type, ts->token );
	
	// get name
	GetToken( ts );
//	printf( "%s\n", ts->token ); getchar();
	strcpy( name, ts->token );

	self = NewClass( type, name );
	
	// expect '{'
	GetToken( ts );
//	printf( "%s\n", ts->token ); getchar();
	if ( ts->token[0] != '{' )
		Error( "ReadClassRecursive: expected '{' of class.\n" );

	for(;;)
	{
		GetToken( ts );

//		printf( "%s\n", ts->token ); getchar();

		if ( ts->token[0] == '}' )
			break; // finish class

		if ( !strcmp( ts->token, "obj" ) )
		{			
			KeepToken( ts );
			hobj = ReadClassRecursive( ts );
			InsertClass( self, hobj );
			continue;
		}

		// it's a pair
		pair = NewHPair();

		// type
		strcpy( pair->type, ts->token );
		// get key
		GetToken( ts );
		strcpy( pair->key, ts->token );
		// get value
		GetToken( ts );
//		strcpy( pair->value, ts->token );
		pair->value = realloc( pair->value, strlen( ts->token )+1 );
		ChkPtr( pair->value );
		strcpy( pair->value, ts->token );

		// is it a mulit token value ?
		for (;;)
		{
			GetToken( ts );
			if ( ts->token[0] == '~' )
			{
				// yes
				len = strlen( pair->value );
				ptr = realloc( pair->value, len + strlen( ts->token ) + 2 );
				ChkPtr( ptr );
				strcpy( ptr+len, &ts->token[1] );
				pair->value = ptr;
			}
			else
			{
				KeepToken( ts );
				break;
			}
		}

		InsertHPair( self, pair );
		
	}

	return self;
}

#else

//
// this version uses a tmp buffer
// 

hobj_t* ReadClassRecursive( tokenstream_t *ts )
{
	char		type[HOBJ_TYPE_SIZE];
	char		name[HOBJ_NAME_SIZE];

	hobj_t		*self;
	hobj_t		*hobj;
	hpair_t		*pair;

	int		len;
//	char		*ptr;

	char		tmpbuf[0x10000];

	// begin new class
	
	GetToken( ts );
	if ( strcmp( ts->token, "obj" ) )
		Error( "ReadClass: expected 'class' token.\n" );
//	printf( "%s\n", ts->token ); getchar();

	// get type
	GetToken( ts );
//	printf( "%s\n", ts->token ); getchar();
	strcpy( type, ts->token );
	
	// get name
	GetToken( ts );
//	printf( "%s\n", ts->token ); getchar();
	strcpy( name, ts->token );

	self = NewClass( type, name );
	
	// expect '{'
	GetToken( ts );
//	printf( "%s\n", ts->token ); getchar();
	if ( ts->token[0] != '{' )
		Error( "ReadClassRecursive: expected '{' of class.\n" );

	for(;;)
	{
		GetToken( ts );

//		printf( "%s\n", ts->token ); getchar();

		if ( ts->token[0] == '}' )
			break; // finish class

		if ( !strcmp( ts->token, "obj" ) )
		{			
			KeepToken( ts );
			hobj = ReadClassRecursive( ts );
			InsertClass( self, hobj );
			continue;
		}

		// it's a pair
		pair = NewHPair();

		// type
		strcpy( pair->type, ts->token );
		// get key
		GetToken( ts );
		strcpy( pair->key, ts->token );
		// get value
		GetToken( ts );
//		strcpy( pair->value, ts->token );
		ChkPtr( pair->value );
		free( pair->value );

//		pair->value = realloc( pair->value, strlen( ts->token )+1 );
//		ChkPtr( pair->value );
//		ptr = tmpbuf;
		strcpy( tmpbuf, ts->token );

		// is it a mulit token value ?
		for (;;)
		{
			GetToken( ts );
			if ( ts->token[0] == '~' )
			{
				// yes
				len = strlen( tmpbuf );
//				ptr = realloc( pair->value, len + strlen( ts->token ) + 2 );
//				ChkPtr( ptr );
				strcpy( tmpbuf+len, &ts->token[1] );
//				pair->value = ptr;
			}
			else
			{
				KeepToken( ts );
				break;
			}
		}

		pair->value = malloc( strlen( tmpbuf ) + 2 );
		ChkPtr( pair->value );
		strcpy( pair->value, tmpbuf );

		InsertHPair( self, pair );
		
	}

	return self;
}
#endif

hobj_t* ReadClass( tokenstream_t *ts )
{
	hobj_t	*root;

	GetToken( ts );
	if ( strcmp( ts->token, "obj" ) )
	{
		printf( "ReadClass: expected 'obj' token. got '%s'.\n", ts->token );
		return NULL;
	}

	KeepToken( ts );
	root = ReadClassRecursive( ts );
	
	return root;
}

/*
  ====================
  TraverseClasses

  ====================
*/
static void TraverseClassesRecursive( hobj_t *obj, void (*func)(hobj_t *self) )
{
	hobj_t	*o;

	func( obj );
	
	for ( o = obj->hobjs; o ; o=o->next )
		TraverseClassesRecursive( o, func );
}

void TraverseClasses( hobj_t *root, void (*func)(hobj_t *self) )
{
	TraverseClassesRecursive( root, func );
}

/*
  ==================================================
  hpair cast stuff

  ==================================================
*/
  
void HPairCastToVec3d( vec3d_t v, hpair_t *pair )
{
	char            *ptr;
	ptr = pair->value;
	v[0] = (fp_t) strtod( ptr, &ptr );
	v[1] = (fp_t) strtod( ptr, &ptr );
	v[2] = (fp_t) strtod( ptr, &ptr );
}

void HPairCastToVec2d( vec2d_t v, hpair_t *pair )
{
	char            *ptr;
	ptr = pair->value;
	v[0] = (fp_t) strtod( ptr, &ptr );
	v[1] = (fp_t) strtod( ptr, &ptr );
}

void HPairCastToFloat( fp_t *f, hpair_t *pair )
{
	*f = atof( pair->value );
}

void HPairCastToInt( int *i, hpair_t *pair )
{
	*i = atoi( pair->value );
}

void HPairCastToString( char *t, hpair_t *pair )
{
	strcpy( t, pair->value );
}

// type safe casts

void HPairCastToVec3d_safe( vec3d_t v, hpair_t *pair )
{
	char            *ptr;

	if ( strcasecmp( "vec3d", pair->type ) )
	{
		printf( "Warning: HPairCastToVec3d_safe: safe cast failed for \"%s\" of type \"%s\".\n",
			pair->key, pair->type );
		Vec3dInit( v, 0, 0, 0 );
		return;
	}
	
	ptr = pair->value;
	v[0] = (fp_t) strtod( ptr, &ptr );
	v[1] = (fp_t) strtod( ptr, &ptr );
	v[2] = (fp_t) strtod( ptr, &ptr );
}

void HPairCastToVec2d_safe( vec2d_t v, hpair_t *pair )
{
	char            *ptr;

	if ( strcasecmp( "vec2d", pair->type ) )
	{
		printf( "Warning: HPairCastToVec2d_safe: safe cast failed for \"%s\" of type \"%s\".\n",
			pair->key, pair->type );
		Vec2dInit( v, 0, 0 );
		return;
	}	

	ptr = pair->value;
	v[0] = (fp_t) strtod( ptr, &ptr );
	v[1] = (fp_t) strtod( ptr, &ptr );
}

void HPairCastToFloat_safe( fp_t *f, hpair_t *pair )
{
	if ( strcasecmp( "float", pair->type ) )
	{
		printf( "Warning: HPairCastToFloat_safe: safe cast failed for \"%s\" of type \"%s\".\n",
			pair->key, pair->type );
		*f = 0.0;
		return;
	}
	
	*f = atof( pair->value );
}

void HPairCastToInt_safe( int *i, hpair_t *pair )
{
	if ( strcasecmp( "int", pair->type ) )
	{
		printf( "Warning: HPairCastToInt_safe: safe cast failed for \"%s\" of type \"%s\".\n",
			pair->key, pair->type );
		*i = 0;
		return;
	}
	
	*i = atoi( pair->value );
}

void HPairCastToString_safe( char *t, hpair_t *pair )
{

	if ( strcasecmp( "string", pair->type ) )
	{
		printf( "Warning: HPairCastToString_safe: safe cast failed for \"%s\" of type \"%s\".\n",
			pair->key, pair->type );
		*t = 0;
		return;
	}	

	strcpy( t, pair->value );
}

void HPairCastToClsref_safe( char *t, hpair_t *pair )
{

	if ( strcasecmp( "ref", pair->type ) )
	{
		printf( "Warning: HPairCastToClsref_safe: safe cast failed for \"%s\" of type \"%s\".\n",
			pair->key, pair->type );
		*t = 0;
		return;
	}	

	strcpy( t, pair->value );
}

int HexCharToInt( unsigned char c )
{
	if ( c >= 48 && c <= 57 )
		return c-48;
	if ( c >= 65 && c <= 106 )
		return (c-65)+10;
	return 0;
}

unsigned char IntToHexChar( int i )
{
	if ( i >= 0 && i <= 9 )
		return 48+i;
	if ( i >= 10 && i <= 16 )
		return 65+(i-10);
	return 48;
}

void HPairCastToBstring_safe( void *ptr, int *max_buf_size, hpair_t *pair )
{
	int		i, len;
	unsigned char		*c;

	c = (unsigned char *) ptr;

	if ( strcasecmp( "bstring", pair->type ) )
	{
		printf( "Warning: HPairCastToBstring_safe: safe cast failed for \"%s\" of type \"%s\".\n",
			pair->key, pair->type );
		*c = 0;
		*max_buf_size = 0;
		return;
	}	

	len = strlen( pair->value );

	if ( len&1 )
	{
		printf( "Warning: HPairCastToBstring_safe: value length is odd in \"%s\" of type \"%s\".\n",
			pair->key, pair->type );		
		*c = 0;
		*max_buf_size = 0;
	}

	if ( len / 2 > (*max_buf_size) )
	{
		printf( "Warning: HPairCastToBstring_safe: destination buffer to small for \"%s\" of type \"%s\".\n",
			pair->key, pair->type );		
		*c = 0;
		*max_buf_size = 0;
	}

	*max_buf_size = 0;
	for ( i = 0; i < len; i+=2 )
	{
		*c = (unsigned char)( HexCharToInt(pair->value[i]) * 16 + HexCharToInt(pair->value[i+1]));
		c++;
		(*max_buf_size)++;
	}       
}

void BstringCastToHPair( void *ptr, int size, hpair_t *pair )
{
	int		i, j;
	unsigned char	*in;

	in = (unsigned char *) ptr;
	pair->value = realloc( pair->value, size * 2 + 1 );

	for ( i = 0, j = 0; i < size; i++, j+=2 )
	{
		pair->value[j] = IntToHexChar(in[i] / 16 );
		pair->value[j+1] = IntToHexChar(in[i] & 15 );
	}
	pair->value[j] = 0;	// arghh!!
}

void HPairCastFromVec3d( vec3d_t v, hpair_t *pair )
{
	char		tt[256];
	int		i;
	int		pos;

	pos = 0;
	for ( i = 0; i < 3; i++ )
	{
		if ( _Rint(v[i]) == v[i] )
			pos+=sprintf( &tt[pos], "%d ", (int)_Rint(v[i]) );
		else
			pos+=sprintf( &tt[pos], "%f ", v[i] );
	}
	
	pair->value = realloc( pair->value, pos+1 );
	strcpy( pair->value, tt );
}

/*
  ==================================================
  easy find and cast

  ==================================================
*/

/*
  ==============================
  EasyFindVec3d

  ==============================
*/
hpair_t * EasyFindVec3d( vec3d_t v, hobj_t *obj, const char *key )
{
	hpair_t		*pair;
	
	pair = FindHPair( obj, key );
	if ( !pair )
		Error( "missing '%s' in name '%s' of type '%s'\n", key, obj->name, obj->type );
	
	HPairCastToVec3d_safe( v, pair );
	return pair;
}

/*
  ==============================
  EasyFindVec2d

  ==============================
*/
hpair_t * EasyFindVec2d( vec2d_t v, hobj_t *obj, const char *key )
{
	hpair_t		*pair;
	
	pair = FindHPair( obj, key );
	if ( !pair )
		Error( "missing '%s' in name '%s' of type '%s'\n", key, obj->name, obj->type );
	
	HPairCastToVec2d_safe( v, pair );	
	return pair;
}

/*
  ==============================
  EasyFindFloat

  ==============================
*/
hpair_t * EasyFindFloat( fp_t *f, hobj_t *obj, const char *key )
{
	hpair_t		*pair;
	
	pair = FindHPair( obj, key );
	if ( !pair )
		Error( "missing '%s' in name '%s' of type '%s'\n", key, obj->name, obj->type );
	
	HPairCastToFloat_safe( f, pair );
	return pair;
}

/*
  ==============================
  EasyFindInt

  ==============================
*/
hpair_t * EasyFindInt( int *i, hobj_t *obj, const char *key )
{
	hpair_t		*pair;
	
	pair = FindHPair( obj, key );
	if ( !pair )
		Error( "missing '%s' in name '%s' of type '%s'\n", key, obj->name, obj->type );
	
	HPairCastToInt_safe( i, pair );
	return pair;
}

/*
  ==============================
  EasyFindString
  
  ==============================
*/
hpair_t * EasyFindString( char *t, hobj_t *obj, const char *key )
{
	hpair_t		*pair;
	
	pair = FindHPair( obj, key );
	if ( !pair )
		Error( "missing '%s' in name '%s' of type '%s'\n", key, obj->name, obj->type );
	
	HPairCastToString_safe( t, pair );
	return pair;
}

/*
  ==============================
  EasyLookupClsref

  ==============================
*/
hobj_t * EasyLookupClsref( hobj_t *obj, char *key, hmanager_t *hm )
{
	hpair_t		*pair;
	hobj_t		*other;

	pair = FindHPair( obj, key );
	if ( !pair )
		Error( "missing key '%s' in class '%s' of type '%s'\n", key, obj->name, obj->type );
	
	other = HManagerSearchClassName( hm, pair->value );
	if ( !other )
		Error( "can't find class '%s' ( lookup clsref key '%s' in class '%s' of type '%s' )\n", pair->value, key, obj->name, obj->type );

	return other;
}


/*
  ==================================================
  easy new and insert

  ==================================================
*/

/*
  ==============================
  EasyNewVec3d

  ==============================
*/
void EasyNewVec3d( hobj_t *obj, char *key, vec3d_t v )
{
	hpair_t	*pair;
	char	tt[256];

	sprintf( tt, "%f %f %f", v[0], v[1], v[2] );
	pair = NewHPair2( "vec3d", key, tt );
	InsertHPair( obj, pair );
}

/*
  ==============================
  EasyNewVec2d

  ==============================
*/
void EasyNewVec2d( hobj_t *obj, char *key, vec2d_t v )
{
	hpair_t	*pair;
	char	tt[256];

	sprintf( tt, "%f %f", v[0], v[1] );
	pair = NewHPair2( "vec2d", key, tt );
	InsertHPair( obj, pair );	
}

/*
  ==============================
  EasyNewFloat

  ==============================
*/
void EasyNewFloat( hobj_t *obj, char *key, fp_t f )
{
	hpair_t	*pair;
	char	tt[256];

	sprintf( tt, "%f", f );
	pair = NewHPair2( "float", key, tt );
	InsertHPair( obj, pair );		
}

/*
  ==============================
  EasyNewInt

  ==============================
*/
void EasyNewInt( hobj_t *obj, char *key, int i )
{
	hpair_t	*pair;
	char	tt[256];

	sprintf( tt, "%d", i );
	pair = NewHPair2( "int", key, tt );
	InsertHPair( obj, pair );
}

/*
  ==============================
  EasyNewString

  ==============================
*/
void EasyNewString( hobj_t *obj, char *key, char *text )
{
	hpair_t		*pair;
	
	pair = NewHPair2( "string", key, text );
	InsertHPair( obj, pair );
}


/*
  ==============================
  EasyNewClsref

  ==============================
*/
void EasyNewClsref( hobj_t *obj, char *key, hobj_t *referenced_obj )
{
	hpair_t		*pair;

	pair = NewHPair2( "ref", key, referenced_obj->name );
	InsertHPair( obj, pair );
}

/*
  ==================================================
  hmanager stuff

  ==================================================
*/
static unsigned int	next_unique_id = 0;

hmanager_t * NewHManager( void )
{
	hmanager_t *hm;

	hm = (hmanager_t *) malloc( sizeof( hmanager_t ) );
	memset( hm, 0, sizeof( hmanager_t ) );

	return hm;
}

hmanager_t * NewHManagerLoadClass( char *class_name )
{
	hmanager_t *hm;
	tokenstream_t	*ts;
	FILE *h;
	char	tt[256];

	printf( "NewHManagerLoadClass: %s\n", class_name );

	ts = BeginTokenStream( class_name );
	if ( !ts )
	{
		printf( "NewHManager2: warning, can't opne class '%s'.\n", class_name );
		return NULL;
	}

	hm = (hmanager_t *) malloc( sizeof( hmanager_t ) );
	memset( hm, 0, sizeof( hmanager_t ) );	

	hm->root = ReadClass( ts );

#if 0
	sprintf( tt, "%s.debug", class_name );
	h = fopen( tt, "w" );
	WriteClass( hm->root, h );
	fclose( h );
#endif

	EndTokenStream( ts );

	if ( !hm->root )
	{
		printf( "NewHManager2: warning, null class '%s'.\n", class_name );
		return NULL;
	}

	HManagerRebuildHash( hm );

	return hm;
}

void FreeHManager( hmanager_t *hm )
{
	free( hm );
}

void DumpHManager( hmanager_t *hm, bool_t verbose )
{
	hmanager_listnode_t	*n;
	int		i, num;
	int		min_entries, max_entries;

	printf( "DumpHManager:\n" );

	min_entries = 1<<30;
	max_entries = -min_entries;
	for ( i = 0; i < HMANAGER_HASH_SIZE; i++ )
	{
		for ( n = hm->hash[i], num = 0; n ; n=n->next, num++ )
		{ }
		if ( verbose )
			printf( " hash[%d]: %d entries\n", i, num );
		if ( num < min_entries )
			min_entries = num;
		if ( num > max_entries )
			max_entries = num;
	}
	printf( "hash entries min: %d, max: %d\n", min_entries, max_entries );
}

/*
  ====================
  HManagerGetRootClass

  ====================
*/
hobj_t * HManagerGetRootClass( hmanager_t *hm )
{
	return hm->root;
}

/*
  ====================
  HManagerSetRootClass

  ====================
*/
void HManagerSetRootClass( hmanager_t *hm, hobj_t *obj )
{
	hm->root = obj;
}

/*
  ====================
  HManagerCalcHashkey

  ====================
*/
int HManagerCalcHashkey( char *name )
{
	int	key;

	for ( key = 0; *name ; name++ )
	{
		key <<= 2;
		key += (int)(*name);		
	}

//	key *= key;
//	key >>= 4;
	
	return abs( key % HMANAGER_HASH_SIZE );
}

/*
  ====================
  HManagerRebuildHash

  ====================
*/
static void HManagerRebuildHashRecursive( hmanager_t *hm, hobj_t *obj )
{
	hobj_t		*o;
//	int		id;

	HManagerHashClass( hm, obj );


#if 0
	// init unique id

	if ( obj->name[0] == '#' )
	{
		id = atoi( &obj->name[1] );
		if ( id > next_unique_id )
			next_unique_id = id+1;
	}
#endif	

	for ( o = obj->hobjs; o ; o=o->next )
		HManagerRebuildHashRecursive( hm, o );
}

void HManagerRebuildHash( hmanager_t *hm )
{
	int		i;
	for ( i = 0; i < HMANAGER_HASH_SIZE; i++ )
		hm->hash[i] = NULL;

	HManagerRebuildHashRecursive( hm, hm->root );
} 

/*
  ====================
  HManagerHashClass

  ====================
*/
void HManagerHashClass( hmanager_t *hm, hobj_t *obj )
{
	int	key;
	hmanager_listnode_t	*node;

	key = HManagerCalcHashkey( obj->name );
//	printf( "key: %d\n", key );
	node = (hmanager_listnode_t *) malloc( sizeof( hmanager_listnode_t ) );

	node->obj = obj;
	node->next = hm->hash[key];
	hm->hash[key] = node;
}

/*
  ====================
  HManagerUnhashClass

  ====================
*/
void HManagerUnhashClass( hmanager_t *hm, hobj_t *obj )
{
	int		key;
	hmanager_listnode_t	*n, *next, *head;

	key = HManagerCalcHashkey( obj->name );

	head = NULL;
	for ( n = hm->hash[key]; n ; n=next )
	{
		next = n->next;
		if ( n->obj == obj )
		{
			free( n );
			continue;
		}
		n->next = head;
		head = n;
	}
	hm->hash[key] = head;
}

/*
  ====================
  HManagerSearchClassName

  ====================
*/
hobj_t * HManagerSearchClassName( hmanager_t *hm, char *name )
{
	int		key;
	hmanager_listnode_t	*n;

	key = HManagerCalcHashkey( name );
	
	for ( n = hm->hash[key]; n ; n=n->next )
	{		
		if ( !n->obj )
			Error( "null\n" );

		if ( !strcmp( n->obj->name, name ) )
		{
			return n->obj;
		}
	}
	return NULL;
}

/*
  ====================
  HManagerIndexClassOfType

  ====================
*/
void HManagerIndexClassesOfType( hmanager_t *hm, int *start, char *type )
{
	int		i;
	hmanager_listnode_t	*n;
	hpair_t		*pair;
	char		tt[256];

	for ( i = 0; i < HMANAGER_HASH_SIZE; i++ )
	{
		for ( n = hm->hash[i]; n ; n=n->next )
		{
			if ( !n->obj )
				continue;

			if ( !strcmp( n->obj->type, type ) )
			{
				sprintf( tt, "%d", (*start) );
				pair = NewHPair2( "int", "index", tt );
				InsertHPair( n->obj, pair );
				(*start)++;
			}
		}
	}
}

/*
  ====================
  HManagerSearchClassName_linear

  ====================
*/
hobj_t * HManagerSearchClassNameRecursive( hobj_t *obj, char *name )
{
	hobj_t	*o;
	hobj_t	*found;

	if ( !strcmp( name, obj->name ) )
		return obj;

	for ( o = obj->hobjs; o ; o=o->next )
	{
		found = HManagerSearchClassNameRecursive( o, name );
		if ( found )
			return found;
	}

	return NULL;
}

hobj_t * HManagerSearchClassName_linear( hmanager_t *hm, char * name )
{
	return HManagerSearchClassNameRecursive( hm->root, name );
}

/*
  ====================
  HManagerInsertClass

  ====================
*/
void HManagerInsertClass( hmanager_t *hm, hobj_t *parent, hobj_t *obj )
{
	InsertClass( parent, obj );
	HManagerHashClass( hm, obj );
}

/*
  ====================
  HManagerRemoveClass

  ====================
*/
void HManagerRemoveClass( hmanager_t *hm, hobj_t *obj )
{
	HManagerUnhashClass( hm, obj );
	RemoveClass2( obj );
}

/*
  ====================
  HManagerDeepDestroyClass

  ====================
*/
static void DestroyClassRecursive( hmanager_t *hm, hobj_t *obj )
{
	hobj_t		*o, *next;
	hpair_t		*p, *pnext;

	for ( o = obj->hobjs; o ; o=next )
	{
		next = o->next;
		DestroyClassRecursive( hm, o );		
	}
	for ( p = obj->pairs; p ; p=pnext )
	{
		pnext = p->next;
		FreeHPair( p );
	}

	HManagerUnhashClass( hm, obj );
	FreeClass( obj );
}

void HManagerDeepDestroyClass( hmanager_t *hm, hobj_t *obj )
{
	DestroyClassRecursive( hm, obj );
}

/*
  ====================
  RemoveAllClassTypes

  ====================
*/
void HManagerRemoveAndDestroyAllClassesOfType( hmanager_t *hm, hobj_t *obj, char *type )
{
	hobj_t		*o, *next, *list;
	
	list = NULL;
	for ( o = obj->hobjs; o ; o=next )
	{
		next = o->next;
		if ( !strcmp( type, o->type ) )
		{
			HManagerDeepDestroyClass( hm, o );
			continue;
		}
		o->next = list;
		list = o;
	}
	obj->hobjs = list;
}


/*
  ====================
  HManagerCheckClassConsistancy

  ====================
*/
bool_t HManagerCheckClassConsistancy( hmanager_t *hm )
{
	int		i;
	hmanager_listnode_t	*n1, *n2;
	bool_t		success;

	printf( "Check class consistancy ...\n" );

	//
	// check hash tabel
	//
	success = true;
	for ( i = 0; i < HMANAGER_HASH_SIZE; i++ )
	{
		for ( n1 = hm->hash[i]; n1 ; n1=n1->next )
		{
			for ( n2 = hm->hash[i]; n2 ; n2=n2->next )
			{
				if ( n1 == n2 )
					continue;
				if ( !strcmp( n1->obj->name, n2->obj->name ) )
				{
					printf( " failed: name \"%s\" found in class types \"%s\" and \"%s\"\n", 
						n1->obj->name, n1->obj->type, n2->obj->type );
					success = false;
				}
			}
		}
	}
	return success;
}

/*
  ====================
  HManagerGetFreeID

  ====================
*/
unsigned int HManagerGetFreeID( void )
{
	unsigned int	id;
	tokenstream_t		*ts;

	if ( next_unique_id == 0 )
	{
		ts = BeginTokenStream( ".ID" );
		if ( ts )
		{
			GetToken( ts );
			next_unique_id = atoi( ts->token );
			EndTokenStream( ts );
		}
		printf( "Load next .ID #%u\n", next_unique_id );		
	}

	id = next_unique_id;
	next_unique_id++;
	return id;
}

void HManagerSaveID( void )
{
	FILE		*h;

	if ( next_unique_id == 0 )
	{
		printf( "No new unique id, don't save\n" );
		return;
	}

	h = fopen( ".ID", "w" );
	if ( !h )
	{
		printf( "HManagerSaveID: can't open file '.ID'\n" );
		return;
	}
	
	fprintf( h, "%u", next_unique_id );
	fclose( h );
}


/*
  ====================
  HManagerInitTypeSearchIterator

  ====================
*/
void HManagerInitTypeSearchIterator( hmanager_type_iterator_t *iter, hmanager_t *hm, char *type )
{
	int		i;

	iter->hm = hm;

	for ( i = 0; i < HMANAGER_HASH_SIZE; i++ )
	{
		if ( hm->hash[i] )
			break;
	}
	
	if ( i == HMANAGER_HASH_SIZE )
	{
		iter->current = NULL;
	}
	else
	{
		iter->current = hm->hash[i];
	}

	iter->hash = i;

	strcpy( iter->type, type );
}

/*
  ====================
  HManagerGetNextClass

  ====================
*/
hobj_t * HManagerGetNextClass( hmanager_type_iterator_t *iter )
{
	hmanager_listnode_t		*cur;

	cur = NULL;

	if ( iter->hash == HMANAGER_HASH_SIZE )
		return NULL;

again:
	for ( ; iter->current; iter->current=iter->current->next )
	{
		if ( !iter->current->obj )
			continue;

		if ( !strcmp( iter->current->obj->type, iter->type ) )
		{
			cur = iter->current;
			break;
		}
	}
	
	if ( !iter->current )
	{
		iter->hash++;
		if ( iter->hash == HMANAGER_HASH_SIZE )
			return NULL;
		iter->current = iter->hm->hash[iter->hash];
		goto again;
	}

	iter->current = iter->current->next;

	return cur->obj;
}

/*
  ==============================
  EasyNewClass

  ==============================
*/
hobj_t * EasyNewClass( char *type )
{
	hobj_t		*obj;
	char		str[256];

	sprintf( str, "#%u", HManagerGetFreeID() );
	obj = NewClass( type, str );
	return obj;
}


/*
  ==================================================
  ClassMap - the better HManager

  ==================================================
*/

/*
  ==============================
  InitClassMap

  ==============================
*/
void InitClassMap( u_map_t *map )
{
	U_InitMap( map, map_default, ClassComparePrimaryKeys, ClassGetPrimaryKey );
}

/*
  ==============================
  ClassMapInsertClassDeep

  ==============================
*/

static u_map_t	*insert_map;

static void MapInsertClassFunc( hobj_t *obj )
{
	// only insert unique id style name
	
	if ( obj->name[0] != '#' )
		return;
	
	if ( !U_MapInsert( insert_map, obj ) )
	{
		Error( "already a class with name '%s'\n", obj->name );
	}
}

void ClassMapInsertClassDeep( u_map_t *map, hobj_t *root )
{
	insert_map = map;
	TraverseClasses( root, MapInsertClassFunc );
}
