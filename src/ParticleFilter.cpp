#include "stdafx.h"
#include "ParticleFilter.h"
#include <time.h>
#include "SVTTracker.h"   


PFParams& PFParams::operator =(const PFParams &p)
{
	this->affineMatrix = p.affineMatrix;
	this->affineParam = p.affineParam;
	this->downLeft = p.downLeft;
	this->upLeft = p.upLeft;
	this->upRight = p.upRight;
	for(int i=0; i<DLONG; i++)
	{
		this->data_LONG_1->data.db[i] = p.data_LONG_1->data.db[i];
	}

	return (*this);
}
ParticleFilter::ParticleFilter(SVTTracker *svtTracker)
{
	this->m_svtTracker = svtTracker;
	for(int i=0; i<PNUM; i++)
	{
		this->pfParams[i].data_LONG_1 = cvCreateMat(DLONG,1,CV_64FC1);
		this->tempParams[i].data_LONG_1 = cvCreateMat(DLONG,1,CV_64FC1);
		cvZero(this->pfParams[i].data_LONG_1);
	}
}


ParticleFilter::~ParticleFilter()
{
	for(int i=0; i<PNUM; i++)
	{
		cvReleaseMat(&this->pfParams[i].data_LONG_1);
		cvReleaseMat(&this->tempParams[i].data_LONG_1);
	}
	this->m_svtTracker = NULL;
}

void ParticleFilter::Resample(TrackerType type)
{
	switch(type)
	{
	case CSC:
		for(int i=0; i<PNUM; i++)
		{
			this->pfParams[i].affineMatrix = this->m_svtTracker->affineMatrix;
			this->pfParams[i].affineParam = this->m_svtTracker->affineParam;
			this->pfParams[i].upLeft = this->m_svtTracker->upLeft;
			this->pfParams[i].downLeft = this->m_svtTracker->downLeft;
			this->pfParams[i].upRight = this->m_svtTracker->upRight;
		}
		break;
		
	case LLC || PF:
		{
			//粒子重采样策略，轮盘赌图方式
			double cumulative[PNUM];
			for(int i=0;i<PNUM;i++)
				cumulative[i]=0;
			cumulative[0]=this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[0];
			for(int i=1;i<PNUM;i++)
			{
				cumulative[i]=cumulative[i-1]+this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i-1];      
			}
			float f=0;
			int m=0;
			for(int i=0;i<PNUM;i++)
			{

				f=(float)rand()/RAND_MAX;     //生成0-1随机数
				for(int j=0;j<PNUM;j++)
				{
					if(f>=cumulative[j]&&f<cumulative[j+1])
					{
						m=j;
						break;
					}
				}
				this->tempParams[i] = this->pfParams[m];
			}
			for(int i=0;i<PNUM;i++)
			{
				this->pfParams[i] = this->tempParams[i];
			}
		}
	}
}

void ParticleFilter::Propograte()
{
	CvMat *randMat = cvCreateMat(6,1,CV_64FC1);
	CvMat *temp = cvCreateMat(6,1,CV_64FC1);

	//设定种子
	//CvRNG rng = cvRNG(time(NULL));
	CvRNG rng = cvRNG(1);

	for(int i=0; i<PNUM; i++)
	{
		cvRandArr(&rng,randMat,1,cvScalar(0,0,0,0),cvScalar(1,1,1,1));
		cvMul(randMat,this->m_svtTracker->std_affine,temp,1);

		this->pfParams[i].affineParam.th += temp->data.db[0];
		this->pfParams[i].affineParam.sc += temp->data.db[1];
		this->pfParams[i].affineParam.sr += temp->data.db[2];
		this->pfParams[i].affineParam.phi += temp->data.db[3];
		this->pfParams[i].affineParam.tx += temp->data.db[4];
		this->pfParams[i].affineParam.ty += temp->data.db[5];
	}

	cvReleaseMat(&randMat);
	cvReleaseMat(&temp);
}

void ParticleFilter::CalculateWeight()
{
	double sum = 0;

	for(int i=0; i<PNUM; i++)
	{
		if(this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i] > 0)
		{
			sum += this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i];
		}
		else
		{
			this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i] = 0;
		}

	}
	for(int i=0; i<PNUM; i++)
		this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i] /= sum;
}

void ParticleFilter::EstimateLocation(IplImage *pImg)
{
	this->m_svtTracker->affineParam.phi = 0;
	this->m_svtTracker->affineParam.sc = 0;
	this->m_svtTracker->affineParam.sr = 0;
	this->m_svtTracker->affineParam.th = 0;
	this->m_svtTracker->affineParam.tx = 0;
	this->m_svtTracker->affineParam.ty = 0;

	for(int i=0; i<PNUM; i++)
	{
		this->m_svtTracker->affineParam.phi += this->pfParams[i].affineParam.phi*this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i];
		this->m_svtTracker->affineParam.sc += this->pfParams[i].affineParam.sc*this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i];
		this->m_svtTracker->affineParam.sr += this->pfParams[i].affineParam.sr*this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i];
		this->m_svtTracker->affineParam.th += this->pfParams[i].affineParam.th*this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i];
		this->m_svtTracker->affineParam.tx += this->pfParams[i].affineParam.tx*this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i];
		this->m_svtTracker->affineParam.ty += this->pfParams[i].affineParam.ty*this->m_svtTracker->m_sparseParams.alpha_PNUM_1->data.db[i];


	}

	this->m_svtTracker->AffineParam2Matrix(this->m_svtTracker->affineParam,this->m_svtTracker->affineMatrix);


	//看看结果是否正确，posMat的最后3个是否都是1
	CvMat *posMat = cvCreateMat(3,DLONG,CV_64FC1);
	this->m_svtTracker->GenerateLocationMatrix(pImg,posMat,this->m_svtTracker->affineMatrix);

	//x,y真麻烦
	this->m_svtTracker->upLeft.x = (int)posMat->data.db[0];
	this->m_svtTracker->upLeft.y = (int)posMat->data.db[DLONG];
	this->m_svtTracker->downLeft.x = (int)posMat->data.db[HEIGHT-1];
	this->m_svtTracker->downLeft.y = (int)posMat->data.db[DLONG+HEIGHT-1];
	this->m_svtTracker->upRight.x = (int)posMat->data.db[DLONG-HEIGHT];           //有问题不，要不要减1呢？
	this->m_svtTracker->upRight.y = (int)posMat->data.db[2*DLONG-HEIGHT];

	cvReleaseMat(&posMat);
}