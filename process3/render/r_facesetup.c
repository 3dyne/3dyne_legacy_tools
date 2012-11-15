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



// r_facesetup.c

#include "render.h"
#include "r_facesetup.h"
#include "r_fsstate.h"

int		fs_vertexnum;
vertex_t		*fs_vertices[MAX_FS_VERTICES];	// r_vertices and frustum clip vertices

int			*fs_facebfrptr;
int			fs_facebfrfree;
int			fs_facebfr[FS_FACEBUFFER_SIZE];



void R_BeginFaceSetup( void )
{
	fs_vertexnum = 0;
	fs_facebfrptr = fs_facebfr;

	fs_vertex_array_num = 0;

	if ( (int)fs_facebfrptr & 3 )
		Error( "R_BeginFaceSetup: alignment check failed for face buffer.\n" );

	fs_facebfrfree = FS_FACEBUFFER_SIZE;
	
	if ( gl_ext.have_arb_multitexture )
	{
		R_FS_InitTMapsBfr_mt();
	}
	else
	{
		R_FS_InitTMapsBfr();
	}
}

void R_EndFaceSetup( void )
{
	// terminate facebfr
	*fs_facebfrptr++ = 0;

	R_FS_SetupVA(); 

	if ( gl_ext.have_arb_multitexture )
	{
		R_FS_FinishTMapsBfr_mt();
	}
	else
	{
		R_FS_FinishTMapsBfr();
	}	

}

void R_DumpFaceSetup( void )
{
	printf( " %d fs_vertices, %.3fk in fs face buffer\n", fs_vertexnum, ((int)fs_facebfrptr-(int)fs_facebfr)/1024.0 );

	if ( gl_ext.have_arb_multitexture )
	{
//		R_FS_DumpTMapsBfr_mt();
	}
	else
	{

	}	
}

void R_FaceSetup_NeedFrustumClip( face_t *f, frustumClipMask clip )
{
	
}



void R_FaceSetup_NoClip( face_t *f )
{
	int		i;
	vertex_t	*v;
	int		*ptr, *facebfr;
	fixpolygon_t	*fix;
	fsTMapsFlags	maps;
	
	facebfr = ptr = fs_facebfrptr;
	fix = f->fixpolygon;

	*ptr++ = fix->pointnum;

	maps = R_FS_GetTMapsFlags( f );
//	R_FS_SetupFX( f );
	*ptr++ = maps;

	*ptr++ = fs_vertex_array_num;

	for ( i = 0; i < fix->pointnum; i++, fs_vertex_array_num++ )
	{
		v = &r_vertices[r_vertexrefs[i+fix->startvertexref].vertex];

		if ( v->fs_count == r_frame_count )
		{
			//
			// r_vertex has a fs_vertex
			//

			fs_transformed_vref[fs_vertex_array_num] = v->fs_vertex;
		}
		else
		{
			//
			// create a new fs_vertex
			//

			fs_vertices[fs_vertexnum] = v;
			fs_transformed_vref[fs_vertex_array_num] = v->fs_vertex = fs_vertexnum++;
			v->fs_count = r_frame_count;			
		}

		FS_GenTexcoord_Vertex( fs_vertex_array_num, r_texdefs[fix->texdef].projection );

		if ( maps & FSTMapsFlags_tmap0 )
			FS_GenTexcoord_tmap0( &r_texdefs[fix->texdef] );
		if ( maps & FSTMapsFlags_lmap0 )
			FS_GenTexcoord_lmap0( &r_lightdefs[fix->lightdef] );
		if ( maps & FSTMapsFlags_lmap1 )
			FS_GenTexcoord_lmap1( &r_lightdefs[fix->lightdef] );
	}

	fs_facebfrptr = ptr;

	if ( gl_ext.have_arb_multitexture )
	{
		if ( ( maps & FSTMapsFlags_tmap0 ) && ( maps & FSTMapsFlags_lmap0 )  )
		{
			// multi texture, tmap0 lmap0
			R_FSFillTMapsBfr_mt_tmap0_lmap0( facebfr, r_texdefs[fix->texdef].texture, r_lightdefs[fix->lightdef].lightmaps[0].lightpage );
		}
		
	}
	else
	{
		if ( maps & FSTMapsFlags_tmap0 )
			R_FS_FillTMapsBfr_tmap0( facebfr, r_texdefs[fix->texdef].texture );
		if ( maps & FSTMapsFlags_lmap0 )
			R_FS_FillTMapsBfr_lmap0( facebfr, r_lightdefs[fix->lightdef].lightmaps[0].lightpage );
		if ( maps & FSTMapsFlags_lmap1 )
			R_FS_FillTMapsBfr_lmap1( facebfr, r_lightdefs[fix->lightdef].lightmaps[1].lightpage );
		
	}
}
