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



#include <stdio.h>
#include <stdlib.h>
#include "cmdpars.h"
#include "shock.h"


typedef struct {
	char		id[4];
	unsigned int	size;
} lumphead_t;

int main( int argc, char *argv[] )
{
	char	*filename;
	char	*lumpname;

	FILE	*fileh;
	FILE	*lumph;

	char	*buf;

	int	fsize;

	lumphead_t	lhead;

	filename = GetCmdOpt( "-f", argc, argv );
	if( !filename )
		__error( "missing opt -f\n" );
	
	lumpname = GetCmdOpt( "-l", argc, argv );
	if( !lumpname )
		__error( "missing opt -l\n" );

	fileh = fopen( filename, "rb" );
	if( !fileh )
		__error( "file %s not found\n", filename );

	fseek( fileh, 0, SEEK_END );
	fsize = ftell( fileh );
	fseek( fileh, 0, SEEK_SET );

	printf( "filesize: %d\n", fsize );
	buf = ( char* )malloc( fsize );
	__chkptr( buf );

	fread( buf, fsize, 1, fileh );
	fclose( fileh );

	lumph = fopen( lumpname, "wb" );
	if( !fileh )
		__error( "cannot write file %s\n", lumpname );

	memcpy( lhead.id, "LUMP", 4 );
	lhead.size = fsize;
	fwrite( &lhead, sizeof( lumphead_t ), 1, lumph );
	fwrite( buf, fsize, 1, lumph );
	
	fclose( lumph );
	free( buf );
	return	0;
}
		
