#pragma once

#include "PointCloud.h"

using namespace std;

class ICP {

public:

	ICP();

	~ICP();

	void setMaxIterations(int32_t val) { max_iter = val; }

	void setMinDeltaParam(double  val) { min_delta = val; }

	void fit(CmDevice* pCmDev, CmProgram* program, PointCloud temp, PointCloud model, int number_of_points, Matrix &R, Matrix &t);

	void fitIterate(CmDevice* pCmDev, CmProgram* program, PointCloud temp, PointCloud model, int number_of_points, Matrix &R, Matrix &t);

	double fitStep(CmDevice* pCmDev, CmProgram* program, PointCloud temp, PointCloud model, int number_of_points, Matrix &R, Matrix &t);

protected:

	int32_t max_iter;

	double  min_delta;
};