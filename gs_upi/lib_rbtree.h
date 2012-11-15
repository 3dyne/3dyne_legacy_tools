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



// lib_rbtree.h

#ifndef __lib_rbtree
#define __lib_rbtree

typedef enum {
	black = 0,
	red = 1
} rbnodeColor;

typedef struct rbnode_s
{
	void		*key;
	void		*data;
	
	rbnodeColor	color;
	struct rbnode_s	*child[2];
} rbnode_t;

typedef struct rbtree_s
{
	rbnode_t	head;
	rbnode_t	z;
	int		zkey;
	int		hkey;

	int		maxdeep;
	int		(*compar)(void *, void *);
} rbtree_t;

rbtree_t * RBTree_New( int (*compar)(void *, void *) );
void RBTree_Free( rbtree_t *tree );

rbnode_t * RBTree_NewNode( void *key, void *data, rbnodeColor color, rbnode_t *c1, rbnode_t *c2 );
void RBTree_FreeNode( rbnode_t *node );

rbnode_t * RBTree_RotateLeft( rbnode_t *node );
rbnode_t * RBTree_RotateRight( rbnode_t *node );

void RBTree_InsertNode( rbtree_t *tree, void *key, void *data );
rbnode_t * RBTree_FindNode( rbtree_t *tree, void *key );
void RBTree_BalanceInsert( rbtree_t *tree, rbnode_t *ins );

#endif
