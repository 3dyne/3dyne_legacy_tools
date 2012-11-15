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



// lib_math_3dnow.h

#ifndef __lib_math_3dnow
#define __lib_math_3dnoe

typedef union
{
	unsigned long long	q;
	unsigned int		d[2];
	float		f[2];
} /*__attribute__ ((aligned (8)))*/ mmx_t;


#define mmx_drsm( op, reg, mem ) \
	__asm__ __volatile__ (#op " %0, %%" #reg \
				: \
				: "X" (mem))

#define mmx_dmsr( op, mem, reg ) \
	__asm__ __volatile__ (#op " %%" #reg ", %0" \
				: "=X" (mem) \
				: )

#define mmx_drsr( op, dreg, sreg ) \
	__asm__ __volatile__ (#op " %" #sreg ", %" #dreg )


#define mmx_femms()	__asm__ __volatile__ ("femms")

#define mmx_reglock( reg ) \
	__asm__ __volatile__ ( "movl %%eax, %%eax\n\n\n" : : : #reg )

#define mmx_reglockall() \
	__asm__ __volatile__ ( "nop\n" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)" )


static __inline__ void Vec3dAdd_3dnow( vec3d_t out, vec3d_t in1, vec3d_t in2 )
{
	mmx_femms();
	mmx_drsm( movq, mm0, in1[0] );	
	mmx_drsm( movd, mm1, in1[2] );
	mmx_drsm( movd, mm2, in2[2] );
	mmx_drsm( pfadd, mm0, in2[0] );
	mmx_drsr( pfadd, mm1, mm2 );
	mmx_dmsr( movq, out[0], mm0 );
	mmx_dmsr( movq, out[2], mm1 );
	mmx_femms();
}

static __inline__ void Vec3dSub_3dnow( vec3d_t out, vec3d_t in1, vec3d_t in2 )
{
	mmx_femms();
	mmx_drsm( movq, mm0, in1[0] );
	mmx_drsm( movd, mm1, in1[2] );
	mmx_drsm( movd, mm2, in2[2] );
	mmx_drsm( pfsub, mm0, in2[0] );
	mmx_drsr( pfsub, mm1, mm2 );
	mmx_dmsr( movq, out[0], mm0 );
	mmx_dmsr( movq, out[2], mm1 );
	mmx_femms();
}

static __inline__ void Vec3dScale_3dnow( vec3d_t out, float scale, vec3d_t in )
{
	mmx_femms();
	mmx_drsm( movd, mm0, scale );
	mmx_drsm( movq, mm3, in[0] );
	mmx_drsr( punpckldq, mm0, mm0 );
	mmx_drsm( movd, mm2, in[2] );
	mmx_drsr( movq, mm1, mm0 );
	mmx_drsr( pfmul, mm0, mm3 );
	mmx_dmsr( movq, out[0], mm0 );
	mmx_drsr( pfmul, mm1, mm2 );
	mmx_dmsr( movd, out[2], mm1 );
	mmx_femms();
}

static __inline__ float Vec3dDotProduct_3dnow( vec3d_t in1, vec3d_t in2 )
{
	float	dot;
//	mmx_reglock( st );
//	mmx_reglock( st(1) );
//	mmx_reglock( st(2) );
//	mmx_reglock( st(3) );
//	mmx_reglockall();
	mmx_femms();
	mmx_drsm( movq, mm0, in1[0] );
	mmx_drsm( movq, mm3, in2[0] );
	mmx_drsr( pfmul, mm0, mm3 );
	mmx_drsm( movd, mm2, in1[2] );
	mmx_drsm( movd, mm1, in2[2] );
	mmx_drsr( pfacc, mm0, mm0 );
	mmx_drsr( pfmul, mm1, mm2 );
	mmx_drsr( pfadd, mm0, mm1 );
	mmx_dmsr( movd, dot, mm0 );
	mmx_femms();

	return dot;
}

static __inline__ void Vec3dDotProduct_arg_3dnow( float *dot, vec3d_t in1, vec3d_t in2 )
{
//	float	dot;
	mmx_femms();
	mmx_drsm( movq, mm0, in1[0] );
	mmx_drsm( movq, mm3, in2[0] );
	mmx_drsr( pfmul, mm0, mm3 );
	mmx_drsm( movd, mm2, in1[2] );
	mmx_drsm( movd, mm1, in2[2] );
	mmx_drsr( pfacc, mm0, mm0 );
	mmx_drsr( pfmul, mm1, mm2 );
	mmx_drsr( pfadd, mm0, mm1 );
	mmx_dmsr( movd, *dot, mm0 );
	mmx_femms();

//	return dot;
}

static __inline__ float Vec3dLen_3dnow( vec3d_t in )
{
	float	len;

	mmx_femms();
//	prof_entry();
	mmx_drsm( movq, mm0, in[0] );
	mmx_drsm( movd, mm1, in[2] );
	mmx_drsr( pfmul, mm0, mm0 );
	mmx_drsr( pfacc, mm0, mm0 );
	mmx_drsr( pfmul, mm1, mm1 );
	mmx_drsr( pfadd, mm0, mm1 );
	mmx_drsr( pfrsqrt, mm1, mm0 );
	mmx_drsr( movq, mm2, mm1 );
	mmx_drsr( pfmul, mm1, mm1 );
	mmx_drsr( pfrsqit1, mm1, mm0 );
	mmx_drsr( pfrcpit2, mm1, mm2 );
	mmx_drsr( pfmul, mm0, mm1 );
	mmx_dmsr( movd, len, mm0 );
//	prof_leave();

	mmx_femms();

	return len;
       
}

static __inline__ void Vec3dUnify_3dnow( vec3d_t inout )
{
        int	tmp;


//	mmx_femms();
	mmx_drsm( movq, mm0, inout[0] );
	mmx_drsm( movd, mm1, inout[2] );
	mmx_drsr( movq, mm4, mm0 );
	mmx_drsr( movq, mm3, mm1 );
	mmx_drsr( pfmul, mm0, mm0 );
	mmx_drsr( pfacc, mm0, mm0 );
	mmx_drsr( pfmul, mm1, mm1 );
	mmx_drsr( pfadd, mm0, mm1 );
	mmx_dmsr( movd, tmp, mm0 );
	mmx_drsr( pfrsqrt, mm1, mm0 );
	mmx_drsr( movq, mm2, mm1 );
	if ( tmp < 0x038d1b717 )
	{
		
	}
	else
	{
		mmx_drsr( pfmul, mm1, mm1 );
		mmx_drsr( pfrsqit1, mm1, mm0 );
		mmx_drsr( pfrcpit2, mm1, mm2 );
		mmx_drsr( punpckldq, mm1, mm1 );
		mmx_drsr( pfmul, mm3, mm1 );
		mmx_dmsr( movd, inout[2], mm3 );
		mmx_drsr( pfmul, mm4, mm1 );
		mmx_dmsr( movq, inout[0], mm4 );
	}
//	mmx_femms();

}


#endif
