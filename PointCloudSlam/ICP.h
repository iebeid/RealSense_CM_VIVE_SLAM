#ifndef FPS_H
#define FPS_H 1

#include "PointCloud.h"

using namespace std;

class ICP {

public:

	ICP();

	~ICP();

	void setMaxIterations(int val) { max_iter = val; }

	void setMinDeltaParam(double  val) { min_delta = val; }

	void fit(CmDevice* pCmDev, CmProgram* program, PointCloud temp, PointCloud model, int number_of_points, Matrix &R, Matrix &t);

	double fitStep(CmDevice* pCmDev, CmProgram* program, PointCloud temp, PointCloud model, int number_of_points, Matrix &R, Matrix &t);

protected:

	int max_iter;

	double  min_delta;

	int new_size;
};

#endif