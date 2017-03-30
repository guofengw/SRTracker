#ifndef _ObjectTemplate
#define _ObjectTemplate
#include <cv.h>
#include "Params.h"

#include <vector>
#include <list>
using namespace std;

class SVTTracker;

//ģ�����ݽṹ
typedef struct {
	CvPoint upLeft;
	CvPoint downLeft;
	CvPoint upRight;
	AffineMatrix affineMatrix;
	AffineParam affineParam;
	CvMat *data_LONG_1;
}TemplateParams;

typedef struct {
	CvMat *data;
	CvMat *dictionary;
	vector<CvMat*> vObjects;
	vector<CvMat*> vDisplayObjects;

}Dictionary;

class ObjectTemplate
{
public:
	ObjectTemplate(SVTTracker *svtTracker);
	~ObjectTemplate();

public:
	void GenerateTemplate(IplImage *pImg);
	void UpdateTemplate(IplImage *pImg);

private:
	void AddObject(IplImage *pImg);
	void TrainDictionary(int dictionaryNum);
	void UpdateTemplateInner(void);
public:
	TemplateParams templateParams[TNUM];   //ģ������
	CvMat *T_LONG_TNUM;     //��ʾģ�����ݵ�
	
	Dictionary m_dictionary;
	bool swith;  //��ҪΪ������ʾdemoʱ �ж��Ƿ��Ѿ������ֵ�ѧϰ
	double tao;  //����ģ���������
private:
	SVTTracker *m_svtTracker;  	
	

};

#endif