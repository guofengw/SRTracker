#include "stdafx.h"
#include "Params.h"
#include "SVTTracker.h"

void SetParam(SVTTracker &tracker,VideoName videoName)
{
	switch(videoName)
	{
	case VID:
		
		//for video
		tracker.std_affine->data.db[0] = 0.001;     //th
		tracker.std_affine->data.db[1] = 0.025;      //sc
		tracker.std_affine->data.db[2] = 0.002;      //sr
		tracker.std_affine->data.db[3] = 0.001;     //phi
		tracker.std_affine->data.db[4] = 5.0;         //tx
		tracker.std_affine->data.db[5] = 5.0;         //ty
		tracker.m_objTemplate->tao = 0.05;              //Control the rate of updating of template
		break;

	case CAM:

		//for camera
		tracker.std_affine->data.db[0] = 0.0;     //th
		tracker.std_affine->data.db[1] = 0.25;      //sc   //0.1 0.025 0.07
		tracker.std_affine->data.db[2] = 0.002;     //sr
		tracker.std_affine->data.db[3] = 0.0;     //phi
		tracker.std_affine->data.db[4] = 15;         //tx 15
		tracker.std_affine->data.db[5] = 15;         //ty
		tracker.m_objTemplate->tao = 0.5;
		break;

	case IMG:

		//for image
		tracker.std_affine->data.db[0] = 0.000;     //th
		tracker.std_affine->data.db[1] = 0.025;      //sc   //0.01 0.02 0.001
		tracker.std_affine->data.db[2] = 0.002;     //sr
		tracker.std_affine->data.db[3] = 0.0005;     //phi
		tracker.std_affine->data.db[4] = 5;         //tx
		tracker.std_affine->data.db[5] = 5;         //ty
		tracker.m_objTemplate->tao = 0.1;		
		break;

	default:
		tracker.std_affine->data.db[0] = 0.001;     //th
		tracker.std_affine->data.db[1] = 0.025;      //sc   //0.01 0.02 0.001
		tracker.std_affine->data.db[2] = 0.002;     //sr
		tracker.std_affine->data.db[3] = 0.001;     //phi
		tracker.std_affine->data.db[4] = 5;         //tx
		tracker.std_affine->data.db[5] = 5;         //ty
		tracker.m_objTemplate->tao = 0.1;
		break;
	}
}

