#include "stdafx.h"
#include "ImageSource.h"

ImageSourceFromCam::ImageSourceFromCam(int index)
{
	this->m_pCap = NULL;
	this->m_pImg = NULL;
	this->m_pGrayImg = NULL;
	this->m_index  = index;
	this->CreateImageSource();
}
ImageSourceFromCam::~ImageSourceFromCam()
{
	cvReleaseCapture(&this->m_pCap);
}
void ImageSourceFromCam::CreateCameraCapture(int index)
{
	m_pCap = cvCreateCameraCapture(index);
	if(m_pCap != NULL)
	{
		//pGrayImg = cvCreateImage(cvSize(
	}

}

void ImageSourceFromCam::CreateImageSource()
{
	this->CreateCameraCapture(this->m_index);
}

IplImage* ImageSourceFromCam::GetImage()
{
	if(this->m_pCap != NULL)
	{
		this->m_pImg = cvQueryFrame(this->m_pCap);
		return this->m_pImg;
	}
	else
		return NULL;
}

IplImage* ImageSourceFromCam::GetGrayImage()
{
	return NULL;

}

int ImageSourceFromCam::GetWidth()
{
	if(this->m_pCap != NULL)
		return (int)cvGetCaptureProperty(m_pCap,CV_CAP_PROP_FRAME_WIDTH);
	else
		return 0;
}

int ImageSourceFromCam::GetHeight()
{
	if(this->m_pCap != NULL)
		return (int)cvGetCaptureProperty(m_pCap,CV_CAP_PROP_FRAME_HEIGHT);
	else
		return 0;
}

bool ImageSourceFromCam::IsOpen()
{
	if(this->m_pCap != NULL)
		return true;
	else
		return false;
}
//////////////////////////////////////////////////////////////////////////
ImageSourceFromVideo::ImageSourceFromVideo(string videoName)
{
	this->m_pCap = NULL;
	this->m_pImg = NULL;
	this->m_pGrayImg = NULL;
	this->m_videoName = videoName;
	this->CreateImageSource();
}

ImageSourceFromVideo::~ImageSourceFromVideo()
{
	cvReleaseCapture(&this->m_pCap);
}

void ImageSourceFromVideo::CreateFileCapture(string videoName)
{
	if(!videoName.empty())
	{
		this->m_pCap = cvCreateFileCapture(videoName.c_str());
	}
	else
		this->m_pCap = NULL;
}

void ImageSourceFromVideo::CreateImageSource()
{
	if(!this->m_videoName.empty())
	{
		this->CreateFileCapture(this->m_videoName);
	}
}

IplImage* ImageSourceFromVideo::GetImage()
{
	if(this->m_pCap != NULL)
	{
		this->m_pImg = cvQueryFrame(this->m_pCap);
		return this->m_pImg;
	}
	else
		return NULL;
}

IplImage* ImageSourceFromVideo::GetGrayImage()
{
	return NULL;
}

int ImageSourceFromVideo::GetWidth()
{
	if(this->m_pCap != NULL)
		return (int)cvGetCaptureProperty(m_pCap,CV_CAP_PROP_FRAME_WIDTH);
	else
		return 0;
}

int ImageSourceFromVideo::GetHeight()
{
	if(this->m_pCap != NULL)
		return (int)cvGetCaptureProperty(m_pCap,CV_CAP_PROP_FRAME_HEIGHT);
	else
		return 0;
}

bool ImageSourceFromVideo::IsOpen()
{
	if(this->m_pCap != NULL)
		return true;
	else
		return false;
}
///////////////////////////////////////////////////////////////////////
ImageSourceFromImage::ImageSourceFromImage(std::string fileName, std::string imageType)
{
	this->m_fileName = fileName;
	this->m_imageType = imageType;
	this->m_count = 0;
	this->m_pImg = NULL;
	this->m_pGrayImg = NULL;
	this->CreateImageSource();
}

ImageSourceFromImage::~ImageSourceFromImage()
{
	for(int i=0; i<this->m_fileLength; i++)
	{
		delete this->file_array[i];
	}
	cvReleaseImage(&m_pImg);
	cvReleaseImage(&m_pGrayImg);
}

short ImageSourceFromImage::Fget_filename(const char *fdirectoryPath, char *filename_array[], const char *type, int &length)
{
	WIN32_FIND_DATA data;  
	HANDLE hFind;   
	int nCount=0;       
	char filename[256];   
	memset(filename,0,256);
	strcpy(filename,fdirectoryPath);   
	strcat(filename,"\\*.");
	strcat(filename,type);
	 
	hFind=FindFirstFile(filename,&data);  
	while(hFind!=INVALID_HANDLE_VALUE)   
	{   
	  int size=0;
	  char temp_c='1';
	  while(temp_c!='\0')
	  {
		  temp_c=data.cFileName[size];
		  size++;
	  }
	  filename_array[nCount]=new char[size];
	  if(data.cFileName[0]!='.')
	  {
		  for(int i=0;i<size;i++)
		  {
			  filename_array[nCount][i]=data.cFileName[i];
		  }
		  nCount++; 
	  }      
	  if(!FindNextFile(hFind,&data))   
	  {     
		  hFind=INVALID_HANDLE_VALUE;   
	  }   
	}   
	length=nCount;   
	return   1;   
}

void ImageSourceFromImage::CreateImageSource()
{
	if(!this->m_fileName.empty())
	{
		this->Fget_filename(this->m_fileName.c_str(),this->file_array,this->m_imageType.c_str(),this->m_fileLength);
	}
}

IplImage *ImageSourceFromImage::GetImage()
{
	if(this->m_count<this->m_fileLength)
	{
		cvReleaseImage(&m_pImg);
		this->m_imgName = this->m_fileName + this->file_array[this->m_count];
		this->m_pImg = cvLoadImage(this->m_imgName.c_str(),1);	
		this->m_count++;
		return this->m_pImg;
	}
	else
		return NULL;
}

IplImage *ImageSourceFromImage::GetGrayImage()
{
	if(this->m_count < this->m_fileLength)
	{
		cvReleaseImage(&m_pGrayImg);
		this->m_imgName = this->m_fileName + this->file_array[this->m_count];
		this->m_pGrayImg = cvLoadImage(this->m_imgName.c_str(),0);
		this->m_count++;
		return this->m_pGrayImg;
		
	}
	else
		return NULL;
}

int ImageSourceFromImage::GetWidth()
{
	if(!this->m_fileName.empty())
	{
		cvReleaseImage(&m_pImg);
		this->m_imgName = this->m_fileName + this->file_array[0];
		this->m_pImg = cvLoadImage(this->m_imgName.c_str(),1);	
		return this->m_pImg->width;
	}
	else
	{
		return 0;
	}
}

int ImageSourceFromImage::GetHeight()
{
	if(!this->m_fileName.empty())
	{
		cvReleaseImage(&m_pImg);
		this->m_imgName = this->m_fileName + this->file_array[0];
		this->m_pImg = cvLoadImage(this->m_imgName.c_str(),1);	
		return this->m_pImg->height;
	}
	else
	{
		return 0;
	}
}

bool ImageSourceFromImage::IsOpen()
{
	if(m_fileLength > 0)
		return true;
	else
		return false;
}