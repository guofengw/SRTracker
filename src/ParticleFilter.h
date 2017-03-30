#ifndef _ParticleFilter
#define _ParticleFilter
#include <cxcore.h>
#include <cv.h>
#include "Params.h"

class SVTTracker;         

//粒子滤波的数据结构
class PFParams {
public:
	PFParams& operator= (const PFParams &p);
public:
	CvPoint upLeft;
	CvPoint downLeft;
	CvPoint upRight;
	AffineMatrix affineMatrix;
	AffineParam affineParam;
	CvMat *data_LONG_1;
};

class ParticleFilter
{
public:
	ParticleFilter(SVTTracker *svtTracker);
	~ParticleFilter();

public:
	void Resample(TrackerType type);
	void Propograte();
	void CalculateWeight();
	void EstimateLocation(IplImage *pImg);

public:
	PFParams pfParams[PNUM];
	PFParams tempParams[PNUM];

protected:
	SVTTracker *m_svtTracker;
	

};

#endif