#include <cm/cm.h>
#include <cm/cm_intrin.h>
#include <cm/cmtl.h>

#define BODIES_CHUNK 32
#define ELEMS_BODY 4
#define BODIES_PER_RW 8
#define SIZE_ELEM sizeof(float)

//extern "C" _GENX_MAIN_ void
//get_correspondance(SurfaceIndex IN_POS_MODEL, SurfaceIndex IN_NORMAL_MODEL, SurfaceIndex IN_POS_TEMP, SurfaceIndex OUTPUT_MATCH_POSITION, SurfaceIndex OUTPUT_MATCH_NORMAL, unsigned id)
//{
//	vector<float, BODIES_CHUNK*ELEMS_BODY> in_pos_model;
//	vector<float, BODIES_CHUNK*ELEMS_BODY> in_normal_model;
//	vector<float, BODIES_CHUNK*ELEMS_BODY> in_pos_temp;
//	vector<float, BODIES_CHUNK> in_pos_temp_distances;
//	//matrix<float, ELEMS_BODY, BODIES_CHUNK> model_match_matrix;
//	//vector<float, BODIES_CHUNK*ELEMS_BODY> model_match_vector;
//	//vector<float, BODIES_CHUNK*ELEMS_BODY> model_normal_match_vector;
//
//	vector<float, BODIES_CHUNK*ELEMS_BODY> least_squares_vector_1;
//	vector<float, BODIES_CHUNK*ELEMS_BODY> least_squares_vector_2;
//
//#pragma unroll
//	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
//	{
//		read(IN_POS_MODEL, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_pos_model.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
//	}
//
//#pragma unroll
//	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
//	{
//		read(IN_NORMAL_MODEL, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_normal_model.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
//	}
//
//#pragma unroll
//	for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW)
//	{
//		read(IN_POS_TEMP, (id*BODIES_CHUNK + i)*ELEMS_BODY*SIZE_ELEM, in_pos_temp.select<ELEMS_BODY*BODIES_PER_RW, 1>(ELEMS_BODY*i));
//	}
//
//	//matrix<float, ELEMS_BODY, BODIES_CHUNK> in_pos_temp_matrix;
//	//in_pos_temp_matrix.row(0) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(0);
//	//in_pos_temp_matrix.row(1) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(1);
//	//in_pos_temp_matrix.row(2) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(2);
//	//in_pos_temp_matrix.row(3) = in_pos_temp.select<BODIES_CHUNK, ELEMS_BODY>(3);
//
//	//matrix<float, ELEMS_BODY, BODIES_CHUNK> in_pos_model_matrix;
//	//in_pos_model_matrix.row(0) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(0);
//	//in_pos_model_matrix.row(1) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(1);
//	//in_pos_model_matrix.row(2) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(2);
//	//in_pos_model_matrix.row(3) = in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(3);
//
//	int h = id;
//	for (int i = 0; i < BODIES_CHUNK; i++){
//		float query_x = in_pos_temp(h);
//		float query_y = in_pos_temp(h+1);
//		float query_z = in_pos_temp(h+2);
//
//		in_pos_temp_distances = cm_sqrt
//			(((in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(0) - query_x)*
//			(in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(0) - query_x)) +
//			((in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(1) - query_y)*
//			(in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(1) - query_y)) +
//			((in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(2) - query_z)*
//			(in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(2) - query_z)));
//
//		in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(3) = in_pos_temp_distances;
//		in_normal_model.select<BODIES_CHUNK, ELEMS_BODY>(3) = in_pos_temp_distances;
//
//		float in_pos_temp_distances_minimum = cm_reduced_min<float, float, BODIES_CHUNK>(in_pos_temp_distances);
//
//		int v = id;
//		for (int j = 0; j < BODIES_CHUNK; j++){
//			//if (in_pos_model_matrix(3, j) == in_pos_temp_distances_minimum){
//			//	model_match_matrix(0, i) = in_pos_model_matrix(0, j);
//			//	model_match_matrix(1, i) = in_pos_model_matrix(1, j);
//			//	model_match_matrix(2, i) = in_pos_model_matrix(2, j);
//			//	model_match_matrix(3, i) = in_pos_model_matrix(3, j);
//			//}
//			
//			if (in_pos_model(3 + v) == in_pos_temp_distances_minimum){
//				//model_match_vector(h) = in_pos_model(v);
//				//model_match_vector(h + 1) = in_pos_model(v + 1);
//				//model_match_vector(h + 2) = in_pos_model(v + 2);
//				//model_match_vector(h + 3) = in_pos_temp_distances_minimum;
//
//				//model_normal_match_vector(h) = in_normal_model(v);
//				//model_normal_match_vector(h + 1) = in_normal_model(v + 1);
//				//model_normal_match_vector(h + 2) = in_normal_model(v + 2);
//				//model_normal_match_vector(h + 3) = in_pos_temp_distances_minimum;
//
//				least_squares_vector_1(h) = (in_normal_model(v + 2) * query_y) - (in_normal_model(v + 1)*query_z);
//				least_squares_vector_1(h + 1) = (in_pos_model(v)*query_z) - (in_normal_model(v + 2)*query_x);
//				least_squares_vector_1(h + 2) = (in_normal_model(v + 1)*query_x) - (in_normal_model(v)*query_y);
//				least_squares_vector_1(h + 3) = in_normal_model(v);
//				least_squares_vector_2(h) = in_normal_model(v+1);
//				least_squares_vector_2(h+1) = in_normal_model(v + 2);
//				least_squares_vector_2(h + 2) = 3.0;
//				least_squares_vector_2(h + 3) = (in_normal_model(v)*(in_pos_model(v) - query_x) + (in_normal_model(v + 1)*(in_pos_model(v + 1) - query_y)) + (in_normal_model(v + 2)*(in_pos_model(v + 2) - query_z)));
//
//			}
//			v = v + 4;
//		}
//		h = h + 4;
//
//
//
//	}
//
//	
//
//
//	//model_match_vector.select<BODIES_CHUNK, 1>(ELEMS_BODY * 0) = model_match_matrix.row(0);
//	//model_match_vector.select<BODIES_CHUNK, 1>(ELEMS_BODY * 1) = model_match_matrix.row(1);
//	//model_match_vector.select<BODIES_CHUNK, 1>(ELEMS_BODY * 2) = model_match_matrix.row(2);
//	//model_match_vector.select<BODIES_CHUNK, 1>(ELEMS_BODY * 3) = model_match_matrix.row(3);
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
//
//}

extern "C" _GENX_MAIN_ void
get_correspondance(SurfaceIndex IN_POS_MODEL, SurfaceIndex IN_NORMAL_MODEL, SurfaceIndex IN_POS_TEMP, SurfaceIndex OUTPUT_MATCH_POSITION, SurfaceIndex OUTPUT_MATCH_NORMAL, unsigned id)
{
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
	int h = id;
	for (int i = 0; i < BODIES_CHUNK; i++){
		float query_x = in_pos_temp(h);
		float query_y = in_pos_temp(h + 1);
		float query_z = in_pos_temp(h + 2);
		in_pos_temp_distances = cm_sqrt
			(((in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(0) - query_x)*
			(in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(0) - query_x)) +
			((in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(1) - query_y)*
			(in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(1) - query_y)) +
			((in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(2) - query_z)*
			(in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(2) - query_z)));
		in_pos_model.select<BODIES_CHUNK, ELEMS_BODY>(3) = in_pos_temp_distances;
		in_normal_model.select<BODIES_CHUNK, ELEMS_BODY>(3) = in_pos_temp_distances;
		float in_pos_temp_distances_minimum = cm_reduced_min<float, float, BODIES_CHUNK>(in_pos_temp_distances);
		int v = id;
		for (int j = 0; j < BODIES_CHUNK; j++){
			if (in_pos_model(3 + v) == in_pos_temp_distances_minimum){
				least_squares_vector_1(h) = (in_normal_model(v + 2) * query_y) - (in_normal_model(v + 1)*query_z);
				least_squares_vector_1(h + 1) = (in_pos_model(v)*query_z) - (in_normal_model(v + 2)*query_x);
				least_squares_vector_1(h + 2) = (in_normal_model(v + 1)*query_x) - (in_normal_model(v)*query_y);
				least_squares_vector_1(h + 3) = in_normal_model(v);
				least_squares_vector_2(h) = in_normal_model(v + 1);
				least_squares_vector_2(h + 1) = in_normal_model(v + 2);
				least_squares_vector_2(h + 2) = 3.0;
				least_squares_vector_2(h + 3) = (in_normal_model(v)*(in_pos_model(v) - query_x) + (in_normal_model(v + 1)*(in_pos_model(v + 1) - query_y)) + (in_normal_model(v + 2)*(in_pos_model(v + 2) - query_z)));
			}
			v = v + 4;
		}
	}
	h = h + 4;
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