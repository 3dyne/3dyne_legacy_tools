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



// portal.c

#include "portal.h"

int		p_planenum;
plane_t		p_planes[MAX_PLANES];

int		p_nodenum = 0;
bspnode_t	p_bspnodes[MAX_NODES];
node_t		p_nodes[MAX_NODES];

int		p_portalnum = 0;
portal_t	p_portals[MAX_PORTALS];


/*
  ========================================
  
  internal portal stuff

  ========================================
*/

portal_t*	NewPortal( void )
{
	if ( p_portalnum == MAX_PORTALS )
		Error( "NewPortal: reached MAX_PORTALS\n" );

	memset( &p_portals[p_portalnum], 0, sizeof( portal_t ) );
	p_portalnum++;

	return &p_portals[p_portalnum-1];
}

/*
  ====================
  AddPortalToNodes

  ====================
*/
void AddPortalToNodes( portal_t *p, node_t *front, node_t *back )
{
	if ( p->nodes[0] || p->nodes[1] )
		Error( "AddPortalToNodes: portal allready got nodes.\n" );

	p->nodes[0] = front;
	p->nodes[1] = back;

	p->next[0] = front->portals;
	front->portals = p;
	front->portalnum++;

	p->next[1] = back->portals;
	back->portals = p;
	back->portalnum++;
} 

/*
  ====================
  RemovePortalFromNode

  ====================
*/
void RemovePortalFromNode( portal_t *p, node_t *n )
{
	portal_t	**pp, *t;

	pp = &n->portals;
	for(;;)
	{
		t = *pp;
		if ( !t )
			Error( "RemovePortalFromNode: portal not found in node.\n" );

		if ( t == p )
			break;
		
		if ( t->nodes[0] == n )
			pp = &t->next[0];
		else if ( t->nodes[1] == n )
			pp = &t->next[1];
		else
			Error( "RemovePortalFromNode: node not found in portal.\n" );

	}
	
	if ( p->nodes[0] == n )
	{
		*pp = p->next[0];
		p->nodes[0] = NULL;
	}
	else if ( p->nodes[1] == n )
	{
		*pp = p->next[1];
		p->nodes[1] = NULL;
	}

	n->portalnum--;
}



/*
  ========================================
  
  main portal

  ========================================
*/
/*
  ====================
  Portal_MakeHeadNode

  ====================
*/

node_t		out_node;

// fix me: use bb again some day
#define		BIG_BOX		( 7500.0 )
void Portal_MakeHeadNode( node_t *n )
{
	int		i, j;
	plane_t		pl[6];
	portal_t	*portals[6];

	//
	// make outnode for tree
	//

//	tree->outnode = NewNode();
	out_node.plane = NODE_PLANE_LEAF_OUTSIDE;
	out_node.index = -1;
	out_node.child[0] = NULL;
	out_node.child[1] = NULL;
	out_node.portals = NULL;

//	tree->outnode->contents = BRUSH_CONTENTS_EMPTY;

	for ( i = 0; i < 3; i++ )
	{
		Vec3dInit( pl[i].norm, 0.0, 0.0, 0.0 );
		pl[i].norm[i] = 1.0;
		pl[i].dist = BIG_BOX; //tree->max[i] + 64.0;
		
		Vec3dInit( pl[i+3].norm, 0.0, 0.0, 0.0 );
		pl[i+3].norm[i] = -1.0;
		pl[i+3].dist = BIG_BOX; //- (tree->min[i] - 64.0);
	}

	for ( i = 0; i < 6; i++ )
	{
		portals[i] = NewPortal();
		memcpy( &portals[i]->plane, &pl[i], sizeof( plane_t ) );

		portals[i]->p = BasePolygonForPlane( pl[i].norm, pl[i].dist );
		AddPortalToNodes( portals[i], &out_node, n );
	}

	for ( i = 0; i < 6; i++ )
	{
		for ( j = 0; j < 6; j++ )
		{
			if( i==j )
				continue;
			if ( portals[i]->p )
				ClipPolygonInPlace( &portals[i]->p, pl[j].norm, pl[j].dist );
		}
		if ( !portals[i]->p )
			Error( "Portal_MakeHeadNode: clipped poly away.\n" );
	}	
}



/*
  ====================
  Portal_MakeNodeChildren

  makes a portal from the node plane
  and adds it to the two children of the node

  ====================
*/
bool_t Portal_MakeNodeChildren( node_t *node )
{
	plane_t		*pl;
	polygon_t	*poly;
	portal_t	*p;

	int		side;
	portal_t	*pnew;

	vec3d_t		norm;
	fp_t		dist;

	if ( node->plane < 0 )
		Error( "Portal_MakeNodeChildren: oops, it's a leaf.\n" );

	pl = &p_planes[node->plane];

	poly = BasePolygonForPlane( pl->norm, pl->dist );

	//
	// clip polygon by all portal planes of this node
	// 

//	printf( "Portal_MakeNodeChildren: " );
	side = 0;
	dist = 0.0;
	for ( p = node->portals; p ; p=p->next[side] )
	{
//		printf("*");
		if ( p->nodes[0] == node )
		{
			side = 0;
			Vec3dFlip( norm, p->plane.norm );
			dist = -p->plane.dist;
		}
		else if ( p->nodes[1] == node )
		{
			side = 1;
			Vec3dCopy( norm, p->plane.norm );
			dist = p->plane.dist;
		}
		else
		{
			Error( "Portal_MakeNodeChildren: node not found in portal.\n" );		      			
		}

		if ( poly )
			ClipPolygonInPlace( &poly, norm, dist );
		       
	}

//	printf( "\n" );

	if ( !poly )
	{
		printf( " * Portal_MakeNodeChildren: clipped base polygon. *\n" );
		return false;
	}

	pnew = NewPortal();
	pnew->p = poly;
	memcpy( &pnew->plane, pl, sizeof( plane_t ) );
	AddPortalToNodes( pnew, node->child[0], node->child[1] );

	return true;
}



/*
  ====================
  Portal_SplitNode

  ====================
*/
void Portal_SplitNode( node_t *node )
{
	plane_t		*pl;
	portal_t		*p, *pnext;
	int		side;
	node_t	*othernode;
	portal_t	*frontportal, *backportal;
	polygon_t	*front, *back;

	pl = &p_planes[node->plane];

	side = 0;
	for ( p = node->portals; p ; p=pnext )
	{
		if ( p->nodes[0] == node )
			side = 0;
		else if ( p->nodes[1] == node )
			side = 1;
		else
			Error( "Portal_SplitNode: can't find node in portal.\n" );

		pnext = p->next[side];

		othernode = p->nodes[!side];

		RemovePortalFromNode( p, p->nodes[0] );
		RemovePortalFromNode( p, p->nodes[1] );

		SplitPolygon( p->p, pl->norm, pl->dist, &front, &back );
		FreePolygon( p->p );


		if ( !front && !back )
			Error( "Portal_SplitNode: no front and back after split.\n" );

		if ( !front )
		{
			// polygon is back
			p->p = back;

			if ( side ) // node was back of portal
				AddPortalToNodes( p, othernode, node->child[1] );
			else // node was front
				AddPortalToNodes( p, node->child[1], othernode );
			continue;
		}

		if ( !back )
		{
			// polygon is front
			p->p = front;

			if ( side ) // node was back of portal
				AddPortalToNodes( p, othernode, node->child[0] );
			else
				AddPortalToNodes( p, node->child[0], othernode );
			continue;
		}

		//
		// portal got split
		//
		frontportal = p;
		frontportal->p = front;

		backportal = NewPortal();
		memcpy( backportal, p, sizeof( portal_t ) );
		backportal->p = back;

		if ( side ) // node was back of portal
		{
			AddPortalToNodes( frontportal, othernode, node->child[0] );
			AddPortalToNodes( backportal, othernode, node->child[1] );
		}
		else
		{
			AddPortalToNodes( frontportal, node->child[0], othernode );
			AddPortalToNodes( backportal, node->child[1], othernode );
		}
	}

	node->portals = NULL;
}



/*
  ====================
  Portal_MakeNodesRecursive

  ====================
*/
void Portal_MakeNodesRecursive( node_t *node )
{       
	if ( node->plane < 0 )
		// it's a leaf
		return;

	if ( !Portal_MakeNodeChildren( node ) )
	{
		printf( " * Portal_MakeNodesRecursive: can't get portal for node. go up. *\n" );


		return;
	}

	Portal_SplitNode( node );

	Portal_MakeNodesRecursive( node->child[0] );
	Portal_MakeNodesRecursive( node->child[1] );
}



/*
  ====================
  Write_Portal

  ====================
*/
void Write_Portal( FILE *h, portal_t *p )
{
	int		i;

	Write_Vec3d( h, p->plane.norm );
	Write_fp( h, p->plane.dist );
	
	// write front leaf, back leaf
	fprintf( h, "%d ", (p->nodes[0] == &out_node) ? (-1) : (p->nodes[0] - p_nodes) );
	fprintf( h, "%d ", (p->nodes[1] == &out_node) ? (-1) : (p->nodes[1] - p_nodes) );

//	fprintf( h, "%d ", p->nodes[0]->index );
//	fprintf( h, "%d ", p->nodes[1]->index );

	// write polygon
	Write_Polygon( h, p->p );
}



/*
  ====================
  Write_Portals

  ====================
*/
void Write_Portals( char *name, char *creator )
{
	FILE		*h;
	int		i;

	h = fopen( name, "w" );
	if ( !h )
		Error( "Write_Portals: can't open file '%s'\n", name );

	fprintf( h, "# portal file\n" );
	fprintf( h, "# generated by %s !!! DON'T EDIT !!!\n", creator ); 
	fprintf( h, "# <portal> <norm> <dist> <front leaf> <back leaf> <polygon>\n" );
	
	for ( i = 0; i < p_portalnum; i++ )
	{
		fprintf( h, "%d ", i );
		Write_Portal( h, &p_portals[i] );
		fprintf( h, "\n" );
	}

	fprintf( h, "end\n" );

	fclose( h );
}



/*
  ====================
  Write_NodePortals

  ====================
*/
void Write_NodePortals( char *name, char *creator )
{
	FILE		*h;
	int		i;
	portal_t	*p, *pnext;

	h = fopen( name, "w" );
	if ( !h )
		Error( "Write_NodePortals: can't open file '%s'\n", name );

	fprintf( h, "# nodeportal file\n" );
	fprintf( h, "# generated by %s !!! DON'T EDIT !!!\n", creator ); 
	fprintf( h, "# <node> <portalnum> [ <portal> ]\n" );
	
	for ( i = 0; i < p_nodenum; i++ )
	{
		fprintf( h, "%d %d ", i, p_nodes[i].portalnum );
		
		for ( p = p_nodes[i].portals; p ; p=pnext )
		{
			if ( p->nodes[0] == &p_nodes[i] )
				pnext = p->next[0];
			else if ( p->nodes[1] == &p_nodes[i] )
				pnext = p->next[1];
			else
				Error( "nnip\n" ); // node not in portal
			fprintf( h, "%d ", p - p_portals );
		}

		fprintf( h, "\n" );
	}

	fprintf( h, "end\n" );
	
	fclose( h );
}
int main( int argc, char *argv[] )
{
	int		i;
	
	char		*in_node_name;
	char		*out_portal_name;
	char		*out_node_portal_name;

	printf( "===== portal - build portals for a bsp tree =====\n" );

	SetCmdArgs( argc, argv );

	in_node_name = GetCmdOpt2( "-i" );
	out_portal_name = GetCmdOpt2( "-o" );
	out_node_portal_name = GetCmdOpt2( "-n" );

	if ( !in_node_name )
		Error( "no input node file.\n" );
	if ( !out_portal_name )
		Error( "no output portal file.\n" );
	if ( !out_node_portal_name )
		Error( "no output node portal file.\n" );

	printf( " input node file: %s\n", in_node_name );
	printf( " output portal file: %s\n", out_portal_name );
	printf( " output node portal file: %s\n", out_node_portal_name );

	p_planenum = MAX_PLANES;
	Read_Planes( "planes.asc", p_planes, &p_planenum );
	printf( " %d planes\n", p_planenum );

	p_nodenum = MAX_NODES;
	Read_NodeArray( p_bspnodes, &p_nodenum, in_node_name );
	printf ( " %d nodes\n", p_nodenum );

	printf( " portalize tree ...\n" );

	// setup special nodes
	for ( i = 0; i < p_nodenum; i++ )
	{
		p_nodes[i].plane = p_bspnodes[i].plane;
		p_nodes[i].index = i;
		p_nodes[i].portalnum = 0;
		if ( p_nodes[i].plane < 0 )
		{
			p_nodes[i].child[0] = NULL;
			p_nodes[i].child[1] = NULL;
		}
		else
		{
			p_nodes[i].child[0] = &p_nodes[p_bspnodes[i].child[0]];
			p_nodes[i].child[1] = &p_nodes[p_bspnodes[i].child[1]];
		}

		p_nodes[i].portals = NULL;
	}

	Portal_MakeHeadNode( &p_nodes[0] );
	Portal_MakeNodesRecursive( &p_nodes[0] );

	printf( " build %d portals\n", p_portalnum );
	printf( " writing files ...\n" );

	Write_Portals( out_portal_name, "portal" );
	Write_NodePortals( out_node_portal_name, "portal" );
	     
}
