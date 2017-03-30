#ifndef _PARAMS
#define _PARAMS
#include <cxcore.h>
#include <cv.h>
#include <string>
using namespace std;

class SVTTracker;

//�������ͣ�VID:video; CAM:camera; IMG:image
enum VideoName{
	VID,CAM,IMG
};

enum TrackerType {
	CSC,LLC,PF
};

#define WIDTH 20
#define HEIGHT 20
#define DLONG (WIDTH*HEIGHT)
#define TNUM 10
#define PNUM 600
#define TENUM (TNUM+1*DLONG)
#define NUMD 2  //ѧϰ�����ֵ����

/*************���Ƹ���ѡ��**************************************/
//#define Cam                 //Cam,Image,Video������һ����������벻ͨ��
//#define Image
#define Video

//#define Saver
//#define Writer

const TrackerType _trackerType = LLC;  //LLC,CSC,PF��ѡһ
const string _initLocation = "david.txt";    //Ŀ��ĳ�ʼλ�ã�·����Ҫ�����Լ�����������

/*************����ԴΪ����ͷ�Ĳ�������**************************/
const VideoName _camName = CAM;

/*************����ԴΪͼ��Ĳ�������****************************/
const string _imagePath = "D:\\����\\BenchMark\\Walking\\img\\";
const string _imageType = "jpg";
const VideoName _imageName = IMG;

/*************����ԴΪ��Ƶ�Ĳ�������****************************/
const string _videoPath = "davidin300.avi";
const VideoName _videoName = VID;

/*************����������ѡ��**********************************/
const string _fileWriter = "cam.avi";
const string _fileSave = "cam.txt";
/***************************************************************/


//����任����
typedef struct {
	double a1;
	double a2;
	double a3;
	double a4;
	double tx;
	double ty;
} AffineMatrix;

//����任ϵ��
typedef struct {
	double th;
	double sc;
	double sr;
	double phi;
	double tx;
	double ty;
} AffineParam;

typedef struct 
{
	CvMat *y_LONG_1;
	CvMat *y_LONG_1_display;
	CvMat *Xt_LONG_1;
	CvMat *D_LONG_PNUM;
	CvMat *alpha_PNUM_1;
	CvMat *B_LONG_TENUM;
	CvMat *T_LONG_TNUM;
	CvMat *c_TENUM_1;
	double residual;

}SparseParams;

void SetParam(SVTTracker &tracker,VideoName videoName);


#endif