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



// r_facesetup.h

#ifndef __r_facesetup
#define __r_facesetup

#include "render.h"
//#include "r_fsstate.h"

//
// r_facesetup.c
//

#define MAX_FS_VERTICES		( 8192 )
#define FS_FACEBUFFER_SIZE	( 256*256 )

void R_BeginFaceSetup( void );		
void R_EndFaceSetup( void );		
void R_DumpFaceSetup( void );		

void R_FaceSetup_NeedFrustumClip( face_t *f, frustumClipMask clip );
void R_FaceSetup_NoClip( face_t *f );					

//
// r_fsglva.c
//

void R_FS_SetupVA( void );


//
// r_fstexcrd.c
//

void FS_GenTexcoord_Vertex( int vindex, projectionType type );
void FS_GenTexcoord_tmap0( texdef_t *td );
void FS_GenTexcoord_lmap0( lightdef_t *ld );
void FS_GenTexcoord_lmap1( lightdef_t *ld );

//
// r_fstmap.c
//

typedef enum
{
	FSTMapsFlags_none = 0,
	FSTMapsFlags_tmap0 = 1,
	FSTMapsFlags_lmap0 = 2,
	FSTMapsFlags_lmap1 = 4
} fsTMapsFlags;

void R_FS_InitTMapsBfr( void );
//void R_FS_FillTMapsBfr( face_t *f, int *facebfr );
fsTMapsFlags R_FS_GetTMapsFlags( face_t *f );

void R_FS_FillTMapsBfr_tmap0( int *facebfr, int mapref );
void R_FS_FillTMapsBfr_lmap0( int *facebfr, int mapref );
void R_FS_FillTMapsBfr_lmap1( int *facebfr, int mapref );

//
// r_fstmapmt.c, multitexture version
//

typedef struct
{
	int		*facebfr;
	int		tmap0ref;
} facebfr_tmap0_sort_t;

void R_FS_InitTMapsBfr_mt( void );
void R_FS_FinishTMapsBfr_mt( void );
void R_FS_DumpTMapsBfr_mt( void );
void R_FS_FillTMapsBfr_mt( face_t *f, int *facebfr );


//
// r_fsglraster.c
//

void R_FS_RasterizeTMapsBfr_mt( void );

#endif
