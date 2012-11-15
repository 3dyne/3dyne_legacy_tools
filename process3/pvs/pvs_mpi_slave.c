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



// pvs_mpi_slave.c

#include "pvs_mpi.h"

void ComplexRejectFlood( int reference );	// pvs_mpi_complex.c

int	visleaf_num;
int	portal_num;

visleaf_t	visleafs[MAX_VISLEAFS];
portal_t	portals[MAX_PORTALS];

void CompilePortalClass( int pt, hobj_t *portal, hmanager_t *planehm )
{
	hobj_t		*plane;
	hpair_t		*pair;
	int		i, num;
	char		tt[256];

	pair = FindHPair( portal, "plane" );
	if ( !pair )
		Error( "missing 'plane' in portal '%s'.\n", portal->name );
	plane = HManagerSearchClassName( planehm, pair->value );
	if ( !plane )
		Error( "portal '%s' can't find plane '%s'.\n", portal->name, pair->value );
	portals[pt].pl = GetClassExtra( plane );

	pair = FindHPair( portal, "pointnum" );
	if ( !pair )
		Error( "missing 'pointnum' in portal '%s'.\n", portal->name );
	HPairCastToInt_safe( &num, pair );
	portals[pt].p = NewPolygon( num );
	portals[pt].p->pointnum = num;
	for ( i = 0; i < num; i++ )
	{
		sprintf( tt, "%d", i );
		pair = FindHPair( portal, tt );
		if ( !pair )
			Error( "missing point '%s' in portal '%s'.\n", tt, portal->name );
		HPairCastToVec3d_safe( portals[pt].p->p[i], pair );
	}

	portals[pt].self = portal;
}

void CompileVisleafClass( hmanager_t *visleafhm, hmanager_t *planehm )
{
	hobj_search_iterator_t	iter;
	hobj_search_iterator_t	portaliter;
	hobj_t		*visleaf;
	hobj_t		*portal;
	hobj_t		*otherleaf;
	hpair_t		*pair;
	char		tt[256];
	int		i, j;

	visleaf_num = 0;
	portal_num = 0;

	InitClassSearchIterator( &iter, HManagerGetRootClass( visleafhm ), "visleaf" );
	for ( ; ( visleaf = SearchGetNextClass( &iter ) ); )
	{
		visleafs[visleaf_num].startportal = portal_num;
		visleafs[visleaf_num].portalnum = 0;
		visleafs[visleaf_num].self = visleaf;

		sprintf( tt, "%d", visleaf_num );
		pair = NewHPair2( "int", "index", tt );
		InsertHPair( visleaf, pair );

		InitClassSearchIterator( &portaliter, visleaf, "portal" );
		for ( ; ( portal = SearchGetNextClass( &portaliter ) ) ; )
		{
			CompilePortalClass( portal_num, portal, planehm );
			visleafs[visleaf_num].portalnum++;
			portal_num++;
		}
		visleaf_num++;
	}

	for ( i = 0; i < visleaf_num; i++ )
	{
		for ( j = 0; j < visleafs[i].portalnum; j++ )
		{
			portals[j+visleafs[i].startportal].visleaf = i;
			pair = FindHPair( portals[j+visleafs[i].startportal].self, "otherleaf" );
			if ( !pair )
			{
				portals[j+visleafs[i].startportal].otherleaf = -1;
				continue;
			}
			otherleaf = HManagerSearchClassName( visleafhm, pair->value );
			if ( !otherleaf )
				Error( "can't find 'otherleaf'\n" );
			pair = FindHPair( otherleaf, "index" );
			if ( !pair )
				Error( "missing 'index' in otherleaf.\n" );
			HPairCastToInt_safe( &portals[j+visleafs[i].startportal].otherleaf, pair );			
		}
	}	
}

/*
  ====================
  ReadPlaneClass

  ====================
*/
hmanager_t * ReadPlaneClass( char *name )
{
	tokenstream_t	*ts;
	hobj_t		*planecls;
	hmanager_t	*hm;
	hobj_search_iterator_t	iter;
	hobj_t		*plane;
	hobj_t		*flipplane;
	cplane_t		*pl;
	int		num;
	hpair_t		*pair;

	ts = BeginTokenStream( name );
	planecls = ReadClass( ts );
	EndTokenStream( ts );

	hm = NewHManager();
	HManagerSetRootClass( hm, planecls );
	HManagerRebuildHash( hm );

	//
	// create compiled planes
	//

	fprintf( stderr, "load plane class and compile ...\n" );

	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		pl = NewCPlane();

		// plane norm
		pair = FindHPair( plane, "norm" );
		if ( !pair )
			Error( "missing plane normal.\n" );
		HPairCastToVec3d_safe( pl->norm, pair );

		// plane dist
		pair = FindHPair( plane, "dist" );
		if ( !pair )
			Error( "missing plane distance.\n" );
		HPairCastToFloat_safe( &pl->dist, pair );
		
		// plane type
		pair = FindHPair( plane, "type" );
		if ( !pair )
			Error( "missing plane type.\n" );
		HPairCastToInt_safe( &pl->type, pair );

		pl->self = plane;
		SetClassExtra( plane, pl );
		
	}

	//
	// resolve clsref_flipplane
	//
	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		// plane flipplane clsref
		pair = FindHPair( plane, "flipplane" );
		if ( !pair )
			Error( "missinig clsref flipplane" );

		flipplane = HManagerSearchClassName( hm, pair->value );
		if ( !flipplane )
			Error( "can't resolve clsref flipplane.\n" );

		pl = GetClassExtra( plane );
		pl->flipplane = GetClassExtra( flipplane );
	}

	printf( " %d planes\n", num );

	return hm;
}


/*
  =============================================================================
  trivial reject

  =============================================================================
*/

void TrivialRejectFlood( portal_t *ref, int leaf, int count )
{
	visleaf_t	*vl;
	portal_t		*test;
	visleaf_t		*otherleaf;
	int			i, j;
	fp_t		d;

	if ( leaf == -1 )
		Error( "leaf == -1\n" );

	vl = &visleafs[leaf];
	vl->count = count;

	ref->trivial_see[leaf>>3] |= 1<<(leaf&7);

	for ( j = 0; j < vl->portalnum; j++ )
	{
		test = &portals[j+vl->startportal];
		if ( ref == test )
			continue;
		if ( test->otherleaf == -1 )
			continue;
		otherleaf = &visleafs[test->otherleaf];
		if ( otherleaf->count == count )
			continue;

		for ( i = 0; i < ref->p->pointnum; i++ )
		{
			d = Vec3dDotProduct( ref->p->p[i], test->pl->norm ) - test->pl->dist;
			if ( d < 0 )
				break;
		}
		if ( i == ref->p->pointnum )
		{
			// no point of refportal is backside
			continue;
		}

		// is any point of testportal backside of refportal
		for ( i = 0; i < test->p->pointnum; i++ )
		{
			d = Vec3dDotProduct( test->p->p[i], ref->pl->norm ) - ref->pl->dist;
			if ( d < 0 )
				break;
		}
		if ( i == test->p->pointnum )
		{
			// no point of testportal frontside of refportal
			continue;
		}


		// ok, flood through portal
		TrivialRejectFlood( ref, test->otherleaf, count );	
	}
}

void TrivialReject( void )
{
	int		i, j, k, l;
	int		count;
	unsigned char	trivial_see[SEE_BUFFER_SIZE];

	count = 1;
	for ( i = 0; i < visleaf_num; i++ )
	{
//		printf( "visleaf %d\n", i );
		memset( trivial_see, 0, SEE_BUFFER_SIZE );	 

		for ( j = 0; j < visleafs[i].portalnum; j++ )
		{
			portals[j+visleafs[i].startportal].state = PortalState_trivial;
			TrivialRejectFlood( &portals[j+visleafs[i].startportal], i, count );

			for ( k = 0; k < SEE_BUFFER_SIZE; k++ )
			{
				for ( l = 0; l < 8; l++ )
				{				
					if ( portals[j+visleafs[i].startportal].trivial_see[k] & (1<<l) )
						portals[j+visleafs[i].startportal].trivial_see_num++;
				}		       
			}	
			count++;
		}		
	}
}

void CalcLeafVisibility( int visleaf )
{
	int		i, j, k;
	unsigned char	complex_see[SEE_BUFFER_SIZE];
	visleaf_t	*vl;
	hpair_t		*pair;
	char		tt[256];

	vl = &visleafs[visleaf];

	memset( complex_see, 0, SEE_BUFFER_SIZE );

	// sees own leaf
	complex_see[visleaf>>3] |= 1<<(visleaf&7);

	for ( i = 0; i < vl->portalnum; i++ )
	{
		portal_t	*pt;

		pt = &portals[i+vl->startportal];
		pt->through_see_num = 0;
		pt->complex_see_num = 0;

		for ( j = 0; j < visleaf_num/8+1; j++ )
		{
			for ( k = 0; k < 8; k++ )
			{
				if ( pt->through_see[j] & (1<<k) )
					pt->through_see_num++;
				if ( pt->complex_see[j] & (1<<k) )
					pt->complex_see_num++;
			}
		}
		

		if ( pt->otherleaf != -1 )
		{
			RemoveAndDestroyAllHPairsOfKey( vl->self, "through_see" );
			pair = NewHPair2( "bstring", "through_see", "x" );
			BstringCastToHPair( pt->through_see, visleaf_num/8+1, pair );
			InsertHPair( pt->self, pair );	
		}

		
		sprintf( tt, "%d", pt->complex_see_num );
		pair = NewHPair2( "int", "complex_num", tt );
		InsertHPair( pt->self, pair );
		
		sprintf( tt, "%d", pt->through_see_num );
		pair = NewHPair2( "int", "through_num", tt );
		InsertHPair( pt->self, pair );

		
		for ( j = 0; j < SEE_BUFFER_SIZE; j++ )
		{
			complex_see[j] |= pt->complex_see[j];
		}
	}

	RemoveAndDestroyAllHPairsOfKey( vl->self, "complex_see" );
	RemoveAndDestroyAllHPairsOfKey( vl->self, "index" );

	sprintf( tt, "%d", visleaf );
	pair = NewHPair2( "int", "bitpos", tt );
	InsertHPair( vl->self, pair );

	pair = NewHPair2( "bstring", "complex_see", "x" );
	BstringCastToHPair( complex_see, visleaf_num/8+1, pair );
	InsertHPair( vl->self, pair );		
}

int main( int argc, char *argv[] )
{
	char		in_plane_name[256];
	char		in_visleaf_name[256];

	hmanager_t	*planehm;
	hmanager_t	*visleafhm;
	

	int		tasknum;
	int		rank;
	MPI_Status	status;
	int		flag;
	int		tag;

	int		update_num = 0;
	int		run_num = 0;

	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &tasknum );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	memset( portals, 0, sizeof( portal_t )*MAX_PORTALS );
	memset( visleafs, 0, sizeof( visleaf_t )*MAX_VISLEAFS );

	//
	// slave main loop
	//

	for( ;; )
	{

		MPI_Probe( 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status );

		tag = status.MPI_TAG;

		if ( tag == TAG_IN_PLANE_NAME )
		{
			//
			// get plane name
			//
			MPI_Recv( in_plane_name, 256, MPI_BYTE, 0, TAG_IN_PLANE_NAME, MPI_COMM_WORLD, &status );
		}

		if ( tag == TAG_IN_VISLEAF_NAME )
		{
			//
			// get visleaf name
			//
			MPI_Recv( in_visleaf_name, 256, MPI_BYTE, 0, TAG_IN_VISLEAF_NAME, MPI_COMM_WORLD, &status );
		}

		if ( tag == TAG_LOAD_CLASSES )
		{
			MPI_Recv( 0, 0, MPI_INT, 0, TAG_LOAD_CLASSES, MPI_COMM_WORLD, &status );			
			//
			// load classes
			//
			memset( visleafs, 0, sizeof(visleaf_t)*MAX_VISLEAFS );
			memset( portals, 0, sizeof(portal_t)*MAX_PORTALS );
			
			planehm = ReadPlaneClass( in_plane_name );
			if ( !planehm )
				MPI_Abort( MPI_COMM_WORLD, ABORT_PLANE_LOAD_FAILED );
			
			visleafhm = NewHManagerLoadClass( in_visleaf_name );
			if ( !visleafhm )
				MPI_Abort( MPI_COMM_WORLD, ABORT_VISLEAF_LOAD_FAILED );
			
			CompileVisleafClass( visleafhm, planehm );
			
			printf( " %d visleafs, %d portals\n", visleaf_num, portal_num );

			sleep( 3 );			
			
		}
		
		if ( tag == TAG_RUN_TRIVIAL_REJECT )
		{
			MPI_Recv( 0, 0, MPI_INT, 0, TAG_RUN_TRIVIAL_REJECT, MPI_COMM_WORLD, &status );			
			printf( "-> node %d : run trivial reject\n", rank );
			TrivialReject();

			MPI_Send( 0, 0, MPI_INT, 0, TAG_READY, MPI_COMM_WORLD );
		}
		

		if ( tag == TAG_QUIT )
		{
			MPI_Recv( 0, 0, MPI_INT, 0, TAG_QUIT, MPI_COMM_WORLD, &status );			
			printf( "-> node %d : say goodbye\n", rank );
			printf( " %d updates, %d runs\n", update_num, run_num );
			MPI_Finalize();
			exit(0);
		}	

		//
		// trivial portal stuff
		//

		if ( tag == TAG_PORTALNUM_REQ )
		{
			MPI_Recv( 0, 0, MPI_INT, 0, TAG_PORTALNUM_REQ, MPI_COMM_WORLD, &status );
			printf( "-> node %d : portalnum request\n", rank );
			MPI_Send( &portal_num, 1, MPI_INT, 0, TAG_PORTALNUM, MPI_COMM_WORLD );
		}

		if ( tag == TAG_PORTAL_TRIVIAL_SEE_REQ )
		{
			int	portal;
			MPI_Recv( &portal, 1, MPI_INT, 0, TAG_PORTAL_TRIVIAL_SEE_REQ, MPI_COMM_WORLD, &status );
			MPI_Send( &portals[portal].trivial_see_num, 1, MPI_INT, 0, TAG_PORTAL_TRIVIAL_SEE, MPI_COMM_WORLD );
		}		

		//
		// complex portal stuff
		//
		if ( tag == TAG_RUN_COMPLEX_SEE_ON_PORTAL )
		{
			int	portal;
			MPI_Recv( &portal, 1, MPI_INT, 0, TAG_RUN_COMPLEX_SEE_ON_PORTAL, MPI_COMM_WORLD, &status );
			printf( "-> node %d : run complex see on portal %d\n", rank, portal );

			ComplexRejectFlood( portal );
			portals[portal].state = PortalState_complex;

			printf( "<- node %d : complex see of portal %d done\n", rank, portal );

			MPI_Send( portals[portal].complex_see, SEE_BUFFER_SIZE, MPI_BYTE, 0, TAG_COMPLEX_SEE_DATA, MPI_COMM_WORLD );
			
			run_num++;
		}

		if ( tag == TAG_UPDATE_COMPLEX_SEE )
		{
			int	portal;
			MPI_Recv( &portal, 1, MPI_INT, 0, TAG_UPDATE_COMPLEX_SEE, MPI_COMM_WORLD, &status );
			MPI_Recv( portals[portal].complex_see, SEE_BUFFER_SIZE, MPI_BYTE, 
				  0, TAG_COMPLEX_SEE_DATA, MPI_COMM_WORLD, &status );

			portals[portal].state = PortalState_complex;

			update_num++;
		}

		if ( tag == TAG_THROUGH_SEE_REQ )
		{
			int	portal;
			MPI_Recv( &portal, 1, MPI_INT, 0, TAG_THROUGH_SEE_REQ, MPI_COMM_WORLD, &status );
			MPI_Send( portals[portal].through_see, SEE_BUFFER_SIZE, MPI_BYTE, 0, TAG_THROUGH_SEE_DATA, MPI_COMM_WORLD );
		}

 		if ( tag == TAG_UPDATE_THROUGH_SEE )
		{
			int	portal;
			int		i;
			unsigned char	through_see[SEE_BUFFER_SIZE];
			MPI_Recv( &portal, 1, MPI_INT, 0, TAG_UPDATE_THROUGH_SEE, MPI_COMM_WORLD, &status );
			MPI_Recv( through_see, SEE_BUFFER_SIZE, MPI_BYTE, 
				  0, TAG_THROUGH_SEE_DATA, MPI_COMM_WORLD, &status );	

			for ( i = 0; i < SEE_BUFFER_SIZE; i++ )
				portals[portal].through_see[i] |= through_see[i];
		}

		//
		// save class
		//

		if ( tag == TAG_SAVE_CLASS )
		{
			char	name[256];
			int	i;
			FILE	*h;

			MPI_Recv( name, 256, MPI_CHAR, 0, TAG_SAVE_CLASS, MPI_COMM_WORLD, &status );

			printf( "-> node %d : save class as '%s'\n", rank, name );

			for ( i = 0; i < visleaf_num; i++ )
				CalcLeafVisibility( i );

			h = fopen( name, "w" );
			if ( !h )
				printf( "can't open output class.\n" );
			else
			{
				WriteClass( HManagerGetRootClass( visleafhm ), h );
				fclose( h );
			}
		}
	}
}
