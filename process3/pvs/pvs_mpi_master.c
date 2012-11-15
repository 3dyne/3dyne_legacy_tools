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



// pvs_mpi_master.c

#include "pvs_mpi.h"



int		portal_num = -1;
mportal_t	portals[MAX_PORTALS];

#define	MAX_UPDATE_PORTALS		( 4096 )

typedef struct runnode_s
{
	int		updatenum;
	int		updates[MAX_UPDATE_PORTALS];

	int		run_portal;	// portal, the node is working on
} runnode_t;

#define MAX_NODES		( 16 )
runnode_t	runnodes[MAX_NODES];


/*
  ====================
  GetBestTrivial

  ====================
*/
void GetBestTrivial( int *bestportal )
{
	int		min;
	int	        i;

	min = 1<<30;
	*bestportal = -1;

	for ( i = 0; i < portal_num; i++ )
	{
		if ( portals[i].state != PortalState_trivial )
			continue;
		if ( portals[i].trivial_see_num < min )
		{
			min = portals[i].trivial_see_num;
			*bestportal = i;
		}
	}
//	printf ( "best: %d\n", min );
}

int main( int argc, char *argv[] )
{
	int		i;
	int		tasknum;
	int		rank;
	MPI_Status	status;

	char		*in_visleaf_name;
	char		*in_plane_name;
	char		*out_visleaf_name;

	hmanager_t	*planehm;
	hmanager_t	*visleafhm;

	tokenstream_t	*ts;
	FILE		*h;

//	visleaf_t		*visleaflist;

	int		portals_to_do;

	char		tt[256];

	SetCmdArgs( argc, argv );

	in_visleaf_name = GetCmdOpt2( "-i" );
	in_plane_name = GetCmdOpt2( "-pl" );
	out_visleaf_name = GetCmdOpt2( "-o" );


	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &tasknum );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	printf( "===== pvs - build potential visibility set for visleafs =====\n" );
	printf( "----- MPI Version -----\n" );

	printf( "rank of master is %d\n", rank );
	printf( "setting up for %d nodes ...\n", tasknum );

	if ( !in_visleaf_name )
	{
		in_visleaf_name = "/home/lam/levels/_visleaf_visleaf.hobj";
		printf( " default input visleaf class: %s\n", in_visleaf_name );
	}
	else
	{
		printf( " input visleaf class: %s\n", in_visleaf_name );
	}

	if ( !in_plane_name )
	{
		in_plane_name = "/home/lam/levels/_plane.hobj";
		printf( " default input plane class: %s\n", in_plane_name );
	}
	else
	{
		printf( " input plane class: %s\n", in_plane_name );
	}

	if ( !out_visleaf_name )
	{
		out_visleaf_name = "_pvsout_visleaf.hobj";
		printf( " default output visleaf class: %s\n", out_visleaf_name );
	}
	else
	{
		printf( " output visleaf class: %s\n", out_visleaf_name );
	}

	
	//
	// send file names
	//

	// plane name
	sprintf( tt, "%s", in_plane_name );
	for ( i = 0; i < tasknum; i++ )
	{
		if ( i == rank )
			continue;

		MPI_Send( tt, 256, MPI_BYTE, i, TAG_IN_PLANE_NAME, MPI_COMM_WORLD );

	}

	// visleaf name
	sprintf( tt, "%s", in_visleaf_name );
	for ( i = 1; i < tasknum; i++ )
	{
		MPI_Send( tt, 256, MPI_BYTE, i, TAG_IN_VISLEAF_NAME, MPI_COMM_WORLD );		
	}
    

	// load classes
	for ( i = 1; i < tasknum; i++ )
	{
		MPI_Send( 0, 0, MPI_INT, i, TAG_LOAD_CLASSES, MPI_COMM_WORLD );
		MPI_Send( 0, 0, MPI_INT, i, TAG_RUN_TRIVIAL_REJECT, MPI_COMM_WORLD );
	}

	// wait for ready
	for ( i = 1; i < tasknum; i++ )
	{
		MPI_Recv( 0, 0, MPI_INT, i, TAG_READY, MPI_COMM_WORLD, &status );
		printf( "<- node %d : ready\n", i );		
	}

	
	//
	// get trivial info from node1
	//
	MPI_Send( 0, 0, MPI_INT, 1, TAG_PORTALNUM_REQ, MPI_COMM_WORLD );
	MPI_Recv( &portal_num, 1, MPI_INT, 1, TAG_PORTALNUM, MPI_COMM_WORLD, &status );
	printf( " %d portals\n", portal_num );
	portals_to_do = portal_num;

	printf( "<- node 1 : get trivial see ...\n" );
	for ( i = 0; i < portal_num; i++ )
	{
		MPI_Send( &i, 1, MPI_INT, 1, TAG_PORTAL_TRIVIAL_SEE_REQ, MPI_COMM_WORLD );
		MPI_Recv( &portals[i].trivial_see_num, 1, MPI_INT, 1, TAG_PORTAL_TRIVIAL_SEE, MPI_COMM_WORLD, &status );
		portals[i].state = PortalState_trivial;
	}
	

	//
	// send initial working orders
	//
	for ( i = 1; i < tasknum; i++ )
	{
		int	bestportal;
		GetBestTrivial( &bestportal );
		if ( bestportal == -1 )
			Error( "not enought portals for initial working order.\n" );
//		printf( "best %d\n", bestportal );
		MPI_Send( &bestportal, 1, MPI_INT, i, TAG_RUN_COMPLEX_SEE_ON_PORTAL, MPI_COMM_WORLD );
		portals[bestportal].state = PortalState_run_complex;
		portals[bestportal].run_on_node = i;
		
		runnodes[i].run_portal = bestportal;
		runnodes[i].updatenum = 0;
		
		portals_to_do--;
	}
	

	//
	// 
	//
	for (;;)
	{
		int		done_portal;
		int		bestportal;
		int		node;

		unsigned char	see_buffer[SEE_BUFFER_SIZE];

		if ( ! (portals_to_do & 15 ) )
			printf( "%d portals to do\n", portals_to_do );

		MPI_Recv( see_buffer, SEE_BUFFER_SIZE, MPI_BYTE, MPI_ANY_SOURCE,
			  TAG_COMPLEX_SEE_DATA, MPI_COMM_WORLD, &status );
		node = status.MPI_SOURCE;
		done_portal = runnodes[node].run_portal;
		portals[done_portal].state = PortalState_complex;
		memcpy( portals[done_portal].complex_see, see_buffer, SEE_BUFFER_SIZE );
		
		// insert the portal into all other runnodes for update
		for ( i = 1; i < tasknum; i++ )
		{
			if ( i == node )
				continue;
			runnodes[i].updates[runnodes[i].updatenum++] = done_portal;
		}

		// send updates to node
		for ( i = 0; i < runnodes[node].updatenum; i++ )
		{
			// send portal index
			MPI_Send( &runnodes[node].updates[i], 1, MPI_INT, node, TAG_UPDATE_COMPLEX_SEE, MPI_COMM_WORLD );
			MPI_Send( portals[runnodes[node].updates[i]].complex_see, SEE_BUFFER_SIZE, MPI_BYTE, 
				  node, TAG_COMPLEX_SEE_DATA, MPI_COMM_WORLD );
		}
		runnodes[node].updatenum = 0;
		runnodes[node].run_portal = -1;
		portals_to_do--;

		GetBestTrivial( &bestportal );
		if ( bestportal == -1 )
		{
			// are there still running nodes ?
			for ( i = 1; i < tasknum; i++ )
			{
				if ( runnodes[i].run_portal != -1 )
					break;	// yes
			}
			
			if ( i == tasknum )
				break;		// no more portals to do
		}
		else
		{
			MPI_Send( &bestportal, 1, MPI_INT, node, TAG_RUN_COMPLEX_SEE_ON_PORTAL, MPI_COMM_WORLD );	       
			portals[bestportal].state = PortalState_run_complex;
			portals[bestportal].run_on_node = node;	
			runnodes[node].run_portal = bestportal;
		}
	}

	//
	// move through_see of all nodes to node 1
	//
	printf( "gather through_see of all nodes an send it to node 1 ...\n" );
	for ( i = 0; i < portal_num; i++ )
	{
		int		j;
		for ( j = 2; j < tasknum; j++ )
		{
			unsigned char	through_see[SEE_BUFFER_SIZE];

			MPI_Send( &i, 1, MPI_INT, j, TAG_THROUGH_SEE_REQ, MPI_COMM_WORLD );
			MPI_Recv( through_see, SEE_BUFFER_SIZE, MPI_BYTE, MPI_ANY_SOURCE,
				  TAG_THROUGH_SEE_DATA, MPI_COMM_WORLD, &status );
			MPI_Send( &i, 1, MPI_INT, 1 /*node 1*/, TAG_UPDATE_THROUGH_SEE, MPI_COMM_WORLD );
			MPI_Send( through_see, SEE_BUFFER_SIZE, MPI_BYTE, 1 /*node 1*/, TAG_THROUGH_SEE_DATA, MPI_COMM_WORLD );
		}
	}

	//
	// node 1 writes its data
	//
	sprintf( tt, "%s", out_visleaf_name );
	MPI_Send( tt, 256, MPI_BYTE, 1, TAG_SAVE_CLASS, MPI_COMM_WORLD );


	//
	// send quit
	//
	for ( i = 1; i < tasknum; i++ )
	{
		MPI_Send( 0, 0, MPI_INT, i, TAG_QUIT, MPI_COMM_WORLD );
	}

	MPI_Finalize();
	exit(0);
}
