#include <cm/cm.h>
#include <cm/cm_intrin.h>
#include <cm/cmtl.h>

#define BODIES_CHUNK 32
#define ELEMS_BODY 4
#define BODIES_PER_RW 8
#define SIZE_ELEM sizeof(float)
#define K_SIZE 4

extern "C" _GENX_MAIN_ void
get_correspondance(SurfaceIndex IN_POS_MODEL, SurfaceIndex IN_POS_TEMP, SurfaceIndex OUTPUT_MATCH, unsigned id)
{
	vector<float, BODIES_CHUNK*ELEMS_BODY> in_pos_model;
	vector<float, BODIES_CHUNK*ELEMS_BODY> in_pos_temp;
	vector<float, BODIES_CHUNK> in_pos_temp_distances;
	matrix<float, ELEMS_BODY, BODIES_CHUNK> model_match_matrix;
	vector<float, BODIES_CHUNK*ELEMS_BODY> model_match_vector;

#pragma unroll
	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
	{
		read(IN_POS_MODEL, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_pos_model.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
	}

#pragma unroll
	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
	{
		read(IN_POS_TEMP, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_pos_temp.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
	}

	matrix<float, ELEMS_BODY, BODIES_CHUNK> in_pos_temp_matrix;
	in_pos_temp_matrix.row(0) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(0);
	in_pos_temp_matrix.row(1) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(1);
	in_pos_temp_matrix.row(2) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(2);
	in_pos_temp_matrix.row(3) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(3);

	matrix<float, ELEMS_BODY, BODIES_CHUNK> in_pos_model_matrix;
	in_pos_model_matrix.row(0) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(0);
	in_pos_model_matrix.row(1) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(1);
	in_pos_model_matrix.row(2) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(2);
	in_pos_model_matrix.row(3) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(3);


	for (int i = 0; i < BODIES_CHUNK; i++){
		float query_x = in_pos_temp_matrix(0, i);
		float query_y = in_pos_temp_matrix(1, i);
		float query_z = in_pos_temp_matrix(2, i);

		in_pos_temp_distances = cm_sqrt
			(((in_pos_model_matrix.row(0) - query_x)*
			(in_pos_model_matrix.row(0) - query_x)) +
			((in_pos_model_matrix.row(1) - query_y)*
			(in_pos_model_matrix.row(1) - query_y)) +
			((in_pos_model_matrix.row(2) - query_z)*
			(in_pos_model_matrix.row(2) - query_z)));

		in_pos_model_matrix.row(3) = in_pos_temp_distances;

		float in_pos_temp_distances_minimum = cm_reduced_min<float, float, BODIES_CHUNK>(in_pos_temp_distances);

		for (int j = 0; j < BODIES_CHUNK; j++){
			int idx = i*BODIES_CHUNK + j;
			if (in_pos_model_matrix(3, j) == in_pos_temp_distances_minimum){
				model_match_matrix(0, i) = in_pos_model_matrix(0, j);
				model_match_matrix(1, i) = in_pos_model_matrix(1, j);
				model_match_matrix(2, i) = in_pos_model_matrix(2, j);
				model_match_matrix(3, i) = in_pos_model_matrix(3, j);
			}
		}
	}

	model_match_vector.select<BODIES_CHUNK, 1>(ELEMS_BODY * 0) = model_match_matrix.row(0);
	model_match_vector.select<BODIES_CHUNK, 1>(ELEMS_BODY * 1) = model_match_matrix.row(1);
	model_match_vector.select<BODIES_CHUNK, 1>(ELEMS_BODY * 2) = model_match_matrix.row(2);
	model_match_vector.select<BODIES_CHUNK, 1>(ELEMS_BODY * 3) = model_match_matrix.row(3);

#pragma unroll
	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
	{
		write(OUTPUT_MATCH, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, model_match_vector.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
	}

}

extern "C" _GENX_MAIN_ void
linear(SurfaceIndex ibuf, SurfaceIndex obuf)
{
	matrix<uchar, 8, 32> in;
	matrix<uchar, 6, 24> out;
	matrix<float, 6, 24> m;

	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	read(ibuf, h_pos * 24, v_pos * 6, in);

	m = in.select<6, 1, 24, 1>(1, 3);

	m += in.select<6, 1, 24, 1>(0, 0);
	m += in.select<6, 1, 24, 1>(0, 3);
	m += in.select<6, 1, 24, 1>(0, 6);

	m += in.select<6, 1, 24, 1>(1, 0);
	m += in.select<6, 1, 24, 1>(1, 6);

	m += in.select<6, 1, 24, 1>(2, 0);
	m += in.select<6, 1, 24, 1>(2, 3);
	m += in.select<6, 1, 24, 1>(2, 6);

	out = m * 0.111f;

	write(obuf, h_pos * 24, v_pos * 6, out);
}