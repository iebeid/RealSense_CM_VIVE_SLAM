#include "ICP.h"

ICP::ICP() : max_iter(50), min_delta(1e-4), new_size(2048) {

}

ICP::~ICP() {
}

void ICP::fit(CmDevice* pCmDev, CmProgram* program, PointCloud temp, PointCloud model, int number_of_points, Matrix &R, Matrix &t) {
	//for (int iter = 0; iter < max_iter; iter++)
	//	if (fitStep(pCmDev, program, temp, model, number_of_points, R, t) < min_delta)
	//		break;
	CmKernel* kernel_distance = NULL;
	SurfaceIndex* pInputIndex1 = NULL;
	SurfaceIndex* pInputIndex2 = NULL;
	SurfaceIndex* pOutputIndex1 = NULL;
	CmQueue* pCmQueue = NULL;
	CmTask *pKernelArray_distance = NULL;
	CmEvent* e = NULL;

	
	float* input_positions_model = (float *)CM_ALIGNED_MALLOC(new_size * 4 * sizeof(float), 0x1000);
	memset(input_positions_model, 0, new_size * 4 * sizeof(float));
	float* input_positions_template = (float *)CM_ALIGNED_MALLOC(new_size * 4 * sizeof(float), 0x1000);
	memset(input_positions_template, 0, new_size * 4 * sizeof(float));
	float* output_match = (float *)CM_ALIGNED_MALLOC(new_size * 4 * sizeof(float), 0x1000);
	memset(output_match, 0, new_size * 4 * sizeof(float));

	int threads = new_size / 32;
	//Calculate distance on gpu
	pCmDev->CreateKernel(program, CM_KERNEL_FUNCTION(get_correspondance), kernel_distance);
	kernel_distance->SetThreadCount(threads);
	CmBufferUP* surf1 = NULL;
	pCmDev->CreateBufferUP(new_size * 4 * sizeof(float), input_positions_model, surf1);
	surf1->GetIndex(pInputIndex1);
	CmBufferUP* surf2 = NULL;
	pCmDev->CreateBufferUP(new_size * 4 * sizeof(float), input_positions_template, surf2);
	surf2->GetIndex(pInputIndex2);
	CmBufferUP* surf3 = NULL;
	pCmDev->CreateBufferUP(new_size * 4 * sizeof(float), output_match, surf3);
	surf3->GetIndex(pOutputIndex1);

	pCmDev->CreateQueue(pCmQueue);
	pCmDev->CreateTask(pKernelArray_distance);
	pKernelArray_distance->AddKernel(kernel_distance);

	int r1 = 0;
	for (int j = 0; j < model.points.size(); j++){
		input_positions_model[r1] = model.points[j].position.x;
		input_positions_model[r1 + 1] = model.points[j].position.y;
		input_positions_model[r1 + 2] = model.points[j].position.z;
		input_positions_model[r1 + 3] = 0.0f;
		r1 = r1 + 4;
	}
	for (int v = (int)model.points.size(); v < new_size; v++){
		input_positions_model[r1] = 0.0f;
		input_positions_model[r1 + 1] = 0.0f;
		input_positions_model[r1 + 2] = 0.0f;
		input_positions_model[r1 + 3] = 0.0f;
		r1 = r1 + 4;
	}
	
	//cout << model.points.size() << endl;
	//cout << temp.points.size() << endl;

	int r = 0;
	for (int j = 0; j < temp.points.size(); j++){
		input_positions_template[r] = temp.points[j].position.x;
		input_positions_template[r + 1] = temp.points[j].position.y;
		input_positions_template[r + 2] = temp.points[j].position.z;
		input_positions_template[r + 3] = 0.0f;
		r = r + 4;
	}
	for (int v = (int)temp.points.size(); v < new_size; v++){
		input_positions_template[r] = 0.0f;
		input_positions_template[r + 1] = 0.0f;
		input_positions_template[r + 2] = 0.0f;
		input_positions_template[r + 3] = 0.0f;
		r = r + 4;
	}

	kernel_distance->SetKernelArg(0, sizeof(SurfaceIndex), pInputIndex1);
	kernel_distance->SetKernelArg(1, sizeof(SurfaceIndex), pInputIndex2);
	kernel_distance->SetKernelArg(2, sizeof(SurfaceIndex), pOutputIndex1);
	int threadId_lookup = 0;
	for (int n = 0; n < threads; n++)
	{
		kernel_distance->SetThreadArg(threadId_lookup, 3, sizeof(int), &threadId_lookup);
		threadId_lookup++;
	}
	pCmQueue->Enqueue(pKernelArray_distance, e);

	vector<float> correspondance;
	correspondance.assign(output_match, output_match + new_size);
	cout << correspondance.size() << endl;



	for (int iter = 0; iter < max_iter; iter++){
		double threshold = 0;

		Matrix p_m(number_of_points, 3);
		Matrix p_t(number_of_points, 3);

		double r00 = R.val[0][0]; double r01 = R.val[0][1]; double r02 = R.val[0][2];
		double r10 = R.val[1][0]; double r11 = R.val[1][1]; double r12 = R.val[1][2];
		double r20 = R.val[2][0]; double r21 = R.val[2][1]; double r22 = R.val[2][2];
		double t0 = t.val[0][0]; double t1 = t.val[1][0]; double t2 = t.val[2][0];

		// init A and b
		Matrix A(number_of_points, 6);
		Matrix b(number_of_points, 1);


		int u_counter = 0;
		// establish correspondences
		//for (int i = 0; i < temp.points.size(); i++){
			//std::vector<float> query(3);

			// transform point according to R|t
			//query[0] = (float)(r00*temp.points[i].position.x + r01*temp.points[i].position.y + r02*temp.points[i].position.z + t0);
			//query[1] = (float)(r10*temp.points[i].position.x + r11*temp.points[i].position.y + r12*temp.points[i].position.z + t1);
			//query[2] = (float)(r20*temp.points[i].position.x + r21*temp.points[i].position.y + r22*temp.points[i].position.z + t2);

			//vector3f neighbor;

		int r = 0;
		for (int j = 0; j < temp.points.size(); j++){
			input_positions_template[r] = (float)(r00*temp.points[j].position.x + r01*temp.points[j].position.y + r02*temp.points[j].position.z + t0);
			input_positions_template[r + 1] = (float)(r10*temp.points[j].position.x + r11*temp.points[j].position.y + r12*temp.points[j].position.z + t1);
			input_positions_template[r + 2] = (float)(r20*temp.points[j].position.x + r21*temp.points[j].position.y + r22*temp.points[j].position.z + t2);
			input_positions_template[r + 3] = 0.0f;
			r = r + 4;
		}
		for (int v = (int)temp.points.size(); v < new_size; v++){
			input_positions_template[r] = 0.0f;
			input_positions_template[r + 1] = 0.0f;
			input_positions_template[r + 2] = 0.0f;
			input_positions_template[r + 3] = 0.0f;
			r = r + 4;
		}

			kernel_distance->SetKernelArg(0, sizeof(SurfaceIndex), pInputIndex1);
			kernel_distance->SetKernelArg(1, sizeof(SurfaceIndex), pInputIndex2);
			kernel_distance->SetKernelArg(2, sizeof(SurfaceIndex), pOutputIndex1);
			pCmQueue->Enqueue(pKernelArray_distance, e);

			//vector<float> correspondance;
			correspondance.clear();
			correspondance.assign(output_match, output_match + new_size);
			cout << correspondance.size() << endl;
			//neighbor.x = correspondance[u_counter];
			//neighbor.y = correspondance[u_counter + 1];
			//neighbor.z = correspondance[u_counter + 2];

			//-------------------------------------------
			for (int i = 0; i < temp.points.size(); i++){
			// model point
				double dx = correspondance[u_counter];
				double dy = correspondance[u_counter + 1];
				double dz = correspondance[u_counter + 2];

			// model point normal
			double nx = temp.points[i].normal_vector.x;
			double ny = temp.points[i].normal_vector.y;
			double nz = temp.points[i].normal_vector.z;

			// template point
			double sx = (float)(r00*temp.points[i].position.x + r01*temp.points[i].position.y + r02*temp.points[i].position.z + t0);
			double sy = (float)(r10*temp.points[i].position.x + r11*temp.points[i].position.y + r12*temp.points[i].position.z + t1);
			double sz = (float)(r20*temp.points[i].position.x + r21*temp.points[i].position.y + r22*temp.points[i].position.z + t2);

			// setup least squares system
			A.val[i][0] = nz*sy - ny*sz;
			A.val[i][1] = nx*sz - nz*sx;
			A.val[i][2] = ny*sx - nx*sy;
			A.val[i][3] = nx;
			A.val[i][4] = ny;
			A.val[i][5] = nz;
			b.val[i][0] = nx*(dx - sx) + ny*(dy - sy) + nz*(dz - sz); //nx*dx+ny*dy+nz*dz-nx*sx-ny*sy-nz*sz;    

			u_counter = u_counter + 4;
		}



		// solve linear least squares

		// use the normal equations
		Matrix A_ = ~A*A;
		Matrix b_ = ~A*b;

		if (!b_.solve(A_)) break; // failure

		// rotation matrix
		Matrix R_ = Matrix::eye(3);
		R_.val[0][1] = -b_.val[2][0];
		R_.val[1][0] = +b_.val[2][0];
		R_.val[0][2] = +b_.val[1][0];
		R_.val[2][0] = -b_.val[1][0];
		R_.val[1][2] = -b_.val[0][0];
		R_.val[2][1] = +b_.val[0][0];

		// orthonormalized rotation matrix
		Matrix U, W, V;
		R_.svd(U, W, V);
		R_ = U*~V;

		// fix improper matrix problem
		if (R_.det() < 0){
			Matrix B = Matrix::eye(3);
			B.val[2][2] = R_.det();
			R_ = V*B*~U;
		}

		// translation vector
		Matrix t_(3, 1);
		t_.val[0][0] = b_.val[3][0];
		t_.val[1][0] = b_.val[4][0];
		t_.val[2][0] = b_.val[5][0];

		// compose: R|t = R_|t_ * R|t
		R = R_*R;
		t = R_*t + t_;

		threshold = max((R_ - Matrix::eye(3)).l2norm(), t_.l2norm());
		if (threshold < min_delta){
			break;
		}

	}

	//pCmDev->DestroyBufferUP(surf1);
	//pCmDev->DestroyBufferUP(surf2);
	//pCmDev->DestroyBufferUP(surf3);
	//pCmDev->DestroyKernel(kernel_distance);
	//pCmDev->DestroyTask(pKernelArray_distance);

}
//
//double ICP::fitStep(CmDevice* pCmDev, CmProgram* program, PointCloud temp, PointCloud model, int number_of_points, Matrix &R, Matrix &t) {
//
//	Matrix p_m(number_of_points, 3);
//	Matrix p_t(number_of_points, 3);
//
//	double r00 = R.val[0][0]; double r01 = R.val[0][1]; double r02 = R.val[0][2];
//	double r10 = R.val[1][0]; double r11 = R.val[1][1]; double r12 = R.val[1][2];
//	double r20 = R.val[2][0]; double r21 = R.val[2][1]; double r22 = R.val[2][2];
//	double t0 = t.val[0][0]; double t1 = t.val[1][0]; double t2 = t.val[2][0];
//
//	// init A and b
//	Matrix A(number_of_points, 6);
//	Matrix b(number_of_points, 1);
//
//	CmKernel* kernel_distance = NULL;
//	SurfaceIndex* pInputIndex1 = NULL;
//	SurfaceIndex* pInputIndex2 = NULL;
//	SurfaceIndex* pOutputIndex1 = NULL;
//	CmQueue* pCmQueue = NULL;
//	CmTask *pKernelArray_distance = NULL;
//	CmEvent* e = NULL;
//
//	int new_size = 2048;
//	float* input_positions_model = (float *)CM_ALIGNED_MALLOC(new_size * 4 * sizeof(float), 0x1000);
//	float* input_positions_template = (float *)CM_ALIGNED_MALLOC(new_size * 4 * sizeof(float), 0x1000);
//	float* output_match = (float *)CM_ALIGNED_MALLOC(new_size * 4 * sizeof(float), 0x1000);
//
//	int threads = new_size / 32;
//	//Calculate distance on gpu
//	pCmDev->CreateKernel(program, CM_KERNEL_FUNCTION(get_correspondance), kernel_distance);
//	kernel_distance->SetThreadCount(threads);
//	CmBufferUP* surf1 = NULL;
//	pCmDev->CreateBufferUP(new_size * 4 * sizeof(float), input_positions_model, surf1);
//	surf1->GetIndex(pInputIndex1);
//	CmBufferUP* surf2 = NULL;
//	pCmDev->CreateBufferUP(new_size * 4 * sizeof(float), input_positions_template, surf2);
//	surf2->GetIndex(pInputIndex2);
//	CmBufferUP* surf3 = NULL;
//	pCmDev->CreateBufferUP(new_size * 4 * sizeof(float), output_match, surf3);
//	surf3->GetIndex(pOutputIndex1);
//
//	kernel_distance->SetKernelArg(0, sizeof(SurfaceIndex), pInputIndex1);
//	kernel_distance->SetKernelArg(1, sizeof(SurfaceIndex), pInputIndex2);
//	kernel_distance->SetKernelArg(2, sizeof(SurfaceIndex), pOutputIndex1);
//	int threadId_lookup = 0;
//	for (int n = 0; n < threads; n++)
//	{
//		kernel_distance->SetThreadArg(threadId_lookup, 3, sizeof(int), &threadId_lookup);
//		threadId_lookup++;
//	}
//
//	pCmDev->CreateQueue(pCmQueue);
//	pCmDev->CreateTask(pKernelArray_distance);
//	pKernelArray_distance->AddKernel(kernel_distance);
//
//	int r = 0;
//	for (int j = 0; j < temp.points.size(); j++){
//		input_positions_template[r] = temp.points[j].position.x;
//		input_positions_template[r + 1] = temp.points[j].position.y;
//		input_positions_template[r + 2] = temp.points[j].position.z;
//		input_positions_template[r + 3] = 1.0f;
//		r = r + 4;
//	}
//	for (int v = (int)temp.points.size(); v < new_size; v++){
//		input_positions_template[r] = 0.0f;
//		input_positions_template[r + 1] = 0.0f;
//		input_positions_template[r + 2] = 0.0f;
//		input_positions_template[r + 3] = 0.0f;
//		r = r + 4;
//	}
//
//	int r1 = 0;
//	for (int j = 0; j < model.points.size(); j++){
//		input_positions_model[r1] = model.points[j].position.x;
//		input_positions_model[r1 + 1] = model.points[j].position.y;
//		input_positions_model[r1 + 2] = model.points[j].position.z;
//		input_positions_model[r1 + 3] = 1.0f;
//		r1 = r1 + 4;
//	}
//	for (int v = (int)model.points.size(); v < new_size; v++){
//		input_positions_model[r1] = 0.0f;
//		input_positions_model[r1 + 1] = 0.0f;
//		input_positions_model[r1 + 2] = 0.0f;
//		input_positions_model[r1 + 3] = 0.0f;
//		r1 = r1 + 4;
//	}
//	cout << model.points.size() << endl;
//	cout << temp.points.size() << endl;
//	int u_counter = 0;
//	// establish correspondences
//	for (int i = 0; i < temp.points.size(); i++){
//		std::vector<float> query(3);
//
//		// transform point according to R|t
//		query[0] = (float)(r00*temp.points[i].position.x + r01*temp.points[i].position.y + r02*temp.points[i].position.z + t0);
//		query[1] = (float)(r10*temp.points[i].position.x + r11*temp.points[i].position.y + r12*temp.points[i].position.z + t1);
//		query[2] = (float)(r20*temp.points[i].position.x + r21*temp.points[i].position.y + r22*temp.points[i].position.z + t2);
//
//		vector3f neighbor;
//
//		kernel_distance->SetKernelArg(0, sizeof(SurfaceIndex), pInputIndex1);
//		kernel_distance->SetKernelArg(1, sizeof(SurfaceIndex), pInputIndex2);
//		kernel_distance->SetKernelArg(2, sizeof(SurfaceIndex), pOutputIndex1);
//		pCmQueue->Enqueue(pKernelArray_distance, e);
//
//		vector<float> correspondance;
//		correspondance.assign(output_match, output_match + new_size);
//		cout << correspondance.size() << endl;
//		neighbor.x = correspondance[u_counter];
//		neighbor.y = correspondance[u_counter + 1];
//		neighbor.z = correspondance[u_counter + 2];
//		
//		//-------------------------------------------
//
//		// model point
//		double dx = neighbor.x;
//		double dy = neighbor.y;
//		double dz = neighbor.z;
//
//		// model point normal
//		double nx = temp.points[i].normal_vector.x;
//		double ny = temp.points[i].normal_vector.y;
//		double nz = temp.points[i].normal_vector.z;
//
//		// template point
//		double sx = query[0];
//		double sy = query[1];
//		double sz = query[2];
//
//		// setup least squares system
//		A.val[i][0] = nz*sy - ny*sz;
//		A.val[i][1] = nx*sz - nz*sx;
//		A.val[i][2] = ny*sx - nx*sy;
//		A.val[i][3] = nx;
//		A.val[i][4] = ny;
//		A.val[i][5] = nz;
//		b.val[i][0] = nx*(dx - sx) + ny*(dy - sy) + nz*(dz - sz); //nx*dx+ny*dy+nz*dz-nx*sx-ny*sy-nz*sz;    
//
//		u_counter = u_counter + 4;
//	}
//
//	pCmDev->DestroyBufferUP(surf1);
//	pCmDev->DestroyBufferUP(surf2);
//	pCmDev->DestroyBufferUP(surf3);
//	pCmDev->DestroyKernel(kernel_distance);
//	pCmDev->DestroyTask(pKernelArray_distance);
//
//	// solve linear least squares
//
//	// use the normal equations
//	Matrix A_ = ~A*A;
//	Matrix b_ = ~A*b;
//
//	if (!b_.solve(A_)) return 0; // failure
//
//	// rotation matrix
//	Matrix R_ = Matrix::eye(3);
//	R_.val[0][1] = -b_.val[2][0];
//	R_.val[1][0] = +b_.val[2][0];
//	R_.val[0][2] = +b_.val[1][0];
//	R_.val[2][0] = -b_.val[1][0];
//	R_.val[1][2] = -b_.val[0][0];
//	R_.val[2][1] = +b_.val[0][0];
//
//	// orthonormalized rotation matrix
//	Matrix U, W, V;
//	R_.svd(U, W, V);
//	R_ = U*~V;
//
//	// fix improper matrix problem
//	if (R_.det() < 0){
//		Matrix B = Matrix::eye(3);
//		B.val[2][2] = R_.det();
//		R_ = V*B*~U;
//	}
//
//	// translation vector
//	Matrix t_(3, 1);
//	t_.val[0][0] = b_.val[3][0];
//	t_.val[1][0] = b_.val[4][0];
//	t_.val[2][0] = b_.val[5][0];
//
//	// compose: R|t = R_|t_ * R|t
//	R = R_*R;
//	t = R_*t + t_;
//	return max((R_ - Matrix::eye(3)).l2norm(), t_.l2norm());
//}