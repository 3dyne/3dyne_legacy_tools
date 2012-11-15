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



// lib_rbtree.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lib_rbtree.h"
#include "lib_hobj.h"
#include "lib_error.h"

#define NEW( x )        ( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )
#define FREE( x )       ( free( x ) )

/*
  ==============================
  RBTree_New

  ==============================
*/

rbtree_t * RBTree_New( int (*compar)(void *, void *) )
{
	rbtree_t		*tree;

	tree = NEW(rbtree_t);
	tree->compar = compar;      

	tree->zkey = 0;
	tree->hkey = -1;
	tree->z.key = &tree->zkey;
	tree->z.child[0] = &tree->z;
	tree->z.child[1] = &tree->z;
	tree->head.key = &tree->hkey;
	tree->head.child[1] = &tree->z;

	return tree;
}

void RBTree_Free( rbtree_t *tree )
{
	FREE( tree );
}


/*
  ==============================
  RBTree_NewNode

  ==============================
*/
static int	total_node_mem = 0;

rbnode_t * RBTree_NewNode( void *key, void *data, rbnodeColor color, rbnode_t *c1, rbnode_t *c2 )
{
	rbnode_t	*node;

	node = NEW( rbnode_t );
	node->key = key;
	node->data = data;
	node->color = color;
	node->child[0] = c1;
	node->child[1] = c2;

	total_node_mem += sizeof( rbnode_t );

	return node;
}

void RBTree_FreeNode( rbnode_t *node )
{
	total_node_mem -= sizeof( rbnode_t );
	FREE( node );
}

/*
  ==============================
  RBTree_Search

  ==============================
*/
void * RBTree_Search( rbtree_t *tree, void *key )
{
	rbnode_t	*x;
	x = tree->head.child[1];

	for(;;)
	{
		int	cmp;
		cmp = tree->compar( key, x->key );
		if ( cmp == 0 )
			return x->data;
		x = ( cmp < 0 ) ? x->child[0] : x->child[1];		
	}
	return NULL;
}

/*
  ==============================
  RBTree_InsertNode

  ==============================
*/

inline rbnode_t * NodeRotate( rbtree_t *tree, void *key, rbnode_t *y )
{
	rbnode_t	*c, *gc;
	int		cmp1, cmp2;

	cmp1 = tree->compar( key, y->key );
	c = ( cmp1 < 0 ) ? y->child[0] : y->child[1];
	cmp2 = tree->compar( key, c->key );
	
	if ( cmp2 < 0 )
	{
		gc = c->child[0];
		c->child[0] = gc->child[1];
		gc->child[1] = c;
	}
	else
	{
		gc = c->child[1];
		c->child[1] = gc->child[0];
		gc->child[0] = c;
	}
	if ( cmp1 < 0 )
	{
		y->child[0] = gc;
	}
	else
	{
		y->child[1] = gc;
	}

	return gc;
}

inline void NodeSplit( rbtree_t *tree, void *key, rbnode_t *x, rbnode_t *p, rbnode_t *g, rbnode_t *gg )
{
	int		cmp1, cmp2;

	x->color = red;
	x->child[0]->color = black;
	x->child[1]->color = black;

	if ( p->color == red )
	{
		g->color = red;
		cmp1 = tree->compar( key, g->key );
		cmp2 = tree->compar( key, p->key );

		if ( cmp1 != cmp2 )
			p = NodeRotate( tree, key, g );
		x = NodeRotate( tree, key, gg );
		x->color = black;		
	}
}

void RBTree_InsertNode( rbtree_t *tree, void *key, void *data )
{
	rbnode_t	*x, *p, *g, *gg;
	int		cmp;
	int		deep = 0;

	x = p = g = &tree->head;

	while( x != &tree->z )
	{
		gg = g;
		g = p;
		p = x;

		cmp = tree->compar( key, x->key );
		if ( cmp == 0 )
			Error( "key already in tree.\n" );
		x = ( cmp < 0 ) ? x->child[0] : x->child[1];
		if ( x->child[0]->color == red && x->child[1]->color == red )
		{
			NodeSplit( tree, key, x, p, g, gg );			
		}
		deep++;
	}

	x = RBTree_NewNode( key, data, red, &tree->z, &tree->z );
	cmp = tree->compar( key, p->key );
	if ( cmp < 0 )
		p->child[0] = x;
	else
		p->child[1] = x;
	NodeSplit( tree, key, x, p, g, gg );
	tree->head.child[1]->color = black;	
	
	if ( deep > tree->maxdeep )
		tree->maxdeep = deep;
}


/*
  ==============================
  RBTree_RotateLeft

  ==============================
*/
rbnode_t * RBTree_RotateLeft( rbnode_t *node )
{
	rbnode_t	*c2;

	c2 = node->child[1];

	node->child[1] = c2->child[0];
	c2->child[0] = node;

	return c2;
}

/*
  ==============================
  RBTree_RotateRight

  ==============================
*/
rbnode_t * RBTree_RotateRight( rbnode_t *node )
{
	rbnode_t	*c1;

	c1 = node->child[0];

	node->child[0] = c1->child[1];
	c1->child[1] = node;

	return c1;		
}



/*
  ==================================================
  test stuff

  ==================================================
*/

typedef struct data_key_s
{
	int		key;
	hobj_t		*obj;

	struct data_key_s *next;
} data_key_t;

data_key_t	*build_list;
int		total_class_num;

void BuildListRecursive( hobj_t *obj )
{
	data_key_t	*dkey;
	hobj_t		*o;

	total_class_num++;
	dkey = NEW( data_key_t );
	dkey->key = atoi( &obj->name[1] );
	dkey->obj = obj;
	dkey->next = build_list;
	build_list = dkey;

	for ( o = obj->hobjs; o ; o=o->next )
	{
		BuildListRecursive( o );
	}
}

data_key_t * BuildListFromClass( hobj_t *obj )
{
	build_list = NULL;
	total_class_num = 0;
	
	BuildListRecursive( obj );
	printf( "BuildListFromClass: %d classes\n", total_class_num );

	return build_list;
}

int key_compar( void *key1, void *key2 )
{
	if ( *((int *)key1) > *((int *)key2) )
		return 1;
	else if ( *((int *)key1) < *((int *)key2) )
	{
		return -1;
	}
	else
		return 0;
}

int main()
{
	hmanager_t	*testhm;
	data_key_t	*list;
	rbtree_t		*tree;

	testhm = NewHManagerLoadClass( "bigtest.flat" );

	list = BuildListFromClass( HManagerGetRootClass( testhm ) );

	tree = RBTree_New( key_compar );
	// insert all data_key_t
	printf( "insert ...\n" );
	{
		data_key_t	*k;
		for ( k = list; k ; k=k->next )
		{
			RBTree_InsertNode( tree, &k->key, k->obj );
		}
	}

	// find all data_key_t 
	printf( "search ...\n" );
	{
		data_key_t	*k;
		for ( k = list; k ; k=k->next )
		{
			void	*data;
			data = RBTree_Search( tree, &k->key );
			if ( !data )
				Error( "not found\n" );
			if ( data != k->obj )
				Error( "wrong found\n" );
		}
	}

	printf( "maxdeep %d\n", tree->maxdeep );
	printf( " %.2f kb rbtree overhead\n", total_node_mem / 1024.0 );
}
