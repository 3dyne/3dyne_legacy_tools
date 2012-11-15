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



// WWM.hh

#ifndef __WWM_included
#define __WWM_included

#include "lib_math.h"
#include "lib_container.h"

#include "vec.h"
#include "brush.h"
#include "archetype.h"

#include "EditAble.hh"
#include "TestBox.hh"
#include "CSurface.hh"
#include "CPoly.hh"

#define		SELECT_NORMAL	( 0 )
#define		SELECT_RED	( 1 )
#define		SELECT_GREEN	( 2 )

#define		SELECT_BLUE	( 4 )
#define		SELECT_PRE	( 8 )

#define	        SELECT_VISIBLE	( 256 )
#define		SELECT_UPDATE	( 512 )


#define BS_BUMMER	( 0 ) // not allowed
#define BS_NORMAL	( 1 )
//#define BS_PRESELECT	( 2 )
#define BS_SINGLESELECT	( 3 )
#define BS_REDSELECT	( 4 )
#define BS_GREENSELECT	( 5 )

#define OS_BUMMER	( 0 )
#define OS_NORMAL	( 1 )
#define OS_SINGLESELECT	( 3 )
#define OS_REDSELECT	( 4 )
#define OS_GREENSELECT	( 5 )

class WWM;

extern WWM		*wwm_i;

/*
  ===============================================
  class: WWM			       
  ===============================================
*/

// unique id
#define ID_START_AUTO_ARCHETYPE		( 45000 )
#define ID_START_USER_ARCHETYPE		( 47000 )
#define ID_START_AUTO_FACE		( 10000 )
#define ID_START_BRUSH			( 0 )
#define ID_START_AUTO_CSURFACE		( 49000 )
#define ID_START_AUTO_CPOLY		( 49500 )

class WWM {

public:
	WWM();
	~WWM();

	unique_t		getID( void );
	void			registerID( unique_t id );
	void			saveID( const char *name );
	void			loadID( const char *name );

	void			validateAllIDs( void );

	//
	// brush stuff
	//

	void			addBrush( brush_t *brush, bool_t add_selected = false );
	void			removeBrush( brush_t *brush );

	int			getBrushNum( void );
	brush_t*		getFirstBrush( void );

	void			findBestBrushForRay( vec3d_t from, vec3d_t to, brush_t **brush, face_t **face, bool_t in_selection = false );

	void			headBrush( brush_t *brush );			

	// move all selections to the head
	void			reorderBrushes( void );

	void			loadWire( const char *name );
	void			saveWire( const char *name );

	// todo: better selection handling
//	brush_t*		getSelectedBrushes( void );
	void			deselectBrushes( void );
	void			selectByRay( vec3d_t start, vec3d_t dir );
	bool_t			intersectBrushAndRay( brush_t *b, vec3d_t start, vec3d_t dir );

	//
	// archetype stuff
	//

	arche_t *		searchArche( char *name );

	void			addArche( arche_t *arche, bool_t add_selected = false );
	void			removeArche( arche_t *arche );
	arche_t*		getFirstArche( void );

	void			deselectArches( void );
	bool_t			intersectArcheAndRay( arche_t *a, vec3d_t start, vec3d_t dir );
	void			selectArcheByRay( vec3d_t start, vec3d_t dir );

	void			saveArcheClass( const char *name );
	void			loadArcheClass( const char *name );
	void 			loadArcheFromOldAts( const char* name );
	//

	void			dump();

	void			allUpdateFlagsTrue( void );
	void			viewBoundsChanged( vec3d_t vmin, vec3d_t vmax );

	//
	// TestBox stuff
	//
	void			insertTestBox( TestBox *obj );
	void			removeTestBox( TestBox *obj );
	TestBox*		getFirstTestBox( void );

	//
	// CSurface stuff
	//
	void			insertCSurface( CSurface *obj, bool add_selected = false );
	void			removeCSurface( CSurface *obj );
	CSurface*		getFirstCSurface( void );
	
	void			saveCSurfaceClass( const char *name );
	void			loadCSurfaceClass( const char *name );

	//
	// CPoly stuff
	//
	void			insertCPoly( CPoly *obj, bool add_selected = false );
	void			removeCPoly( CPoly *obj );
	CPoly*			getFirstCPoly( void );

	void			findBestCPolyForRay( vec3d_t from, vec3d_t dir, CPoly **cpoly );
	
	void			saveCPolyClass( const char *name );
	void			loadCPolyClass( const char *name );

	void			checkConsistency( void );

private:
	unsigned int			next_free_id;

	int			brushnum;
	brush_t			*brushes;

	int			brushprenum;	// number of pre-selected brushes
	int			brushpre;	// number of current blue in pre-selected brushes

	vec3d_t			viewmin, viewmax;

	//
	// archetype stuff
	//

	int			archenum;
	arche_t			*arches;

	vec3d_t			arche_pos;	// temp brush origin, needed to calc delta of moveBrush
	brush_t			*arche_brush;	// temp brush for arche selection with 6 faces, no polys

	//
	// private TestBox stuff
	//
	int			testboxnum;
	TestBox			*testboxes;	// list

	//
	// private CSurface stuff
	//
	int			csurfacenum;
	CSurface		*csurfaces;	// list

	//
	// private CPoly stuff
	//
	int			cpolynum;
	CPoly			*cpolys;		// list
};

#endif /* __WWM_included */

