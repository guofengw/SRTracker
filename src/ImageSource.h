#ifndef _ImageSource
#define _ImageSource

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <string>
#include <windows.h>
using namespace std;

class ImageSource
{
public:
	virtual ~ImageSource(){};
	virtual IplImage *GetImage() = 0;
	virtual IplImage *GetGrayImage() = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual bool IsOpen() = 0;
private:
	virtual void CreateImageSource() = 0;   
};

class ImageSourceFromCam : public ImageSource
{
public:
	ImageSourceFromCam(int index);
	~ImageSourceFromCam();
public:
	virtual IplImage *GetImage();
	virtual IplImage *GetGrayImage();
	virtual int GetWidth();
	virtual int GetHeight();
	virtual bool IsOpen();
private:
	virtual void CreateImageSource();
	void CreateCameraCapture(int index);
private:
	CvCapture *m_pCap;
	IplImage *m_pImg;
	IplImage *m_pGrayImg;
	int m_index;
};

class ImageSourceFromVideo : public ImageSource  
{
public:
	ImageSourceFromVideo(string videoName);
	~ImageSourceFromVideo();
public:
	virtual IplImage *GetImage();
	virtual IplImage *GetGrayImage();
	virtual int GetWidth();
	virtual int GetHeight();
	virtual bool IsOpen();
private:
	virtual void CreateImageSource();
	void CreateFileCapture(string videoName);
private:
	CvCapture *m_pCap;
	IplImage *m_pImg;
	IplImage *m_pGrayImg;
	string m_videoName;

};

class ImageSourceFromImage : public ImageSource
{
public:
	ImageSourceFromImage(string fileName, string imageType);
	~ImageSourceFromImage();
public:
	IplImage *GetImage();
	IplImage *GetGrayImage();
	int GetWidth();
	int GetHeight();
	bool IsOpen();
private:
	void CreateImageSource();
	short  Fget_filename(const char* fdirectoryPath,char* filename_array[],const char *type,int& length);
private:
	string m_fileName;
	string m_imageType;
	int m_fileLength;
	char *file_array[8000];
	string m_imgName;
	int m_count;
	IplImage *m_pImg;
	IplImage *m_pGrayImg;
};


#endif