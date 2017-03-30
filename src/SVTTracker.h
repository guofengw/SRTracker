#ifndef _SVT
#define _SVT

#include <cv.h>
#include <cxcore.h>
#include "ObjectTemplate.h"
#include "ParticleFilter.h"
#include "Params.h"


class SVTTracker
{
public:
	SVTTracker();
	~SVTTracker();
public:
	static void MouseCallback(int event, int x, int y, int flags, void *param);
public:
	void Init(IplImage *pImg);
	void Tracking(IplImage *pImg, IplImage *pColorImg);

	void Gaussian(CvMat *dataMat);
	void Normalization(CvMat *dataMat);

	//����任�����ϵ��֮��ı任��ϵ
	void AffineMatrix2Param(AffineMatrix &affineMatrix, AffineParam &affineParam);
	void AffineParam2Matrix(AffineParam &affineParam, AffineMatrix &affineMatrix);

	//���ɷ���任����
	int GenerateAffineMatrix(CvPoint upLeft, CvPoint downLeft, CvPoint upRight, AffineMatrix &affineMatrix, IplImage *pImg);

	//����Ŀ���λ���Լ���Ӧ������
	void GenerateLocationMatrix(IplImage *pImg, CvMat *mat, AffineMatrix affineMatrix);
	void GenerateDataMatrix(IplImage *pImg,CvMat *dataMat,CvMat *locationMat);	
	void ComputeMatrixLLC(SparseParams sparseParams);
	void ComputeMatrixCSC(SparseParams sparseParams);
	void ComputeOriginalPF(SparseParams sparseParams);
	void DrawRectangle(CvPoint upLeft, CvPoint downLeft, CvPoint upRight, IplImage *pImg);

public:
	CvPoint upLeft;
	CvPoint downLeft;
	CvPoint upRight;
	AffineMatrix affineMatrix;
	AffineParam affineParam;
	CvMat *std_affine;

	SparseParams m_sparseParams;
	ObjectTemplate *m_objTemplate;
	TrackerType type;
	std::vector<double> x_t_1;
private:
	ParticleFilter *m_pf;
	CvMat *origin;
	CvMat *affine;

	
};


#endif