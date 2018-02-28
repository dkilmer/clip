#include <unistd.h>
#include "window.h"
#include "game.h"
#include "misc_util.h"
#include "quaternion.h"
#define MATH_3D_IMPLEMENTATION
#include "common.h"
#include "render_util.h"
#include "easing.h"
#include "clip.h"

typedef struct {
	int pidx[3];
} tidx;

tidx cidxs[] = {
	{0, 1, 2},
	{3, 2, 1},

	{5, 4, 7},
	{6, 7, 4},

	{4, 0, 6},
	{2, 6, 0},

	{1, 5, 3},
	{7, 3, 5},

	{4, 5, 0},
	{1, 0, 5},

	{2, 3, 6},
	{7, 6, 3}
};

// used to sort points clockwise around a center point
t_pt tmp_center;
t_pt tmp_normal;
int cw_cmp (const void * a, const void * b) {
	t_pt *p1 = (t_pt *)a;
	t_pt *p2 = (t_pt *)b;
	t_pt cross = v3_cross(v3_sub(*p1, tmp_center), v3_sub(*p2, tmp_center));
	float dot = v3_dot(cross, tmp_normal);
	if (dot > 0.001f) {
		return -1;
	} else if (dot < -0.001f) {
		return 1;
	}
	return 0;
}

void print_tri(t_tri t) {
	printf("(%f, %f, %f) ", t.p[0].x, t.p[0].y, t.p[0].z);
	printf("(%f, %f, %f) ", t.p[1].x, t.p[1].y, t.p[1].z);
	printf("(%f, %f, %f)\n", t.p[2].x, t.p[2].y, t.p[2].z);
}

void print_pt(const char *txt, t_pt p) {
	printf("%s (%f, %f, %f)\n", txt, p.x, p.y, p.z);
}

int make_cube(t_pt *pts, t_tri *tris) {
	for (int i=0; i<12; i++) {
		tris[i].p[0] = pts[cidxs[i].pidx[0]];
		tris[i].p[1] = pts[cidxs[i].pidx[1]];
		tris[i].p[2] = pts[cidxs[i].pidx[2]];
	}
	return 12;
}

float distance(t_pt p1, t_pt p2) {
	float xx = p1.x - p2.x;
	float yy = p1.y - p2.y;
	float zz = p1.z - p2.z;
	return sqrtf((xx*xx)+(yy*yy)+(zz*zz));
}

float line_distance(t_pt x1, t_pt x2, t_pt x0) {
	t_pt num = v3_cross(v3_sub(x2, x1), v3_sub(x1, x0));
	t_pt den = v3_sub(x2, x1);
	return v3_length(num) / v3_length(den);
}

int reduce_pts(t_pt *src, t_pt *dst, int scnt) {
	t_pt idst[scnt];
	int icnt = 0;
	for (int i=0; i<scnt; i++) {
		bool use = true;
		for (int j=0; j<icnt; j++) {
			float dist = distance(src[i], idst[j]);
			if (dist < 0.01f) {
				use = false;
				break;
			}
		}
		if (use) {
			idst[icnt] = src[i];
			icnt++;
		}
	}
	printf("there were %d intermediate points\n", icnt);
	int ocnt = 0;
	for (int i=0; i<icnt; i++) {
		bool use = true;
		for (int j=0; j<icnt; j++) {
			if (j == i) continue;
			for (int k=0; k<icnt; k++) {
				if (k == i || k == j) continue;
				float d = line_distance(idst[j], idst[k], idst[i]);
				if (d < 0.01f) {
					float d1 = distance(idst[j], idst[k]);
					float d2 = distance(idst[i], idst[j]);
					float d3 = distance(idst[i], idst[k]);
					if (d2 < d1 && d3 < d1) {
						use = false;
						break;
					}
				}
			}
			if (!use) break;
		}
		if (use) {
			dst[ocnt] = idst[i];
			ocnt++;
		}
	}
	return ocnt;
}

int slice(t_tri *src, t_tri *dst, t_pt *dpts, int scnt, t_pt pp, t_pt pnorm) {
	tri_buf tb;
	tb.out = dst;
	tb.opts = dpts;
	// clip all the triangles and collect the resulting triangles
	// as well as the intersection points
	int didx = 0;
	int dpidx = 0;
	for (int i=0; i<scnt; i++) {
		tb.oidx = didx;
		tb.opidx = dpidx;
		clip(src[i], pp, pnorm, &tb);
		didx += tb.ocnt;
		dpidx += tb.opcnt;
	}
	printf("there were %d intersect points\n", dpidx);
	// reduce the intersection points by getting rid of points that
	// are either really close to each other or fall on a line between
	// other points
	t_pt apts[dpidx];
	int acnt = reduce_pts(dpts, apts, dpidx);
	printf("after reduction there are %d intersect points\n", acnt);
	// sort the points so that they are listed in a clockwise direction.
	// we do that by first finding the center point of all the points...
	tmp_center = vec3(0, 0, 0);
	for (int i=0; i<acnt; i++) {
		tmp_center = v3_add(tmp_center, apts[i]);
	}
	tmp_center = v3_divs(tmp_center, (float)acnt);
	// ...then we sort the list of points based on the cross product
	// of any two points with respect to the center. We use the dot product
	// between that cross product and the normal of the slicing plane to
	// determine which direction they wind with respect to the plane.
	tmp_normal = pnorm;
	qsort(apts, (size_t)acnt, sizeof(t_pt), cw_cmp);
	/*
	for (int i=0; i<acnt; i++) {
		print_pt(apts[i]);
	}
	*/
	// now we make triangles out of the list of points. for now we just handle
	// the two easy cases of 3 or 4 points.
	if (acnt == 3) {
		t_tri fill = {apts[0], apts[1], apts[2]};
		dst[didx++] = fill;
	} else if (acnt == 4) {
		t_tri fill1 = {apts[0], apts[1], apts[2]};
		t_tri fill2 = {apts[2], apts[3], apts[0]};
		dst[didx++] = fill1;
		dst[didx++] = fill2;
	} else {
		printf("ERROR: we had too many points (%d) to fill in the intersection\n",acnt);
	}
	return didx;
}

int plane_tris(t_pt pp, t_pt *ps, float scale, t_tri *dst) {
	t_tri d0 = {v3_add(v3_muls(ps[0], scale), pp), v3_add(v3_muls(ps[1], scale), pp), v3_add(v3_muls(ps[2], scale), pp)};
	t_tri d1 = {v3_add(v3_muls(ps[2], scale), pp), v3_add(v3_muls(ps[3], scale), pp), v3_add(v3_muls(ps[0], scale), pp)};
	dst[0] = d0;
	dst[1] = d1;
	return 2;
}

t_pt plane_axis(t_pt pp, t_pt pnorm, t_pt *ps, int dir) {
	switch(dir) {
		case DIR_U: return v3_norm(ps[0]);
		case DIR_R: return v3_norm(ps[1]);
		case DIR_D: return v3_norm(ps[2]);
		case DIR_L: return v3_norm(ps[3]);
		case DIR_F: return v3_norm(pnorm);
		case DIR_B: return v3_norm(v3_muls(pnorm, -1));
		default: return ps[0];
	}
}

t_pt rotate(t_pt pnorm, t_pt *ps, t_pt ax, float rad) {
	versor curv = {0, pnorm.x, pnorm.y, pnorm.z};
	versor rotv = quat_from_axis_rad(0.02f, ax.x ,ax.y, ax.z);
	versor rotv_prime = quat_from_axis_rad(0.02f, -ax.x ,-ax.y, -ax.z);
	versor v = q_mul(q_mul(rotv, curv), rotv_prime);
	pnorm.x = v.q[1];
	pnorm.y = v.q[2];
	pnorm.z = v.q[3];
	pnorm = v3_norm(pnorm);
	for (int i=0; i<4; i++) {
		versor pcurv = {0, ps[i].x, ps[i].y, ps[i].z};
		versor protv = quat_from_axis_rad(rad, ax.x ,ax.y, ax.z);
		versor protv_prime = quat_from_axis_rad(rad, -ax.x ,-ax.y, -ax.z);
		versor pv = q_mul(q_mul(protv, pcurv), protv_prime);
		ps[i].x = pv.q[1];
		ps[i].y = pv.q[2];
		ps[i].z = pv.q[3];
		//printf("ps[%d]=(%f,%f,%f)\n", i, ps[i].x, ps[i].y, ps[i].z);
	}
	return pnorm;
}

void run() {
	char pwd[1024];
	getcwd(pwd, 1024);
	printf("%s\n", pwd);

	screen_w = 800;
	screen_h = 600;
	if (!init_window("clip", screen_w, screen_h)) return;
	print_sdl_gl_attributes();

	float unit_w = (float)screen_w / 100.0f;
	float unit_h = (float)screen_h / 100.0f;
	float half_w = unit_w / 2.0f;
	float half_h = unit_h / 2.0f;
	float aspect = unit_w / unit_h;
	float far = 100.0f;
	float fov = 67.0f;
	float rads = (fov / 2.0f) * (float)ONE_DEG_IN_RAD;
	float tang = tanf(rads);
	float cam_z = (half_h / tang);
	//printf("cam_z is %f\n",cam_z);
	float near = cam_z - 1.0f;
	vec3_t cpos = { 2, 6, cam_z };
	vec3_t cat = {half_w, half_h, 0.0f};
	vec3_t up = {0.0f, 1.0f, 0.0f};
	mat4_t p_mat = m4_perspective(fov, aspect, near, far);
	mat4_t m_mat = m4_identity();
	mat4_t v_mat = m4_look_at(cpos, cat, up);
	mat4_t vp_mat = m4_mul(m4_mul(p_mat, v_mat), m_mat);

	/*
	// ortho camera
	vec3_t cpos = { half_w, half_h, 20.0f};
	vec3_t cat = {half_w, half_h, 0.0f};
	vec3_t up = {0.0f, 1.0f, 0.0f};
	mat4_t p_mat = m4_ortho(-half_w, half_w, -half_h, half_h, 30.0f, 8.8f);
	mat4_t m_mat = m4_identity();
	mat4_t v_mat = m4_look_at(cpos, cat, up);
	mat4_t vp_mat = m4_mul(m4_mul(p_mat, v_mat), m_mat);
	*/

	render_def buf;
	buf.num_bufs = 3;
	buf.num_items = (GLuint)(100);
	buf.verts_per_item = 3;
	//alloc_buffers(&buf);

	char vsname[1024];
	char fsname[1024];
	sprintf(vsname, "%s/../shaders/vert.glsl", pwd);
	sprintf(fsname, "%s/../shaders/frag.glsl", pwd);
	setup_render_def(&buf,
	   vsname,
	   fsname,
	   (GLfloat *)&vp_mat
	);

	bool loop = true;
	bool kdown[NUM_KEYS];
	bool kpress[NUM_KEYS];
	for (int i=0; i<NUM_KEYS; i++) {
		kdown[i] = false;
		kpress[i] = false;
	}
	int key_map[NUM_KEYS];
	set_default_key_map(key_map);
	mouse_input mouse;

	GLenum err;
	glUseProgram(buf.shader);
	GLint lpUnif = glGetUniformLocation(buf.shader, "light_pos");
	glUniform3f(lpUnif, -0.5f, 0.25f, 1.0f);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("lpUnif is %d: %d\n", lpUnif, err);
	}

	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, screen_w, screen_h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	t_clr pclrs[] = {
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 0, 1, 1},
		{1, 1, 0, 1},
		{1, 0, 1, 1},
		{0, 1, 1, 1},
		{1, 1, 1, 1}
	};
	t_pt cube[] = {
		{3, 4, 0},
		{5, 4, 0},
		{3, 2, 0},
		{5, 2, 0},
		{3, 4, -2},
		{5, 4, -2},
		{3, 2, -2},
		{5, 2, -2}
	};
	t_clr c = { 0.8f, 0.8f, 1.0f, 0.5f };
	t_clr c2 = { 0.8f, 0.2f, 0.2f, 1.0f };

	int ibidx = 0;
	int obidx = 1;
	t_tri *btri[2];
	btri[0] = (t_tri *)malloc(sizeof(t_tri) * 60);
	btri[1] = (t_tri *)malloc(sizeof(t_tri) * 60);
	t_pt *bpts = (t_pt *)malloc(sizeof(t_pt) * 40);
	t_tri *ptri = (t_tri *)malloc(sizeof(t_tri) * 10);

	int icnt = make_cube(cube, btri[ibidx]);
	int ocnt = 0;

	t_pt pnorm = { 0, 0, -1 };
	t_pt pp = { 4, 3, 0.25f };
	t_pt pps[] = {
		{0, 1, 0},
		{1, 0, 0},
		{0, -1, 0},
		{-1, 0, 0}
	};
	pnorm = v3_norm(pnorm);
	int oidx = 0;
	int did_slice = 0;

	int pcnt = plane_tris(pp, pps, 2, ptri);

	int frame = 0;
	while (loop) {
		get_input(kdown, kpress, key_map, &mouse);
		if (kpress[KEY_QUIT]) {
			loop = false;
		}
		if (kpress[KEY_FIRE]) {
			ocnt = slice(btri[ibidx], btri[obidx], bpts, icnt, pp, pnorm);
			printf("sliced %d tris to %d tris\n", icnt, ocnt);
			ibidx = ((ibidx + 1) % 2);
			obidx = ((ibidx + 1) % 2);
			icnt = ocnt;
			did_slice = 1;
		}
		int dir = DIR_N;
		int rot = DIR_N;
		if (kdown[KEY_UP]) {
			if (kdown[KEY_USE]) {
				rot = DIR_L;
			} else {
				dir = DIR_U;
			}
		} else if (kdown[KEY_DOWN]) {
			if (kdown[KEY_USE]) {
				rot = DIR_R;
			} else {
				dir = DIR_D;
			}
		} else if (kdown[KEY_LEFT]) {
			if (kdown[KEY_USE]) {
				rot = DIR_D;
			} else {
				dir = DIR_L;
			}
		} else if (kdown[KEY_RIGHT]) {
			if (kdown[KEY_USE]) {
				rot = DIR_U;
			} else {
				dir = DIR_R;
			}
		} else if (kdown[KEY_JUMP]) {
			dir = DIR_F;
		} else if (kdown[KEY_SPECIAL]) {
			dir = DIR_B;
		}
		if (dir != DIR_N) {
			t_pt ax = plane_axis(pp, pnorm, pps, dir);
			mat4_t m = m4_translation(v3_muls(ax,0.1f));
			pp = m4_mul_pos(m, pp);
			plane_tris(pp, pps, 2, ptri);
		} else if (rot != DIR_N) {
			t_pt ax = plane_axis(pp, pnorm, pps, rot);
			pnorm = rotate(pnorm, pps, ax, 0.02f);
			plane_tris(pp, pps, 2, ptri);
		}

		glBindVertexArray(buf.vao);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//GLenum err = glGetError();
		//if (err != GL_NO_ERROR) {
		//	printf("err: %d on frame %d\n", err, frame);
		//}

		for (int i=0; i<icnt; i++) {
			t_clr tclr = pclrs[i % 7];
			render_tri(&buf, &(btri[ibidx][i]), &c2);
		}

		//if (!did_slice) {
			for (int i=0; i<pcnt; i++) {
				render_tri(&buf, &(ptri[i]), &c);
			}
		//}

		render_buffer(&buf);

		swap_window();
		frame = (frame + 1) % 60;
		render_advance(&buf);
	}

	free_render_def(&buf);
	free(btri[0]);
	free(btri[1]);
	free(ptri);
	free(bpts);
}
