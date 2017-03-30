#include "stdafx.h"
#include "ObjectTemplate.h"
#include "SVTTracker.h"
#include <highgui.h>
#include <time.h>
#include <dicts.h>


ObjectTemplate::ObjectTemplate(SVTTracker *svtTracker)
{
	this->m_svtTracker = svtTracker;
	for(int i=0; i<TNUM; i++)
	{
		this->templateParams[i].data_LONG_1 = cvCreateMat(DLONG,1,CV_64FC1);
		cvZero(this->templateParams[i].data_LONG_1);
	}
	this->T_LONG_TNUM = cvCreateMat(DLONG,TNUM,CV_64FC1);
	cvZero(this->T_LONG_TNUM);


	this->m_dictionary.dictionary = cvCreateMat(DLONG,NUMD,CV_64FC1);


	this->swith = false;
	this->tao = 0.05;
	
	
}

ObjectTemplate::~ObjectTemplate()
{
	for(int i=0; i<TNUM; i++)
	{
		cvReleaseMat(&this->templateParams[i].data_LONG_1);
	}
	this->m_svtTracker = NULL;
	cvReleaseMat(&this->T_LONG_TNUM);
}

void ObjectTemplate::GenerateTemplate(IplImage *pImg)
{
	CvMat *location = cvCreateMat(3,DLONG,CV_64FC1);

	for(int i=0; i<TNUM; i++)
	{
		this->m_svtTracker->GenerateAffineMatrix(this->templateParams[i].upLeft,this->templateParams[i].downLeft,this->templateParams[i].upRight,this->templateParams[i].affineMatrix,pImg);
		this->m_svtTracker->AffineMatrix2Param(this->templateParams[i].affineMatrix,this->templateParams[i].affineParam);
		this->m_svtTracker->GenerateLocationMatrix(pImg,location,this->templateParams[i].affineMatrix);
		this->m_svtTracker->GenerateDataMatrix(pImg,this->templateParams[i].data_LONG_1,location);

		//这个数据是用来显示模板的
		for(int j=0; j<DLONG; j++)
		{
			this->T_LONG_TNUM->data.db[i*DLONG+j] = this->templateParams[i].data_LONG_1->data.db[j];

		}

		//下面是模板数据
		this->m_svtTracker->Gaussian(this->templateParams[i].data_LONG_1);

		cvNormalize(this->templateParams[i].data_LONG_1,this->templateParams[i].data_LONG_1);

		for(int j=0; j<DLONG; j++)
		{
			this->m_svtTracker->m_sparseParams.B_LONG_TENUM->data.db[i*DLONG +j] = this->templateParams[i].data_LONG_1->data.db[j];
			this->m_svtTracker->m_sparseParams.T_LONG_TNUM->data.db[i*DLONG+j] = this->templateParams[i].data_LONG_1->data.db[j];
			
		}

	}
	//模板y赋初值,用来做模型的初始值
	for(int i=0; i<DLONG; i++)
	{
		this->m_svtTracker->m_sparseParams.y_LONG_1->data.db[i] = this->templateParams[0].data_LONG_1->data.db[i];
		this->m_svtTracker->x_t_1[i] = this->templateParams[0].data_LONG_1->data.db[i];
	}
	
	
	cvReleaseMat(&location);
}

void ObjectTemplate::TrainDictionary(int dictionaryNum)
{
	int K = NUMD;
	int batchsize = 512;
	int numThreads = -1;
	Trainer<double> *trainer = new Trainer<double>(K,batchsize,numThreads);
	ParamDictLearn<double> param;
	param.lambda = 0.15;
	param.iter = 100;
	param.verbose = false;
	Matrix<double> X(this->m_dictionary.data->data.db,DLONG,dictionaryNum);
	Matrix<double> D(this->m_dictionary.dictionary->data.db,DLONG,NUMD);
	trainer->train(X,param);
	trainer->getD(D);
	Vector<double> vD;
	for(int i=0; i<NUMD; i++)
	{
		D.getData(vD,i);
		for(int j=0; j<DLONG; j++)
		{
			this->m_dictionary.dictionary->data.db[i*DLONG+j] = vD[j];
		}
	}
	delete trainer;



}
void ObjectTemplate::AddObject(IplImage *pImg)
{

	CvMat *posMat = cvCreateMat(3,DLONG,CV_64FC1);
	CvMat *tempMat = cvCreateMat(DLONG,1,CV_64FC1);
	CvMat *displayMat;


	this->m_svtTracker->GenerateLocationMatrix(pImg,posMat,this->m_svtTracker->affineMatrix);
	this->m_svtTracker->GenerateDataMatrix(pImg,tempMat,posMat);

	displayMat = (CvMat*)cvClone(tempMat);
	this->m_dictionary.vDisplayObjects.push_back(displayMat);  //保存显示的目标



	this->m_svtTracker->Gaussian(tempMat);
	this->m_svtTracker->Normalization(tempMat);

	this->m_dictionary.vObjects.push_back(tempMat);

	cvReleaseMat(&posMat);


}

void ObjectTemplate::UpdateTemplateInner(void)
{
	int size = this->m_dictionary.vObjects.size();  //模板数据个数，大于10就要更新
	int k = 0;
	if(size > 0)
	{
		this->m_dictionary.data = cvCreateMat(DLONG,size,CV_64FC1);
		vector<CvMat*>::iterator iter;


		
		if(size >= 10 || this->m_svtTracker->m_sparseParams.residual > this->tao)    //判断目标数，大于10，字典学习
		{
			//cout<<"Now, it is updating the object template"<<endl;
			
			k = 0;
			for(iter=this->m_dictionary.vObjects.begin(); iter!=this->m_dictionary.vObjects.end(); iter++)
			{
				for(int j=0; j<DLONG; j++)
				{
					this->m_dictionary.data->data.db[k*DLONG+j] = (*iter)->data.db[j];  //把vector数据转化为CvMat数据，可以用来字典学习
				}
				k++;
			}

			if(size >= NUMD)
			{
				this->swith = true;  //开启
				//this->TrainDictionary(size);   //字典学习过程
				this->TrainDictionary(size);
				this->m_dictionary.vObjects.clear();
				this->m_dictionary.vDisplayObjects.clear();


				double dotSum = 0;
				double temp1 = 0;
				double temp2 = 0;
				int position1 = 0;
				int position2 = 0;
				for(int i=0; i<DLONG; i++)
				{
					temp1 += this->m_svtTracker->m_sparseParams.B_LONG_TENUM->data.db[DLONG+i] * this->m_svtTracker->m_sparseParams.y_LONG_1->data.db[i];
					temp2 += this->m_svtTracker->m_sparseParams.B_LONG_TENUM->data.db[2*DLONG+i] * this->m_svtTracker->m_sparseParams.y_LONG_1->data.db[i];
					position1 = 1;
					position2 = 2;
				}

				for(int i=3; i<TNUM; i++)
				{
					for(int j=0; j<DLONG; j++)
					{
						dotSum += this->m_svtTracker->m_sparseParams.B_LONG_TENUM->data.db[i*DLONG+j] * this->m_svtTracker->m_sparseParams.y_LONG_1->data.db[j];
					}
					if(dotSum < temp1)
					{
						temp1 = dotSum;
						dotSum = 0;
						position1 = i;
					}
					else if(dotSum < temp2)
					{
						temp2 = dotSum;
						dotSum = 0;
						position2 = i;
					}
				}

				for(int i=0; i<DLONG; i++)
				{

					this->m_svtTracker->m_sparseParams.B_LONG_TENUM->data.db[position1*DLONG+i] = this->m_dictionary.dictionary->data.db[i];
					this->m_svtTracker->m_sparseParams.B_LONG_TENUM->data.db[position2*DLONG+i] = this->m_dictionary.dictionary->data.db[DLONG+i];
					this->m_svtTracker->m_sparseParams.T_LONG_TNUM->data.db[position1*DLONG+i] = this->m_dictionary.dictionary->data.db[i];
					this->m_svtTracker->m_sparseParams.T_LONG_TNUM->data.db[position2*DLONG+i] = this->m_dictionary.dictionary->data.db[DLONG+i];

					this->T_LONG_TNUM->data.db[position1*DLONG+i] = this->m_dictionary.dictionary->data.db[i] * 690 + 100;
					this->T_LONG_TNUM->data.db[position2*DLONG+i] = this->m_dictionary.dictionary->data.db[DLONG+i] * 690 + 100;
				}
			
			}
			else
			{
				for(int i=0; i<size*DLONG; i++)
				{
					this->m_svtTracker->m_sparseParams.B_LONG_TENUM->data.db[(TNUM-size)*DLONG+i] = this->m_dictionary.data->data.db[i];
					this->m_svtTracker->m_sparseParams.T_LONG_TNUM->data.db[(TNUM-size)*DLONG+i] = this->m_dictionary.data->data.db[i];
					this->T_LONG_TNUM->data.db[(TNUM-size)*DLONG+i] = this->m_dictionary.data->data.db[i] * 690 + 100;
				}

				this->m_dictionary.vDisplayObjects.clear();
				this->m_dictionary.vObjects.clear();

			}
		}
		cvReleaseMat(&this->m_dictionary.data);
	}
}
void ObjectTemplate::UpdateTemplate(IplImage *pImg)
{
	this->AddObject(pImg);    //增加当前帧目标，把它保存在vector中
	
	this->UpdateTemplateInner();

}

