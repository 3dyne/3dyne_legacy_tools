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



#if defined( irix_mips )
// swap 16bit
unsigned short SYS_SwapShort( unsigned short x )
{
	unsigned short tmp;
	unsigned char swap[3];
	
	memcpy( swap, &x, 2 );
	swap[2] = swap[0];
	swap[0] = swap[1];
	swap[1] = swap[2];
	memcpy( &tmp, swap, 2 );
	return tmp;
//	return (x>>8) | (x<<8);
}

// swap 32bit
unsigned int SYS_SwapInt( unsigned int x )
{
	return
        	(x>>24)
        	| ((x>>8) & 0xff00)
        	| ((x<<8) & 0xff0000)
        	| (x<<24);
}
#else
unsigned short SYS_SwapShort( unsigned short x )
{
    return x;
}

unsigned int SYS_SwapInt( unsigned int x )
{
    return x;
}

#endif
