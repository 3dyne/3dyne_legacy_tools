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



// r_lightmap.c

#include "render.h"


/*
  ==================================================
  lightdef / lightmap rep

  ==================================================
*/



int		r_lightdefnum;
lightdef_t	r_lightdefs[MAX_LIGHTDEFS];

void CompileLightdefClass( hmanager_t *lightdefhm )
{
	hobj_search_iterator_t	iter;
	hobj_t		*lightdef;
	hpair_t		*pair;
	char		tt[256];

	r_lightdefnum = 0;
	InitClassSearchIterator( &iter, HManagerGetRootClass( lightdefhm ), "lightdef" );
	for ( ;( lightdef = SearchGetNextClass( &iter ) ); )
	{
		sprintf( tt, "%d", r_lightdefnum );
		pair = NewHPair2( "int", "index", tt );
		InsertHPair( lightdef, pair );

		pair = FindHPair( lightdef, "patchsize" );
		if ( !pair )
			Error( "missing 'patchsize' in lightdef '%s'.\n", lightdef->name );
		HPairCastToFloat_safe( &r_lightdefs[r_lightdefnum].patchsize, pair );
		
		// OPT
		r_lightdefs[r_lightdefnum].scale = 1.0 / (r_lightdefs[r_lightdefnum].patchsize*128.0);

		
		pair = FindHPair( lightdef, "height" );
		if ( !pair )
			Error( "missing 'height' in lightdef '%s'.\n", lightdef->name );
		HPairCastToInt_safe( &r_lightdefs[r_lightdefnum].height, pair );

		if ( r_lightdefs[r_lightdefnum].height > 128 )
			Error( "lightdef: height > 128\n" );

		pair = FindHPair( lightdef, "width" );
		if ( !pair )
			Error( "missing 'width' in lightdef '%s'.\n", lightdef->name );
		HPairCastToInt_safe( &r_lightdefs[r_lightdefnum].width, pair );

		if ( r_lightdefs[r_lightdefnum].width > 128 )
			Error( "lightdef: width > 128\n" );

		pair = FindHPair( lightdef, "projection" );
		if ( !pair )
			Error( "missing 'projection' in lightdef '%s'.\n", lightdef->name );
		HPairCastToInt_safe( &r_lightdefs[r_lightdefnum].projection, pair );

		pair = FindHPair( lightdef, "shift" );
		if ( !pair )
			Error( "missing 'shift' in lightdef '%s'.\n", lightdef->name );
		HPairCastToVec2d_safe( r_lightdefs[r_lightdefnum].shift, pair );

		r_lightdefs[r_lightdefnum].lightmapnum = 0;

		pair = FindHPair( lightdef, "diffuse" );
		if ( pair )
		{
			r_lightdefs[r_lightdefnum].lightmaps[r_lightdefs[r_lightdefnum].lightmapnum].type = LightmapType_diffuse;
			r_lightdefs[r_lightdefnum].lightmaps[r_lightdefs[r_lightdefnum].lightmapnum].lightpage = -1;
			r_lightdefs[r_lightdefnum].lightmapnum++;
		}

		pair = FindHPair( lightdef, "specular" );
		if ( pair )
		{
			r_lightdefs[r_lightdefnum].lightmaps[r_lightdefs[r_lightdefnum].lightmapnum].type = LightmapType_specular;
			r_lightdefs[r_lightdefnum].lightmaps[r_lightdefs[r_lightdefnum].lightmapnum].lightpage = -1;
			r_lightdefs[r_lightdefnum].lightmapnum++;
		}		

		r_lightdefs[r_lightdefnum].self = lightdef;

		r_lightdefnum++;
	}

	printf( " %d lightdefs\n", r_lightdefnum );
 
//	SetupLightPages();
}

/*
  ==================================================
  lightpage stuff

  ==================================================
*/


int		r_lightpagenum;
lightpage_t	r_lightpages[MAX_LIGHTPAGES];

void LightPage_InsertSubImage( lightpage_t *lp, int xpos, int ypos, int width, int height, unsigned short *image )
{
	int		x, y;
	unsigned short	flat;

	flat = _Random() & 0xffff;

	for ( x = 0; x < width; x++ )
	{
		for ( y = 0; y < height; y++ )
		{
			lp->image[xpos+x + (ypos+y)*LIGHTPAGE_WIDTH] = image[x + y*width];
//			lp->image[xpos+x + (ypos+y)*LIGHTPAGE_WIDTH] = flat;
		}
	}	
}



void LightPage_GenTexture( lightpage_t *lp )
{
	unsigned char		*image888;


	image888 = Image565ToImage888( lp->image, LIGHTPAGE_WIDTH*LIGHTPAGE_HEIGHT );

	glEnable( GL_TEXTURE_2D );
	glGenTextures( 1, &lp->texobj );
	if ( glGetError() != GL_NO_ERROR )
		Error( "glGenTextures failed\n" );
	glBindTexture( GL_TEXTURE_2D, lp->texobj );

#if 1
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif	
#if 1
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, LIGHTPAGE_WIDTH, LIGHTPAGE_HEIGHT, 
		      0, GL_RGB, GL_UNSIGNED_BYTE, image888 );

	if ( glGetError() != GL_NO_ERROR )
		Error( "glTexImage2D failed\n" );

	free( image888 );
}

#if 0
/*
  ====================
  GetBestLightdef

  returns index of the lightdef, that
  comes cloesest to width and height

  or: -1 = no more lightdefs
      -2 = no lightdef found, that came close
  ====================
*/
void GetBestLightdef( int width, int height, lightmapType type, int *bestlightdef, int *lightmap )
{
	int	i, j;
	int	dwmin, dhmin;
	int	dw, dh;
	int	best;
	bool_t	toobig = false;
		
	dwmin = dhmin = 8192;
	*bestlightdef = -1;
	*lightmap = -1;

//	printf( "(%dx%d): ", width, height );

	for ( i = 0; i < r_lightdefnum; i++ )
	{
		for ( j = 0; j < r_lightdefs[i].lightmapnum; j++ )
		{
			if ( r_lightdefs[i].lightmaps[j].type != type )
				continue;

			if ( r_lightdefs[i].lightmaps[j].lightpage != -1 )
				continue;
			
			dw = width - r_lightdefs[i].width;
			dh = height - r_lightdefs[i].height;
			
			if ( dw < 0 || dh < 0 )
			{
				toobig = true;
				continue;
			}
			
			if ( dw <= dwmin && dh <= dhmin )
			{
				dwmin = dw;
				dhmin = dh;
				*bestlightdef = i;
				*lightmap = j;
			}
		}
	}
//	if ( best >= 0 )
//		printf( "%dx%d\n", r_lightdefs[best].width, r_lightdefs[best].height );
	
	if ( *bestlightdef == -1 && toobig )
		*bestlightdef = -2;
}


int	lightdefnum_in_page;

int FillLightPageRecursive( int lightpage, int x, int y, int width, int height, lightmapType type )
{
	int		w, h;
	int		lightdef, lightmap;

	// get best lightdef for width and height

//	printf( "%d %d %d %d %d: ", lightpage, x, y, width, height );

	GetBestLightdef( width, height, type, &lightdef, &lightmap );
	if ( lightdef < 0 )
	{
//		printf( "*\n" );
		return lightdef;
	}
	w = r_lightdefs[lightdef].width;
	h = r_lightdefs[lightdef].height;
//	printf( "%d %d\n", w, h );
	r_lightdefs[lightdef].lightmaps[lightmap].lightpage = lightpage;

	r_lightdefs[lightdef].lightmaps[lightmap].xofs = x;
	r_lightdefs[lightdef].lightmaps[lightmap].yofs = y;
	
	// OPT
	r_lightdefs[lightdef].lightmaps[lightmap].xofs2 = x/128.0;
	r_lightdefs[lightdef].lightmaps[lightmap].yofs2 = y/128.0;


	lightdefnum_in_page++;

	// fill lower left
	if ( height - h > 0 )
	{
		lightdef= FillLightPageRecursive( lightpage, x, y + h, w, (height - h), type );
	}
	
	// fill upper right
	if ( width - w > 0 )
	{
		lightdef= FillLightPageRecursive( lightpage, x + w, y, (width - w), h, type );
	}

	// fill lower right
	if ( width - w > 0 && height - h > 0 )
	{
		lightdef= FillLightPageRecursive( lightpage, x + w, y + h, (width - w), (height - h), type );
	}

	return lightdef;
}
#endif

void SetupLightPages( void )
{
	int		i, j;


	//
	// diffuse lightmaps
	//

	printf( "setup diffuse lightpages ...\n" );
	R_LightPage_BeginRegister();
	for ( i = 0; i < r_lightdefnum; i++ )
	{
		for ( j = 0; j < r_lightdefs[i].lightmapnum; j++ )
		{

			if ( r_lightdefs[i].lightmaps[j].type == LightmapType_diffuse )
			{
				r_lightdefs[i].lightmaps[j].box = R_LightPage_RegisterBox( r_lightdefs[i].width, r_lightdefs[i].height );			      
			}
		}
	}
	R_LightPage_EndRegister();
	R_LightPage_FillRegisteredBoxes();



	//
	// specular lightmaps
	//

	printf( "setup specular lightpages ...\n" );
	R_LightPage_BeginRegister();
	for ( i = 0; i < r_lightdefnum; i++ )
	{
		for ( j = 0; j < r_lightdefs[i].lightmapnum; j++ )
		{

			if ( r_lightdefs[i].lightmaps[j].type == LightmapType_specular )
			{
				r_lightdefs[i].lightmaps[j].box = R_LightPage_RegisterBox( r_lightdefs[i].width, r_lightdefs[i].height );
			}
		}
	}
	R_LightPage_EndRegister();
	R_LightPage_FillRegisteredBoxes();


       	//
	// now really copy the lightmaps
	//

	for ( i = 0; i < r_lightdefnum; i++ )
	{
		for ( j = 0; j < r_lightdefs[i].lightmapnum; j++ )
		{
			int		max_lightmap_size;
			hpair_t		*pair;
			hobj_t		*lightdef;
			unsigned short		lightmap[128*128];

			r_lightdefs[i].lightmaps[j].lightpage = r_lightdefs[i].lightmaps[j].box->lightpage;
			r_lightdefs[i].lightmaps[j].xofs = r_lightdefs[i].lightmaps[j].box->xofs;
			r_lightdefs[i].lightmaps[j].yofs = r_lightdefs[i].lightmaps[j].box->yofs;
			r_lightdefs[i].lightmaps[j].xofs2 = r_lightdefs[i].lightmaps[j].xofs / 128.0;
			r_lightdefs[i].lightmaps[j].yofs2 = r_lightdefs[i].lightmaps[j].yofs / 128.0;
			
			// hack
			FREE( r_lightdefs[i].lightmaps[j].box );
			r_lightdefs[i].lightmaps[j].box = NULL;

			if ( r_lightdefs[i].lightmaps[j].type == LightmapType_diffuse )
			{
				pair = FindHPair( r_lightdefs[i].self, "diffuse" );
				if ( !pair )
					Error( "missing 'diffuse' in lightdef '%s'.\n", r_lightdefs[i].self->name );
				
				max_lightmap_size = 128*128*2;
				HPairCastToBstring_safe( lightmap, &max_lightmap_size, pair );
				
				LightPage_InsertSubImage( &r_lightpages[r_lightdefs[i].lightmaps[j].lightpage],
							  r_lightdefs[i].lightmaps[j].xofs,
							  r_lightdefs[i].lightmaps[j].yofs,
							  r_lightdefs[i].width,
							  r_lightdefs[i].height,
							  lightmap );
			}
			else if ( r_lightdefs[i].lightmaps[j].type == LightmapType_specular )
			{
				pair = FindHPair( r_lightdefs[i].self, "specular" );
				if ( !pair )
					Error( "missing 'diffuse' in lightdef '%s'.\n", r_lightdefs[i].self->name );
				
				max_lightmap_size = 128*128*2;
				HPairCastToBstring_safe( lightmap, &max_lightmap_size, pair );
				
				LightPage_InsertSubImage( &r_lightpages[r_lightdefs[i].lightmaps[j].lightpage],
							  r_lightdefs[i].lightmaps[j].xofs,
							  r_lightdefs[i].lightmaps[j].yofs,
							  r_lightdefs[i].width,
							  r_lightdefs[i].height,
							  lightmap );
			}
			else
			{
				Error( "unkown lightmap type.\n" );
			}
		}
	}

}

void R_LightPage_GenTextures( void )
{
	int		i;

	//
	// build opengl textures
	//	
	for ( i = 0; i < r_lightpagenum; i++ )
	{
		LightPage_GenTexture( &r_lightpages[i] );	
	}      
}

#if 0
void SetupLightPages_old( void )
{
	int		i, j;
	int		count;

	//
	// fill lightdefs into lightpages
	//
	count = 0;
	r_lightpagenum = 0;
	// first diffuse lightmaps
	for ( ; r_lightpagenum < MAX_LIGHTPAGES && count != -1; r_lightpagenum++ )
	{
		lightdefnum_in_page = 0;
		count = FillLightPageRecursive( r_lightpagenum, 0, 0, 128, 128, LightmapType_diffuse );

		printf( " %d lightdefs in lightpage %d\n", lightdefnum_in_page, r_lightpagenum );
		
	}
	count = 0;

	// second specular lightmaps
	for ( ; r_lightpagenum < MAX_LIGHTPAGES && count != -1; r_lightpagenum++ )
	{
		lightdefnum_in_page = 0;
		count = FillLightPageRecursive( r_lightpagenum, 0, 0, 128, 128, LightmapType_specular );
		
		printf( " %d lightdefs in lightpage %d\n", lightdefnum_in_page, r_lightpagenum );
		
	}	


	if ( r_lightpagenum == MAX_LIGHTPAGES )
		Error( "reached MAX_LIGHTPAGES\n" );

	printf( " %d lightpages\n", r_lightpagenum );

	for ( i = 0; i < r_lightpagenum; i++ )
		memset( r_lightpages[i].image, 0xff, 128*128*2 );

	//
	// now really copy the lightmaps
	//
	{
		int		max_lightmap_size;
		hpair_t		*pair;
		hobj_t		*lightdef;
		unsigned short		lightmap[128*128];	
		

		for ( i = 0; i < r_lightdefnum; i++ )
		{
			for ( j = 0; j < r_lightdefs[i].lightmapnum; j++ )
			{
				if ( !r_lightdefs[i].lightmaps[j].lightpage == -1 )
					Error( "lightdef got no lightpage.\n" );
		
				if ( r_lightdefs[i].lightmaps[j].type == LightmapType_diffuse )
				{
					pair = FindHPair( r_lightdefs[i].self, "diffuse" );
					if ( !pair )
						Error( "missing 'diffuse' in lightdef '%s'.\n", r_lightdefs[i].self->name );
			
					max_lightmap_size = 128*128*2;
					HPairCastToBstring_safe( lightmap, &max_lightmap_size, pair );
					
					LightPage_InsertSubImage( &r_lightpages[r_lightdefs[i].lightmaps[j].lightpage],
								  r_lightdefs[i].lightmaps[j].xofs,
								  r_lightdefs[i].lightmaps[j].yofs,
								  r_lightdefs[i].width,
								  r_lightdefs[i].height,
								  lightmap );
				}
				else if ( r_lightdefs[i].lightmaps[j].type == LightmapType_specular )
				{
					pair = FindHPair( r_lightdefs[i].self, "specular" );
					if ( !pair )
						Error( "missing 'diffuse' in lightdef '%s'.\n", r_lightdefs[i].self->name );
					
					max_lightmap_size = 128*128*2;
					HPairCastToBstring_safe( lightmap, &max_lightmap_size, pair );
					
					LightPage_InsertSubImage( &r_lightpages[r_lightdefs[i].lightmaps[j].lightpage],
								  r_lightdefs[i].lightmaps[j].xofs,
								  r_lightdefs[i].lightmaps[j].yofs,
								  r_lightdefs[i].width,
								  r_lightdefs[i].height,
								  lightmap );
				}
				else
				{
					Error( "unkown lightmap type.\n" );
				}

			}
		}

	}

	//
	// build opengl textures
	//
	for ( i = 0; i < r_lightpagenum; i++ )
	{
		LightPage_GenTexture( &r_lightpages[i] );	
	}	
}
#endif

void LightPage_DebugDraw( int lightpage )
{
	int		i;
	fp_t		x, y;

	glDisable(GL_CULL_FACE);
	glEnable(GL_POLYGON_SMOOTH);	
	glEnable( GL_TEXTURE_2D );
	glDisable(GL_BLEND);
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_DECAL );
	glColor3f( 1.0, 1.0, 1.0 );

	glMatrixMode( GL_MODELVIEW );
       	glLoadIdentity();	

	x = y = -2.0;

	lightpage = abs( lightpage % r_lightpagenum );

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, r_lightpages[lightpage].texobj );	
	
	glBegin( GL_TRIANGLE_FAN );
	glTexCoord2f( 0.0, 0.0 );
	glVertex3f( -1.0/2.0, 1.0/2.0, 1.0/2.0 );
	
	glTexCoord2f( 1.0, 0.0 );
	glVertex3f( 1.0/2.0, 1.0/2.0, 1.0/2.0 );
	
	glTexCoord2f( 1.0, 1.0 );
	glVertex3f( 1.0/2.0, -1.0/2.0, 1.0/2.0 );
	
	glTexCoord2f( 0.0, 1.0 );
	glVertex3f( -1.0/2.0, -1.0/2.0, 1.0/2.0 );
	glEnd();
	
}
