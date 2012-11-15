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



// lib_pfiles.h

#ifndef __outfiles
#define __outfiles

typedef struct {
	float	p[3];
	
} dvertex_t;

typedef struct {
	int	dv[2];
} dedge_t;


typedef int	dfaceedge_t; 

typedef struct {
	int	texdef;
	int	plane;
	int	side;
	int	firstfaceedge;
	int	faceedgenum;
} dface_t;

typedef char	dtexture_t[56]; // fix me: TEXTURE_IDENT_SIZE

typedef struct {
	int	texture;
	float	rotate;
	float	shift[2];
	float	scale[2];

	float	vecs[2][4];
} dtexdef_t;	

typedef struct {
	float	norm[3];
	float	dist;
} dplane_t;

typedef struct {
	int	plane;
	int	children[2];
} dnode_t;

typedef struct {
	int	contents;
} dleaf_t;

#define		MAX_DVERTICES	( 16384 * 4 )
#define		MAX_DEDGES	( 8192 * 4 )
#define		MAX_DFACEEDGES	( 16384 * 8 )
#define		MAX_DFACES	( 8192 * 8 )
#define		MAX_DTEXTURES	( 1024 )
#define		MAX_DTEXDEFS	( 4096 )
#define		MAX_DPLANES	( 2048 )
#define		MAX_DLEAFS	( 8192 )
#define		MAX_DNODES	( 8192 )

#define		LUMP_VERSION		( "LMP1" )

#define		LUMP_IDENT_DVERTEX		( "VERT" )
#define		LUMP_IDENT_DEDGE		( "EDGE" )
#define		LUMP_IDENT_DFACEEDGE		( "FAEG" )
#define		LUMP_IDENT_DFACE		( "FACE" )
#define		LUMP_IDENT_DTEXTURE		( "TXTR" )
#define		LUMP_IDENT_DTEXDEF		( "TDEF" )
#define		LUMP_IDENT_DPLANE		( "HNFP" )
#define		LUMP_IDENT_DLEAF		( "BSPL" )
#define		LUMP_IDENT_DNODE		( "BSPN" )

// still use p_texdefs[]
//#define		MAX_DTEXDEF	

// from outface.c
extern int			out_dvertexnum;
extern dvertex_t		out_dvertices[ MAX_DVERTICES ];

extern int			out_dedgenum;
extern dedge_t			out_dedges[ MAX_DEDGES ];

extern int			out_dfaceedgenum;
extern dfaceedge_t		out_dfaceedges[ MAX_DFACEEDGES ];

extern int			out_dfacenum;
extern dface_t			out_dfaces[ MAX_DFACES ];

extern int			out_dtexturenum;
extern dtexture_t		out_dtextures[ MAX_DTEXTURES ];

extern int			out_dtexdefnum;
extern dtexdef_t		out_dtexdefs[ MAX_DTEXDEFS ];

extern int			out_dplanenum;
extern dplane_t			out_dplanes[ MAX_DPLANES ];

extern int			out_dleafnum;
extern dleaf_t			out_dleafs[ MAX_DLEAFS ];

extern int			out_dnodenum;
extern dnode_t			out_dnodes[ MAX_DNODES ];

//

void WriteDVertices( void );
void WriteDEdges( void );
void WriteDFaceEdges( void );
void WriteDFaces( void );

void WriteDTextures( void );
void WriteDTexdefs( void );
void WriteDPlanes( void );

//
void WriteDLeafs( void );
void WriteDNodes( void );
//

void DumpPData( void );
void InitPData( void );
void WritePFile( char* );
void ReadPFile( char* );

#endif // __outfiles_included
