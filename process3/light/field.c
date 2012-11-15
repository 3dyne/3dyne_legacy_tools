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



// field.c

#include "light.h"


void WriteFieldBinary( map3_t *map )
{
	FILE		*h;
	int		i;
	veccell_t	*c;

	h = fopen( "_fieldmap.bin", "w" );
	
	// write cell num
	fwrite( &map->totalitemnum, 4, 1, h );

	for ( i = 0; i < MAP3_HASHSIZE; i++ )
	{
		for ( c = map->hash[i]; c ; c=c->next )
		{
			short		s;

			s = (short)c->x;
			fwrite( &s, 2, 1, h );
			s = (short)c->y;
			fwrite( &s, 2, 1, h );
			s = (short)c->z;
			fwrite( &s, 2, 1, h );
			
			fwrite( &c->vec[0], 4, 1, h );
			fwrite( &c->vec[1], 4, 1, h );
			fwrite( &c->vec[2], 4, 1, h );
		}
	}
	fclose( h );
}

void DistributeFieldVectors( face_t *list )
{
	int		x, y, z;
	map3_t		*map;
	int		patch_num = 0;
	int		cell_num = 0;
	face_t		*f;
	patch_t		*p;
	
	map = NewMap3Hash();

	for ( f = list; f ; f=f->next )
	{
		for ( p = f->patches; p ; p=p->next )
		{
			vec3d_t		h;
			veccell_t	*c;

#if 0
			// 0.0 hull
//			Vec3dMA( h, 1.0, p->norm, p->origin );
			Vec3dCopy( h, p->origin );
			x = (int)rint(h[0]/16.0);
			y = (int)rint(h[1]/16.0);
			z = (int)rint(h[2]/16.0);
			

			if ( ( c = Map3FindCell( map, x, y, z ) ) )
			{
				Vec3dAdd( c->vec, c->vec, p->norm );
			}
			else
			{
				c = NEW( veccell_t );
				c->x = x;
				c->y = y;
				c->z = z;
				Map3InsertCell( map, c );
				Vec3dCopy( c->vec, p->norm );
				cell_num++;
			}			
#endif

			// 16.0 hull
			Vec3dMA( h, 8.0, p->norm, p->origin );
//			x = (int)rint(h[0]/16.0);
//			y = (int)rint(h[1]/16.0);
//			z = (int)rint(h[2]/16.0);
			x = (int)(h[0]/16.0+0.5);
			y = (int)(h[1]/16.0+0.5);
			z = (int)(h[2]/16.0+0.5);
			

			if ( ( c = Map3FindCell( map, x, y, z ) ) )
			{
				Vec3dAdd( c->vec, c->vec, p->norm );
			}
			else
			{
				c = NEW( veccell_t );
				c->x = x;
				c->y = y;
				c->z = z;
				Map3InsertCell( map, c );
				Vec3dCopy( c->vec, p->norm );
				cell_num++;
			}

#if 0
			// 32.0 hull 
			Vec3dMA( h, 32.0, p->norm, p->origin );
			x = (int)rint(h[0]/16.0);
			y = (int)rint(h[1]/16.0);
			z = (int)rint(h[2]/16.0);
			

			if ( ( c = Map3FindCell( map, x, y, z ) ) )
			{
				Vec3dAdd( c->vec, c->vec, p->norm );
			}
			else
			{
				c = NEW( veccell_t );
				c->x = x;
				c->y = y;
				c->z = z;
				Map3InsertCell( map, c );
				Vec3dCopy( c->vec, p->norm );
				cell_num++;
			}
#endif
			patch_num++;
		}


	}

//	Map3Dump( map );
	printf( " %d cells for %d patches\n", cell_num, patch_num );
	WriteFieldBinary( map );
}
