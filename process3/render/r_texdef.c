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



// r_texdef.c

#include "render.h"

int		r_texdefnum;
texdef_t	r_texdefs[MAX_TEXDEFS];

void CompileTexdefClass( hmanager_t *texdefhm, hmanager_t *texturehm )
{
	hobj_search_iterator_t	iter;
	hobj_t		*texdef;
	hobj_t		*texture;
	hpair_t		*pair;
	char		tt[256];

	r_texdefnum = 0;
	InitClassSearchIterator( &iter, HManagerGetRootClass( texdefhm ), "texdef" );
	for ( ; ( texdef = SearchGetNextClass( &iter ) ) ; )
	{
		if ( r_texdefnum == MAX_TEXDEFS )
			Error( "reached MAX_TEXDEFS\n" );

		// set index
		sprintf( tt, "%d", r_texdefnum );
		pair = NewHPair2( "int", "index", tt );
		InsertHPair( texdef, pair );

		// get texture index
		pair = FindHPair( texdef, "texture" );
		if ( !pair )
			Error( "missing 'texture' in texdef '%s'.\n", texdef->name );
		texture = HManagerSearchClassName( texturehm, pair->value );
		if ( !texture )
			Error( "texdef '%s' can't find texture '%s'.\n", texdef->name, pair->value );
		pair = FindHPair( texture, "index" );
		if ( !pair )
			Error( "missing 'index' in texture '%s'.\n", texture->name );
		HPairCastToInt_safe( &r_texdefs[r_texdefnum].texture, pair );

		// get ProjectionType
		pair = FindHPair( texdef, "flags" );
		if ( !pair )
			Error( "missing 'flags' ( aka projectionType ) in texdef '%s'.\n", texdef->name );
		HPairCastToInt_safe( &r_texdefs[r_texdefnum].projection, pair );

		// get shift
		pair = FindHPair( texdef, "shift" );
		if ( !pair )
			Error( "missing 'shift' in texdef '%s'.\n", texdef->name );
		HPairCastToVec2d_safe( r_texdefs[r_texdefnum].shift, pair );

		// get vecs
		pair = FindHPair( texdef, "vec0" );
		if ( !pair )
			Error( "missing 'vec0' in texdef '%s'.\n", texdef->name );
		HPairCastToVec2d_safe( r_texdefs[r_texdefnum].vecs[0], pair );

		pair = FindHPair( texdef, "vec1" );
		if ( !pair )
			Error( "missing 'vec1' in texdef '%s'.\n", texdef->name );
		HPairCastToVec2d_safe( r_texdefs[r_texdefnum].vecs[1], pair );

		r_texdefnum++;
	}

	printf( " %d texdefs\n", r_texdefnum );
}
