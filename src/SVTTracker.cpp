#include "stdafx.h"
#include <time.h>
#include "SVTTracker.h"
#include <highgui.h>
#include <decomp.h>

//初始化各个参数
SVTTracker::SVTTracker()
{
	this->upLeft.x = 0;
	this->upLeft.y = 0;
	this->upRight.x = 0;
	this->upRight.y = 0;
	this->downLeft.x = 0;
	this->downLeft.y = 0;
/*************************SVTTracker参数分配空间**************************/
	this->std_affine = cvCreateMat(6,1,CV_64FC1);
	this->m_pf = new ParticleFilter(this);
	this->m_objTemplate = new ObjectTemplate(this);
	cvZero(this->std_affine);

/*************************SparseParams参数分配空间*************************/
	this->m_sparseParams.Xt_LONG_1 = cvCreateMat(DLONG,1,CV_64FC1);
	this->m_sparseParams.y_LONG_1 = cvCreateMat(DLONG,1,CV_64FC1);
	this->m_sparseParams.alpha_PNUM_1 = cvCreateMat(PNUM,1,CV_64FC1);
	this->m_sparseParams.c_TENUM_1 = cvCreateMat(TENUM,1,CV_64FC1);
	this->m_sparseParams.D_LONG_PNUM = cvCreateMat(DLONG,PNUM,CV_64FC1);
	this->m_sparseParams.B_LONG_TENUM = cvCreateMat(DLONG,TENUM,CV_64FC1);
	this->m_sparseParams.y_LONG_1_display = cvCreateMat(DLONG,1,CV_64FC1);
	this->m_sparseParams.T_LONG_TNUM = cvCreateMat(DLONG,TNUM,CV_64FC1);

	origin = cvCreateMat(3,DLONG,CV_64FC1);
	affine = cvCreateMat(3,3,CV_64FC1);
	x_t_1.resize(DLONG);
}

SVTTracker::~SVTTracker()
{

/************************SparseParams参数内存释放************************/
	cvReleaseMat(&this->m_sparseParams.Xt_LONG_1);
	cvReleaseMat(&this->m_sparseParams.y_LONG_1);
	cvReleaseMat(&this->m_sparseParams.alpha_PNUM_1);
	cvReleaseMat(&this->m_sparseParams.D_LONG_PNUM);
	cvReleaseMat(&this->m_sparseParams.B_LONG_TENUM);
	cvReleaseMat(&this->m_sparseParams.y_LONG_1_display);
	cvReleaseMat(&this->m_sparseParams.T_LONG_TNUM);
	cvReleaseMat(&this->std_affine);
	delete this->m_pf;
	delete this->m_objTemplate;

	cvReleaseMat(&origin);
	cvReleaseMat(&affine);
}

void SVTTracker::DrawRectangle(CvPoint upLeft, CvPoint downLeft, CvPoint upRight, IplImage *pImg)
{
	CvPoint downRight;
	downRight.x = downLeft.x + upRight.x - upLeft.x;
	downRight.y = downLeft.y + upRight.y - upLeft.y;
	
	//used for Demo
	if(pImg->nChannels == 3)
	{
		cvLine(pImg,upLeft,upRight,CV_RGB(255,0,0),2);
		cvLine(pImg,upLeft,downLeft,CV_RGB(255,0,0),2);
		cvLine(pImg,upRight,downRight,CV_RGB(255,0,0),2);
		cvLine(pImg,downLeft,downRight,CV_RGB(255,0,0),2);
	}
	else
	{
		cvLine(pImg,upLeft,upRight,CV_RGB(255,255,255),2);
		cvLine(pImg,upLeft,downLeft,CV_RGB(255,255,255),2);
		cvLine(pImg,upRight,downRight,CV_RGB(255,255,255),2);
		cvLine(pImg,downLeft,downRight,CV_RGB(255,255,255),2);
	}

}

//初始化粒子滤波
void SVTTracker::Init(IplImage *pImg)
{
	if(pImg == NULL)
	{
		throw "Cann't exit Image";
		exit(0);
	}
	if(this->upLeft.x < 0) this->upLeft.x = 0;
	if(this->upLeft.y < 0) this->upLeft.y = 0;
	if(this->downLeft.x < 0) this->downLeft.x = 0;
	if(this->downLeft.y > pImg->height) this->downLeft.y = pImg->height;
	if(this->upRight.x > pImg->width) this->upRight.x = pImg->width;
	if(this->upRight.y < 0) this->upRight.y = 0;

	this->GenerateAffineMatrix(this->upLeft,this->downLeft,this->upRight,this->affineMatrix,pImg);
	this->AffineMatrix2Param(this->affineMatrix,this->affineParam);

/**************************SparseParams初始化*************************/
	cvZero(this->m_sparseParams.Xt_LONG_1);
	cvZero(this->m_sparseParams.y_LONG_1);
	cvZero(this->m_sparseParams.D_LONG_PNUM);
	cvZero(this->m_sparseParams.B_LONG_TENUM);
	cvZero(this->m_sparseParams.alpha_PNUM_1);
	cvZero(this->m_sparseParams.c_TENUM_1);
	cvZero(this->m_sparseParams.y_LONG_1_display);
	cvZero(this->m_sparseParams.T_LONG_TNUM);
	int j = 0;
	int k = 0;
	for(int i=TNUM; i<TENUM; i++)
	{
		if(i<TNUM+DLONG)
		{
			this->m_sparseParams.B_LONG_TENUM->data.db[i*DLONG+j] = 1;
			j++;
		}
		else
		{
			this->m_sparseParams.B_LONG_TENUM->data.db[i*DLONG+k] = -1;
			k++;
		}

	}

/**************************粒子初始化*********************************/
	
	CvMat *randMat = cvCreateMat(6,1,CV_64FC1);
	CvMat *_temp = cvCreateMat(6,1,CV_64FC1);
	//CvRNG rng = cvRNG(time(NULL));
	CvRNG rng = cvRNG(1);
	for(int i=0; i<PNUM; i++)
	{
		this->m_pf->pfParams[i].upLeft = this->upLeft;
		this->m_pf->pfParams[i].downLeft = this->downLeft;
		this->m_pf->pfParams[i].upRight = this->upRight;
		this->m_pf->pfParams[i].affineMatrix = this->affineMatrix;
		this->m_pf->pfParams[i].affineParam = this->affineParam;

		cvRandArr(&rng,randMat,1,cvScalar(0,0,0,0),cvScalar(1,1,1,1));
		cvMul(randMat,this->std_affine,_temp,1);

		this->m_pf->pfParams[i].affineParam.tx += _temp->data.db[4];
		this->m_pf->pfParams[i].affineParam.ty += _temp->data.db[5];
		this->m_pf->pfParams[i].affineParam.th += _temp->data.db[0];
		this->m_pf->pfParams[i].affineParam.sc += _temp->data.db[1];
		this->m_pf->pfParams[i].affineParam.sr += _temp->data.db[2];
		this->m_pf->pfParams[i].affineParam.phi += _temp->data.db[3];

	}

	cvReleaseMat(&randMat);
	cvReleaseMat(&_temp);

/**************************模板初始化*********************************/
	
	for(int i=0; i<TNUM; i++)
	{
			this->m_objTemplate->templateParams[i].upLeft = this->upLeft;
			this->m_objTemplate->templateParams[i].downLeft = this->downLeft;            
			this->m_objTemplate->templateParams[i].upRight = this->upRight;
		
	}
	
	if(TNUM >=10)
	{
		this->m_objTemplate->templateParams[1].upLeft.x -= 1;
		this->m_objTemplate->templateParams[2].upLeft.y += 2;
		this->m_objTemplate->templateParams[2].downLeft.y += 1;
		this->m_objTemplate->templateParams[3].upRight.x -= 2;
		this->m_objTemplate->templateParams[4].upRight.x += 1;
		this->m_objTemplate->templateParams[5].downLeft.x -= 2;
		this->m_objTemplate->templateParams[6].downLeft.x += 1;
		this->m_objTemplate->templateParams[7].upLeft.x -= 2;
		this->m_objTemplate->templateParams[7].downLeft.x -= 1;
		this->m_objTemplate->templateParams[8].upLeft.x += 2;
		this->m_objTemplate->templateParams[8].downLeft.x += 1;
		this->m_objTemplate->templateParams[9].upLeft.y -= 2;
		this->m_objTemplate->templateParams[9].downLeft.y -= 1;
	}

	this->m_objTemplate->GenerateTemplate(pImg);


}


void SVTTracker::Gaussian(CvMat *dataMat)
{
	
	if(dataMat->width == 1)
	{
		CvScalar m_scalar;
		CvScalar std_scalar;
		cvAvgSdv(dataMat,&m_scalar,&std_scalar);
		
		
		cvSubS(dataMat,m_scalar,dataMat);
		cvAddWeighted(dataMat,1.0/std_scalar.val[0],dataMat,0,0,dataMat);


	}
	else
	{
		int length;
		double mean;
		double std;
		length = dataMat->height;
		CvMat *data = cvCreateMat(dataMat->height,1,CV_64FC1);
		for(int i=0; i<dataMat->width; i++)
		{

			//先把数据拷贝到data中去
			for(int j=0; j<dataMat->height; j++)
			{
				data->data.db[j] = dataMat->data.db[j*dataMat->width+i];
			}

			CvScalar m_scalar = cvAvg(data);
			CvScalar std_scalar;
			cvAvgSdv(data,&m_scalar,&std_scalar);
			mean = m_scalar.val[0];
			std = std_scalar.val[0];
			for(int j=0; j<length; j++)
			{
				//把数据拷回来
				dataMat->data.db[j*dataMat->width+i] = (data->data.db[j]-mean)/std;
			}
		}
		cvReleaseMat(&data);
	}
}

void SVTTracker::Normalization(CvMat *dataMat)  //速度太慢
{
	int length = dataMat->height;
	CvMat *data = cvCreateMat(dataMat->height,1,CV_64FC1);
	for(int i=0; i<dataMat->width; i++)
	{
		double sum = 0;
		for(int j=0; j<length; j++)
		{
			data->data.db[j] = dataMat->data.db[j*dataMat->width+i];
			sum += data->data.db[j]*data->data.db[j];
		}
		sum = sqrt(sum);

		for(int j=0; j<length;j++)
		{
			dataMat->data.db[j*dataMat->width+i] = data->data.db[j]/sum;
		}
	}
	cvReleaseMat(&data);
}

int SVTTracker::GenerateAffineMatrix(CvPoint upLeft, CvPoint downLeft, CvPoint upRight, AffineMatrix &affineMatrix, IplImage *pImg)
{
	//先判断目标的位置是否准确
	if(upLeft.x<0 || upLeft.y<0)
		return 0;
	if(downLeft.x<0 || downLeft.y>pImg->height)
		return 0;
	if(upRight.x>pImg->width || upRight.y<0)
		return 0;

	//mat1代表原始的像素坐标
	CvMat *mat1 = cvCreateMat(3,3,CV_64FC1);
	mat1->data.db[0] = -WIDTH/2;  mat1->data.db[1] = -WIDTH/2; mat1->data.db[2] = WIDTH/2;
	mat1->data.db[3] = -HEIGHT/2;  mat1->data.db[4] = HEIGHT/2;  mat1->data.db[5] = -HEIGHT/2;
	mat1->data.db[6] = 1;  mat1->data.db[7] = 1;  mat1->data.db[8] = 1;

	//mat2代表当前图像中相应像素的坐标，注意图像中横坐标为x，纵坐标为y，这里刚好相反，很容易出错
	CvMat *mat2 = cvCreateMat(3,3,CV_64FC1);
	mat2->data.db[0] = upLeft.x;  mat2->data.db[1] = downLeft.x;  mat2->data.db[2] = upRight.x;
	mat2->data.db[3] = upLeft.y;  mat2->data.db[4] = downLeft.y;  mat2->data.db[5] = upRight.y;
	mat2->data.db[6] = 1;         mat2->data.db[7] = 1;          mat2->data.db[8] = 1;

	//保存矩阵的中间变量
	CvMat *mat3 = cvCreateMat(3,3,CV_64FC1);

	//求解仿射变换矩阵
	cvInvert(mat1,mat3,CV_NORMAL);
	cvMatMul(mat2,mat3,mat1);

	//把仿射变换矩阵保存到类中相应的变量_affineMatrix中
	affineMatrix.a1 = mat1->data.db[0];
	affineMatrix.a2 = mat1->data.db[1];
	affineMatrix.a3 = mat1->data.db[3];
	affineMatrix.a4 = mat1->data.db[4];
	affineMatrix.tx = mat1->data.db[2];
	affineMatrix.ty = mat1->data.db[5];

	//释放申请的矩阵空间
	cvReleaseMat(&mat1);
	cvReleaseMat(&mat2);
	cvReleaseMat(&mat3);

	return true;
}

void SVTTracker::AffineMatrix2Param(AffineMatrix &affineMatrix, AffineParam &affineParam)
{
	affineParam.tx = affineMatrix.tx;
	affineParam.ty = affineMatrix.ty;
	affineParam.sc = sqrt(affineMatrix.a1*affineMatrix.a1+affineMatrix.a3*affineMatrix.a3);

	if(affineMatrix.a1<0 && affineMatrix.a3<0)
	{
		affineParam.th = -acos(affineMatrix.a1/affineParam.sc);
	}
	else if(affineMatrix.a1<0)
	{
		affineParam.th = acos(affineMatrix.a1/affineParam.sc);
	}
	else if(affineMatrix.a3<0)
	{
		affineParam.th = asin(affineMatrix.a3/affineParam.sc);
	}
	else
	{
		affineParam.th = acos(affineMatrix.a1/affineParam.sc);
	}


	double sinth = sin(affineParam.th);
	double costh = cos(affineParam.th);
	affineParam.phi = (affineMatrix.a4*sinth+affineMatrix.a2*costh)/(affineMatrix.a4*costh-affineMatrix.a2*sinth);
	if((affineParam.phi*costh - sinth)==0)
	{
		affineParam.sr = affineMatrix.a4/(affineParam.phi*sinth+costh);
		affineParam.sr = affineParam.sr/affineParam.sc;
	}
	else
	{
		affineParam.sr = affineMatrix.a2/(affineParam.phi*costh - sinth);
		affineParam.sr = affineParam.sr/affineParam.sc;
	}
}

void SVTTracker::AffineParam2Matrix(AffineParam &affineParam, AffineMatrix &affineMatrix)
{
	affineMatrix.tx = affineParam.tx;
	affineMatrix.ty = affineParam.ty;
	double sinth = sin(affineParam.th);
	double costh = cos(affineParam.th);
	affineMatrix.a1 = affineParam.sc*costh;
	double sy = affineParam.sc*affineParam.sr;
	affineMatrix.a2 = affineParam.phi*sy*costh - sy*sinth;
	affineMatrix.a3 = affineParam.sc*sinth;
	affineMatrix.a4 = affineParam.phi*sy*sinth + sy*costh;
}

void SVTTracker::GenerateLocationMatrix(IplImage *pImg, CvMat *mat, AffineMatrix affineMatrix)
{

	int row = mat->rows;
	int col = mat->cols;


	//标准坐标下目标的位置
	for(int i=-WIDTH/2; i<WIDTH/2.0; i++)
		for(int j=-HEIGHT/2; j<HEIGHT/2.0; j++)
		{
			origin->data.db[DLONG*0+HEIGHT*(i+WIDTH/2)+j+HEIGHT/2] = i;
			origin->data.db[DLONG*1+HEIGHT*(i+WIDTH/2)+j+HEIGHT/2] = j;
			origin->data.db[DLONG*2+HEIGHT*(i+WIDTH/2)+j+HEIGHT/2] = 1;
		}

	affine->data.db[0] = affineMatrix.a1;
	affine->data.db[1] = affineMatrix.a2;
	affine->data.db[3] = affineMatrix.a3;
	affine->data.db[4] = affineMatrix.a4;
	affine->data.db[2] = affineMatrix.tx;
	affine->data.db[5] = affineMatrix.ty;

	affine->data.db[6] = 0;
	affine->data.db[7] = 0;
	affine->data.db[8] = 1;

	cvMatMul(affine,origin,mat);



}

void SVTTracker::GenerateDataMatrix(IplImage *pImg, CvMat *dataMat, CvMat *locationMat)
{
	//locationMat是个3*DLONG的矩阵，第一行为高，第二行为宽，第三行为1，对应图像中的y和x，很容易位置出错
	if(locationMat != NULL && dataMat != NULL)
	{
		//x，y代表图像中的位置，跟locationMat中的位置相反
		int x = 0;
		int y = 0;
		int channel = CV_MAT_CN(dataMat->type);


		//数据是按列先排列
		for(int i=0; i<DLONG; i++)
		{
			y = (int)locationMat->data.db[DLONG+i];           //x,y要增加判断条件，否则会出现访问错误
			x = (int)locationMat->data.db[i];
			if(channel == 1)
			{
				//得到xy坐标下的数据值
				if(x>0 && x<pImg->width && y>0 && y<pImg->height)
					dataMat->data.db[i] = (uchar)pImg->imageData[pImg->widthStep*y+x];
				else
					dataMat->data.db[i] = 0;
			}
			if(channel == 3)
			{
				//得到xy坐标下的数据值
				if(x>0 && x<pImg->width && y>0 && y<pImg->height)
				{
					dataMat->data.db[i*3] = (uchar)pImg->imageData[pImg->widthStep*y+3*x];
					dataMat->data.db[i*3+1] = (uchar)pImg->imageData[pImg->widthStep*y+3*x+1];
					dataMat->data.db[i*3+2] = (uchar)pImg->imageData[pImg->widthStep*y+3*x+2];
				}
				else
				{
					dataMat->data.db[i*3] = 0;
					dataMat->data.db[i*3+1] = 0;
					dataMat->data.db[i*3+2] = 0;
				}

			}

		}

		//cvShowImage("l5",dataMat);
	}
}

#pragma region LLC
void SVTTracker::ComputeMatrixLLC(SparseParams sparseParams)
{
	Matrix<double> y(sparseParams.y_LONG_1->data.db,DLONG,1);
	Matrix<double> D(sparseParams.D_LONG_PNUM->data.db,DLONG,PNUM);
	Matrix<double> B(sparseParams.B_LONG_TENUM->data.db,DLONG,TENUM);
	Matrix<double> Xt(sparseParams.Xt_LONG_1->data.db,DLONG,1);
	SpMatrix<double> alpha;
	SpMatrix<double> c;

	cvZero(this->m_sparseParams.alpha_PNUM_1);
	Matrix<double> y1;
	lasso2<double>(y,B,c,20,0.06,0,PENALTY,true,-1,NULL);
	B.mult(c,y1,false,false,1.0,0.0);

	LCC_Coding<double>(y1,D,alpha,15,0.0001f);
	Matrix<double> x;
	D.mult(alpha,x,false,false,1.0,0.0);

	Vector<double> vx;
	x.getData(vx,0);
	Vector<double> vy;
	y.getData(vy,0);
	double residual = 0;
	for(int i=0; i<vx.n(); i++)
	{
		residual +=(vx[i]-vy[i])*(vx[i]-vy[i]);
	}

	int count=0;
	Matrix<double> tmp;
	double residual1 = 0;
	while(residual > 0.001)  
	{
		residual1 = residual;
		residual = 0;
		tmp.copy(x);
		
		lasso2<double>(x,B,c,20,0.06,0,PENALTY,true,-1,NULL);
		B.mult(c,y1,false,false,1.0,0.0);
		LCC_Coding<double>(y1,D,alpha,15,0.0001f);
		D.mult(alpha,x,false,false,1.0,0.0);
		x.getData(vx,0);
		tmp.getData(vy,0);

		for(int i=0; i<vx.n(); i++)
		{
			residual +=(vx[i]-vy[i])*(vx[i]-vy[i]);
		}
		if(residual >= residual1 || count > 10)
			break;
		count++;

	}

	
	Vector<double> v_a;
	alpha.getData(v_a,0);
	for(int i=0;i<v_a.n(); i++)
	{
		this->m_sparseParams.alpha_PNUM_1->data.db[i] = v_a[i];            //把值赋给alpha
	}


	Vector<double> v_B;
	Vector<double> v_c;
	c.getData(v_c,0);
	CvMat *yy = cvCreateMat(DLONG,1,CV_64FC1);
	cvZero(yy);
	for(int i=0; i<TNUM; i++)
	{
		B.getData(v_B,i);

		for(int j=0; j<DLONG; j++)
		{
			yy->data.db[j] += v_B[j]*v_c[i];
		}
	}
	y.getData(vy,0);
	residual = 0;
	for(int i=0; i<vy.n(); i++)
	{
		residual += (yy->data.db[i]-x_t_1[i])*(yy->data.db[i]-x_t_1[i]);
		x_t_1[i] = yy->data.db[i];
	}

	this->m_sparseParams.residual = residual;

	for(int i=0; i<DLONG; i++)
	{
		this->m_sparseParams.y_LONG_1->data.db[i] = vx[i];
	}
	cvReleaseMat(&yy);
	
}

#pragma endregion

#pragma region CSC
void SVTTracker::ComputeMatrixCSC(SparseParams sparseParams)
{
	Matrix<double> y(sparseParams.y_LONG_1->data.db,DLONG,1);
	Matrix<double> D(sparseParams.D_LONG_PNUM->data.db,DLONG,PNUM);
	Matrix<double> B(sparseParams.B_LONG_TENUM->data.db,DLONG,TENUM);
	Matrix<double> Xt(sparseParams.Xt_LONG_1->data.db,DLONG,1);
	SpMatrix<double> alpha;
	SpMatrix<double> c;

	cvZero(this->m_sparseParams.alpha_PNUM_1);
	Matrix<double> y1;
	lasso2<double>(y,B,c,20,0.06,0,PENALTY,true,-1,NULL);
	B.mult(c,y1,false,false,1.0,0.0);
	lasso2<double>(y1,D,alpha,30,0.06,0,PENALTY,true,-1,NULL);
	alpha.scal(1/alpha.asum());
	Matrix<double> x;
	D.mult(alpha,x,false,false,1.0,0.0);

	Vector<double> vx;
	x.getData(vx,0);
	Vector<double> vy;
	y.getData(vy,0);
	double residual = 0;
	for(int i=0; i<vx.n(); i++)
	{
		residual +=(vx[i]-vy[i])*(vx[i]-vy[i]);
	}

	int count=0;
	Matrix<double> tmp;
	double residual1 = 0;
	while(residual > 0.001)  
	{
		residual1 = residual;
		residual = 0;
		tmp.copy(x);
		
		lasso2<double>(x,B,c,20,0.06,0,PENALTY,true,-1,NULL);
		B.mult(c,y1,false,false,1.0,0.0);
		lasso2<double>(y1,D,alpha,30,0.06,0,PENALTY,true,-1,NULL);
		alpha.scal(1/alpha.asum());
		D.mult(alpha,x,false,false,1.0,0.0);
		x.getData(vx,0);
		tmp.getData(vy,0);

		for(int i=0; i<vx.n(); i++)
		{
			residual +=(vx[i]-vy[i])*(vx[i]-vy[i]);
		}
		if(residual >= residual1 || count > 10)
			break;
		count++;

	}

	
	Vector<double> v_a;
	alpha.getData(v_a,0);
	for(int i=0;i<v_a.n(); i++)
	{
		this->m_sparseParams.alpha_PNUM_1->data.db[i] = v_a[i];            //把值赋给alpha
	}


	Vector<double> v_B;
	Vector<double> v_c;
	c.getData(v_c,0);
	CvMat *yy = cvCreateMat(DLONG,1,CV_64FC1);
	cvZero(yy);
	for(int i=0; i<TNUM; i++)
	{
		B.getData(v_B,i);

		for(int j=0; j<DLONG; j++)
		{
			yy->data.db[j] += v_B[j]*v_c[i];
		}
	}
	y.getData(vy,0);
	residual = 0;
	for(int i=0; i<vy.n(); i++)
	{
		residual += (yy->data.db[i]-vy[i])*(yy->data.db[i]-vy[i]);
	}
	this->m_sparseParams.residual = residual;

	for(int i=0; i<DLONG; i++)
	{
		this->m_sparseParams.y_LONG_1->data.db[i] = vx[i];
	}
	cvReleaseMat(&yy);

}

#pragma endregion

#pragma region PF
void SVTTracker::ComputeOriginalPF(SparseParams sparseParams)
{
	Matrix<double> y(sparseParams.y_LONG_1->data.db,DLONG,1);
	Matrix<double> D(sparseParams.D_LONG_PNUM->data.db,DLONG,PNUM);
	Matrix<double> B(sparseParams.B_LONG_TENUM->data.db,DLONG,TENUM);
	Matrix<double> Xt(sparseParams.Xt_LONG_1->data.db,DLONG,1);
	Matrix<double> T(sparseParams.T_LONG_TNUM->data.db,DLONG,TNUM);
	Vector<double> alpha(PNUM);
	Vector<double> r(DLONG);
	Vector<double> c2(TNUM);   //TNUM个系数，与T相乘用
	Vector<double> best_c(TNUM);
	SpMatrix<double> c;
	Vector<double> d;
	Vector<double> b;
	Vector<double> c1;//Bc的系数
	double res = 0;   //当前残差
	double pre = 100000;  //先前残差
	double sum = 0;
	//Matrix<double> md(DLONG,1);
	cvZero(this->m_sparseParams.alpha_PNUM_1);

	int64 t = cvGetTickCount();
	lasso2<double>(D,B,c,20,0.06,0,PENALTY,true,-1,NULL);   //直接一次求，速度快很多倍

	t = cvGetTickCount() - t;

	for(int i=0; i<PNUM; i++)
	{
		D.getData(d,i);

		c.getData(c1,i);
		for(int j=0; j<TNUM; j++)
		{
			 c2[j] = c1[j];
		}
		
		//求粒子i的残差
		T.mult(c2,r,1,0);
		res = 0;
		for(int j=0; j<DLONG; j++)
		{
			res += (d[j]-r[j])*(d[j]-r[j]);
		}
		if(res < pre)
		{
			for(int j=0; j<TNUM; j++)
			{
				best_c[j] = c1[j];
			}
		}
		pre = res;
		res = exp(-30*sqrt(res));
		///////////////////////////////
		alpha[i] = res;
		sum += res;  //用来后面归一化alpha用
		
	}
	//alpha赋值给m_sparseParams
	for(int i=0; i<PNUM; i++)
	{
		alpha[i] = alpha[i]/sum;  //先归一化
		this->m_sparseParams.alpha_PNUM_1->data.db[i] = alpha[i];  //再赋值
	}


	//Solve the residual for controling the update rate of template
	T.mult(best_c,r,1,0);
	res = 0;
	for(int i=0; i<DLONG; i++)
	{
		res += (y(i,0)-r[i])*(y(i,0)-r[i]);
	}
	this->m_sparseParams.residual = res;
	
	//用当前帧的目标代表y值，做下一帧初始化
	D.mult(alpha,r,1,0);
	for(int i=0; i<DLONG; i++)
	{
		this->m_sparseParams.y_LONG_1->data.db[i] = r[i];
	}

}

#pragma endregion


void SVTTracker::Tracking(IplImage *pImg, IplImage *pColorImg)
{
	double x_center = 0;
	double y_center = 0;
	x_center = abs(this->upRight.x - this->upLeft.x)/2.0;
	y_center = abs(this->downLeft.y - this->upLeft.y)/2.0;

	if(x_center>0 && y_center>0)
	{
		if(this->type == CSC)
			this->m_pf->Resample(CSC);
		else
			this->m_pf->Resample(LLC);
		this->m_pf->Propograte();
		CvMat *location = cvCreateMat(3,DLONG,CV_64FC1);
		CvMat *data = cvCreateMat(DLONG,PNUM,CV_64FC3);
		CvMat *tmpData = cvCreateMat(DLONG,1,CV_64FC3);
		

		for(int i=0; i<PNUM; i++)
		{
			this->AffineParam2Matrix(this->m_pf->pfParams[i].affineParam,this->m_pf->pfParams[i].affineMatrix);
			this->GenerateLocationMatrix(pImg,location,this->m_pf->pfParams[i].affineMatrix);
			this->GenerateDataMatrix(pImg,this->m_pf->pfParams[i].data_LONG_1,location);
			this->GenerateDataMatrix(pColorImg,tmpData,location);
			//用来显示粒子的
			for(int j=0; j<DLONG; j++)
			{
				//data->data.db[i*DLONG +j] = this->m_pf->pfParams[i].data_LONG_1->data.db[j];
				data->data.db[i*DLONG*3+3*j] = tmpData->data.db[3*j];
				data->data.db[i*DLONG*3+3*j+1] = tmpData->data.db[3*j+1];
				data->data.db[i*DLONG*3+3*j+2] = tmpData->data.db[3*j+2];
			}

			this->Gaussian(this->m_pf->pfParams[i].data_LONG_1);                               //靠，有这2条，程序慢20多ms啊啊啊啊啊
			cvNormalize(this->m_pf->pfParams[i].data_LONG_1,this->m_pf->pfParams[i].data_LONG_1);
			

			for(int j=0; j<DLONG; j++)
			{
				this->m_sparseParams.D_LONG_PNUM->data.db[i*DLONG +j] = this->m_pf->pfParams[i].data_LONG_1->data.db[j];
			}
		}
	

		switch(type)
		{
		case PF:
			this->ComputeOriginalPF(this->m_sparseParams);
			break;
		case CSC:
			this->ComputeMatrixCSC(this->m_sparseParams);
			break;
		case LLC:
			this->ComputeMatrixLLC(this->m_sparseParams);
			break;
		default:
			this->ComputeMatrixLLC(this->m_sparseParams);
			this->type = LLC;
			break;
		}


		this->m_pf->CalculateWeight();
		this->m_pf->EstimateLocation(pImg);
		this->m_objTemplate->UpdateTemplate(pImg);

		this->DrawRectangle(this->upLeft,this->downLeft,this->upRight,pImg);
		this->DrawRectangle(this->upLeft,this->downLeft,this->upRight,pColorImg);

		cvReleaseMat(&location);
		cvReleaseMat(&data);
		cvReleaseMat(&tmpData);

	}
}