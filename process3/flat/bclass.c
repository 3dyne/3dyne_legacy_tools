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



// bclass.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>    
                                                                            
#include "cmdpars.h"
#include "wire.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_unique.h"
#include "lib_hobj.h"

#define BCLASS_CMD_BEGIN_CLASS		( 128 )		// 80
#define BCLASS_CMD_END_CLASS		( 129 )		// 81
#define BCLASS_CMD_SIZE_PREFIX_L1	( 32 )
#define BCLASS_CMD_SIZE_PREFIX_L2	( 64 )
#define BCLASS_CMD_SIZE_PREFIX_L4	( 64+32 )
#define BCLASS_CMD_PTYPE_ASC		( 130 )// 82
#define BCLASS_CMD_PTYPE_BIN		( 131 )// 83
#define BCLASS_CMD_PKEY_ASC		( 132 )// 84
#define BCLASS_CMD_PKEY_BIN		( 133 )// 85
#define BCLASS_CMD_PVALUE_ASC		( 134 )// 86
#define BCLASS_CMD_PVALUE_BIN		( 135 )// 87
#define BCLASS_CMD_CTYPE_ASC		( 136 )// 88
#define BCLASS_CMD_CTYPE_BIN		( 137 )// 89
#define BCLASS_CMD_CNAME_ASC		( 138 )// 8a
#define BCLASS_CMD_CNAME_BIN		( 139 )// 8b
#define BCLASS_LAST_CMD			( 159 )		// 128+31

#define BCLASS_PTYPE_INT		( 1 )
#define BCLASS_PTYPE_FLOAT		( 2 )
#define BCLASS_PTYPE_VEC2D		( 3 )
#define BCLASS_PTYPE_VEC3D		( 4 )
#define BCLASS_PTYPE_STRING		( 5 )
#define BCLASS_PTYPE_BSTRING		( 6 )
#define BCLASS_PTYPE_CLSREF		( 7 )
#define BCLASS_PTYPE_LCLSREF		( 8 )
#define BCLASS_PTYPE_UNKOWN		( 9 )

unsigned int TryToResolve( char *ident, hmanager_t *resolvehm )
{
	int		i;
	hpair_t		*pair;
	unsigned int		bin_ident;
	unsigned int		max;
	hpair_search_iterator_t		iter;

	if ( !resolvehm )
		return 0xffffffff;

	// if first character is digit
	// the ident is a array index

	if ( isdigit( ident[0] ) )
	{
		return atoi( ident );
	}

	// if first character is alpha
	// resolve ident 

	pair = FindHPair( HManagerGetRootClass( resolvehm ), ident );
	if ( pair )
	{
		HPairCastToInt_safe( &bin_ident, pair );
		return bin_ident;
	}

	// not found. create bin_ident.
	// search highest bin_ident, and use 
	// next as new bin_ident for the asc_ident
	max = 0x7fff0000U;
	InitHPairSearchIterator( &iter, HManagerGetRootClass( resolvehm ), "*" );
	
	for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
	{
		HPairCastToInt_safe( &bin_ident, pair );
		if ( bin_ident > max )
			max = bin_ident;
	}

	bin_ident = max+1;
	if ( bin_ident == 0x7fffffff )
		Error( "bin_ident reached 0x7fffffff.\n" );

	printf( "bin_ident for '%s' is %u\n", ident, bin_ident );
	{
		char	tt[256];
		sprintf( tt, "%u", bin_ident );
		pair = NewHPair2( "int", ident, tt );
		InsertHPair( HManagerGetRootClass( resolvehm ), pair );
	}
	
	return bin_ident;
}

unique_t ConvertToUniqueID( char *ident )
{
	unique_t	id;
	char		tt[256];

	if ( ident[0] != '#' )
		return 0;
//		Error( "missing leading '#' in ident\n" );

	id = atoi( &ident[1] );
	sprintf( tt, "#%u", id );
	if ( strcmp( tt, ident ) )
		return 0;
//		Error( "can't extract unique id from ident\n" );

	return id;
}

void WriteBinaryClassRecursive( hobj_t *obj, hmanager_t *resolvehm, FILE *h, FILE *hindex )
{
	unsigned int		bin_type;
	unsigned int		bin_name;
	unsigned int		bin_key;

	int		index_pos;
	
	index_pos = ftell( h );

	// write 'begin class' cmd
	fprintf( h, "%c", BCLASS_CMD_BEGIN_CLASS );

	// write class type
	bin_type = TryToResolve( obj->type, resolvehm );
	if ( bin_type != 0xffffffff )
	{
		// resolve successed
		fprintf( h, "%c", BCLASS_CMD_CTYPE_BIN );
		// write bin_type
		fwrite( &bin_type, 4, 1, h );
	}
	else
	{
		int		len;
		printf( "can't resolve key!\n" );

		// resolve failed keep string
		fprintf( h, "%c", BCLASS_CMD_CTYPE_ASC );
	
		// write strlen
		len = strlen( obj->type );
		fwrite( &len, 4, 1, h );
		// write string
		fwrite( obj->type, len, 1, h );
	}
	
	// write class name
	bin_name = ConvertToUniqueID( obj->name );
	if ( bin_name != 0 )
	{
		// convert successed
		fprintf( h, "%c", BCLASS_CMD_CNAME_BIN );
		
		// write bin_name
		fwrite( &bin_name, 4, 1, h );

		// build index
		if ( hindex )
		{
			fwrite( &bin_name, 4, 1, hindex );
			fwrite( &index_pos, 4, 1, hindex );
		}
	}
	else
	{
		int		len;

		// convert failed keep string
		fprintf( h, "%c", BCLASS_CMD_CNAME_ASC );

		// write strlen
		len = strlen( obj->name );
		fwrite( &len, 4, 1, h );
		// write string
		fwrite( obj->name, len, 1, h );
	}

	// write classes recursive
	{
		hobj_t		*o;
		for ( o = obj->hobjs; o ; o=o->next )
		{
			WriteBinaryClassRecursive( o, resolvehm, h, hindex );	
		}
	}
       
	// write pairs
	{
		hpair_t		*p;

		for ( p = obj->pairs; p ; p=p->next )
		{
			bin_key = TryToResolve( p->key, resolvehm );
			
			if ( bin_key != 0xffffffff )
			{
				fprintf( h, "%c", BCLASS_CMD_PKEY_BIN );
				fwrite( &bin_key, 4, 1, h );
			}
			else
			{				
				int		len, i;

				printf( "can't resolve key!\n" );

				fprintf( h, "%c", BCLASS_CMD_PKEY_ASC );
				len = strlen( p->key );
				fwrite( &len, 4, 1, h );
				for ( i = 0; i < len; i++ )
				{
					fprintf( h, "%c", p->key[i] );
				}
			}

			if ( !strcmp( p->type, "int" ) )
			{
				int		il, tmp;
				fprintf( h, "%c", BCLASS_CMD_PTYPE_BIN );
				fprintf( h, "%c", BCLASS_PTYPE_INT );
//				tmp = BCLASS_PTYPE_INT;
//				fwrite( &tmp, 4, 1, h );

				fprintf( h, "%c", BCLASS_CMD_PVALUE_BIN );
				tmp = 4;	// size of 'int'
				fwrite( &tmp, 4, 1, h );

				HPairCastToInt_safe( &il, p );
				fwrite( &il, 4, 1, h );				
			}
			else if ( !strcmp( p->type, "float" ) )
			{
				int		tmp;
				float		fl;
				fprintf( h, "%c", BCLASS_CMD_PTYPE_BIN );
				fprintf( h, "%c", BCLASS_PTYPE_FLOAT );
//				tmp = BCLASS_PTYPE_FLOAT;
//				fwrite( &tmp, 4, 1, h );

				fprintf( h, "%c", BCLASS_CMD_PVALUE_BIN );
				tmp = 4;	// size of 'float'
				fwrite( &tmp, 4, 1, h );
				
				HPairCastToFloat_safe( &fl, p );
				fwrite( &fl, 4, 1, h );
			}
			else if ( !strcmp( p->type, "vec2d" ) )
			{
				int		tmp;
				float		fv[2];
				fprintf( h, "%c", BCLASS_CMD_PTYPE_BIN );
				fprintf( h, "%c", BCLASS_PTYPE_VEC2D );
//				tmp = BCLASS_PTYPE_VEC2D;
//				fwrite( &tmp, 4, 1, h );

				fprintf( h, "%c", BCLASS_CMD_PVALUE_BIN );
				tmp = 8;	// size of 'vec2d_t'
				fwrite( &tmp, 4, 1, h );
				
				HPairCastToVec2d_safe( fv, p );				
				fwrite( fv, 8, 1, h );
			}
			else if ( !strcmp( p->type, "vec3d" ) )
			{
				int		tmp;
				float		fv[3];
				fprintf( h, "%c", BCLASS_CMD_PTYPE_BIN );
				fprintf( h, "%c", BCLASS_PTYPE_VEC3D );
//				tmp = BCLASS_PTYPE_VEC3D;
//				fwrite( &tmp, 4, 1, h );

				fprintf( h, "%c", BCLASS_CMD_PVALUE_BIN );
				tmp = 12;
				fwrite( &tmp, 4, 1, h );

				HPairCastToVec3d_safe( fv, p );
				fwrite( fv, 12, 1, h );
			}
			else if ( !strcmp( p->type, "string" ) )
			{
				int		tmp;
				int		len;
				int		i;
				fprintf( h, "%c", BCLASS_CMD_PTYPE_BIN );
				fprintf( h, "%c", BCLASS_PTYPE_STRING );
//				tmp = BCLASS_PTYPE_STRING;
//				fwrite( &tmp, 4, 1, h );
				
				fprintf( h, "%c", BCLASS_CMD_PVALUE_BIN );				
				len = strlen( p->value );
				fwrite( &len, 4, 1, h );

				for ( i = 0; i < len; i++ )
				{
					fprintf( h, "%c", p->value[i] );
				}
			}
			else if ( !strcmp( p->type, "bstring" ) )
			{
				int		tmp;
				int		len;
				int		i;
				char		buf[0x10000];
				fprintf( h, "%c", BCLASS_CMD_PTYPE_BIN );
				fprintf( h, "%c", BCLASS_PTYPE_BSTRING );
//				tmp = BCLASS_PTYPE_BSTRING;
//				fwrite( &tmp, 4, 1, h );
				
				fprintf( h, "%c", BCLASS_CMD_PVALUE_BIN );				

				len = 0x10000;
				HPairCastToBstring_safe( buf, &len, p );
				fwrite( &len, 4, 1, h );
				
				for ( i = 0; i < len; i++ )
				{
					fprintf( h, "%c", buf[i] );
				}				
			}
			else if ( !strcmp( p->type, "ref" ) )
			{
				int		tmp;
				fprintf( h, "%c", BCLASS_CMD_PTYPE_BIN );				
				fprintf( h, "%c", BCLASS_PTYPE_CLSREF );
//				tmp = BCLASS_PTYPE_CLSREF;
//				fwrite( &tmp, 4, 1, h );

				fprintf( h, "%c", BCLASS_CMD_PVALUE_BIN );
				tmp = 4;	// size of 'clsref'
				fwrite( &tmp, 4, 1, h );
				
				tmp = ConvertToUniqueID( p->value );
				fwrite( &tmp, 4, 1, h );

			}
			else if ( !strcmp( p->type, "lclsref" ) )
			{
				int		tmp;
				fprintf( h, "%c", BCLASS_CMD_PTYPE_BIN );				
				fprintf( h, "%c", BCLASS_PTYPE_LCLSREF );
//				tmp = BCLASS_PTYPE_LCLSREF;
//				fwrite( &tmp, 4, 1, h );

				fprintf( h, "%c", BCLASS_CMD_PVALUE_BIN );
				tmp = 4;	// size of 'clsref'
				fwrite( &tmp, 4, 1, h );
				
				tmp = ConvertToUniqueID( p->value );
				fwrite( &tmp, 4, 1, h );
			}
			else
			{
				Error( "unkown type\n" );
			}
		}
	}

	// finish class
	fprintf( h, "%c", BCLASS_CMD_END_CLASS );
}

int main( int argc, char *argv[] )
{
	char	*in_class_name;
	char	*in_resolve_name;
	char	*out_bclass_name;
	char	*out_bindex_name;

	hmanager_t	*classhm;
	hmanager_t	*resolvehm;

	FILE	*h;
	FILE	*hindex;

	printf( "===== bclass - converts a ascii class to a binary class =====\n" );

	SetCmdArgs( argc, argv );

	in_class_name = GetCmdOpt2( "-i" );
	in_resolve_name = GetCmdOpt2( "-r" );
	out_bclass_name = GetCmdOpt2( "-o" );
	out_bindex_name = GetCmdOpt2( "-index" );
	
	if ( !in_class_name )
		Error( "no input class\n" );
	if ( !in_resolve_name )
		Error( "no resolve class\n" );
	if ( !out_bclass_name )
		Error( "no output binary class\n" );
	if ( out_bindex_name )
	{
		printf( "run with index generation\n" );
		hindex = fopen( out_bindex_name, "w" );
		if ( !hindex )
			Error( "can't open index output file\n" );
	}
	else
	{
		hindex = NULL;
	}

	printf( "loading input class ...\n" );
	classhm = NewHManagerLoadClass( in_class_name );
	if ( !classhm )
		Error( "failed\n" );

	printf( "loading resolve class ...\n" );
	resolvehm = NewHManagerLoadClass( in_resolve_name );
	if ( !resolvehm )
	{
		hobj_t		*root;
		resolvehm = NewHManager();
		root = NewClass( "resolve", "resolve1" );
		HManagerSetRootClass( resolvehm, root );
	}

	h = fopen( out_bclass_name, "w" );
	if ( !h )
		Error( "can't open output file\n" );
	WriteBinaryClassRecursive( HManagerGetRootClass( classhm ), resolvehm, h, hindex );
	fclose( h );
	if ( hindex )
		fclose( hindex );

	h = fopen( in_resolve_name, "w" );
	if ( !h )
		Error( "can't write resolve class\n" );
	WriteClass( HManagerGetRootClass( resolvehm ), h );
	fclose( h );
	
} 
