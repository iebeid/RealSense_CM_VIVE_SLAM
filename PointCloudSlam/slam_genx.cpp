#include <cm/cm.h>
#include <cm/cm_intrin.h>
#include <cm/cmtl.h>

#define BODIES_CHUNK 32
#define ELEMS_BODY 4
#define BODIES_PER_RW 8
#define SIZE_ELEM sizeof(float)

extern "C" _GENX_MAIN_ void
calculate_least_squares(SurfaceIndex IN_POS_MODEL, SurfaceIndex IN_NORMAL_MODEL, SurfaceIndex IN_POS_TEMP, SurfaceIndex OUTPUT_MATCH_POSITION, SurfaceIndex OUTPUT_MATCH_NORMAL, int threads)
{
	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();
	int id = v_pos * threads + h_pos;
	vector<float, BODIES_CHUNK*ELEMS_BODY> in_pos_model;
	vector<float, BODIES_CHUNK*ELEMS_BODY> in_normal_model;
	vector<float, BODIES_CHUNK*ELEMS_BODY> in_pos_temp;
	vector<float, BODIES_CHUNK> in_pos_temp_distances;
	vector<float, BODIES_CHUNK*ELEMS_BODY> least_squares_vector_1;
	vector<float, BODIES_CHUNK*ELEMS_BODY> least_squares_vector_2;
#pragma unroll
	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
	{
		read(IN_POS_MODEL, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_pos_model.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
	}
#pragma unroll
	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
	{
		read(IN_NORMAL_MODEL, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_normal_model.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
	}
#pragma unroll
	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
	{
		read(IN_POS_TEMP, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_pos_temp.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
	}
	int h = 0;
	for (int i = 0; i < BODIES_CHUNK; i++){
		float query_temp_x = in_pos_temp(h);
		float query_temp_y = in_pos_temp(h + 1);
		float query_temp_z = in_pos_temp(h + 2);
		in_pos_temp_distances = cm_sqrt(((in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(0) - query_temp_x)*(in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(0) - query_temp_x)) + 
			((in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(1) - query_temp_y)*(in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(1) - query_temp_y)) + 
			((in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(2) - query_temp_z)*(in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(2) - query_temp_z)));
		in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(3) = in_pos_temp_distances;
		float in_pos_temp_distances_minimum = cm_reduced_min<float, float, BODIES_CHUNK>(in_pos_temp_distances);
		float match_model_x = 0;
		float match_model_y = 0;
		float match_model_z = 0;
		float match_model_normal_x = 0;
		float match_model_normal_y = 0;
		float match_model_normal_z = 0;
		int v = 0;
		for (int j = 0; j < BODIES_CHUNK; j++){
			if (in_pos_model(v + 3) == in_pos_temp_distances_minimum){
				match_model_x = in_pos_model(v);
				match_model_y = in_pos_model(v + 1);
				match_model_z = in_pos_model(v + 2);
				match_model_normal_x = in_normal_model(v);
				match_model_normal_y = in_normal_model(v + 1);
				match_model_normal_z = in_normal_model(v + 2);
			}
			v = v + 4;
		}
		if (query_temp_x != 0.0f && query_temp_y != 0.0f && query_temp_z != 0.0f && ((query_temp_x - match_model_x)*match_model_normal_x + (query_temp_y - match_model_y)*match_model_normal_y + (query_temp_z - match_model_z)*match_model_normal_z)<3.0f){
			least_squares_vector_1(h) = (match_model_normal_z * query_temp_y) - (match_model_normal_y*query_temp_z);
			least_squares_vector_1(h + 1) = (match_model_normal_x*query_temp_z) - (match_model_normal_z*query_temp_x);
			least_squares_vector_1(h + 2) = (match_model_normal_y*query_temp_x) - (match_model_normal_x*query_temp_y);
			least_squares_vector_1(h + 3) = match_model_normal_x;
			least_squares_vector_2(h) = match_model_normal_y;
			least_squares_vector_2(h + 1) = match_model_normal_z;
			least_squares_vector_2(h + 2) = ((query_temp_x - match_model_x)*match_model_normal_x + (query_temp_y - match_model_y)*match_model_normal_y + (query_temp_z - match_model_z)*match_model_normal_z);
			least_squares_vector_2(h + 3) = (match_model_normal_x*match_model_x) + (match_model_normal_y*match_model_y) + (match_model_normal_z*match_model_z) - (match_model_normal_x*query_temp_x) - (match_model_normal_y*query_temp_y) - (match_model_normal_z*query_temp_z);
		}
		else{
			least_squares_vector_1(h) = 0.0f;
			least_squares_vector_1(h + 1) = 0.0f;
			least_squares_vector_1(h + 2) = 0.0f;
			least_squares_vector_1(h + 3) = 0.0f;
			least_squares_vector_2(h) = 0.0f;
			least_squares_vector_2(h + 1) = 0.0f;
			least_squares_vector_2(h + 2) = 0.0;
			least_squares_vector_2(h + 3) = 0.0f;
		}
		h = h + 4;
	}
#pragma unroll
	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
	{
		write(OUTPUT_MATCH_POSITION, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, least_squares_vector_1.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
	}
#pragma unroll
	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
	{
		write(OUTPUT_MATCH_NORMAL, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, least_squares_vector_2.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
	}
}

//Optimized version
//extern "C" _GENX_MAIN_ void
//calculate_least_squares(SurfaceIndex IN_POS_MODEL, SurfaceIndex IN_NORMAL_MODEL, SurfaceIndex IN_POS_TEMP, SurfaceIndex OUTPUT_MATCH_POSITION, SurfaceIndex OUTPUT_MATCH_NORMAL, int threads)
//{
//	int h_pos = get_thread_origin_x();
//	int v_pos = get_thread_origin_y();
//	int id = v_pos * threads + h_pos;
//
//	vector<float, BODIES_CHUNK*ELEMS_BODY> in_pos_model;
//	vector<float, BODIES_CHUNK*ELEMS_BODY> in_normal_model;
//	vector<float, BODIES_CHUNK*ELEMS_BODY> in_pos_temp;
//
//	vector<float, BODIES_CHUNK> in_pos_temp_distances;
//
//	vector<float, BODIES_CHUNK*ELEMS_BODY> least_squares_vector_1;
//	vector<float, BODIES_CHUNK*ELEMS_BODY> least_squares_vector_2;
//
//	matrix<float, BODIES_CHUNK, ELEMS_BODY + 1> in_pos_model_matrix;
//	matrix<float, BODIES_CHUNK, ELEMS_BODY + 1> in_normal_model_matrix;
//	matrix<uchar, BODIES_CHUNK, ELEMS_BODY + 1> in_pos_model_matrix_mask;
//	matrix<uchar, BODIES_CHUNK, ELEMS_BODY + 1> in_normal_model_matrix_mask;
//	matrix<float, BODIES_CHUNK, ELEMS_BODY> in_pos_temp_matrix;
//
//	in_pos_model_matrix.row(0) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(0);
//	in_pos_model_matrix.row(1) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(1);
//	in_pos_model_matrix.row(2) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(2);
//	in_pos_model_matrix.row(3) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(3);
//
//	in_normal_model_matrix.row(0) = in_normal_model.select<BODIES_CHUNK, ELEMS_BODY>(0);
//	in_normal_model_matrix.row(1) = in_normal_model.select<BODIES_CHUNK, ELEMS_BODY>(1);
//	in_normal_model_matrix.row(2) = in_normal_model.select<BODIES_CHUNK, ELEMS_BODY>(2);
//	in_normal_model_matrix.row(3) = in_normal_model.select<BODIES_CHUNK, ELEMS_BODY>(3);
//
//	in_pos_temp_matrix.row(0) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(0);
//	in_pos_temp_matrix.row(1) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(1);
//	in_pos_temp_matrix.row(2) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(2);
//	in_pos_temp_matrix.row(3) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(3);
//
//#pragma unroll
//	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
//	{
//		read(IN_POS_MODEL, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_pos_model.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
//	}
//#pragma unroll
//	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
//	{
//		read(IN_NORMAL_MODEL, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_normal_model.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
//	}
//#pragma unroll
//	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
//	{
//		read(IN_POS_TEMP, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_pos_temp.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
//	}
//	
//	int h = 0;
//	for (int i = 0; i < BODIES_CHUNK; i++){
//		float query_temp_x = in_pos_temp_matrix(0,h);
//		float query_temp_y = in_pos_temp_matrix(1,h + 1);
//		float query_temp_z = in_pos_temp_matrix(2,h + 2);
//		in_pos_temp_distances = cm_sqrt(((in_pos_model_matrix.row(0) - query_temp_x)*(in_pos_model_matrix.row(0) - query_temp_x)) +
//			((in_pos_model_matrix.row(1) - query_temp_y)*(in_pos_model_matrix.row(1) - query_temp_y)) +
//			((in_pos_model_matrix.row(2) - query_temp_z)*(in_pos_model_matrix.row(2) - query_temp_z)));
//		in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(3) = in_pos_temp_distances;
//		in_normal_model.select<BODIES_CHUNK, ELEMS_BODY>(3) = in_pos_temp_distances;
//		in_pos_model_matrix.row(3) = in_pos_temp_distances;
//		in_normal_model_matrix.row(3) = in_pos_temp_distances;
//		in_pos_model_matrix.row(4) = i;
//		in_normal_model_matrix.row(4) = i;
//		float in_pos_temp_distances_minimum = cm_reduced_min<float, float, BODIES_CHUNK>(in_pos_temp_distances);
//		in_pos_model_matrix_mask = (in_pos_model_matrix.row(3) == in_pos_temp_distances_minimum);
//		in_normal_model_matrix = in_normal_model_matrix*in_pos_model_matrix_mask;
//		h = h + 4;
//	}
//
//#pragma unroll
//	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
//	{
//		write(OUTPUT_MATCH_POSITION, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, least_squares_vector_1.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
//	}
//#pragma unroll
//	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
//	{
//		write(OUTPUT_MATCH_NORMAL, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, least_squares_vector_2.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
//	}
//}