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



// type_simple.c

#include "type_simple.h"

// write simple types to a stream

bool_t NoFrac( fp_t b )
{
	fp_t	r;
	r = (fp_t)(rint(b));
	if ( r == b )
		return true;
	return false;
}

void Write_fp( FILE *h, fp_t b )                                                         
{                                                                               
        if ( NoFrac( b ) )                                                      
                fprintf( h , "%.0f ", b );                                
        else                                                                    
                fprintf( h, "%f ", b );                                  
}

void Write_Vec2d( FILE *h, vec2d_t v )                                                   
{                                                                               
        int             i;                                                      
        fprintf( h, "( " );                                              
        for ( i = 0; i < 2; i++ )                                               
        {                                                                       
                Write_fp( h, v[i] );                                               
        }                                                                       
        fprintf( h, ") " );                                              
}


void Write_Vec3d( FILE *h, vec3d_t v )                                                   
{                                                                               
        int             i;                                                      
        fprintf( h, "( " );                                              
        for ( i = 0; i < 3; i++ )                                               
        {                                                                       
                Write_fp( h, v[i] );                                               
        }                                                                       
        fprintf( h, ") " );                                              
}



void Write_Polygon( FILE *h, polygon_t *p )
{
	int		i;
	
	fprintf( h, "%d ", p->pointnum );

	for ( i = 0; i < p->pointnum; i++ )
		Write_Vec3d( h, p->p[i] );
}


// read simple types
void Read_Vec3d( tokenstream_t *ts, vec3d_t v )
{
	int		i;

	// skip '('
	GetToken( ts );

	for ( i = 0; i < 3; i++ )
	{
		GetToken( ts );
		v[i] = atof( ts->token );
	}

	// skip ')'
	GetToken( ts );
}



void Read_Vec2d( tokenstream_t *ts, vec2d_t v )
{
	int		i;

	// skip '('
	GetToken( ts );

	for ( i = 0; i < 2; i++ )
	{
		GetToken( ts );
		v[i] = atof( ts->token );
	}

	// skip ')'
	GetToken( ts );
}



polygon_t* Read_Polygon( tokenstream_t *ts )
{
	int		i;
	polygon_t	*p;

	// get pointnum
	GetToken( ts );
	i = atoi( ts->token );
	p = NewPolygon( i );
	p->pointnum = i;
	for ( i = 0; i < p->pointnum; i++ )
		Read_Vec3d( ts, p->p[i] );

	return p;
}
