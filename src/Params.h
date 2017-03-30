#ifndef _PARAMS
#define _PARAMS
#include <cxcore.h>
#include <cv.h>
#include <string>
using namespace std;

class SVTTracker;

//输入类型，VID:video; CAM:camera; IMG:image
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
#define NUMD 2  //学习到的字典个数

/*************控制各个选项**************************************/
//#define Cam                 //Cam,Image,Video三个用一个，否则编译不通过
//#define Image
#define Video

//#define Saver
//#define Writer

const TrackerType _trackerType = LLC;  //LLC,CSC,PF三选一
const string _initLocation = "david.txt";    //目标的初始位置，路径需要根据自己的重新设置

/*************输入源为摄像头的参数设置**************************/
const VideoName _camName = CAM;

/*************输入源为图像的参数设置****************************/
const string _imagePath = "D:\\程序\\BenchMark\\Walking\\img\\";
const string _imageType = "jpg";
const VideoName _imageName = IMG;

/*************输入源为视频的参数设置****************************/
const string _videoPath = "davidin300.avi";
const VideoName _videoName = VID;

/*************输出结果保存选项**********************************/
const string _fileWriter = "cam.avi";
const string _fileSave = "cam.txt";
/***************************************************************/


//仿射变换矩阵
typedef struct {
	double a1;
	double a2;
	double a3;
	double a4;
	double tx;
	double ty;
} AffineMatrix;

//仿射变换系数
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