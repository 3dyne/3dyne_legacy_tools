/*
 * VecMath.hh
 *
 *  Created on: Nov 2, 2012
 *      Author: mcb
 */

#ifndef VECMATH_HH_
#define VECMATH_HH_

#include "lib_math.h"
#include "lib_error.h"

class Vec2 {
public:
	float x;
	float y;

	Vec2( vec2d_t v ) {
		x = v[0];
		y = v[1];
	}

	void get( vec2d_t v ) {
		v[0] = x;
		v[1] = y;
	}

	float operator[](int index) {
		switch (index) {
		case 0:
			return x;
		case 1:
			return y;
		default:
			Error("invalid index %d\n", index);
		}
		return 0;
	}
};

class Vec3 {
public:
	float x;
	float y;
	float z;

	Vec3( vec3d_t v ) {
		x = v[0];
		y = v[1];
		z = v[2];
	}

	void get( vec3d_t v ) {
		v[0] = x;
		v[1] = y;
		v[2] = z;
	}

	float operator[](int index) {
			switch (index) {
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			default:
				Error("invalid index %d\n", index);
			}
			return 0;
		}
};

#endif /* VECMATH_HH_ */
