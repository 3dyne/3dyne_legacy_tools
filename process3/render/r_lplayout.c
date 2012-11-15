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



// r_lplayout.c

#include "r_lplayout.h"

static int		boxnum;
static lp_box_t		*boxes;
static lp_box_t		*done;

/*
  ==============================
  R_LightPage_BeginRegister

  ==============================
*/
void R_LightPage_BeginRegister( void )
{
	boxnum = 0;
	boxes = NULL;
	done = NULL;
}


/*
  ==============================
  R_LightPage_EndRegister

  ==============================
*/
void R_LightPage_EndRegister( void )
{

}

/*
  ==============================
  R_LightPage_MoreBoxes

  ==============================
*/
bool_t R_LightPage_MoreBoxes( void )
{
	if ( boxes )
		return true;
	else
		return false;
}

/*
  ==============================
  R_LightPage_RegisterBox

  ==============================
*/
lp_box_t * R_LightPage_RegisterBox( int width, int height )
{
	lp_box_t		*box;

	
	box = NEW( lp_box_t );
	box->width = width;
	box->height = height;

	box->next = boxes;
	boxes = box;
	boxnum++;

	return box;
}

/*
  ==============================
  R_LightPage_GetBestBox

  ==============================
*/
lp_box_t *R_LightPage_GetBestBox( int width, int height )
{
	lp_box_t	*box, *next, *head;
	lp_box_t		*bestbox;
	int	dwmin, dhmin;
	int	dw, dh;
	bool_t		toobig;
	
	dwmin = dhmin = 0xffff;
	toobig = false;

	if ( !boxes )
		Error( "R_LightPage_GetBestBox: no more boxes\n" );

	bestbox = NULL;
	for ( box = boxes; box ; box=box->next )
	{
		dw = width - box->width;
		dh = height - box->height;

		if ( dw < 0 || dh < 0 )
		{
			toobig = true;
			continue;
		}

		if ( dw <= dwmin && dh <= dhmin )
		{
			dwmin = dw;
			dhmin = dh;
			bestbox = box;
		}
	}

	if ( !bestbox && toobig )
		return NULL;

	//
	// remove bestbox from list
	//
	head = NULL;
	for ( box = boxes; box ; box=next )
	{
		next = box->next;

		if ( box == bestbox )
		{
			// move box to 'done' list
			box->next = done;
			done = box;
			boxnum--;
			continue;
		}

		box->next = head;
		head = box;
	}
	boxes = head;

	return bestbox;
}

/*
  ==============================
  R_LightPage_Fill

  ==============================
*/
void R_LightPage_FillRecursive( int lightpage, int x, int y, int width, int height )
{
	int		w, h;
	lp_box_t	*box;

	if ( !boxes )
	{
		return;
	}

//	printf( "(%dx%d): ", width, height );
//	printf( "boxnum: %d\n", boxnum );
	box = R_LightPage_GetBestBox( width, height );
	if ( !box )
	{
		return;
	}

	w = box->width;
	h = box->height;

//	printf( "(%dx%d) ", w, h );

	box->xofs = x;
	box->yofs = y;
	box->lightpage = lightpage;

	// fill lower left
	if ( height - h > 0 )
	{
		R_LightPage_FillRecursive( lightpage, x, y + h, w, (height - h) );
	}

	// fill upper right
	if ( width - w > 0 )
	{
		R_LightPage_FillRecursive( lightpage, x + w, y, (width - w), h );
	}
	
	// fill lower right
	if ( width - w > 0 && height - h > 0 )
	{
		R_LightPage_FillRecursive( lightpage, x + w, y + h, (width - w), (height - h) );
	}
}

void R_LightPage_Fill( int lightpage, int width, int height )
{
	printf( "%d ", lightpage );
	R_LightPage_FillRecursive( lightpage, 0, 0, width, height );	     
}



/*
  ==============================
  R_LightPage_Init

  ==============================
*/
void R_LightPage_Init( void )
{
	r_lightpagenum = 0;
}

/*
  ==============================
  R_LightPage_FillRegisteredBoxes

  ==============================
*/
void R_LightPage_FillRegisteredBoxes( void )
{
	int		lastboxnum;

	for ( ;; r_lightpagenum++ )
	{
		if ( r_lightpagenum == MAX_LIGHTPAGES )
			Error( "reached MAX_LIGHTPAGES\n" );		

		if ( !boxes )
			break;
		
		lastboxnum = boxnum;
		R_LightPage_Fill( r_lightpagenum, 128, 128 );

		if ( boxnum == lastboxnum )
			Error( "R_LightPage_FillRegisteredBoxes: R_LightPage_Fill failed\n" );
	}	
}

  
