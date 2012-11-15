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



// WWM.cc

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

// #include "xinterface2.h"
// #include "x3dinterface.h"

#include "lib_unique.h"
#include "lib_token.h"

// #include "wired_parse.h"
#include "Customize.hh"
#include "Wired.hh"
#include "vec.h"
#include "WWM.hh"
#include "brush.h"
#include "archetype.h"

WWM		*wwm_i;

/*
  ===============================================
  class: WWM			       
  ===============================================
*/

WWM::WWM() 
{
	
	wwm_i = this;

	Vec3dInit( viewmin, 0, 0, 0 );
	Vec3dInit( viewmax, 0, 0, 0 );

	next_free_id = 1;

	brushnum = 0;
	brushes = NULL;

	archenum = 0;
	arches = NULL;

	//
	// create temp brush for arche selection ( 6 faces, 64x64x64 )
	//
	
	arche_brush = brushtool_i->createBrush( 64 );
	Vec3dInit( arche_pos, 0.0, 0.0, 0.0 );

	//
	// testbox stuff
	//
	testboxnum = 0;
	testboxes = NULL;

	//
	// CSurface stuff
	//
	csurfacenum = 0;
	csurfaces = NULL;

	//
	// CPoly stuff
	//
	cpolynum = 0;
	cpolys = NULL;
//	EAL_Init( &testboxes );
}

WWM::~WWM() {
	brush_t		*b, *bnext;
	arche_t		*a, *anext;

	printf("WWM::~WWM()\n");

	DumpStat();

	for ( b = brushes; b ; b=bnext ) {
		
		bnext = b->next;
		FreeBrushFaces( b );
		FreeBrush( b );
	}
	printf(" freed all brushes.\n");
	printf(" memory leaks:\n");
	// brush.c statistic
	DumpStat();

	for ( a = arches; a ; a=anext )
	{
		anext = a->next;
		AT_FreeArche( a );
	}
	printf(" freed all archetypes.\n");

	//
	// fixme: free testboxs, csurfaces, cpolys
	//
}

unsigned int WWM::getID( void )
{
	next_free_id++;
	return next_free_id-1;
}

void WWM::registerID( unsigned int id )
{
	if ( id >= next_free_id )
	{
		next_free_id = id+1;
	}
}

void WWM::saveID( const char *name )
{
	FILE	*h;

	h = fopen( name, "w" );
	if ( !h )
	{
		printf( "WARNING: WWM::saveID can't open file\n" );
		return;
	}

	fprintf( h, "%u\n", next_free_id );
	fclose( h );
}

void WWM::loadID( const char *name )
{
	tokenstream_t		*ts;

	ts = BeginTokenStream( (char*) name );
	if ( !ts )
	{
		printf( "WARNING: WWM::loadID can't open file\n" );
		next_free_id = 1;
		return;
	}

	GetToken( ts );

	next_free_id = atoi( ts->token );

	EndTokenStream( ts );
}

void WWM::validateAllIDs( void )
{
	int	face_num = 0;
	int	brush_num = 0;
	int	csurface_num = 0;
	int	cpoly_num = 0;
	int	arche_num = 0;

	int		none_style_num = 0;

	//
	// register all IDs
	//

	//
	// register brushes and faces
	//
	brush_t		*b;
	face_t		*f;

	for ( b = brushes; b ; b=b->next )
	{
		if ( b->id != UNIQUE_INVALIDE )
		{
			registerID( b->id );
		}

		for ( f = b->faces; f ; f=f->next )
		{
			if ( f->id != UNIQUE_INVALIDE )
			{
				registerID( f->id );
			}
		}
	}
	
	//
	// register cpolys
	//
	CPolyIterator		iter( cpolys );
	CPoly			*cpoly;

	for ( iter.reset(); ( cpoly = iter.getNext() ) ; )
	{
		if ( cpoly->getID() != UNIQUE_INVALIDE )
		{
			registerID( cpoly->getID() );
		}
	}

	//
	// register csurfaces
	//
	CSurfaceIterator		cs_iter( csurfaces );
	CSurface			*cs;

	for ( cs_iter.reset(); ( cs = cs_iter.getNext() ) ; )
	{
		if ( cs->getID() != UNIQUE_INVALIDE )
		{
			registerID( cs->getID() );
		}
	}
	
	//
	// register archetype names
	//
	arche_t		*a;
	kvpair_t		*p;

	for ( a = arches; a ; a=a->next )
	{
		p = AT_GetPair( a, "name" );

		if ( !p )
		{
			Error( "no key 'name' in archetype\n" );
		}

		if ( strcmp( p->value, "none" ) )
		{
			unique_t	id;

			id = StringToUnique( p->value );
			if ( id != UNIQUE_INVALIDE )
			{
				registerID( id );
			}
			else
			{
				none_style_num++;
			}
		}
	}	

	
	//
	// validate invalide IDs
	//

	//
	// validate brushes and faces
	//

	for ( b = brushes; b ; b=b->next )
	{
		if ( b->id == UNIQUE_INVALIDE )
		{
			b->id = getID();
			brush_num++;
		}

		for ( f = b->faces; f ; f=f->next )
		{
			if ( f->id == UNIQUE_INVALIDE )
			{
				f->id = getID();
				face_num++;
			}
		}
	}
	
	//
	// validate cpolys
	//
	for ( iter.reset(); ( cpoly = iter.getNext() ) ; )
	{
		if ( cpoly->getID() == UNIQUE_INVALIDE )
		{
			cpoly->setID( getID() );
			cpoly_num++;
		}
	}

	//
	// validate csurfaces
	//
	for ( cs_iter.reset(); ( cs = cs_iter.getNext() ) ; )
	{
		if ( cs->getID() == UNIQUE_INVALIDE )
		{
			cs->setID( getID() );
			csurface_num++;
		}
	}
	
	//
	// validate archetype names
	//
	for ( a = arches; a ; a=a->next )
	{
		p = AT_GetPair( a, "name" );

		if ( !p )
		{
			Error( "no key 'name' in archetype\n" );
		}
		
		if ( !strcmp( p->value, "none" ) )
		{
			sprintf( p->value, "#%u", getID() );			
			arche_num++;
		}
	}	

	printf( "WWM::validateAllIDs\n" );
	printf( " %d brushes\n", brush_num );
	printf( " %d faces\n", face_num );
	printf( " %d csurfaces\n", csurface_num );
	printf( " %d cpolys\n", cpoly_num );
	printf( " %d archetypes\n", arche_num );

	printf( " %d archetypes with none-unique-id-style names\n", none_style_num );
}

void WWM::checkConsistency( void )
{
	printf( "WWM::checkConsistency\n" );

	int	num;

	CSurface	*cs;
	for ( num = 0, cs = csurfaces; cs ; cs=cs->getNext(), num++ )
	{
	}
	if ( num != csurfacenum )
	{
		printf( "csurface num %d\n", num );
		printf( " expected %d !\n", csurfacenum );
		getchar();
	}

	CPoly		*cpoly;
	for ( num = 0, cpoly = cpolys; cpoly; cpoly=cpoly->getNext(), num++ )
	{
	}
	if ( num != cpolynum )
	{
		printf( "cpoly num %d\n", num );
		printf( " expected %d !\n", cpolynum );
		getchar();
	}
}

void WWM::addBrush( brush_t *brush, bool_t add_selected )
{
	brushnum++;
	brush->next = brushes;
	brushes = brush;

	if ( add_selected )
	{
		deselectBrushes();
		brush->select = SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE | SELECT_BLUE;
	}
	else
	{
		brush->select = SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE;
	}
//	brush->status = BS_NORMAL;
}

void WWM::removeBrush( brush_t *brush )
{
	// re-link list without the brush to be removed

	brush_t		*b, *bnext;
	brush_t		*head;

	head = NULL;

	for ( b = brushes; b ; b=bnext )
	{
		bnext = b->next;

		if ( b == brush )
		{		
			b->next = NULL;
			continue;
		}

		b->next = head;
		head = b;
	}

	brushes = head;
}

#if 0
void WWM::reorderBrushes( void )
{
	//
	// move all selected brushes to the head for more performence
	//
	brush_t		*b, *bnext;

	brush_t		*head1, *last1; // all selections
	brush_t		*head2, *last2; // all normals

	brush_t		*head, *last;	// the new list

	head1 = last1 = NULL;
	head2 = last2 = NULL;

	for ( b = brushes; b ; b=bnext )
	{
		bnext=b->next;

		if ( abs(b->status) > BS_NORMAL )
		{
			if ( !last1 )
				last1 = b;
			
			b->next = head1;
			head1 = b;
		}
		else
		{
			if ( !last2 )
				last2 = b;

			b->next = head2;
			head2 = b;
		}
	}		

	head = NULL;
	last = NULL;

	// new order: head->selected->normal

	if ( head2 )
	{
		head = head2;
	}

	if ( head1 && last1 )
	{
		if ( head )
			last1->next = head;
		
		head = head1;
	}

	brushes = head;
}
#endif

brush_t* WWM::getFirstBrush( void ) 
{
	return brushes;
}

int WWM::getBrushNum( void )
{
	return brushnum;
}


// ??? remove this
void WWM::headBrush( brush_t *brush )
{
	// move the brush to the head of the list

	brush_t		*b;

	if ( brush == brushes )
		// brush is allready head
		return;

	for ( b = brushes; b ; b=b->next )
	{
		if ( b->next == brush ) {
			b->next = brush->next;
			break;
		}
	}

	if ( !b ) 
	{
		printf("WWM::headBrush: warning. brush to head not found in the list.\n");
		return;
	}

	// new list head
	brush->next = brushes;
	brushes = brush;
}

void WWM::allUpdateFlagsTrue( void )
{
	brush_t		*b;
	arche_t		*a;
	
	for ( b = brushes; b ; b=b->next )
	{
		b->select|=SELECT_UPDATE;
	}

	for ( a = arches; a ; a=a->next )
		a->select|=SELECT_UPDATE;

	//
	// new EditAble stuff
	//
	TestBoxIterator		iter( testboxes );
	EAL_AllUpdateFlagsTrue( &iter );

	//
	// CSurface stuff
	//
	CSurfaceIterator	csiter( csurfaces );
	EAL_AllUpdateFlagsTrue( &csiter );

	//
	// CPoly stuff
	//
	CPolyIterator		cpi( cpolys );
	EAL_AllUpdateFlagsTrue( &cpi );
}

void WWM::viewBoundsChanged( vec3d_t vmin, vec3d_t vmax )
{
	int		j;
	brush_t		*b;

	arche_t		*a;
	kvpair_t		*pair;
	vec3d_t		v;

	int		allbrushes = 0;
	int		visbrushes = 0;
	int		allarches = 0;
	int		visarches = 0;

	Vec3dCopy( viewmin, vmin );
	Vec3dCopy( viewmax, vmax );

	for ( b = brushes; b ; b=b->next ) {
		b->select|=SELECT_VISIBLE;
		b->select&=~SELECT_PRE;
		allbrushes++;
		for ( j = 0; j < 3; j++ ) {
			if ( b->max[j] < vmin[j] || b->min[j] > vmax[j] ) {
				b->select&=~SELECT_VISIBLE;
				break;
			}
		}
		if ( j==3 ) {
			visbrushes++;
			b->select|=SELECT_UPDATE;
		}
	}	

	for ( a = arches; a ; a=a->next )
	{
		a->select|=SELECT_VISIBLE;
		a->select&=~SELECT_PRE;
		allarches++;

		//
		// get origin of archetype
		//
		pair = AT_GetPair( a, "origin" );
		if ( !pair )
		{
			printf( "WWM::viewBoundsChanged: can't find key \"origin\"\n" );
			continue;
		}

		AT_CastValueToVec3d( v, pair->value );

		for ( j = 0; j < 3; j++ )
		{
			if ( v[j] < vmin[j] || v[j] > vmax[j] ) 
			{
				a->visible&=~SELECT_VISIBLE;
				break;
			}
		}
		
		if ( j == 3 )
		{
			visarches++;
		        a->select|=SELECT_UPDATE;
		}
	}

	printf("WWM::viewBoundsChanged\n");
	printf(" allbrushes = %d\n", allbrushes );
	printf(" visbrushes = %d\n", visbrushes );
	printf(" allarches = %d\n", allarches );
	printf(" visarches = %d\n", visarches );

	//
	// new EditAble stuff
	//
	TestBoxIterator		iter( testboxes );
	EAL_CheckViewBounds( &iter, viewmin, viewmax );

	//
	// CSurface stuff
	//
	CSurfaceIterator	csiter( csurfaces );
	EAL_CheckViewBounds( &csiter, viewmin, viewmax );

	//
	// CPoly stuff
	//
	CPolyIterator		cpi( cpolys );
	EAL_CheckViewBounds( &cpi, viewmin, viewmax );

	this->dump();
}


void WWM::findBestBrushForRay( vec3d_t start, vec3d_t dir, brush_t **brushptr, face_t **faceptr, bool_t in_selection )
{
	vec3d_t		ray;
	vec3d_t		s0, s1;

	brush_t		*b;
	brush_t		*bestbrush;
	face_t		*hit;
	face_t		*bestface;

	float		l, bestl; 

	vec3d_t		from, to;

	Vec3dCopy( from,start );
	Vec3dScale( to, 16000, dir );
	Vec3dAdd( to, start, to );

	bestbrush = NULL;
	bestface = NULL;
	bestl = 999999;

	for ( b = brushes; b ; b=b->next ) {

		if ( in_selection && !(b->select&SELECT_BLUE) )
			continue;
		
		if ( !(b->select&SELECT_VISIBLE) )
			continue;
		
		Vec3dCopy( s0, from );
		Vec3dCopy( s1, to );
		
		hit = ClipRayByBrush( b, s0, s1 );
		
		if ( hit ) {

			Vec3dSub( ray, s0, from );
			l = Vec3dLen( ray );
			printf(" l = %f\n", l );
			if ( !bestface ) {
				bestface = hit;
				bestbrush = b;
				bestl = l;
			}
			else {
				// is the last hitface better than the bestface ???
//				if ( Vec3dDotProduct( from, hit->plane.norm ) - hit->plane.dist <
//				     Vec3dDotProduct( from, bestface->plane.norm ) - bestface->plane.dist )

				if ( l < bestl ) {
					bestface = hit;
					bestbrush = b;
					bestl = l;
				}
			}
		}		
	}

	printf( "best brush: %p\n", bestbrush );
	printf( "best face: %p\n", bestface );
	
	*brushptr = bestbrush;
	*faceptr = bestface;
}


bool_t WWM::intersectBrushAndRay( brush_t *b, vec3d_t start, vec3d_t dir )
{
	face_t*		f;
	vec3d_t		from;
	vec3d_t		to;

	Vec3dCopy( from, start );
	Vec3dScale( to, 16000, dir );
	Vec3dAdd( to, start, to );

	f = ClipRayByBrush( b, from, to );

	if ( f )
		return true;

	return false;
}

void WWM::deselectBrushes( void )
{
	brush_t		*b;
	
	for ( b = brushes; b ; b=b->next )
	{
		if ( b->select&SELECT_BLUE )
		{
			b->select&=~SELECT_BLUE;
			b->select&=~SELECT_PRE;
			b->select|=SELECT_UPDATE;
		}
	}
}

void WWM::selectByRay( vec3d_t start, vec3d_t dir )
{
	brush_t		*b;
	brush_t		*blue;
	brush_t		*blue_new;
	bool_t		hit;
	bool_t		pre_new;

	//
	// pre-select brushes
	//
	pre_new = false;
	for ( b = brushes; b ; b=b->next )
	{
		if ( !(b->select&SELECT_VISIBLE) )
		{
			b->select&=~SELECT_PRE;
			continue;
		}

		hit = intersectBrushAndRay( b, start, dir );

		if ( hit )
			printf( "hit: %p\n", b );

		if ( hit && !(b->select&SELECT_PRE ) )
		{
			// hit but not pre-selected => new set
			pre_new = true;
//			printf( "hit brush: %p !\n", b );
			b->select|=SELECT_PRE;
			continue;
		}

		if ( !hit && (b->select&SELECT_PRE ) )
		{
			// not hit but pre-selected => new set
			pre_new = true;
			b->select&=~SELECT_PRE;
			continue;
		}
	}

	printf( "pre_new: %d\n", pre_new );

	//
	// search old blue and unblue it
	//
	blue = NULL;
	for ( b = brushes; b ; b=b->next )
	{
		if ( b->select&SELECT_BLUE )
		{
			blue = b;
			b->select&=~SELECT_BLUE;
			b->select|=SELECT_UPDATE;
			break;
		}
	}
	printf( "old blue: %p\n", blue );

	blue_new = NULL;
	//
	// if blue found and the pre-selection not changed, search next pre 
	//
	if ( blue && !pre_new )
	{
		for ( b = blue->next; b ; b=b->next )
		{
			if ( b->select&SELECT_PRE )
			{
				b->select|=SELECT_BLUE;
				b->select|=SELECT_UPDATE;
				blue_new = b;
				break;
			}
		}		
	}

	//
	// if not found or pre-selection changed, begin at start
	//
	if ( !blue_new || pre_new )
	{
		for ( b = brushes; b ; b=b->next )
		{
			if ( b->select&SELECT_PRE )
			{
				b->select|=SELECT_BLUE;
				b->select|=SELECT_UPDATE;
				blue_new = b;
				break;
			}
		}
	}
	

	printf( "new_blue: %p\n", blue_new );
//	printf( "pre_new: %d\n", pre_new );


}

//
// archetypes ...
//

arche_t * WWM::searchArche( char *name )
{
	arche_t		*a;
	kvpair_t	*p;

	for ( a = arches; a ; a=a->next )
	{
		p = AT_GetPair( a, "name" );
		if ( !p )
		{
			continue;
		}

		if ( !strcmp( name, p->value ) )
		{
			return a;
		}
	}
	return NULL;
}

void WWM::addArche( arche_t *arche, bool_t add_selected )
{
	kvpair_t		*pair;
	vec3d_t		v;

	archenum++;
	arche->next = arches;
	arches = arche;
//	arche->visible = 1;
 
	pair = AT_GetPair( arche, "origin" );
	if ( !pair )
	{
		printf( "WWM::addArche: can't find key \"origin\"\n" );
		return;
	}	
	AT_CastValueToVec3d( v, pair->value );

	if ( add_selected )
	{
		deselectArches();
		arche->select = SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE | SELECT_BLUE;
	}
	else
	{
		arche->select = SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE;
	}

#if 0
	arche->visible = 1;
	for ( j = 0; j < 3; j++ ) 
	{
		if ( v[j] < viewmin[j] || v[j] > viewmax[j] ) 
		{
			arche->visible = 0;
			break;
		}
	}
#endif
}	

void WWM::removeArche( arche_t *arche )
{
	// re-link list without the arche to be removed

	arche_t		*a, *anext;
	arche_t		*head;

	head = NULL;

	for ( a = arches; a ; a=anext )
	{
		anext = a->next;

		if ( a == arche )
		{		
			a->next = NULL;
			continue;
		}

		a->next = head;
		head = a;
	}

	arches = head;

}

arche_t* WWM::getFirstArche( void )
{
	return arches;
}


void WWM::dump()
{
	printf("WWM::Dump\n");
	printf(" brushnum = %d\n", brushnum );
//	printf(" spmax = %d\n", spmax );
	printf(" archenum = %d\n", archenum );
	printf(" testboxnum = %d\n", testboxnum );
}

bool_t WWM::intersectArcheAndRay( arche_t *a, vec3d_t start, vec3d_t dir )
{
	kvpair_t	*pair;
	vec3d_t		v;
	vec3d_t		delta;
	face_t		*f;
	vec3d_t		from;
	vec3d_t		to;

	Vec3dCopy( from, start );
	Vec3dScale( to, 16000, dir );
	Vec3dAdd( to, start, to );

	//
	// get origin of arche
	//
	pair = AT_GetPair( a, "origin" );
	if ( !pair )
	{
		printf( "WWM::intersectArcheAndRay: can't find key \"origin\"\n" );
		return false;
	}	
	AT_CastValueToVec3d( v, pair->value );
	
	//
	// set planes of temp brush for clip test, awful fake
	//
	Vec3dSub( delta, v, arche_pos );
	Vec3dCopy( arche_pos, v );
	brushtool_i->moveBrush( arche_brush, delta );

	f = ClipRayByBrush( arche_brush, from, to );

	if ( f )
		return true;

	return false;	
}

void WWM::deselectArches( void )
{
	arche_t		*a;
	
	for ( a = arches; a ; a=a->next )
	{
		if ( a->select&SELECT_BLUE )
		{
			a->select&=~SELECT_BLUE;
			a->select|=SELECT_UPDATE;
		}
	}
}


void WWM::selectArcheByRay( vec3d_t start, vec3d_t dir )
{
	arche_t		*a;
	arche_t		*blue;
	arche_t		*blue_new;
	bool_t		hit;
	bool_t		pre_new;

	pre_new = false;
	for ( a = arches; a ; a=a->next )
	{
		if ( !(a->select&SELECT_VISIBLE) )
			continue;
		
		hit = intersectArcheAndRay( a, start, dir );

		if ( hit )
			printf( "arche: %p hit.\n", a );

		if ( hit && !(a->select&SELECT_PRE ) )
		{
			// hit but not pre-selected => new set
			pre_new = true;
//			printf( "hit brush: %p !\n", b );
			a->select|=SELECT_PRE;
			continue;
		}

		if ( !hit && (a->select&SELECT_PRE ) )
		{
			// not hit but pre-selected => new set
			pre_new = true;
			a->select&=~SELECT_PRE;
			continue;
		}
	}

	printf( "pre_new: %d\n", pre_new );

	//
	// search old blue and unblue it
	//
	blue = NULL;
	for ( a = arches; a ; a=a->next )
	{
		if ( a->select&SELECT_BLUE )
		{
			blue = a;
			a->select&=~SELECT_BLUE;
			a->select|=SELECT_UPDATE;
			break;
		}
	}
	printf( "old blue: %p\n", blue );
	
	blue_new = NULL;
	//
	// if blue found and the pre-selection not changed, search next pre 
	//
	if ( blue && !pre_new )
	{
		for ( a = blue->next; a ; a=a->next )
		{
			if ( a->select&SELECT_PRE )
			{
				a->select|=SELECT_BLUE;
				a->select|=SELECT_UPDATE;
				blue_new = a;
				break;
			}
		}		
	}
	

	//
	// if not found or pre-selection changed, begin at start
	//
	if ( !blue_new || pre_new )
	{
		for ( a = arches; a ; a=a->next )
		{
			if ( a->select&SELECT_PRE )
			{
				a->select|=SELECT_BLUE;
				a->select|=SELECT_UPDATE;
				blue_new = a;
				break;
			}
		}
	}
	

	printf( "new_blue: %p\n", blue_new );
//	printf( "pre_new: %d\n", pre_new );
	
}


#include "lib_hobj.h"

void Test_WriteHObj( const char * name )
{
	char		text[256];
	hobj_t		*root;       
	brush_t		*b;
	face_t		*f;

	root = NewClass( "sbrushes", "sbrush0" );

	for( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		hobj_t		*bobj;
		sprintf( text, "#%d", b->id );
		bobj = NewClass( "brush", text );

		hpair_t		*p;
		// contents
		p = NewHPair();
		strcpy( p->type, "int" );
		strcpy( p->key, "content" );
		sprintf( p->value, "%u", b->contents );
		InsertHPair( bobj, p );

		// faces
		for ( f = b->faces; f ; f=f->next )
		{
			hobj_t	*fobj;
			sprintf( text, "#%d", f->id );
 
			fobj = NewClass( "face", text );

			// norm
			p = NewHPair();
			strcpy( p->type, "vec3d" );
			strcpy( p->key, "norm" );
			sprintf( p->value, "%f %f %f", f->plane.norm[0], f->plane.norm[1], f->plane.norm[2] ); 
			InsertHPair( fobj, p );

			// dist
			p = NewHPair();
			strcpy( p->type, "float" );
			strcpy( p->key, "dist" );
			sprintf( p->value, "%f", f->plane.dist );
			InsertHPair( fobj, p );

			// contents
			p = NewHPair();
			strcpy( p->type, "int" );
			strcpy( p->key, "content" );
			sprintf( p->value, "%u", f->contents );
			InsertHPair( fobj, p );

			//
			// texdef
			//
			// strig ident
			p = NewHPair();
			strcpy( p->type, "string" );
			strcpy( p->key, "ident" );
			sprintf( p->value, "%s", f->texdef.ident );
			InsertHPair( fobj, p );

			// fp_t rotate
			p = NewHPair();
			strcpy( p->type, "float" );
			strcpy( p->key, "rotate" );
			sprintf( p->value, "%f", f->texdef.rotate );
			InsertHPair( fobj, p );

			// vec2d_t shift
			p = NewHPair();
			strcpy( p->type, "vec2d" );
			strcpy( p->key, "shift" );
			sprintf( p->value, "%f %f", f->texdef.shift[0], f->texdef.shift[1] );
			InsertHPair( fobj, p );

			// vec2d_t scale
			p = NewHPair();
			strcpy( p->type, "vec2d" );
			strcpy( p->key, "scale" );
			sprintf( p->value, "%f %f", f->texdef.scale[0], f->texdef.scale[1] );
			InsertHPair( fobj, p );

			InsertClass( bobj, fobj );
		}
		InsertClass( root, bobj );
	}

	DeepDumpClass( root );
	       

	FILE	*h;
	sprintf( text, "%s.hobj", name );
	h = fopen( text, "w" );
	WriteClass( root, h );
	fclose ( h );

	{
		// hack for deep destroy class
		hmanager_t *hm;
		hm = NewHManager();
		HManagerSetRootClass( hm, root );
		HManagerRebuildHash( hm ); 
		HManagerDeepDestroyClass( hm, root );
		FreeHManager( hm );
	}
}

#if 1
void WWM::loadWire( const char *name )
{
	vec3d_t		norm;
	float		dist;

	face_t		*fnew;
	face_t		*facehead;

	brush_t		*bnew;
	unique_t	id;
	unique_t	fid;
	unsigned int	bcontents, scontents;
	tokenstream_t	*ts;


	printf("WWM::loadWire\n");
	printf(" loading %s ...\n", name );

	ts = BeginTokenStream( (char*) name );
	
	if ( !ts )
	{
		printf( " ... can't open file.\n" );
		return;
	}

//	InitUniqueNumber();

	// check Wire version
	GetToken( ts );	
	if ( strcmp( ts->token, "WireType" ) )
		goto parse_error;
	
	GetToken( ts );
	if ( atoi( ts->token ) != 2 )
		goto parse_error;
	     
	// expect '{' of world
	GetToken( ts );
	if ( ts->token[0] != '{' )
		goto parse_error;
	
	printf(" start wwm\n");
	// brush loop
	for(;;) { 

		GetToken( ts );
		if ( ts->token[0] == '}' ) // wwm end
			break;
		if ( ts->token[0] == '{' ) { // brush start
//			printf("\n  start brush: ");
			// brush loop
			facehead = NULL;
			id = UNIQUE_INVALIDE;

			//
			// unique id if given
			//

			GetToken( ts );
			if ( ts->token[0] =='*' )
			{
				id = (unique_t) atoi( &ts->token[1] );
//				TestUniqueNumber( id );
				registerID( id );
			}
			else
			{
				id = UNIQUE_INVALIDE;
				KeepToken( ts );
			}

			//
			// brush contents if given
			//
			
			GetToken( ts );
			bcontents = 0;
			if ( ts->token[0] == 'b' )
			{
				bcontents = atoi( &ts->token[1] );
			}
			else
			{
				KeepToken( ts );
			}
			if ( bcontents == 0 )
			{
				bcontents = DEFAULT_BRUSH_CONTENTS;
			}
			
			
			for(;;) {
				GetToken( ts );

				if ( ts->token[0] == '}' ) // brush end
					break;
				
				if ( ts->token[0] == '{' ) { // face start
				
					fnew = NewFace();

//					printf("face ");  
					GetToken( ts ); // '('
					if ( ts->token[0] != '(' )
						goto parse_error;

					GetToken( ts ); // 'float'
					norm[0] = atof( ts->token );
					GetToken( ts ); // 'float'
					norm[1] = atof( ts->token );
					GetToken( ts ); // 'float'
					norm[2] = atof( ts->token );
					GetToken( ts ); // ')'
					if ( ts->token[0] != ')' )
						goto parse_error;
					GetToken( ts ); // 'float'
					dist = atof( ts->token );

					GetToken( ts ); // string 'ident'
					if ( strlen( ts->token ) > 31 || !isalpha(ts->token[0]) )
						goto parse_error;
					
					strcpy( fnew->texdef.ident, ts->token );

					GetToken( ts ); // float 'rotate'
					fnew->texdef.rotate = atof( ts->token );

					GetToken( ts ); // float 'scale[0]'
					fnew->texdef.scale[0] = atof( ts->token );

					GetToken( ts ); // float 'scale[1]'
					fnew->texdef.scale[1] = atof( ts->token );

					GetToken( ts ); // float 'shift[0]'
					fnew->texdef.shift[0] = atof( ts->token );

					GetToken( ts ); // float 'shift[1]'
					fnew->texdef.shift[1] = atof( ts->token );

					//
					// get surface contents if given
					//
					
					GetToken( ts );
					scontents = 0;
					if ( ts->token[0] == 's' )
					{
						scontents = atoi( &ts->token[1] );
						// miss-customized default surface hack:
						// open+texture for default was wrong ...
						if ( bcontents == BRUSH_CONTENTS_SOLID &&
						     scontents == (SURFACE_CONTENTS_OPEN|SURFACE_CONTENTS_TEXTURE ))
							scontents = ( SURFACE_CONTENTS_CLOSE|SURFACE_CONTENTS_TEXTURE );
					}
					else
					{
						KeepToken( ts );
					}
					if ( scontents == 0 )
					{
						scontents = DEFAULT_SURFACE_CONTENTS;
					}

					
					//
					// unique id if given
					//

					GetToken( ts );
					if ( ts->token[0] =='*' )
					{
						fid = (unique_t) atoi( &ts->token[1] );
						registerID( fid );
					}
					else
					{
						fid = UNIQUE_INVALIDE;
						KeepToken( ts );
					}
					
					
					GetToken( ts ); // '}'
					if ( ts->token[0] != '}' )
						goto parse_error;
										
					Vec3dCopy( fnew->plane.norm, norm );
					fnew->plane.dist = dist;
					fnew->id = fid;
					fnew->contents = scontents;
					fnew->next = facehead;
					facehead = fnew;
					continue;
				}
			}
			
			bnew = NewBrush();
			bnew->id = id;
			bnew->contents = bcontents;
			bnew->faces = facehead;
			bnew->select = SELECT_NORMAL;
//			ClipBrushFaces( bnew );
			CleanUpBrush( bnew );
			if ( !bnew->faces )
			{
				printf( " ignore degenerated brush.\n" );
				FreeBrush( bnew );
			}
			else
			{
				addBrush( bnew );
			}
			continue;
		}

		goto parse_error;

	}

	EndTokenStream( ts );
	
	return;

parse_error:
	printf(" parse error.\n");
	exit(0);
	
}
#endif 


void WWM::saveWire( const char* name )
{
//	printf(" not implemented.\n");
//	exit(-1);
//#if 0
//	int		point;
	int		i;

	brush_t		*b;
	face_t		*f;

	FILE	*handle;

	printf("WWM::saveWire\n");
	printf(" saving %s ...\n", name );

	// get unique id for all brushes with UNIQUE_INVALIDE
	for ( b = brushes; b; b=b->next )
		if ( b->id == UNIQUE_INVALIDE )
			b->id = GetUniqueNumber();
	

	handle = fopen( name, "wb" );
	if ( handle == NULL ) {
		printf("can't open file.\n");
		goto save_error;
	} 

	// write Wire version
	fprintf( handle, "WireType 2\n" );

	// write WWM start {
	fprintf( handle, "{\n" );

	for ( b = brushes; b ; b=b->next ) {
	
		// write Brush start {
		fprintf( handle, " {\n" );

		// write unique id
		fprintf( handle, " *%u\n", b->id );

		// write brush contents
		fprintf( handle, " b%d\n", b->contents );
		
		for ( f = b->faces; f ; f=f->next ) {

			fprintf( handle, "\t{ ( " );
			for ( i = 0; i < 3; i++ ) {
				if ( f->plane.norm[i] == (float)((int)(f->plane.norm[i] ) ) )
					fprintf( handle, "%d ", (int)(f->plane.norm[i]) );
				else
					fprintf( handle, "%f ", f->plane.norm[i] );	
			}
			fprintf( handle, ") %f ", f->plane.dist );

			// write ident
			fprintf( handle, "%s ", f->texdef.ident );

			// write rotate
			if ( f->texdef.rotate == (float)((int)(f->texdef.rotate ) ) )
				fprintf( handle, "%d ", (int)(f->texdef.rotate) );
			else
				fprintf( handle, "%f ", f->texdef.rotate );

			// write scale
			for ( i = 0; i < 2; i++ ) {
				if ( f->texdef.scale[i] == (float)((int)(f->texdef.scale[i] ) ) )
					fprintf( handle, "%d ", (int)(f->texdef.scale[i]) );
				else
					fprintf( handle, "%f ", f->texdef.scale[i] );
			}
			
			// write shift
			for ( i = 0; i < 2; i++ ) {
				if ( f->texdef.shift[i] == (float)((int)(f->texdef.shift[i] ) ) )
					fprintf( handle, "%d ", (int)(f->texdef.shift[i]) );
				else
					fprintf( handle, "%f ", f->texdef.shift[i] );
			}
			

			// write face contents
			fprintf( handle, "s%d ", f->contents );

			// write face id
			fprintf( handle, "*%u ", f->id );

			fprintf( handle, " }\n");
			
		}
		fprintf( handle, " }\n" );

	}
	fprintf( handle, "}\n" );

	fclose( handle );

	Test_WriteHObj( name );

save_error:
	return;
//#endif
}


void WWM::saveArcheClass( const char* file ) 
{
	hobj_t		*root;
	hobj_t		*obj;

	arche_t		*a;
	FILE		*h;

	root = NewClass( "archetypes", "archetypes0" );
	
	for ( a = arches; a ; a=a->next )
	{

		obj = AT_BuildClassFromArche( a );
		InsertClass( root, obj );

	}
	
	h = fopen( file, "w" );
	if ( !h )
	{
		printf( "can't open file.\n" );
	}
	else
	{
		WriteClass( root, h );
		fclose( h );
	}

	{
		// hack for deep destroy class
		hmanager_t *hm;
		hm = NewHManager();
		HManagerSetRootClass( hm, root );
		HManagerRebuildHash( hm ); 
		HManagerDeepDestroyClass( hm, root );
		FreeHManager( hm );
	}	
}

/*
  ==============================
  loadArcheClass

  ==============================
*/
void WWM::loadArcheClass( const char* name ) 
{
	tokenstream_t		*ts;
	hobj_t			*root;

	hobj_search_iterator_t	iter;
	hobj_t			*arche;
	
	arche_t		*a;

	ts = BeginTokenStream( (char*) name );
	if ( !ts )
	{
		printf( "WARNING: WWM::loadArcheClass can't open file\n" );
		return;
	}

	root = ReadClass( ts );
	EndTokenStream( ts );

	
	InitClassSearchIterator( &iter, root, "*" );
//	arches = NULL;
//	archenum = 0;
	for ( ; ( arche = SearchGetNextClass( &iter ) ) ; )
	{
		a = AT_NewArche();
		AT_InitArcheFromClass( a, arche );
	
		a->status = OS_NORMAL;
		addArche( a );       
	}

	{
		hmanager_t *hm;
		hm = NewHManager();
		HManagerSetRootClass( hm, root );
		HManagerRebuildHash( hm ); 
		HManagerDeepDestroyClass( hm, root );
		FreeHManager( hm );		
	}
}

/*
  ==============================
  loadArcheFromOldAts

  ==============================
*/

/*
  ====================
  ReadArche

  ====================
*/

static arche_t* ReadArchetype( tokenstream_t *ts ) {
	
	kvpair_t		*pair;

	char		type[256];
	char		key[256];
	char		value[256];
		
	GetToken(ts);
	
	if ( ts->token[0] != '{' ) {
		Error( "ReadArchetype: expect '{' for begin of archetype.\n" );
	}
	
	arche_t *arche = AT_NewArche();
	
	for (;;) {
		
		GetToken( ts );
		if ( ts->token[0] == '}' ) {
			break;
		}
		else if ( ts->token[0] != '{' ) {
			Error( "ReadArchetype: expect '{' for begin of pair.\n" );
		}
		
		// get type
		GetToken( ts );
		strcpy( type, ts->token );

		// get key
		GetToken( ts );
		strcpy( key, ts->token );

		// get value
		GetToken( ts );
		strcpy( value, ts->token );
		
		// expect '}'
		GetToken( ts );
		if ( ts->token[0] != '}' )
			Error( "ReadArchetype: expect '}'\n" );

		pair = AT_NewPair( type, key, value );
		
		// link list
		pair->next = arche->pairs;
		arche->pairs = pair;
	}
	
	return arche;
}

static arche_t* ReadArchetypes( tokenstream_t *ts )
{
	arche_t		*arche;

	arche = 0;
	

	GetToken( ts );
	if ( strcmp( ts->token, "ArcheType" ) ) {
		Error( "ReadArchetypes: expect 'ArcheType'\n" );
	}

	GetToken( ts );
	if ( strcmp( ts->token, "1" ) ) {
		Error( "ReadArchetypes: expect '1'\n" );
	}
	
	GetToken( ts );
	if ( ts->token[0] != '{' ) {
		Error( "ReadArchetypes: expect '{'\n" );
	}
	
	
	for (;;)
	{
		GetToken( ts );
		if ( ts->token[0] == '}' ) {
			break;		
		}
		else if ( ts->token[0] == '{' ) {
			KeepToken( ts );
			arche_t *a = ReadArchetype( ts );
			a->next = arche;
			arche = a;
		}
		else {
			Error( "ReadArchetypes: expect '{'\n" );
		}		
	}

	return arche;
}

void WWM::loadArcheFromOldAts( const char* filename ) 
{
	tokenstream_t		*ts;
	
	arche_t *head;
	arche_t		*arche;

	ts = BeginTokenStream( filename );
	if ( !ts )
	{
		printf( "WARNING: WWM::loadArcheFromOldAts can't open file\n" );
		return;
	}

	head = ReadArchetypes( ts );
	
	EndTokenStream( ts );


	int count = 0;
	char name[256];
	for ( arche = head; arche; arche=arche->next ) {
		unique_t id = getID();
		sprintf( name, "#%d", id );
		AT_SetPair( arche, "STRING", "name", name );
		count++;
	}
	
	printf("loadArcheFromOldAts: count=%d\n", count );
	
	arche_t *next;
 	for ( arche = head; arche; arche = next )
 	{
		next = arche->next;
		arche->next = 0;
		 	
 		arche->status = OS_NORMAL;
 		addArche( arche );       
 	}
}




/*
  ==================================================
  new EditAble stuff

  ==================================================
*/

/*
  ========================================
  TestBox stuff

  ========================================
*/

/*
  ====================
  insertTestBox

  ====================
*/
void WWM::insertTestBox( TestBox *obj )
{
	printf( "WWM::insertTestBox\n" );
	testboxnum++;
//	EAL_Insert( &testboxes, obj );
	
	obj->setNext( testboxes );
	testboxes = obj;

	obj->setSelect( SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE );
}



/*
  ====================
  removeTestBox

  ====================
*/
void WWM::removeTestBox( TestBox *obj )
{
	TestBox		*head;
	TestBox		*next, *b;

	printf( "WWM::removeTestBox\n" );
	testboxnum--;
//	EAL_Remove( &testboxes, obj );

	head = NULL;
	for( b = testboxes; b ; b=next )
	{
		next = b->getNext();

		if ( obj == b )
		{
			obj->setNext( NULL );
			continue;
		}
		
		b->setNext( head );
		head = b;
	}
	testboxes = head;
}



/*
  ====================
  getFirstTestBox

  ====================
*/
TestBox* WWM::getFirstTestBox( void )
{
	return testboxes;
}



/*
  ========================================
  CSurface stuff

  ========================================
*/

/*
  ====================
  insertCSurface

  ====================
*/
void WWM::insertCSurface( CSurface *obj, bool add_selected )
{
	printf( "WWM::insertCSurface\n" );
	csurfacenum++;

	obj->setNext( csurfaces );
	csurfaces = obj;

	if ( add_selected )
	{
		CSurfaceIterator	iter(csurfaces);

		EAL_DeselectBlue( &iter );
		obj->setSelect( SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE | SELECT_BLUE );
	}
	else
	{
		obj->setSelect( SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE );
	}
}



/*
  ====================
  removeCSurface
  
  ====================
*/
void WWM::removeCSurface( CSurface *obj )
{
	CSurface		*head;
	CSurface		*next, *b;

	printf( "WWM::removeCSurface\n" );


	head = NULL;
	for( b = csurfaces; b ; b=next )
	{
		next = b->getNext();

		if ( obj == b )
		{
			obj->setNext( NULL );
			csurfacenum--;
			continue;
		}
		
		b->setNext( head );
		head = b;
	}
	csurfaces = head;	
}



/*
  ====================
  getFirstCSurface

  ====================
*/
CSurface* WWM::getFirstCSurface( void )
{
	return csurfaces;
}



/*
  ====================
  saveCSurfacesClass

  ====================
*/
void WWM::saveCSurfaceClass( const char *name )
{      
	hobj_t		*root;
	FILE		*h;
	CSurfaceIterator		iter( csurfaces );
	CSurface			*cs;

	root = NewClass( "csurfaces", "csurface0" );

	for ( iter.reset(); ( cs = iter.getNext() ); )
	{
		hobj_t		*csurf;

		csurf = cs->buildClass();
		InsertClass( root, csurf );
	}

	h = fopen( name, "w" );
	if ( !h )
	{
		printf( "can't open file.\n" );
	}
	else
	{
		WriteClass( root, h );
		fclose( h );
	}

	{
		hmanager_t *hm;
		hm = NewHManager();
		HManagerSetRootClass( hm, root );
		HManagerRebuildHash( hm ); 
		HManagerDeepDestroyClass( hm, root );
		FreeHManager( hm );		
	}
}

/*
  ==============================
  loadCSurfaceClass

  ==============================
*/
void WWM::loadCSurfaceClass( const char *name )
{
	tokenstream_t		*ts;
	hobj_t			*root;

	hobj_search_iterator_t	iter;
	hobj_t			*csurface;

	ts = BeginTokenStream( (char*) name );
	if ( !ts )
	{
		printf( "WARNING: WWM::loadCSurfacesClass can't open file\n" );
		return;
	}

	root = ReadClass( ts );
	EndTokenStream( ts );

	InitClassSearchIterator( &iter, root, "csurface" );
	csurfaces = NULL;
	csurfacenum = 0;
	for ( ; ( csurface = SearchGetNextClass( &iter ) ) ; )
	{
		CSurface	*cs;

		cs = new CSurface( 0, 0 );
		cs->initFromClass( csurface );

		cs->setNext( csurfaces );
		csurfaces = cs;		
		csurfacenum++;
	}

	{
		hmanager_t *hm;
		hm = NewHManager();
		HManagerSetRootClass( hm, root );
		HManagerRebuildHash( hm ); 
		HManagerDeepDestroyClass( hm, root );
		FreeHManager( hm );		
	}
}



/*
  ==================================================
  CPoly stuff

  ==================================================
*/

/*
  ====================
  insertCPoly

  ====================
*/
void WWM::insertCPoly( CPoly *obj, bool add_selected )
{	
	printf( "WWM::insertCPoly\n" );
	cpolynum++;

	obj->setNext( cpolys );
	cpolys = obj;

	if ( add_selected )
	{
		CPolyIterator		iter( cpolys );
		
		EAL_DeselectBlue( &iter );
		obj->setSelect( SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE | SELECT_BLUE );
	}
	else
	{
		obj->setSelect( SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE );
	}
}



/*
  ====================
  removeCPoly

  ====================
*/
void WWM::removeCPoly( CPoly *obj )
{
	CPoly		*head;
	CPoly		*next, *b;

	printf( "WWM::removeCPoly\n" );

	head = NULL;
	for ( b = cpolys; b ; b=next )
	{
		next = b->getNext();
		
		if ( obj == b )
		{
			obj->setNext( NULL );
			cpolynum--;
			continue;
		}
		
		b->setNext( head );
		head = b;
	}
	
	cpolys = head;	
}



/*
  ====================
  getFirstCPoly

  ====================
*/
CPoly* WWM::getFirstCPoly( void )
{
	return cpolys;
}

/*
  ==============================
  findBestCPolyForRay

  ==============================
*/
void WWM::findBestCPolyForRay( vec3d_t from, vec3d_t dir, CPoly **sel_cpoly )
{
	CPolyIterator		iter( cpolys );
	CPoly	*cpoly;

	fp_t		len, min_len;

	min_len = 999999.9;

	*sel_cpoly = NULL;

	for ( iter.reset(); ( cpoly = iter.getNext() ); )
	{
		if ( !cpoly->testFlagSelect( SELECT_VISIBLE ) )
			continue;

		SelectBrush	*sel;
		sel = cpoly->getSelectBrush();

		if ( !sel->intersectWithRay( from, dir ) )
			continue;

		len = sel->rayLength();

		if ( len < 0.0 )
			continue;

		if ( len < min_len )
		{
			*sel_cpoly = cpoly;
			min_len = len;
		}		
	}      	
}

/*
  ==============================
  saveCPolyClass

  ==============================
*/
void WWM::saveCPolyClass( const char *name )
{
	hobj_t		*root;
	FILE		*h;
	CPolyIterator		iter( cpolys );
	CPoly			*cpoly;

	root = NewClass( "cpolys", "cpolys0" );
	
	for ( iter.reset(); ( cpoly = iter.getNext() ); )
	{
		hobj_t		*cls;

		cls = cpoly->buildClass();
		InsertClass( root, cls );
	}

	h = fopen( name, "w" );
	if ( !h )
	{
		printf( "can't open file.\n" );
	}
	else
	{
		WriteClass( root, h );
		fclose( h );
	}

	{
		hmanager_t *hm;
		hm = NewHManager();
		HManagerSetRootClass( hm, root );
		HManagerRebuildHash( hm ); 
		HManagerDeepDestroyClass( hm, root );
		FreeHManager( hm );		
	}	
}

/*
  ==============================
  loadCPolyClass

  ==============================
*/
void WWM::loadCPolyClass( const char *name )
{
	tokenstream_t		*ts;
	hobj_t			*root;

	hobj_search_iterator_t	iter;
	hobj_t			*cpoly;

	ts = BeginTokenStream( (char*) name );
	if ( !ts )
	{
		printf( "WARNING: WWM::loadCPolyClass can't open file\n" );
		return;
	}

	root = ReadClass( ts );
	EndTokenStream( ts );

	InitClassSearchIterator( &iter, root, "cpoly" );
	cpolys = NULL;
	cpolynum = 0;
	for ( ; ( cpoly = SearchGetNextClass( &iter ) ) ; )
	{	
		CPoly	*cp;

		cp = new CPoly( 0 );
		cp->initFromClass( cpoly );

		cp->setNext( cpolys );
		cpolys = cp;			
		cpolynum++;
	}

	{
		hmanager_t *hm;
		hm = NewHManager();
		HManagerSetRootClass( hm, root );
		HManagerRebuildHash( hm ); 
		HManagerDeepDestroyClass( hm, root );
		FreeHManager( hm );		
	}	
}

