#ifndef _ObjectTemplate
#define _ObjectTemplate
#include <cv.h>
#include "Params.h"

#include <vector>
#include <list>
using namespace std;

class SVTTracker;

//模板数据结构
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
	TemplateParams templateParams[TNUM];   //模板数据
	CvMat *T_LONG_TNUM;     //显示模板数据的
	
	Dictionary m_dictionary;
	bool swith;  //主要为后面显示demo时 判断是否已经经过字典学习
	double tao;  //控制模板更新速率
private:
	SVTTracker *m_svtTracker;  	
	

};

#endif