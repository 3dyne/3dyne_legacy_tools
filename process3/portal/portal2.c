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



// portal2.c

#include "portal2.h"

int		p_planenum;
//plane_t		p_planes[MAX_PLANES];

int		p_nodenum = 0;
//bspnode_t	p_bspnodes[MAX_NODES];
//node_t		p_nodes[MAX_NODES];

int		p_portalnum = 0;
//portal_t	p_portals[MAX_PORTALS];


#define NEW( x )	( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )

cnode_t * NewNode( void )
{
	cnode_t		*node;

	node = NEW( cnode_t );
	p_nodenum++;
	return node;
}

void FreeNode( cnode_t *node )
{
	p_nodenum--;
	free( node );
}


/*
  ========================================
  
  internal portal stuff

  ========================================
*/

portal_t*	NewPortal( void )
{
	portal_t	*portal;

	p_portalnum++;
	portal = NEW( portal_t );
	return portal;
}

/*
  ====================
  AddPortalToNodes

  ====================
*/
void AddPortalToNodes( portal_t *p, cnode_t *front, cnode_t *back )
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
void RemovePortalFromNode( portal_t *p, cnode_t *n )
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

cnode_t		*out_node;

cplane_t	out_planes[6];



//
//#define		BIG_BOX		( 128.0*256.0 - 1024.0 )
// 
// BIG_BOX moved to shared/defs.h !

void Portal_MakeHeadNode( cnode_t *n )
{
	int		i, j;
	// 6 planes
//	vec3d_t		norms[6];
//	fp_t		dists[6];
	portal_t	*portals[6];

	//
	// make outnode for tree
	//

//	tree->outnode = NewNode();
	out_node = NewNode();
	out_node->type = NodeType_outside;
	out_node->child[0] = NULL;
	out_node->child[1] = NULL;
	out_node->portals = NULL;

//	tree->outnode->contents = BRUSH_CONTENTS_EMPTY;

	for ( i = 0; i < 3; i++ )
	{
		Vec3dInit( out_planes[i].norm, 0.0, 0.0, 0.0 );
		out_planes[i].norm[i] = 1.0;
		out_planes[i].dist = BIG_BOX; //tree->max[i] + 64.0;
		out_planes[i].self = NULL;
		
		Vec3dInit( out_planes[i+3].norm, 0.0, 0.0, 0.0 );
		out_planes[i+3].norm[i] = -1.0;
		out_planes[i+3].dist = BIG_BOX; //- (tree->min[i] - 64.0);
		out_planes[i+3].self = NULL;
	}

	for ( i = 0; i < 6; i++ )
	{
		portals[i] = NewPortal();
//		Vec3dCopy( portals[i]->norm, norms[i] );
//		portals[i]->dist = dists[i];
		portals[i]->pl = &out_planes[i];

		portals[i]->p = BasePolygonForPlane( out_planes[i].norm, out_planes[i].dist );
		AddPortalToNodes( portals[i], out_node, n );
		
	}

	for ( i = 0; i < 6; i++ )
	{
		for ( j = 0; j < 6; j++ )
		{
			if( i==j )
				continue;
			if ( portals[i]->p )
				ClipPolygonInPlace( &portals[i]->p, out_planes[i].norm, out_planes[i].dist );
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

bool_t Portal_MakeNodeChildren( cnode_t *node )
{
	cplane_t		*pl;
	polygon_t	*poly;
	portal_t	*p;

	int		side;
	portal_t	*pnew;

//	vec3d_t		norm;
//	fp_t		dist;
	cplane_t	*pln;


	if ( node->type == NodeType_leaf )
		Error( "Portal_MakeNodeChildren: oops, it's a leaf.\n" );

	pl = node->pl;

	poly = BasePolygonForPlane( pl->norm, pl->dist );

	//
	// clip polygon by all portal planes of this node
	// 

//	printf( "Portal_MakeNodeChildren: " );
	side = 0;
//	dist = 0.0;
	for ( p = node->portals; p ; p=p->next[side] )
	{	 

//		printf("*");
		if ( p->nodes[0] == node )
		{
			side = 0;
//			Vec3dFlip( norm, p->norm );
//			dist = -p->dist;

			pln = p->pl->flipplane;
		}
		else if ( p->nodes[1] == node )
		{
			side = 1;
//			Vec3dCopy( norm, p->norm );
//			dist = p->dist;
			
			pln = p->pl;
		}
		else
		{
			Error( "Portal_MakeNodeChildren: node not found in portal.\n" );		      			
		}

		if ( pl == p->pl->flipplane || pl == p->pl )
			Error( "split plane == portal plane\n" );

		if ( poly )
			ClipPolygonInPlace( &poly, pln->norm, pln->dist );
		       
	}

//	printf( "\n" );

	if ( !poly )
	{
		printf( " * Portal_MakeNodeChildren: clipped base polygon. *\n" );
		return false;
	}

	pnew = NewPortal();
	pnew->p = poly;
	pnew->pl = pl;
//	Vec3dCopy( pnew->norm, pl->norm );
//	pnew->dist = pl->dist;
//	memcpy( &pnew->plane, pl, sizeof( plane_t ) );
	AddPortalToNodes( pnew, node->child[0], node->child[1] );

	return true;
}



/*
  ====================
  Portal_SplitNode

  ====================
*/
void Portal_SplitNode( cnode_t *node )
{
	cplane_t		*pl;
	portal_t		*p, *pnext;
	int		side;
	cnode_t		*othernode;
	portal_t	*frontportal, *backportal;
	polygon_t	*front, *back;

	pl = node->pl;

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

int	portalized_num = 0;
FILE	*failed_file;

void DebugWriteNodePortals( cnode_t *node  )
{
	int		i;
	portal_t		*p, *pnext;
	int		side;

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
		
		fprintf( failed_file, "%d : ", p->p->pointnum );
		for ( i = 0; i < p->p->pointnum; i++ )
		{
			fprintf( failed_file, "( %f %f %f ) ", p->p->p[i][0], p->p->p[i][1], p->p->p[i][2] );
		}
		fprintf( failed_file, "\n" );
	}
}

void Portal_MakeNodesRecursive( cnode_t *node, int stop_depth )
{       
	hpair_t		*pair;

//	if ( node->plane < 0 )
	if ( node->type == NodeType_leaf )
	{
		// it's a leaf
		pair = NewHPair2( "string", "portalized_leaf", "normal_leaf" );
		InsertHPair( node->self, pair );		
		portalized_num++;
		return;
	}

#if 0
	// hack: contents deep portalization
	if ( node->contents < stop_depth && node->type == NodeType_node )
	{
		pair = NewHPair2( "string", "portalized_leaf", "deep_stop" );
		InsertHPair( node->self, pair );
		portalized_num++;
		return;
	}
#endif

	if ( !Portal_MakeNodeChildren( node ) )
	{
		printf( " * Portal_MakeNodesRecursive: can't get portal for node '%s'. go up. *\n", node->self->name );
#if 1
		// test:
		// if portalization fail for a sub-tree, ignore it
		// and remove all portals, to the *node*
		{
			portal_t	*p, *next;
			int		side;

			DebugWriteNodePortals( node );
			
			for ( p = node->portals; p ; p=next )
			{
				printf( "." );
				{
					if ( p->nodes[0] == node )
						side = 0;
					else if ( p->nodes[1] == node )
						side = 1;
					else
						Error( "Portal_SplitNode: can't find node in portal.\n" );
					
					next = p->next[side];
					
					RemovePortalFromNode( p, p->nodes[0] );
					RemovePortalFromNode( p, p->nodes[1] );
					free( p );
				}
			}
		}
#endif
		pair = NewHPair2( "string", "portalized_leaf", "node_split_failed" );
		InsertHPair( node->self, pair );
		portalized_num++;
		return;
	}




	Portal_SplitNode( node );

	Portal_MakeNodesRecursive( node->child[0], stop_depth );
	Portal_MakeNodesRecursive( node->child[1], stop_depth );
}




/*
  ==================================================
  class stuff

  ==================================================
*/

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
		pl->count = 0;

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
  ====================
  ReadNodeClass

  ====================
*/




void BuildNodeClassRecursive( hobj_t *node, hmanager_t *planecls )
{
	cnode_t		*n;
	hobj_t		*child;
	hobj_t		*plane;
	hpair_t		*pair;

	// first remove data of an older portalization
	RemoveAndDestroyAllHPairsOfKey( node, "portal" );
	RemoveAndDestroyAllHPairsOfKey( node, "portalized_leaf" );

	n = NewNode();
	SetClassExtra( node, n );
	n->self = node;

	pair = FindHPair( node, "plane" );
	if ( !pair )
	{
		// leaf
		pair = FindHPair( node, "contents" );
		if ( !pair )
			Error( "missing 'contents' in node '%s'.\n", node->name );
		HPairCastToInt( &n->contents, pair );
		
		n->type = NodeType_leaf;
		
	}
	else
	{
		// node

		n->type = NodeType_node;

		plane = HManagerSearchClassName( planecls, pair->value );
		if ( !plane )
			Error( "node '%s' can't find plane '%s'.\n", node->name, pair->value );
		n->pl = GetClassExtra( plane );
		

		child = FindClassType( node, "bspnode_front" );
		if ( !child )
			Error( "missing 'bspnode_front' in node class.\n" );
		BuildNodeClassRecursive( child, planecls );
		n->child[0] = GetClassExtra( child );

		child = FindClassType( node, "bspnode_back" );
		if ( !child )
			Error( "missing 'bspnode_back' in node class.\n" );
		BuildNodeClassRecursive( child, planecls );
		n->child[1] = GetClassExtra( child );
	}
}

hmanager_t * ReadNodeClass( char *name, hmanager_t *planecls )
{
	tokenstream_t	*ts;
	hobj_t		*nodecls;
	hmanager_t	*hm;

	printf( "load and compile bspnode class ...\n" );
	
	ts = BeginTokenStream( name );
	nodecls = ReadClass( ts );
	EndTokenStream( ts );

	hm = NewHManager();
	HManagerSetRootClass( hm, nodecls );
	HManagerRebuildHash( hm );

	BuildNodeClassRecursive( nodecls, planecls );

	printf( " %d nodes\n", p_nodenum );

	return hm;
}

/*
  ====================
  BuildPortalClass

  ====================
*/
hobj_t * BuildPortalClass( portal_t *p )
{
	hobj_t		*portal;
	hpair_t		*pair;
	int		i;
	char		tt[256];
	
	sprintf( tt, "#%u", HManagerGetFreeID() );
	portal = NewClass( "portal", tt );

	sprintf( tt, "%s", p->nodes[0]->self->name );
	pair = NewHPair2( "ref", "frontnode", tt );
	InsertHPair( portal, pair );

	pair = FindHPair( p->nodes[0]->self, "contents" );
	if ( !pair )
		Error( "missing 'contents' in node '%s'.\n", p->nodes[0]->self->name );
	pair = NewHPair2( "int", "front_contents", pair->value );
	InsertHPair( portal, pair );

	sprintf( tt, "%s", p->nodes[1]->self->name );
	pair = NewHPair2( "ref", "backnode", tt );
	InsertHPair( portal, pair );

	pair = FindHPair( p->nodes[1]->self, "contents" );
	if ( !pair )
		Error( "missing 'contents' in node '%s'.\n", p->nodes[1]->self->name );
	pair = NewHPair2( "int", "back_contents", pair->value );
	InsertHPair( portal, pair );

#if 0
	sprintf( tt, "%f %f %f", p->norm[0], p->norm[1], p->norm[2] );
	pair = NewHPair2( "vec3d", "norm", tt );
	InsertHPair( portal, pair );

	sprintf( tt, "%f", p->dist );
	pair = NewHPair2( "float", "dist", tt );
	InsertHPair( portal, pair );
#else
	sprintf( tt, "%s", p->pl->self->name );
	pair = NewHPair2( "ref", "plane", tt );
	InsertHPair( portal, pair );

#endif


	sprintf( tt, "%d", p->p->pointnum );
	pair = NewHPair2( "int", "pointnum", tt );
	InsertHPair( portal, pair );       

	for ( i = 0; i < p->p->pointnum; i++ )
	{
		pair = NewHPair( );
		sprintf( pair->type, "vec3d" );
		sprintf( pair->key, "%d", i );
		HPairCastFromVec3d( p->p->p[i], pair );
//		sprintf( pair->value, "%f %f %f", p->p->p[i][0], p->p->p[i][1], p->p->p[i][2] );
		InsertHPair( portal, pair );
	}

	p->self = portal;

	return portal;
}

void BuildPortalClassRecursive( cnode_t *node, hobj_t *portalcls )
{        
	portal_t		*p, *pnext;
	cnode_t			*othernode;
	hpair_t			*pair;
	int			side;

	pair = FindHPair( node->self, "portalized_leaf" );
	if ( !pair /*node->type == NodeType_node*/ )
	{
		// node 

		BuildPortalClassRecursive( node->child[0], portalcls );
		BuildPortalClassRecursive( node->child[1], portalcls );
	}
	else
	{
		// leaf

		// first remove all old clsrefs to portals
		RemoveAndDestroyAllHPairsOfKey( node->self, "portal" );

		// insert new portals
		for ( p = node->portals; p; p=pnext )
		{
			if ( p->nodes[0] == node )
				side = 0;
			else if ( p->nodes[1] == node )
				side = 1;
			else
				Error( "Portal_SplitNode: can't find node in portal.\n" );
			
			pnext = p->next[side];
			othernode = p->nodes[!side];

			if ( othernode->type == NodeType_outside )
			{
				pair = NewHPair2( "int", "touch_outside", "1" );
				InsertHPair( node->self, pair );
				continue;
			}

			if ( !p->self )
			{
				// needs a portal
				InsertClass( portalcls, BuildPortalClass( p ) );
			}

			// clsref to portal into node
			pair = NewHPair2( "ref", "portal", p->self->name );
			InsertHPair( node->self, pair ); 			
		}
	}
}

void WritePortalClass( char *portal_name, char *node_name, cnode_t *topnode )
{
	hobj_t		*portalcls;
	FILE		*h;

	portalcls = NewClass( "portals", "portals0" );
	BuildPortalClassRecursive( topnode, portalcls );
	
	DeepDumpClass( portalcls );
	DeepDumpClass( topnode->self );

	h = fopen( portal_name, "w" );
	WriteClass( portalcls, h );
	fclose( h );

	h = fopen( node_name, "w" );
	WriteClass( topnode->self, h );
	fclose( h );
}


int main( int argc, char *argv[] )
{
	int		i;
	
	char		*in_node_name;
	char		*out_portal_name;
	char		*out_node_name;
	char		*in_plane_name;

	hmanager_t	*planecls;
	hmanager_t	*nodecls;

	void	*topnode;
	int		stop_depth;

	printf( "===== portal - build portals for a bsp tree =====\n" );

	SetCmdArgs( argc, argv );
//	SetEpsilon( 0.000001, -0.000001 );

	in_node_name = GetCmdOpt2( "-i" );
	out_node_name = GetCmdOpt2( "-o" );
	out_portal_name = GetCmdOpt2( "-p" );
	in_plane_name = GetCmdOpt2( "-pl" );       
//	out_node_portal_name = GetCmdOpt2( "-n" );
	
	if ( GetCmdOpt2( "--stop-depth" ) )
	{
		stop_depth = atoi( GetCmdOpt2( "--stop-depth" ) );
		printf ( "Switch: set portalization max depth to %d\n", stop_depth );
	}
	else
	{
		stop_depth = 0;
		printf ( "default portalization max depth id %d\n", stop_depth );
	}

	if ( !in_node_name )
	{
		Error( "no input node file.\n" );
	}
	else
	{
		printf( " input bspnode class: %s\n", in_node_name );
	}

	if ( !out_node_name )
	{
		out_node_name = "_portalout_bspnode.hobj";
		printf( " default output bspnode class: %s\n", out_node_name );
	}
	else
	{
		printf( " output bspnode class: %s\n", out_node_name );
	}

	if ( !out_portal_name )
	{		
		out_portal_name = "_portalout_portal.hobj";
		printf( " default output portal class: %s\n", out_portal_name );
	}
	else
	{
		printf( " output portal class: %s\n", out_portal_name );
	}

	if ( !in_plane_name )
	{
		in_plane_name = "_plane.hobj";
		printf( " default input plane class: %s\n", in_plane_name );
	}
	else
	{
		printf( " input plane class: %s\n", in_plane_name );
	}

	planecls = ReadPlaneClass( in_plane_name );
	nodecls = ReadNodeClass( in_node_name, planecls );

	topnode = GetClassExtra( HManagerGetRootClass( nodecls ) );
	printf( " topnode %p\n", topnode );

//	getchar();

	printf( "portalize tree ...\n" );

#if 0
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
#endif

	failed_file = fopen( "portalize_fail", "w" );
	if ( !failed_file )
		Error( "can't open portalize_fail file.\n" );

	Portal_MakeHeadNode( topnode );
	Portal_MakeNodesRecursive( topnode, stop_depth );

	fprintf( failed_file, "end" );
	fclose ( failed_file );

	printf( " build %d portals\n", p_portalnum );
	printf( " %d portalized leafs\n", portalized_num );
	printf( "build portal class and update bspnode class ...\n" );

	WritePortalClass( out_portal_name, out_node_name, topnode );
//	Write_NodePortals( out_node_portal_name, "portal" );

	HManagerSaveID();

	exit(0);	     
}
