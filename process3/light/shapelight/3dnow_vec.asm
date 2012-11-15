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
;
; input:
; esp+8  : vec3d_t in1
; esp+12 : vec3d_t in2
;
; output:
; esp+4  : vec3d_t out
;
global vec3d_add_3dnow
vec3d_add_3dnow:
        femms
        mov         eax,[esp+8]
        mov         edx,[esp+12]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        mov         eax,[esp+4]
        movd        mm2,[edx+8]
        pfadd       mm0,[edx]
        pfadd       mm1,mm2
        movq        [eax],mm0
        movd        [eax+8],mm1
        femms
        ret


;
; input:
; esp+8  : vec3d_t in1
; esp+12 : vec3d_t in2
;
; output:
; esp+4  : vec3d_t out
;
global vec3d_sub_3dnow
vec3d_sub_3dnow:
        femms
        mov         eax,[esp+8]
        mov         edx,[esp+12]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        mov         eax,[esp+4]
        pfsub       mm0,[edx]
        movd        mm2,[edx+8]
        pfsub       mm1,mm2
        movq        [eax],mm0
        movd        [eax+8],mm1
        femms
        ret	

;
; input:
; esp+8  : float scale
; esp+12 : vec3d_t in
;
; output:
; esp+4  : vec3d_t out
;
global vec3d_scale_3dnow
vec3d_scale_3dnow:
        femms
        mov         eax,[esp+12]
        movd        mm0,[esp+8]
        movq        mm3,[eax]
        punpckldq   mm0,mm0
        movd        mm2,[eax+8]
        movq        mm1,mm0
        pfmul       mm0,mm3
        mov         eax,[esp+4] 
        pfmul       mm1,mm2 
        movq        [eax],mm0
        movd        [eax+8],mm1
        femms
        ret

;
; input:
; esp+8 : vec3d_t in
;
; output
; esp+4 : float *len
;
global vec3d_len_3dnow
vec3d_len_3dnow:
        femms
	
	mov	    eax,[esp+8]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        pfmul       mm0,mm0
        pfmul       mm1,mm1
        pfacc       mm0,mm0
        pfadd       mm0,mm1
        pfrsqrt     mm1,mm0
        movq        mm2,mm1
        pfmul       mm1,mm1
        pfrsqit1    mm1,mm0
        pfrcpit2    mm1,mm2
        pfmul       mm0,mm1
	movd	    [esp+4], mm0
        femms
        ret


global vec3d_femms_3dnow
vec3d_femms_3dnow:
	femms
	ret

;
; input:
; esp+8 : vec3d_t in1
; esp+12 : vec3d_t in2
;
; output:
; esp+4 : float dot
;
global vec3d_dotproduct_3dnow
vec3d_dotproduct_3dnow:
        femms                                                                 
        mov         eax,[esp+8]                                                      
        mov         edx,[esp+12]                                                 
        movq        mm0,[eax]                                                   
        movq            mm3,[edx]                                               
        pfmul       mm0,mm3
        movd            mm2,[eax+8]                                             
        movd        mm1,[edx+8]                                                 
        pfacc       mm0,mm0                                                   
        pfmul       mm1,mm2                                                   
        pfadd       mm0,mm1  
	mov	    eax, [esp+4]	                                                 
        movd        [eax],mm0                                                       
        femms 
	ret

;
; input:
; esp+8 : vec3d_t in
;
; output:
; esp+4 : vec3d_t out
;
global vec3d_unify_3dnow
vec3d_unify_3dnow:
        femms
        mov         eax,[esp+8]
        mov         edx,[esp+4]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        movq        mm4,mm0
        movq        mm3,mm1
        pfmul       mm0,mm0 
        pfmul       mm1,mm1 
        pfacc       mm0,mm0 
        pfadd       mm0,mm1 
        movd        eax,mm0
        pfrsqrt     mm1,mm0 
        movq        mm2,mm1
        cmp         eax,0038D1B717h     ; = 1.0 / 10000.0
        jl          short zero_mag

        pfmul       mm1,mm1
        pfrsqit1    mm1,mm0
        pfrcpit2    mm1,mm2
        punpckldq   mm1,mm1
        pfmul       mm3,mm1
        pfmul       mm4,mm1
        movd        [edx+8],mm3
        movq        [edx],mm4
        femms
        ret

        ALIGN       32
zero_mag:
        mov         eax,[esp+8]
        movq        mm0,[eax]
        movd        mm1,[eax+8]
        movq        [edx],mm0
        movd        [edx+8],mm1
        femms
        ret	
