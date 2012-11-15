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



// lib_mprof.h

#ifndef __lib_mprof
#define __lib_mprof

#include <stdio.h>
#include <math.h>

#ifndef NO_MPROF

typedef struct {
	unsigned int		totalcallnum;
	unsigned long long		totalcountnum;
	char		name[128-20];
} func_profile_t;

typedef union {
	unsigned long long	u64;
	unsigned int		u32[2];
} profile_u64_t;

void MicroProfile_begin( void );
void MicroProfile_end( void );

func_profile_t * MicroProfile_RegisterFunc( char *name );


#define MPROF_ENTRY \
	static func_profile_t	*profile = NULL; \
	static profile_u64_t		profile_count1; \
	static profile_u64_t		profile_count2; \
	if ( !profile ) \
		profile = MicroProfile_RegisterFunc( __PRETTY_FUNCTION__ ); \
	__asm__ __volatile__ (	"rdtsc\n" \
				"movl %%eax, %0\n" \
				"movl %%edx, %1\n" \
				: "=X" (profile_count1.u32[0]), "=X" (profile_count1.u32[1]) \
				: \
				: "eax", "edx" ); \
	profile->totalcallnum++;


#define MPROF_LEAVE \
	__asm__ __volatile__ ( "rdtsc\n" \
				"movl %%eax, %0\n" \
				"movl %%edx, %1\n" \
				: "=X" (profile_count2.u32[0]), "=X" (profile_count2.u32[1]) \
				: \
				: "eax", "edx" ); \
 	profile->totalcountnum+= ( -0+profile_count2.u64-profile_count1.u64 ); \

#define MPROF_STOP \
	__asm__ __volatile__ ( "rdtsc\n" \
				"movl %%eax, %0\n" \
				"movl %%edx, %1\n" \
				: "=X" (profile_count2.u32[0]), "=X" (profile_count2.u32[1]) \
				: \
				: "eax", "edx" ); \
	profile->totalcountnum+= ( -0+profile_count2.u64-profile_count1.u64 ); \

#define MPROF_CONT \
	__asm__ __volatile__ ( "rdtsc\n" \
				"movl %%eax, %0\n" \
				"movl %%edx, %1\n" \
				: "=X" (profile_count1.u32[0]), "=X" (profile_count1.u32[1]) \
				: \
				: "eax", "edx" ); \


#else
#define MicroProfile_begin()	;
#define MicroProfile_end()	;

#define MPROF_ENTRY
#define MPROF_LEAVE
#define MPROF_STOP
#define MPROF_CONT

#endif


#endif

