// Main.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "SVTTracker.h"
#include "ImageSource.h"
#include "Params.h"
#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <string>
#include <time.h>
#include <fstream>
using namespace std;
#include <opencv2\core\core.hpp>

struct {
	CvPoint upLeft;
	CvPoint downRight;
	CvPoint upRight;
	CvPoint downLeft;
}Position;
IplImage *pMouseImage = NULL;

void SVTTracker::MouseCallback(int event, int x, int y, int flags, void *param)
{
	pMouseImage = (IplImage*)cvClone(param);

	if(event == CV_EVENT_LBUTTONDOWN)
	{
		Position.upLeft.x = x;
		Position.upLeft.y = y;

	}
	else if((event == CV_EVENT_MOUSEMOVE) && (flags & CV_EVENT_FLAG_LBUTTON))
	{
		Position.downRight.x = x;
		Position.downRight.y = y;
		cvRectangle(pMouseImage,Position.upLeft,Position.downRight,CV_RGB(255,0,0),2);
		cvShowImage("SVTTracker",pMouseImage);  
		
	}
	else if(event == CV_EVENT_LBUTTONUP)
	{
		Position.downRight.x = x;
		Position.downRight.y = y;

		Position.downLeft.x = Position.upLeft.x;
		Position.downLeft.y = Position.downRight.y;
		Position.upRight.x = Position.downRight.x;
		Position.upRight.y = Position.upLeft.y;
	}
	cvReleaseImage(&pMouseImage);


}

int _tmain(int argc, _TCHAR* argv[])
{

#ifdef Cam
	VideoName videoName = _camName;
#endif
#ifdef Image
	string filePath = _imagePath;
	string imageType = _imageType;
	VideoName videoName = _imageName;
#endif
#ifdef Video
	string filePath = _videoPath;
	VideoName videoName = _videoName;
#endif

#ifdef Saver
	FILE *fResult = NULL;
	if(!(fResult = fopen(_fileSave.c_str(),"w")))
	{
		cout<<"Can not open the file!"<<endl;
		exit(0);
	}
#endif

	//从文件中读取目标初始值位置，可以直接从benchmark的文件中读取
	ifstream file(_initLocation.c_str(),ios::in);   
	char initObj[20];
	int a,b,c,d;
	file.getline(initObj,20);

	for(int k=0; k<(int)strlen(initObj); k++)
	{
		if(initObj[k] == ',')
		{
			sscanf(initObj,"%d,%d,%d,%d",&a,&b,&c,&d);
			break;
		}
		if(initObj[k] == 9)  //9代表空格
		{
			sscanf(initObj,"%d %d %d %d",&a,&b,&c,&d);
			break;
		}
	}
	CvPoint p1 = {0,0};
	CvPoint p2 = {0,0};
	p1.x = a;
	p1.y = b;
	p2.x = p1.x + c;
	p2.y = p1.y + d;
	file.close();

/////////////////////////////////////////////////


	ImageSource *pImgSource = NULL;
	IplImage *pImg = NULL;
	IplImage *pGrayImg = NULL;

	cvNamedWindow("SVTTracker");
	SVTTracker svtTracker;

	
#ifdef Cam
	pImgSource = new ImageSourceFromCam(1);
#endif
#ifdef Image
	pImgSource = new ImageSourceFromImage(filePath,imageType);
#endif
#ifdef Video
	pImgSource = new ImageSourceFromVideo(filePath);
#endif

	if(!pImgSource->IsOpen())
	{
		cout<<"Can not open the source file!"<<endl;
		exit(0);
	}
	int width = pImgSource->GetWidth();
	int height = pImgSource->GetHeight();
	pGrayImg = cvCreateImage(cvSize(width,height),8,1);


////////////////////参数设置部分////////////////////////////////
#ifdef Writer
	CvVideoWriter *cvVideoWriter = cvCreateVideoWriter(_fileWriter.c_str(),CV_FOURCC('D','I','V','X'),30,cvSize(pGrayImg->width,pGrayImg->height),1);
#endif
///////////////////////////////////////////////////////////////
	
	//字体处理，显示字体
	CvFont *cFont = &cvFont(1,2);
	cvInitFont(cFont,CV_FONT_HERSHEY_SIMPLEX,1,1,0,2);
	string stext = "#";
	char *cNum = new char[1];  //

	bool flag = true;
	int num = 0;
	int key = 0;

	while(pImg = pImgSource->GetImage())
	{
		double t = (double)cvGetTickCount();
		num++;
		
		if(pImg->nChannels == 3)
		{
			cvCvtColor(pImg,pGrayImg,CV_BGR2GRAY);
		}
		
		if(flag == true)
		{		
			SetParam(svtTracker,videoName);

			svtTracker.upLeft.x = p1.x;
			svtTracker.upLeft.y = p1.y;
			svtTracker.downLeft.x = p1.x;
			svtTracker.downLeft.y = p2.y;
			svtTracker.upRight.x = p2.x;
			svtTracker.upRight.y = p1.y;

			svtTracker.Init(pGrayImg);
			svtTracker.type = _trackerType;
			flag = false;
		}
		key = cvWaitKey(1);
		if(key == 'q' || 'Q' == key)
		{
			break;
		}
		else if(key == 'a' || 'A' == key)
		{
			
			cvSetMouseCallback("SVTTracker",SVTTracker::MouseCallback,pImg);
			cvShowImage("SVTTracker",pImg);
			cvWaitKey();
			svtTracker.upLeft = Position.upLeft;
			svtTracker.downLeft = Position.downLeft;
			svtTracker.upRight = Position.upRight;
			svtTracker.Init(pGrayImg);
			cvSetMouseCallback("SVTTracker",NULL,NULL);

		}
		else if(key == 's' || 'S' == key)
		{
			cvWaitKey();
		}

		svtTracker.Tracking(pGrayImg,pImg);
				
		stext = "#";
		stext += itoa(num,cNum,10);
		cvPutText(pImg,stext.c_str(),cvPoint(0,30),cFont,CV_RGB(255,0,0));
		cvPutText(pGrayImg,stext.c_str(),cvPoint(0,30),cFont,CV_RGB(255,255,255));

		cvShowImage("SVTTracker",pImg);
		//stext = stext+".png";
		//cvSaveImage(stext.c_str(),pImg);
		//cvWaitKey();

		//输出目标位置中心到文件
		int x_center = 0;
		int y_center = 0;
		x_center = abs(svtTracker.upRight.x - svtTracker.upLeft.x)/2 + svtTracker.upLeft.x;
		y_center = abs(svtTracker.downLeft.y - svtTracker.upLeft.y)/2 + svtTracker.upLeft.y;
		CvPoint rightDown;
		rightDown.x = svtTracker.upRight.x;
		rightDown.y = svtTracker.downLeft.y;

#ifdef Saver
		if(!fprintf(fResult,"%d %d %d %d\n",svtTracker.upLeft.x,svtTracker.upLeft.y,rightDown.x,rightDown.y))
		{
			cout<<"Error!"<<endl;
		}
#endif


#ifdef Writer
		cvWriteFrame(cvVideoWriter,pImg);
#endif


//////////////////////////////////////////////////////////////////////////////
		t = (double)cvGetTickCount() - t;
		printf( "Frame rate = %g frame/s\n", 1000/(t/((double)cvGetTickFrequency()*1000.)) );


	}

	
#ifdef Writer
	cvReleaseVideoWriter(&cvVideoWriter);
#endif
#ifdef Saver
	fclose(fResult);
#endif
	
	cvReleaseImage(&pGrayImg);
	cvDestroyAllWindows();
	delete pImgSource;

	return 0;
	
}