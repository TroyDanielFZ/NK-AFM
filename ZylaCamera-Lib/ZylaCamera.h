// ----------------------------------
// Author:		Troy_Daniel
// Email:		Troy_Daniel@163.com
// Date:		2019/04/25
// ----------------------------------
#pragma once

#define USE_OPENCV

#include "atcore.h"

#ifdef USE_OPENCV
#include <opencv2\opencv.hpp>
#endif
// ----------------------------------------------------------------------------------
// Wirte at the beginning
// !! This class for Andor Zyla is not fully implemented.
// There are some basic knowledge we should konw before we start the programming
// 1. the andor Zyla serials is black-white camera with maximum deepth being 32bits,
//    thus 1(or several bytes represent(s) a single gray-scale pixel in image.
// If there are errors, check atdebug.log file for details
// ----------------------------------------------------------------------------------
#include <iostream>
class ZylaCamera
{
public:
	enum PixelEncoding { MONO12 = 0x201, 
		MONO16 = 0x203, 
		MONO12PACKED = 0x202,
		MONO32 = 0x404, 
		UNKNOWN = -1 };
	class RECT {
		int m_nLeft;
		int m_nTop;
		int m_nWidth;
		int m_nHeight;
	public:
		RECT(int left, int top, int width, int height);
		int width();
		int height();
		int left();
		int right();
		int top();
		int bottom();
	};
private:
public:
	struct DataHeader {
		int m_nHeight;
		int m_nWidth;
		long m_nImageSize;
		PixelEncoding m_nPixelEncoding;
	};

private:
	bool m_bStatusOK;					// whether the camera is usable
	int m_nImageSize;					// image size in bytes
	int m_nSuperPixel;
	AT_H m_hCamera;						// camera handle
	AT_WC m_strSerialNumber[64];		// serial number
	//AT_WC m_strPixelEncoding[32];		// Pixel encoding type
	double m_dExplosuerTime;
	PixelEncoding m_nPixelEncoding;
	RECT m_rectROI;

	AT_64 m_nHeight, m_nWidth;
	void log(std::ostream & os, const char *pStr);

public:
	
	ZylaCamera();
	virtual ~ZylaCamera();

	void setExplosureTime(double dExplosuerTime);
	//void setPixelEncoding(const wchar_t * pMode);
	void setPixelEncoding(PixelEncoding type);
	bool openCamera(int nCameraIndex = 0, std::ostream & os = std::cout);
#ifdef USE_OPENCV
	cv::Mat accquisitionMat(bool bAutoscale = true);
#endif
	bool accquisition(unsigned char * pdata /*= nullptr*/, /*inout */int & nBytesRead, std::ostream & os = std::cout);
	bool accquisition2file(char * pFilename = nullptr);
	cv::Mat loadFromFile(char *pFilename);
	void setROI(RECT rectROI, int nSuperPixel = 2);

};

