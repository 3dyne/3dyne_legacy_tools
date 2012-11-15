#-------------------------------------------------------------------------------
# 3dyne Legacy Tools GPL Source Code
# 
# Copyright (C) 1996-2012 Matthias C. Berger & Simon Berger.
# 
# This file is part of the 3dyne Legacy Tools GPL Source Code ("3dyne Legacy
# Tools Source Code").
#   
# 3dyne Legacy Tools Source Code is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
# 
# 3dyne Legacy Tools Source Code is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with
# 3dyne Legacy Tools Source Code.  If not, see <http://www.gnu.org/licenses/>.
# 
# In addition, the 3dyne Legacy Tools Source Code is also subject to certain
# additional terms. You should have received a copy of these additional terms
# immediately following the terms and conditions of the GNU General Public
# License which accompanied the 3dyne Legacy Tools Source Code.
# 
# Contributors:
#     Matthias C. Berger (mcb77@gmx.de) - initial API and implementation
#     Simon Berger (simberger@gmail.com) - initial API and implementation
#-------------------------------------------------------------------------------
; trace_3dnow.asm

extern com_p1
extern com_p2
extern com_norm
extern com_dist
extern com_d1
extern com_d2
extern com_scale
extern com_tmp

; 
global trace_2dists_3dnow_test
trace_2dists_3dnow_test:
	femms
	mov		eax, [com_p1]
	mov		edx, [com_norm]
	movq		mm0, [eax]
	movq		mm3, [edx]
	pfmul		mm0, mm3
	movd		mm2, [eax+8]
	movd		mm1, [edx+8]
	pfacc		mm0, mm0
	pfmul		mm1, mm2
	pfadd		mm0, mm1
	movd		[com_d1], mm0
	femms
	ret

global trace_2dists_3dnow
trace_2dists_3dnow:
	femms
	
	mov		eax, [com_p1]
	mov		ebx, [com_p2]
	mov		edx, [com_norm]

	movq		mm0, [eax]	; p1[0] to mm0 low, p1[1] to mm0 high
	movq		mm1, [ebx]	; p2[0] to mm1 low, p2[1] to mm1 high
	movq		mm2, [edx]	; n[0] to mm2 low, n[1] to mm2 high

	pfmul		mm0, mm2	; p1[0]*n[0] to mm0 low (t1[0]), p1[1]*n[1] to mm0 high (t1[1])
	pfacc		mm0, mm0	; t1[0]+t1[1] to low and high

	pfmul		mm1, mm2	; p2[0]*n[0] to mm1 low (t2[0]), p2[1]*n[1] to mm1 high (t2[1])
	pfacc		mm1, mm1	; t2[0]+t2[1] to low and high

	punpckldq	mm0, mm1	; mm1 low to mm0 high, t1[0]+t1[1] in mm0 low, t2[0]+t2[1] in mm0 high

	movd		mm1, [eax+8]	; p1[2] to mm1 low
	punpckldq	mm1, [ebx+8]	; p2[2] to mm1 high

	movd		mm2, [edx+8]	; n[2] to mm2 low
	punpckldq	mm2, mm2	; n[2] to mm2 high

	pfmul		mm1, mm2	; p1[2]*n[2] to mm1 low, p2[2]*n[2] to mm1 high
	pfadd		mm0, mm1	

	movq		mm1, [com_dist]
	punpckldq	mm1, mm1
	pfsub		mm0, mm1
	
	movd		[com_d1], mm0
	punpckhdq	mm0, mm0
	movd		[com_d2], mm0

	femms
	ret	

global trace_split_3dnow
trace_split_3dnow:
	femms
	mov		eax, [com_p1]
	mov		ebx, [com_p2]
	mov		edx, [com_tmp]

        movq        mm0,[ebx]		; p2[0] to mm0 low, p2[1] to mm0 high
        movd        mm1,[ebx+8]		; p2[2] to mm1 low
        pfsub       mm0,[eax]		; p2[0]-p1[0] to mm0 low, p2[1]-p1[1] to mm0 high
        pfsub       mm1,[eax+8]		; p2[2]-p1[2] to mm1 low

	movq	    mm2,[com_scale]
	punpckldq   mm2, mm2
	pfmul	    mm0, mm2
	pfmul	    mm1, mm2

	pfadd       mm0, [eax]
	pfadd	    mm1, [eax+8]

	movq	    [com_tmp], mm0
	movd	    [com_tmp+8], mm1

        femms
        ret		
