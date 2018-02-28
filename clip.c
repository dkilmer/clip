#include "clip.h"
#include "common.h"

t_pt intersect(t_pt v1, t_pt v2, t_pt pp, t_pt pnorm) {
	t_pt ray = v3_sub(v2, v1);
	float cosA = v3_dot(ray, pnorm);
	float deltaD = v3_dot(pp, pnorm) - v3_dot(v1, pnorm);
	float length = deltaD / cosA;
	t_pt np = v3_muls(ray, length);
	return v3_add(np, v1);
}

// t is the triangle to test
// pp is a point on the plane used to clip
// pnorm is the normal vector to the clip plane
// out is a list of triangles that we'll write to
// oidx is the starting index to write to in the output list
int clip(t_tri t, t_pt pp, t_pt pnorm, tri_buf *tb) {
	t_tri *out = tb->out;
	int oidx = tb->oidx;
	t_pt *opts = tb->opts;
	int opidx = tb->opidx;
	tb->opcnt = 0;

	float ds[3];
	int cnt = 0;
	int ridx = 0;
	int cidx = 0;
	for (int i=0; i<3; i++) {
		ds[i] = v3_dot(pnorm, v3_sub(t.p[i], pp));
		if (ds[i] > 0.0001f) {
			ridx = i;
			cnt++;
		} else {
			cidx = i;
		}
	}
	if (cnt == 0) {
		// everything was sliced out
		tb->ocnt = 0;
	} else if (cnt == 1) {
		// one point remains
		//          2
		//      \  . .
		//       \.   .
		//       .\    .
		//      .  \    .
		//     1....\....0
		//           \
			//
		// we're going to output 1 triangle
		// it will be the 0/1 intersect, 1, and the 1/2 intersect
		// those are in order - we need to have clockwise wrapping
		int pidx0 = ridx-1;
		if (pidx0 < 0) pidx0 = 2;
		int pidx1 = ridx;
		int pidx2 = ridx + 1;
		if (pidx2 > 2) pidx2 = 0;
		out[oidx].p[0] = intersect(t.p[pidx0], t.p[pidx1], pp, pnorm);
		out[oidx].p[1] = t.p[pidx1];
		out[oidx].p[2] = intersect(t.p[pidx1], t.p[pidx2], pp, pnorm);
		tb->ocnt = 1;

		opts[opidx++] = out[oidx].p[0];
		opts[opidx++] = out[oidx].p[2];
		tb->opcnt += 2;
 	} else if (cnt == 2) {
		// two points remain
		// 0 and 1 are the points that remain.
		// 2 is the one that will be sliced
		//          2
		//         . .
		//        .   .    normal
		//  -----x-----x-----|-----
		//      .       .    v
		//     1.........0
		//
		// we're going to output 2 triangles
		// the first will be the 2/0 intersect, 0 and 1
		// the second will be 1, the 1/2 intersect and the 2/0 intersect
		// those are in order - we need to have clockwise wrapping
		int pidx2 = cidx;
		int pidx0 = cidx+1;
		if (pidx0 > 2) pidx0 = 0;
		int pidx1 = pidx0+1;
		if (pidx1 > 2) pidx1 = 0;
		// first triangle
		out[oidx].p[0] = intersect(t.p[pidx2], t.p[pidx0], pp, pnorm);
		out[oidx].p[1] = t.p[pidx0];
		out[oidx].p[2] = t.p[pidx1];
		// second triangle
		out[oidx+1].p[0] = t.p[pidx1];
		out[oidx+1].p[1] = intersect(t.p[pidx2], t.p[pidx1], pp, pnorm);
		out[oidx+1].p[2] = intersect(t.p[pidx2], t.p[pidx0], pp, pnorm);
		tb->ocnt = 2;

		opts[opidx++] = out[oidx].p[0];
		opts[opidx++] = out[oidx+1].p[1];
		opts[opidx++] = out[oidx+1].p[2];
		tb->opcnt += 3;
 	} else if (cnt == 3) {
		// the whole triangle remains
		out[oidx] = t;
		tb->ocnt = 1;
	}
	return tb->ocnt;
}
