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



// ClipperTool.hh

#ifndef __ClipperTool_included
#define __ClipperTool_included

#include <qobject.h>

class ClipperTool;
extern ClipperTool	*clippertool_i;

#include "Wired.hh"
#include "WWM.hh"
#include "XZView.hh"
#include "YView.hh"

#include "vec.h"
#include "brush.h"


/*                                                                              
  ===============================================                               
  class: ClipperTool                                                              
  ===============================================                               
*/                                                                              

class ClipperTool : public QObject
{
	Q_OBJECT

public:
	ClipperTool( QObject* parent = NULL, char* name = NULL );
	~ClipperTool();

	void	reset( void );
	void	checkClipper( void );
	void	clipBrushes( void );
	void	splitBrushes( void );
	void	drawSelf( void );
	void	flip( void );
	void	setSplitPlane( plane_t *pl, vec3d_t center );	// center is any point on the plane ( only for drawSelf )

	bool	isPlaneValid( void );
	void	getSplitPlane( plane_t *pl );
//	void	useClipper( void );

	void xzPressSlot( vec2d_t v );
	void yPressSlot( vec2d_t v );

	void csgBrushes( void );
private:

	bool		clippersane;
	
	int		xznum;
	int		ynum;

	int		vnum;
	vec3d_t		vec[3];

	vec3d_t		up;
	vec3d_t		right;
	vec3d_t		origin;

	plane_t		clipplane;
};

#endif // __ClipperTool_included
