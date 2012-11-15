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



// lib_gldbg.c

#include "lib_gldbg.h"

/*
  ==============================
  EmitToken

  internal

  ==============================
*/
static void EmitToken( gld_session_t *s, char *token )
{
//	char	str[256];
	
	fprintf( s->handle, "\"%s\"\n", token );	
}


/*
  ==================================================
  control functions

  ==================================================
*/

/*
  ==============================
  GLD_BeginSession

  ==============================
*/
gld_session_t * GLD_BeginSession( char *name )
{
	gld_session_t *s;
	
	s = (gld_session_t *) malloc( sizeof( gld_session_t ) );
	memset( s, 0, sizeof( gld_session_t ) );

	strcpy( s->name, name );
	s->record_active = false;

	s->handle = fopen( name, "w" );
	
	return s;
}

/*
  ==============================
  GLD_EndSession

  ==============================
*/
void GLD_EndSession( gld_session_t *s )
{
	EmitToken( s, "cmd_quit" );
	fclose( s->handle );
}

/*
  ==============================
  GLD_StartRecord

  ==============================
*/
void GLD_StartRecord( gld_session_t *s )
{
	s->record_active = true;
}

/*
  ==============================
  GLD_StopRecord

  ==============================
*/
void GLD_StopRecord( gld_session_t *s )
{
	s->record_active = false;
}

/*
  ==============================
  GLD_IsRecording

  ==============================
*/
bool_t GLD_IsRecording( gld_session_t *s )
{
	return s->record_active;
}

/*
  ==================================================
  recordable commands

  ==================================================
*/

/*
  ==============================
  GLD_Pause

  ==============================
*/
void GLD_Pause( gld_session_t *s )
{
	if ( s->record_active )
	{
		EmitToken( s, "cmd_pause" );
	}
}

/*
  ==============================
  GLD_Update

  ==============================
*/
void GLD_Update( gld_session_t *s )
{
	if ( s->record_active )
	{
		EmitToken( s, "cmd_update" );
	}
}


/*
  ==============================
  GLD_BeginList

  ==============================
*/
void GLD_BeginList( gld_session_t *s, char *name, char *polygon_mode )
{
	if ( s->record_active )
	{
		EmitToken( s, "cmd_begin_list" );
		EmitToken( s, name ); 
		EmitToken( s, polygon_mode );
	}
}

/*
  ==============================
  GLD_EndList

  ==============================
*/
void GLD_EndList( gld_session_t *s )
{
	if ( s->record_active )
	{
		EmitToken( s, "cmd_end_list" );
	}
}

/*
  ==============================
  GLD_Color3f

  ==============================
*/
void GLD_Color3f( gld_session_t *s, fp_t r, fp_t g, fp_t b )
{
	char	str[256];

	if ( s->record_active )
	{
		EmitToken( s, "cmd_color3f" );
		sprintf( str, "%.3f", r );
		EmitToken( s, str );
		sprintf( str, "%.3f", g );
		EmitToken( s, str );
		sprintf( str, "%.3f", b );
		EmitToken( s, str );
	}
}

 
/*
  ==============================
  GLD_Begin

  ==============================
*/
void GLD_Begin( gld_session_t *s, char *primitive )
{
	if ( s->record_active )
	{
		EmitToken( s, "cmd_begin_primitive" );
		EmitToken( s, primitive );
	}
}

/*
  ==============================
  GLD_End

  ==============================
*/
void GLD_End( gld_session_t *s )
{
	if ( s->record_active )
	{
		EmitToken( s, "cmd_end_primitive" );
	}
}


/*
  ==============================
  GLD_Vertex3f

  ==============================
*/
void GLD_Vertex3f( gld_session_t *s, fp_t x, fp_t y, fp_t z )
{
	char	str[256];

	if ( s->record_active )
	{
		EmitToken( s, "cmd_vertex3f" );
		sprintf( str, "%.3f", x );
		EmitToken( s, str );
		sprintf( str, "%.3f", y );
		EmitToken( s, str );
		sprintf( str, "%.3f", z );
		EmitToken( s, str );	
	}
}

/*
  ==============================
  GLD_Vertex3fv

  ==============================
*/
void GLD_Vertex3fv( gld_session_t *s, vec3d_t v )
{
	char	str[256];

	if ( s->record_active )
	{
		EmitToken( s, "cmd_vertex3f" );
		sprintf( str, "%.3f", v[0] );
		EmitToken( s, str );
		sprintf( str, "%.3f", v[1] );
		EmitToken( s, str );
		sprintf( str, "%.3f", v[2] );
		EmitToken( s, str );		
	}
}

/*
  ==================================================
  util funcs

  ==================================================
*/

/*
  ==============================
  GLD_EasyPolygon

  ==============================
*/
void GLD_EasyPolygon( gld_session_t *s, polygon_t *poly )
{
	int		i;

	if ( !poly )
		return;

	GLD_Begin( s, "polygon" );
	for ( i = 0; i < poly->pointnum; i++ )
	{
		GLD_Vertex3fv( s, poly->p[i] );
	}
	GLD_End( s );
}


/*
  ==================================================
  test stuff

  ==================================================
*/
#if 0

int main()
{
	gld_session_t	*s;
	vec3d_t	v1={-0.5,-0.5,0.1};
	vec3d_t	v2={-0.5,0.5,0.1};
	vec3d_t	v3={0.5,0.5,0.1};
	vec3d_t v4={0.5,-0.5,0.1};

	s = GLD_BeginSession( "gldbg.log" );

	GLD_StartRecord( s );
	
	GLD_BeginList( s, "list1" );
	GLD_Begin( s, "polygon" );
	GLD_Vertex3fv( s, v1 );
	GLD_Vertex3fv( s, v2 );
	GLD_Vertex3fv( s, v3 );
	GLD_End( s );
	GLD_EndList( s );
	GLD_Update( s );
	
	GLD_BeginList( s, "list1" );
	GLD_Begin( s, "polygon" );	
	GLD_Color3f( s, 1, 0, 0 );
	GLD_Vertex3fv( s, v2 );
	GLD_Vertex3fv( s, v3 );
	GLD_Vertex3fv( s, v4 );
	GLD_End( s );	
	GLD_EndList( s );
	GLD_Update( s );

	GLD_Pause( s );
	GLD_EndSession( s );
	
}


#endif
