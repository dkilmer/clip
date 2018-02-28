#ifndef CLIP_QUATERNION_H
#define CLIP_QUATERNION_H

#include "math_3d.h"

#define ONE_DEG_IN_RADS ((2.0f * M_PI) / 360.0f) // 0.017444444

typedef struct {
	float q[4];
} versor;

versor q_divs(versor v, float rhs);
versor q_muls(versor v, float rhs);
versor q_mul(versor v, versor rhs);
versor q_add(versor v, versor rhs);
versor quat_from_axis_rad (float radians, float x, float y, float z);
versor quat_from_axis_deg (float degrees, float x, float y, float z);
mat4_t quat_to_mat4 (versor q);
versor q_normalize (versor q);
float q_dot (versor q, versor r);
versor q_slerp (versor q, versor r, float t);

#endif //CLIP_QUATERNION_H
