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



// balance.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <lib_math.h>
#include <lib_container.h>

#include "light.h"

static u_map_t		*balance_map;
static int		balance_node_num = 0;
static int		balance_patch_num = 0;

typedef struct balance_key_s
{
	cplane_t	*pl;
	int		x, y;
	fp_t		patchsize;
} balance_key_t;

typedef struct balance_scrap_node_s
{
	fp_t		c_comp;
	fp_t		s_comp;
	fp_t		area;
	struct balance_scrap_node_s	*next;
} balance_scrap_node_t;

typedef struct balance_node_s
{
	// valid after balancing all scraps in list
	fp_t		c_sum;
	fp_t		s_sum;

//	int		num;
	
	// key
	balance_key_t	key;

	balance_scrap_node_t	*scrap_head;
} balance_node_t;

static void * GetKey_balance( const void *obj )
{
	balance_node_t		*bn;

	bn = ( balance_node_t * ) obj;

	return (void *) &bn->key;
}

static int KeyCompare_balance( const void *key1, const void *key2 )
{
	return memcmp( key1, key2, sizeof( balance_key_t ) );
}

void Balance_Init( void )
{
	balance_map = NEWTYPE( u_map_t );
	U_InitMap( balance_map, map_default, KeyCompare_balance, GetKey_balance );       
}

void Balance_AddWorldPatchScrap( fp_t scrap_area, cplane_t *pl, int x, int y, fp_t patchsize, fp_t c_comp, fp_t s_comp )
{
	balance_node_t		tmp;
	balance_node_t		*search;

	tmp.key.pl = pl;
	tmp.key.x = x;
	tmp.key.y = y;
	tmp.key.patchsize = patchsize;

	search = U_MapSearch( balance_map, &tmp.key );

	if ( !search )
	{
		// create a new balance node and insert it into the map
		// 'p' gets the first patch in the list

		balance_node_t	*bn;
		balance_scrap_node_t	*scrap;
		bn = NEWTYPE( balance_node_t );

		scrap = NEWTYPE( balance_scrap_node_t );
		
		scrap->c_comp = c_comp;
		scrap->s_comp = s_comp;
		scrap->area = scrap_area;
		
		bn->scrap_head = scrap;
//		bn->num = 1;

		bn->key.pl = pl;
		bn->key.x = x;
		bn->key.y = y;
		bn->key.patchsize = patchsize;

		U_MapInsert( balance_map, bn );
		balance_node_num++;
		balance_patch_num++;
	}
	else
	{
		balance_scrap_node_t	*scrap;

		scrap = NEWTYPE( balance_scrap_node_t );
		
		scrap->c_comp = c_comp;
		scrap->s_comp = s_comp;
		scrap->area = scrap_area;		

		scrap->next = search->scrap_head;
		search->scrap_head = scrap;

//		search->num++;
//		Vec3dAdd( search->c_sum, search->c_sum, c_comp );
//		Vec3dAdd( search->s_sum, search->s_sum, s_comp );
		
		balance_patch_num++;
	}
}

void Balance_Dump( void )
{
	printf( "BalanceDump:\n" );
	printf( " %d balance nodes\n", balance_node_num );
	printf( " %d balance patches\n", balance_patch_num );
}

static void BalanceNodeScraps( void *obj )
{
	balance_node_t	*bn;
	balance_scrap_node_t	*scrap;
	fp_t		total_area;
	fp_t		scale;

	bn = obj;

	if ( bn->scrap_head )
	{

		for ( total_area = 0.0, scrap = bn->scrap_head; scrap; scrap=scrap->next )
		{
			total_area += scrap->area;
		}

		if ( total_area == 0.0 )
//			Error( "BalanceNodeScraps: total_area == 0.0\n" );
			return;

		for ( scrap = bn->scrap_head; scrap; scrap=scrap->next )
		{
			scale = scrap->area / total_area;
			bn->c_sum += scale*scrap->c_comp;
			bn->s_sum += scale*scrap->s_comp;
		}
	}
}

void Balance_AllScraps( void )
{
//	printf( "balancing scraps ...\n" );
	U_MapForEach( balance_map, BalanceNodeScraps );
}

static void CleanUpNode( void *obj )
{
	balance_node_t	*bn;
	balance_scrap_node_t	*scrap, *next;

	bn = obj;

	for ( scrap = bn->scrap_head; scrap ; scrap=next )
	{
		next=scrap->next;
		FREE( scrap );
	}

	if ( bn->scrap_head )
	{
		bn->scrap_head = NULL;
		bn->c_sum = 0.0;
		bn->s_sum = 0.0;
	}
}

void Balance_CleanUpNodes( void )
{
	U_MapForEach( balance_map, CleanUpNode );	
}

bool_t Balance_GetWorldPatch( cplane_t *pl, int x, int y, fp_t patchsize, fp_t *c_comp, fp_t *s_comp )
{
	balance_node_t		tmp;
	balance_node_t		*search;

	tmp.key.pl = pl;
	tmp.key.x = x;
	tmp.key.y = y;
	tmp.key.patchsize = patchsize;

	search = U_MapSearch( balance_map, &tmp.key );

	if ( !search )
	{
		return false;
	}
	else
	{
		*c_comp = search->c_sum;
		*s_comp = search->s_sum;
	}
	return true;
}
