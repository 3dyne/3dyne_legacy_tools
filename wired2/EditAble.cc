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



// EditAble.cc

#include "EditAble.hh"


/*
  ==================================================
  SelectBrush

  ==================================================
*/

SelectBrush::SelectBrush()
{
	U_InitList( &this->plane_list );
}

SelectBrush::~SelectBrush()
{
	U_CleanUpList( &this->plane_list, free );	
}

void SelectBrush::reset( void )
{
	U_CleanUpList( &this->plane_list, free );	
	U_InitList( &this->plane_list );
}

void SelectBrush::addPlane( vec3d_t norm, fp_t dist )
{
	plane_t		*pl;

	pl = NEWTYPE( plane_t );
	Vec3dCopy( pl->norm, norm );
	pl->dist = dist;

	U_ListInsertAtHead( &this->plane_list, pl );
}
 
void SelectBrush::setupFromBB( vec3d_t min, vec3d_t max )
{
	int		i;
	vec3d_t		norm;
	fp_t		dist;

	for ( i = 0; i < 3; i++ )
	{
		Vec3dInit( norm, 0.0, 0.0, 0.0 );
		norm[i] = 1.0;
		dist = max[i];
		this->addPlane( norm, dist );

		norm[i] = -1.0;
		dist = -min[i];
		this->addPlane( norm, dist );
	}
}

bool SelectBrush::intersectWithRay( vec3d_t origin, vec3d_t dir )
{
	vec3d_t		from;
	vec3d_t		to;
	vec3d_t		len;

	int		i;
	fp_t		d0, d1, m;

	u_list_iter_t	iter;
	plane_t		*pl;
	plane_t		*hitplane, *leaveplane;

	Vec3dCopy( from, origin );
	Vec3dMA( to, 16000.0, dir, from );

	hitplane = NULL;
	leaveplane = NULL;

	this->ray_len = -1;

	U_ListIterInit( &iter, &this->plane_list );
	for ( ; ( pl = (plane_t *) U_ListIterNext( &iter ) ) ; )
	{
		d0 = Vec3dDotProduct( pl->norm, from ) - pl->dist;
		d1 = Vec3dDotProduct( pl->norm, to ) - pl->dist;

		if ( d0 > 0 && d1 > 0 )
			return false;

		if ( d0 > 0 && d1 < 0 )
		{
			hitplane = pl;

			m = d0 / (d0-d1);

			for ( i = 0; i < 3; i++ )
				from[i] = from[i] + m*(to[i]-from[i]);
		}
		if ( d0 < 0 && d1 > 0 )
		{
			leaveplane = pl;

			m = d0 / (d0-d1);

			for ( i = 0; i < 3; i++ )
				to[i] = from[i] + m*(to[i]-from[i]);
		}
	}

	Vec3dSub( len, from, origin );
	this->ray_len = Vec3dLen( len );

	return true;
}

fp_t SelectBrush::rayLength( void )
{
	return this->ray_len;
}

/*
  ==================================================
  EAL functions

  ==================================================
*/



#if 0
/*
  ====================
  EAL_Init

  ====================
*/
void EAL_Init( EditAble **list )
{
	*list = NULL;
}



/*
  ====================
  EAL_Insert

  ====================
*/
void EAL_Insert( EditAble **list, EditAble *obj )
{
	EditAble	*head;

	head = *list;

	obj->setNext( head );
	*list = obj;
}



/*
  ====================
  EAL_Remove

  ====================
*/
void EAL_Remove( EditAble **list, EditAble *obj )
{
	EditAble	*head;
	EditAble	*next, *e;
	
	head = NULL;

	for ( e = *list; e ; e=next )
	{
		next = e->getNext();

		if ( e == obj )
		{
			e->setNext( NULL );
			continue;
		}

		e->setNext( head );
		head = e;
	}
		
	*list = head;
}
#endif


/*
  ====================
  EAL_CheckViewBounds

  ====================
*/
void EAL_CheckViewBounds( EAIterator *iter, vec3d_t viewmin, vec3d_t viewmax )
{
	EditAble	*e;

	for ( iter->reset(); (e=iter->getNext()); )
	{
		e->setFlagSelect( SELECT_VISIBLE );
		e->resetFlagSelect( SELECT_PRE );
		
		e->checkViewBounds( viewmin, viewmax );
		if ( e->isVisible() )
			e->setFlagSelect( SELECT_UPDATE );		
	}
}



/*
  ====================
  EAL_AllUpdateFlagsTrue

  ====================
*/
void EAL_AllUpdateFlagsTrue( EAIterator *iter )
{
	EditAble	*e;

	for ( iter->reset(); (e=iter->getNext()); )
	{
//		printf( "EAL_AllUpdateFlagsTrue: %p\n", e );
		e->setFlagSelect( SELECT_UPDATE );
	}
}



/*
  ====================
  EAL_CalcBB

  calc total bound box of
  all objects in list
  ====================
*/
void EAL_CalcTotalBB( EAIterator *iter, vec3d_t totalmin, vec3d_t totalmax )
{
	EditAble	*e;
	vec3d_t		min, max;

	Vec3dInitBB( totalmin, totalmax, 9999999.9 );
	for ( iter->reset(); (e=iter->getNext()); )
	{
		e->getBB( min, max );
		Vec3dAddToBB( totalmin, totalmax, min );
		Vec3dAddToBB( totalmin, totalmax, max );
	}
}


/*
  ====================
  EAL_BlueRaySelector

  ====================
*/
#if 0
void EAL_BlueRaySelector( EditAble *list, vec3d_t start, vec3d_t dir )
{
	EditAble	*e;
	EditAble	*blue;
	EditAble	*blue_new;
	bool_t		hit;
	bool_t		pre_new;

	//
	// pre-select editable
	//
	pre_new = false;
	for ( e = list; e ; e=e->getNext() )
	{
		if ( !e->testFlagSelect( SELECT_VISIBLE ) )
		{
			e->resetFlagSelect( SELECT_PRE );
			continue;
		}

		hit = e->intersectWithRay( start, dir );

		if ( hit )
			printf( "hit: %p\n", e );

		if ( hit && !e->testFlagSelect( SELECT_PRE ) )
		{
			// hit but not pre_selected => new set
			pre_new = true;
			printf( "hit EditAble: %p !\n", e );
			e->setFlagSelect( SELECT_PRE );
			continue;
		}

		if ( !hit && e->testFlagSelect( SELECT_PRE ) )
		{
			// not hit but pre-selected => new set 
			pre_new = true;
			e->resetFlagSelect( SELECT_PRE );
			continue;
		}
	}

	printf( "pre_new: %d\n", pre_new );

	//
	// search old blue and unblue it
	//
	blue = NULL;
	for ( e = list; e ; e=e->getNext() )
	{
		if ( e->testFlagSelect( SELECT_BLUE ) )
		{
			blue = e;
			e->resetFlagSelect( SELECT_BLUE );
			e->setFlagSelect( SELECT_UPDATE );
			break;
		}
	}
	printf( "old blue: %p\n", blue );

	blue_new = NULL;
	//
	// if blue was found and the pre-selection not changed, search next pre
	//
	if ( blue && !pre_new )
	{
		for ( e = blue->getNext(); e ; e=e->getNext() )
		{
			if ( e->testFlagSelect( SELECT_PRE ) )
			{
				e->setFlagSelect( SELECT_BLUE );
				e->setFlagSelect( SELECT_UPDATE );
				blue_new = e;
				break;
			}
		}
	}

	//
	// if not found or pre-selection changed, begin at start
	//
	if ( !blue_new || pre_new )
	{
		for ( e = list; e ; e=e->getNext() )
		{
			if ( e->testFlagSelect( SELECT_PRE ) )
			{
				e->setFlagSelect( SELECT_BLUE );
				e->setFlagSelect( SELECT_UPDATE );
				blue_new = e;
				break;
			}
		}
	}

	printf( "new_blue: %p\n", blue_new );

}
#endif

/*
  ====================
  EAL_AllDeselect

  ====================
*/
void EAL_DeselectBlue( EAIterator *iter )
{
	EditAble	*obj;

	for ( iter->reset(); (obj = iter->getNext()); )
	{
		if ( obj->testFlagSelect( SELECT_BLUE ) )
		{
			obj->resetFlagSelect( SELECT_BLUE );
			obj->resetFlagSelect( SELECT_PRE );
			obj->setFlagSelect( SELECT_UPDATE );
		}
	}
}

void EAL_BlueRaySelector( EAIterator *iter, vec3d_t start, vec3d_t dir )
{
	EditAble	*e;
	EditAble	*blue;
	EditAble	*blue_new;
	bool_t		hit;
	bool_t		pre_new;

	//
	// pre-select editable
	//
	pre_new = false;
	for ( iter->reset(); (e=iter->getNext()); )
	{

		
		if ( !e->testFlagSelect( SELECT_VISIBLE ) )
		{
			e->resetFlagSelect( SELECT_PRE );
			continue;
		}

		hit = e->intersectWithRay( start, dir );

		if ( hit )
			printf( "hit: %p\n", e );

		if ( hit && !e->testFlagSelect( SELECT_PRE ) )
		{
			// hit but not pre_selected => new set
			pre_new = true;
			printf( "hit EditAble: %p !\n", e );
			e->setFlagSelect( SELECT_PRE );
			continue;
		}

		if ( !hit && e->testFlagSelect( SELECT_PRE ) )
		{
			// not hit but pre-selected => new set 
			pre_new = true;
			e->resetFlagSelect( SELECT_PRE );
			continue;
		}
	}

	printf( "pre_new: %d\n", pre_new );

	//
	// search old blue and unblue it
	//
	blue = NULL;
	for ( iter->reset(); (e=iter->getNext()); )
	{
		if ( e->testFlagSelect( SELECT_BLUE ) )
		{
			blue = e;
			e->resetFlagSelect( SELECT_BLUE );
			e->setFlagSelect( SELECT_UPDATE );
			break;
		}
	}
	printf( "old blue: %p\n", blue );

	blue_new = NULL;
	//
	// if blue was found and the pre-selection not changed, search next pre
	//
	if ( blue && !pre_new )
	{
		for ( ; (e=iter->getNext()); )
		{
			if ( e->testFlagSelect( SELECT_PRE ) )
			{
				e->setFlagSelect( SELECT_BLUE );
				e->setFlagSelect( SELECT_UPDATE );
				blue_new = e;
				break;
			}
		}
	}

	//
	// if not found or pre-selection changed, begin at start
	//
	if ( !blue_new || pre_new )
	{
		for ( iter->reset(); (e=iter->getNext());  )
		{
			if ( e->testFlagSelect( SELECT_PRE ) )
			{
				e->setFlagSelect( SELECT_BLUE );
				e->setFlagSelect( SELECT_UPDATE );
				blue_new = e;
				break;
			}
		}
	}

	printf( "new_blue: %p\n", blue_new );

}

/*
  ====================
  EAL_BlueMove

  ====================
*/
void EAL_BlueMove( EAIterator *iter, vec3d_t from, vec3d_t to )
{
	EditAble	*e;

	for ( iter->reset(); (e=iter->getNext()); )
	{
		if ( e->testFlagSelect( SELECT_BLUE ) )
		{
			e->moveSelf( from, to );
		}
	}
}



/*
  ====================
  EAL_Move

  ====================
*/
void EAL_Move( EAIterator *iter, vec3d_t from, vec3d_t to )
{
	EditAble	*e;

//	Vec3dPrint( from );
//	Vec3dPrint( to );
	iter->reset();
	for ( ; (e=iter->getNext()); )
	{
//		printf( "EAL_Move: %p\n", e );
		e->moveSelf( from, to );
	}
}

#if 0
int main()
{
	vec3d_t		start = { 0.5, 0.5, 0.5 };
	vec3d_t		dir = { 1, 0, 0 };
	vec3d_t		min, max;
	int		i;

	TestBox	*b1 = new TestBox;
	TestBox	*b2 = new TestBox;
	TestBox	*b3 = new TestBox;

	Vec3dInit( min, 0,0,0 );
	Vec3dInit( max, 1,1,1 );
	b1->setBB( min, max );
	Vec3dInit( min, 1,0,0 );
	Vec3dInit( max, 2,1,1 );
	b2->setBB( min, max );
	Vec3dInit( min, 2,0,0 );
	Vec3dInit( max, 3,1,1 );
	b3->setBB( min, max );

	b1->setFlagSelect( SELECT_VISIBLE );
	b2->setFlagSelect( SELECT_VISIBLE );
	b3->setFlagSelect( SELECT_VISIBLE );

	b1->setNext( b2 );
	b2->setNext( b3 );
	b3->setNext( NULL );

	for ( i = 0; i < 6; i++ )
		EAL_BlueRaySelector( b1, start, dir );

}
#endif
