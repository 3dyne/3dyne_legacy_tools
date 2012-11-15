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



// test_container.c

#include "lib_container.h"

void Test_list()
{
	u_list_t	*list;
	u_list_iter_t	iter;

	char		*item1 = "item1";
	char		*item2 = "item2";
	char		*item3 = "item3";
	char		*item4 = "item4";
	char		*item;

	list = U_NewList();

	U_ListIterInit( &iter, list );
	U_ListIterInsertAsPrev( &iter, item1 );
	U_ListIterInsertAsPrev( &iter, item2 );
	U_ListIterInsertAsPrev( &iter, item3 );

	U_ListIterToHead( &iter );
	for ( ; ( item = U_ListIterNext( &iter ) ); )
	{
		printf( "item: %s\n", item );
		if ( item == item2 )
		{
			U_ListIterInsertAsNext( &iter, item4 );
		}
	}

	printf( "\n" );

	U_ListIterToHead( &iter );
	for ( ; ( item = U_ListIterNext( &iter ) ); )
	{
		printf( "item: %s\n", item );
	}
}

typedef struct testobj_s
{
	int	key;
	char	value[32];
} testobj_t;

void * Testobj_get_key_func( const void *obj )
{
	return (void*)&(((testobj_t*)(obj))->key);       
}

int Testobj_key_compare_func( const void *k1, const void *k2 )
{
//	return strcmp( (char*)(k1), (char*)(k2) );
	if ( *(int*)k1 < *(int*)k2 )
		return -1;
	else if ( *(int*)k1 > *(int*)k2 )
		return 1;
	return 0;
}

void Test_map()
{
	u_map_t		*map;

	testobj_t	o1;
	testobj_t	o2;
	testobj_t	o3;
	
	testobj_t	*obj;

	strcpy( o1.key, "o1" );
	strcpy( o2.key, "o2" );
	strcpy( o3.key, "o3" );
	
	strcpy( o1.value, "i am o1" );
	strcpy( o2.value, "i am o2" );
	strcpy( o3.value, "i am o3" );

	printf( "Test_map:\n" );

	map = U_NewMap( map_linear, Testobj_key_compare_func, Testobj_get_key_func );
	U_MapInsert( map, &o1 );
	U_MapInsert( map, &o2 );
	U_MapInsert( map, &o3 );

	obj = U_MapSearch( map, "o1" );
	printf( "obj: %p\n", obj );
	printf( "key: '%s', value '%s'\n", obj->key, obj->value );
}

void PrintTestobj( void *obj )
{
	testobj_t	*t;

	if ( !obj )
		printf( "PrintTestobj: obj is null\n" );

	t = (testobj_t *)obj;
	printf( "key %d, value '%s'\n", t->key, t->value );
	
}

void Test_avl_tree()
{
	int		i, j;
	u_avl_tree_t	*tree;
	char		value[32];
	testobj_t		*res;
	int		max_deep;

	testobj_t		names[100000];

	tree = U_NewAVLTree( Testobj_key_compare_func, Testobj_get_key_func );

	for ( i = 0; i < 10; i++ )
	{
		for(;;)
		{
			names[i].key = random()&0xffffff;
			sprintf( names[i].value, "i am %d", names[i].key );

			if ( U_AVLTreeInsert( tree, &names[i] ) )
				break;
		}
	}		

	U_AVLTreeForEach( tree, PrintTestobj );

#if 0
	for ( i = 0; i < 50000; i++ )
	{
		U_AVLTreeRemove( tree, &names[i].key );
	}

//	max_deep = -1;
	for ( i = 50000; i < 100000; i++ )
	{
		int	deep;
		res = U_AVLTreeSearch( tree, &names[i].key );
		if ( !res )
			Error( "missing key\n" );
//		printf( "key: %d '%s' deep %d\n", i, res->value, deep );
	}	
#endif
}

int main()
{
	// Test_list();

	// Test_map();

	Test_avl_tree();
}
