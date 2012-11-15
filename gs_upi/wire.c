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



// wire.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_token.h"
#include "wire.h"

int W_ParseProject( w_project_t *project, unsigned char *path, unsigned char name[256] )
{
	int	i, line, lcomp;
	tokenstream_t	*ts;

	if( !project )
	{
		return 1;  // undefined behaviour
	}

	if( !name )
	{
		strcpy( project->errortext, "argument name is NULL" );
		return 1;
	}

//	h = fopen( name, "rb" );
	ts = BeginTokenStream( name );
	
	if( !ts )
	{
		sprintf( project->errortext, "cannot open file:\n%s", name );
		return 1;
	}

	GetToken( ts );
	if( strcmp( "WiredProjectFile", ts->token ) )
	{
		sprintf( project->errortext, "file is not an Wired project file:\n%s", name );
//		fclose( h );
		EndTokenStream( ts );
		return 1;
	}

	lcomp = 0;
	line = 2;
	memset( project->sbrush, 0, 256 );
	for( i = 0; i < 16; i++ )
	{
		GetToken( ts );

		if( !strcmp( ts->token, "StructureBrushes" ))
		{
		       
			GetToken( ts );
			sprintf( project->sbrush, "%s/%s", path, ts->token );
			printf( "sbrush: %s\n", project->sbrush );
			lcomp = 1;
			continue;
		}

		if( !strcmp( ts->token, "StructureBrushes" ))
		{
		       
			GetToken( ts );
			sprintf( project->sbrush, "%s/%s", path, ts->token );
			printf( "sbrush: %s\n", project->sbrush );
			lcomp = 1;
			continue;
		}
	
                if( !strcmp( ts->token, "ArcheTypes" ))
                {

                        GetToken( ts );
                        sprintf( project->ats, "%s/%s", path, ts->token );
                        printf( "ats: %s\n", project->ats );
                        lcomp = 1;
                        continue;
                }
	
		if( !strcmp( ts->token, "LevelOutputDir" ) )
		{
			GetToken( ts );
			strcpy( project->outdir, ts->token );
	//		printf( "outdir: %s\n", project->outdir );	
			continue;
		}

		if( !strcmp( ts->token, "Version" ) )
		{
			GetToken( ts );
			project->version = atoi( ts->token );
			continue;
		}

		if( !strcmp( ts->token, "EndOfFile" ))
		{
		
			printf( "regular end of file\n" );
			break;
		}
	}
	EndTokenStream( ts );
//	fclose( h );

	if( !strlen( project->sbrush ))
	{
		sprintf( project->errortext, "missing key 'StructureBrushes' in file:\n%s", name );
		return 1;
	}

	if( !strlen( project->outdir ))
	{
		sprintf( project->errortext, "missing key 'LevelOutputDir'in file:\n%s", name );
		return 1;
	}
	return 0;
}
