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



// lib_pfiles.c

#include <stdio.h>
#include <stdlib.h>

#include "lib_pfiles.h"


int		out_dvertexnum;
dvertex_t	out_dvertices[ MAX_DVERTICES ];

int		out_dedgenum;
dedge_t		out_dedges[ MAX_DEDGES ];

int			out_dfaceedgenum;
dfaceedge_t		out_dfaceedges[ MAX_DFACEEDGES ];

int			out_dfacenum;
dface_t			out_dfaces[ MAX_DFACES ];

int			out_dtexturenum;
dtexture_t		out_dtextures[ MAX_DTEXTURES ];

int			out_dtexdefnum;
dtexdef_t		out_dtexdefs[ MAX_DTEXDEFS ];

int			out_dplanenum;
dplane_t			out_dplanes[ MAX_DPLANES ];


int			out_dleafnum;
dleaf_t			out_dleafs[ MAX_DLEAFS ];

int			out_dnodenum;
dnode_t			out_dnodes[ MAX_DNODES ];

//

FILE	*h;

// =============================================================================
// write pdata ...
// =============================================================================

void WriteLumpStreamHeader( char *ident, int lump_size, int lump_num )
{
	fwrite( LUMP_VERSION, 1, 4, h );
	fwrite( ident, 1, 4, h );
	fwrite( &lump_size, 1, 4, h );
	fwrite( &lump_num, 1, 4, h );
}

void WriteDVertices( void )
{
	int		i;

	WriteLumpStreamHeader( LUMP_IDENT_DVERTEX, sizeof( dvertex_t ), out_dvertexnum );
	
	for ( i = 0; i < out_dvertexnum; i++ ) 
		fwrite( &out_dvertices[i], 1, sizeof( dvertex_t ), h );

	printf(" vertex size: \t%d\n", out_dvertexnum * sizeof( dvertex_t ) );
}

void WriteDEdges( void )
{
	int		i;

	WriteLumpStreamHeader( LUMP_IDENT_DEDGE, sizeof( dedge_t ), out_dedgenum );

	for ( i = 0; i < out_dedgenum; i++ ) 
		fwrite( &out_dedges[i], 1, sizeof( dedge_t ), h );

	printf(" edge size: \t%d\n", out_dedgenum * sizeof( dedge_t ) );
}


void WriteDFaceEdges( void )
{
	int		i;

	WriteLumpStreamHeader( LUMP_IDENT_DFACEEDGE, sizeof( dfaceedge_t ), out_dfaceedgenum );
	
	for ( i = 0; i < out_dfaceedgenum; i++ ) 
		fwrite( &out_dfaceedges[i], 1, sizeof( dfaceedge_t ), h );

	printf(" faceedge size: %d\n", out_dfaceedgenum * sizeof( dfaceedge_t ) );
}


void WriteDFaces( void )
{
	int		i;

	WriteLumpStreamHeader( LUMP_IDENT_DFACE, sizeof( dface_t ), out_dfacenum );

	for ( i = 0; i < out_dfacenum; i++ ) 
		fwrite( &out_dfaces[i], 1, sizeof( dface_t ), h );

	printf(" face size: \t%d\n", out_dfacenum * sizeof( dface_t ) );
}

void WriteDTextures( void )
{
	int		i;

	WriteLumpStreamHeader( LUMP_IDENT_DTEXTURE, sizeof( dtexture_t ), out_dtexturenum );

	for ( i = 0; i < out_dtexturenum; i++ )
		fwrite( &out_dtextures[i], 1, sizeof( dtexture_t ), h );

	printf(" texture size: \t%d\n", out_dtexturenum * sizeof( dtexture_t ) );
}

void WriteDTexdefs( void )
{
	int		i;

	WriteLumpStreamHeader( LUMP_IDENT_DTEXDEF, sizeof( dtexdef_t ), out_dtexdefnum );

	for ( i = 0; i < out_dtexdefnum; i++ )
		fwrite( &out_dtexdefs[i], 1, sizeof( dtexdef_t ), h );
	
	printf(" texdef size: \t%d\n", out_dtexdefnum * sizeof( dtexdef_t ) );
}

void WriteDPlanes( void )
{
	int		i;

	// hesse - normal - form - plane
	WriteLumpStreamHeader( LUMP_IDENT_DPLANE, sizeof( dplane_t ), out_dplanenum );

	for ( i = 0; i < out_dplanenum; i++ )
		fwrite( &out_dplanes[i], 1, sizeof( dplane_t ), h );		

	printf(" plane size: \t%d\n", out_dplanenum * sizeof( dplane_t ) );
}

void WriteDLeafs( void )
{
	int		i;

	WriteLumpStreamHeader( LUMP_IDENT_DLEAF, sizeof( dleaf_t ), out_dleafnum );

	for ( i = 0; i < out_dleafnum; i++ )
	{
		fwrite( &out_dleafs[i], 1, sizeof( dleaf_t ), h );
	}

	printf(" leaf size: \t%d\n", out_dleafnum * sizeof( dleaf_t ) );
}

void WriteDNodes( void )
{
	int		i;

	WriteLumpStreamHeader( LUMP_IDENT_DNODE, sizeof( dnode_t ), out_dnodenum );

	for ( i = 0; i < out_dnodenum; i++ )
	{
		fwrite( &out_dnodes[i], 1, sizeof( dnode_t ), h );
	}

	printf(" node size: \t%d\n", out_dnodenum * sizeof( dnode_t ) );
}

// =============================================================================
// read pdata ...
// =============================================================================

void CheckLumpStreamHeader( char *ident, int lump_size, int *lump_num )
{
	char		dlump[4];
	char		dident[4];
	int		dsize;
	int		dnum;

	fread( &dlump, 1, 4, h );
	fread( &dident, 1, 4, h );
	fread( &dsize, 1, 4, h );
	fread( &dnum, 1, 4, h );

	if ( memcmp( &dlump, LUMP_VERSION, 4 ) )
	{
		printf( "CheckLumpStreamHeader: expected lump version '%.4s' ( found '%.4s' ).\n", LUMP_VERSION, dlump );
		exit(-1);
	}

	if ( memcmp( &dident, ident, 4 ) )
	{
		printf( "CheckLumpStreamHeader: expected lump '%.4s' ( found '%.4s ).\n", ident, dident );
		exit(-1);
	}

	if ( dsize != lump_size )
	{
		printf( "CheckLumpStreamHeader: expected size %d ( found %d ).\n", lump_size, dsize );
		exit(-1);
	}

	if ( dnum > *lump_num )
	{
		printf( "CheckLumpStreamHeader: max lumps %d ( found %d ).\n", *lump_num, dnum );
		exit(-1);
	}

	*lump_num = dnum;
}

void ReadDVertices( void )
{
	out_dvertexnum = MAX_DVERTICES;
	CheckLumpStreamHeader( LUMP_IDENT_DVERTEX, sizeof( dvertex_t ), &out_dvertexnum );
	
	fread( out_dvertices, sizeof( dvertex_t ), out_dvertexnum, h );
}

void ReadDEdges( void )
{
	out_dedgenum = MAX_DEDGES;
	CheckLumpStreamHeader( LUMP_IDENT_DEDGE, sizeof( dedge_t ), &out_dedgenum );

	fread( out_dedges, sizeof( dedge_t ), out_dedgenum, h );
}

void ReadDFaceEdges( void )
{
	out_dfaceedgenum = MAX_DFACEEDGES;
	CheckLumpStreamHeader( LUMP_IDENT_DFACEEDGE, sizeof( dfaceedge_t ), &out_dfaceedgenum );

	fread( out_dfaceedges, sizeof( dfaceedge_t ), out_dfaceedgenum, h );
}

void ReadDFaces( void )
{
	out_dfacenum = MAX_DFACES;
	CheckLumpStreamHeader( LUMP_IDENT_DFACE, sizeof( dface_t ), &out_dfacenum );

	fread( out_dfaces, sizeof( dface_t ), out_dfacenum, h );
}


void ReadDTextures( void )
{
	out_dtexturenum = MAX_DTEXTURES;
	CheckLumpStreamHeader( LUMP_IDENT_DTEXTURE, sizeof( dtexture_t ), &out_dtexturenum );

	fread( out_dtextures, sizeof( dtexture_t ), out_dtexturenum, h );
}

void ReadDTexdefs( void )
{
	out_dtexdefnum = MAX_DTEXDEFS;
	CheckLumpStreamHeader( LUMP_IDENT_DTEXDEF, sizeof( dtexdef_t ), &out_dtexdefnum );

	fread( out_dtexdefs, sizeof( dtexdef_t ), out_dtexdefnum, h );
}

void ReadDPlanes( void )
{
	out_dplanenum = MAX_DPLANES;
	CheckLumpStreamHeader( LUMP_IDENT_DPLANE, sizeof( dplane_t ), &out_dplanenum );

	fread( out_dplanes, sizeof( dplane_t ), out_dplanenum, h );
}

void ReadDLeafs( void )
{
	out_dleafnum = MAX_DLEAFS;
	CheckLumpStreamHeader( LUMP_IDENT_DLEAF, sizeof( dleaf_t ), &out_dleafnum );

	fread( out_dleafs, sizeof( dleaf_t ), out_dleafnum, h );
}

void ReadDNodes( void )
{
	out_dnodenum = MAX_DNODES;
	CheckLumpStreamHeader( LUMP_IDENT_DNODE, sizeof( dnode_t ), &out_dnodenum );

	fread( out_dnodes, sizeof( dnode_t ), out_dnodenum, h );
}

// =============================================================================

void InitPData( void )
{
	out_dvertexnum = 0;
	out_dedgenum = 0;
	out_dfaceedgenum = 0;
	out_dfacenum = 0;
	out_dtexturenum = 0;
	out_dtexdefnum = 0;
	out_dplanenum = 0;
	out_dleafnum = 0;
	out_dnodenum = 0;
}

void DumpPData( void )
{
	printf( "DumpPData:\n" );
	printf( " Vertices:  %d\n", out_dvertexnum );
	printf( " Edges:     %d\n", out_dedgenum );
	printf( " Faceedges: %d\n", out_dfaceedgenum );
	printf( " Faces:     %d\n", out_dfacenum );
	printf( " Textures:  %d\n", out_dtexturenum );
	printf( " Texdefs:   %d\n", out_dtexdefnum );
	printf( " Planes:    %d\n", out_dplanenum );
	printf( " Leafs:     %d\n", out_dleafnum );
	printf( " Nodes:     %d\n", out_dnodenum );
}

void WritePFile( char *file ) 
{
	printf(" --- WritePFile ---\n");
	printf("file: %s\n", file );

	h = fopen( file, "wb" );

	if ( !h ) {
		printf("WritePFile: can't open file.\n");
		exit(-1);
	}

	WriteDVertices();
	WriteDEdges();
	WriteDFaceEdges();
	WriteDFaces();

	WriteDTextures();
	WriteDTexdefs();
	WriteDPlanes();

	WriteDLeafs();
	WriteDNodes();

	fclose( h );
}

void ReadPFile( char *file )
{
	printf( " --- ReadPFile ---\n" );
	printf( "file: %s\n", file );

	h = fopen( file, "rb" );
	if ( !h )
	{
		printf( "ReadPFile: can't open file.\n" );
		exit(-1);
	}

	ReadDVertices();
	ReadDEdges();
	ReadDFaceEdges();
	ReadDFaces();

	ReadDTextures();
	ReadDTexdefs();
	ReadDPlanes();

	ReadDLeafs();
	ReadDNodes();
	
	fclose( h );
}
