#include "quaternion.h"
#include "math_3d.h"

versor q_divs(versor v, float rhs) {
	versor result;
	result.q[0] = v.q[0] / rhs;
	result.q[1] = v.q[1] / rhs;
	result.q[2] = v.q[2] / rhs;
	result.q[3] = v.q[3] / rhs;
	return result;
}

versor q_muls(versor v, float rhs) {
	versor result;
	result.q[0] = v.q[0] * rhs;
	result.q[1] = v.q[1] * rhs;
	result.q[2] = v.q[2] * rhs;
	result.q[3] = v.q[3] * rhs;
	return result;
}

versor q_mul(versor v, versor rhs) {
	versor result;
	result.q[0] = rhs.q[0] * v.q[0] - rhs.q[1] * v.q[1] -
	              rhs.q[2] * v.q[2] - rhs.q[3] * v.q[3];
	result.q[1] = rhs.q[0] * v.q[1] + rhs.q[1] * v.q[0] -
	              rhs.q[2] * v.q[3] + rhs.q[3] * v.q[2];
	result.q[2] = rhs.q[0] * v.q[2] + rhs.q[1] * v.q[3] +
	              rhs.q[2] * v.q[0] - rhs.q[3] * v.q[1];
	result.q[3] = rhs.q[0] * v.q[3] - rhs.q[1] * v.q[2] +
	              rhs.q[2] * v.q[1] + rhs.q[3] * v.q[0];
	// re-normalise in case of mangling
	return q_normalize(result);
}

versor q_add(versor v, versor rhs) {
	versor result;
	result.q[0] = rhs.q[0] + v.q[0];
	result.q[1] = rhs.q[1] + v.q[1];
	result.q[2] = rhs.q[2] + v.q[2];
	result.q[3] = rhs.q[3] + v.q[3];
	// re-normalise in case of mangling
	return q_normalize(result);
}

versor quat_from_axis_rad (float radians, float x, float y, float z) {
	versor result;
	result.q[0] = cosf(radians / 2.0f);
	result.q[1] = sinf(radians / 2.0f) * x;
	result.q[2] = sinf(radians / 2.0f) * y;
	result.q[3] = sinf(radians / 2.0f) * z;
	return result;
}

versor quat_from_axis_deg (float degrees, float x, float y, float z) {
	return quat_from_axis_rad (ONE_DEG_IN_RADS * degrees, x, y, z);
}

mat4_t quat_to_mat4 (versor q) {
	float w = q.q[0];
	float x = q.q[1];
	float y = q.q[2];
	float z = q.q[3];
	return mat4 (
		1.0f - 2.0f * y * y - 2.0f * z * z,
		2.0f * x * y + 2.0f * w * z,
		2.0f * x * z - 2.0f * w * y,
		0.0f,
		2.0f * x * y - 2.0f * w * z,
		1.0f - 2.0f * x * x - 2.0f * z * z,
		2.0f * y * z + 2.0f * w * x,
		0.0f,
		2.0f * x * z + 2.0f * w * y,
		2.0f * y * z - 2.0f * w * x,
		1.0f - 2.0f * x * x - 2.0f * y * y,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	);
}

versor q_normalize (versor q) {
	// norm(q) = q / magnitude (q)
	// magnitude (q) = sqrt (w*w + x*x...)
	// only compute sqrt if interior sum != 1.0
	float sum =
		q.q[0] * q.q[0] + q.q[1] * q.q[1] +
		q.q[2] * q.q[2] + q.q[3] * q.q[3];
	// NB: floats have min 6 digits of precision
	const float thresh = 0.0001f;
	if (fabsf(1.0f - sum) < thresh) {
		return q;
	}
	float mag = sqrtf(sum);
	return q_divs(q, mag);
}

float q_dot (versor q, versor r) {
	return q.q[0] * r.q[0] + q.q[1] * r.q[1] + q.q[2] * r.q[2] + q.q[3] * r.q[3];
}

versor q_slerp (versor q, versor r, float t) {
	// angle between q0-q1
	float cos_half_theta = q_dot(q, r);
	// as found here http://stackoverflow.com/questions/2886606/flipping-issue-when-interpolating-rotations-using-quaternions
	// if dot product is negative then one quaternion should be negated, to make
	// it take the short way around, rather than the long way
	// yeah! and furthermore Susan, I had to recalculate the d.p. after this
	if (cos_half_theta < 0.0f) {
		for (int i = 0; i < 4; i++) {
			q.q[i] *= -1.0f;
		}
		cos_half_theta = q_dot(q, r);
	}
	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (fabsf(cos_half_theta) >= 1.0f) {
		return q;
	}
	// Calculate temporary values
	float sin_half_theta = sqrtf(1.0f - cos_half_theta * cos_half_theta);
	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	versor result;
	if (fabsf(sin_half_theta) < 0.001f) {
		for (int i = 0; i < 4; i++) {
			result.q[i] = (1.0f - t) * q.q[i] + t * r.q[i];
		}
		return result;
	}
	float half_theta = acosf(cos_half_theta);
	float a = sinf((1.0f - t) * half_theta) / sin_half_theta;
	float b = sinf(t * half_theta) / sin_half_theta;
	for (int i = 0; i < 4; i++) {
		result.q[i] = q.q[i] * a + r.q[i] * b;
	}
	return result;
}
