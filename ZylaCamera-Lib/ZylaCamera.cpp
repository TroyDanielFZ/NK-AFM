// ----------------------------------
// Author:		Troy_Daniel
// Email:		Troy_Daniel@163.com
// Date:		2019/04/25
// ----------------------------------
#include "stdafx.h"
#include "ZylaCamera.h"
#pragma comment(lib, "C:\\Program Files\\Andor SDK3\\atcorem.lib")


void ZylaCamera::log(std::ostream & os, const char * pStr)
{
	os << pStr << std::endl;
}



ZylaCamera::ZylaCamera()
	: m_bStatusOK(false)
	, m_dExplosuerTime(0.01)
	//, m_strPixelEncoding(L"Mono16") // L"Mono32" , L"Mono16", L"Mono12", L"Mono12Packed"
	, m_nPixelEncoding(MONO16)
	, m_nSuperPixel(1)
	, m_nHeight(0)
	, m_strSerialNumber{ L"" }
	, m_nWidth(0)
	, m_hCamera(0)
	, m_nImageSize(0)
	, m_rectROI(-1,-1,-1,-1)
{

}

ZylaCamera::~ZylaCamera()
{
	if (m_bStatusOK) {

		AT_Flush(m_hCamera);	// flush 
		AT_Close(m_hCamera);	// and close the camera
		AT_FinaliseLibrary();	// release the resources
	}
}

void ZylaCamera::setExplosureTime(double dExplosuerTime)
{
	m_dExplosuerTime = dExplosuerTime;
}
//
//void ZylaCamera::setPixelEncoding(const wchar_t * pMode)
//{
//	// Mono12
//	// Mono12Packed
//	// Mono16
//	// Mono32
//	wcscpy(m_strPixelEncoding, pMode);
//}

void ZylaCamera::setPixelEncoding(PixelEncoding type)
{
	m_nPixelEncoding = type;
	if (m_nPixelEncoding != MONO12
		&& m_nPixelEncoding != MONO16
		&& m_nPixelEncoding != MONO12PACKED
		&& m_nPixelEncoding != MONO32) 
		m_nPixelEncoding = UNKNOWN;
}

bool ZylaCamera::openCamera(int nCameraIndex, std::ostream & os){

	// ------------------------------------
	// Initialize library
	// ------------------------------------
	os << "Initialising ..." << std::endl << std::endl;
	int nRet = AT_InitialiseLibrary();
	if (nRet != AT_SUCCESS) {
		log(os, "Error initialising library");
		//os << "Error initialising library" << endl << endl;
		return false;
	}
	// ------------------------------------
	// Search for camera
	// ------------------------------------
	AT_64 iNumberDevices = 0;
	AT_GetInt(AT_HANDLE_SYSTEM, L"Device Count", &iNumberDevices);
	if (iNumberDevices <= 0) {
		log(os, "No cameras detected");
		//os << "No cameras detected" << endl;
		AT_FinaliseLibrary();
		return false;
	}
	// ------------------------------------
	// Initialize camera
	// ------------------------------------
	if (nCameraIndex >= iNumberDevices) {
		os << "Only " << iNumberDevices << "found, but you want to open the " << nCameraIndex << "(th) camera." << std::endl
			<< "Note: the maximum of nCameraIndex starts from 0, and should be less " << iNumberDevices << "." << std::endl;
		return false;
	}
	nRet = AT_Open(nCameraIndex, &m_hCamera);		// open the first camera
	if (nRet != AT_SUCCESS) {
		log(os, "Error condition, could not initialise camera");
		//os << "Error condition, could not initialise camera" << endl << endl;
		AT_Close(m_hCamera);
		AT_FinaliseLibrary();
	}
	//log(os, "Successfully initialised camera");
	// ------------------------------------
	// Set parameters
	// ------------------------------------
	//Set the pixel Encoding to the desired settings Mono8 Data
	// ref to: Andor Software Development Kit 3.pdf Seciont 4.4
	os << "Set Pixel Encoding" << std::endl;
	wchar_t strPixelEncoding[64];
	wsprintf(strPixelEncoding, L"%s",  
		m_nPixelEncoding == MONO12 ? L"Mono12" :
		m_nPixelEncoding == MONO12PACKED ? L"Mono12Packed" :
		m_nPixelEncoding == MONO16 ? L"Mono16" :
		m_nPixelEncoding == MONO32 ? L"Mono32" : L"unknown" );

		// L"Mono32" , L"Mono16", L"Mono12", L"Mono12Packed"
	if (AT_SUCCESS != AT_SetEnumeratedString(m_hCamera, L"Pixel Encoding", strPixelEncoding)) return false;
	//if (AT_SUCCESS != AT_SetEnumeratedString(m_hCamera, L"Pixel Encoding", m_strPixelEncoding)) return false;
	//Set the pixel Readout Rate to 100 MHz
	// 280 MHz, 100 MHz available	for Zyla
	os << "Pixel Readout Rate" << std::endl;
	if (AT_SUCCESS != AT_SetEnumeratedString(m_hCamera, L"Pixel Readout Rate", L"100 MHz")) return false;

	//Set the exposure time for this camera to specific milliseconds
	os << "Exposure Time" << std::endl;
	if (AT_SUCCESS != AT_SetFloat(m_hCamera, L"Exposure Time", m_dExplosuerTime)) return false;

	// ------------------------------------
	// Set ROI
	// ------------------------------------
	// todo:settle with this
	if (AT_SUCCESS != AT_SetInt(m_hCamera, L"AOIHBin", m_nSuperPixel)) return false;
	if (AT_SUCCESS != AT_SetInt(m_hCamera, L"AOIVBin", m_nSuperPixel)) return false;

	if (AT_SUCCESS != AT_SetInt(m_hCamera, L"AOIWidth", m_rectROI.width())) return false;
	if (AT_SUCCESS != AT_SetInt(m_hCamera, L"AOILeft", m_rectROI.left())) return false;
	if (AT_SUCCESS != AT_SetInt(m_hCamera, L"AOIHeight", m_rectROI.height())) return false;
	if (AT_SUCCESS != AT_SetInt(m_hCamera, L"AOITop", m_rectROI.top())) return false;

	//Get the number of bytes required to store one frame


	// ------------------------------------
	// Fetch the [width/height/ image size bytes]of the image
	// ------------------------------------
	os << "AOIHeight" << std::endl;
	if (AT_SUCCESS != AT_GetInt(m_hCamera, L"AOIHeight", &m_nHeight)) return false;
	os << "AOIWidth" << std::endl;
	if (AT_SUCCESS != AT_GetInt(m_hCamera, L"AOIWidth", &m_nWidth)) return false;
	os << "Image Size Bytes" << std::endl;
	AT_64 tmpImageSizeBytes;
	if (AT_SUCCESS != AT_GetInt(m_hCamera, L"Image Size Bytes", &tmpImageSizeBytes)) return false;
	m_nImageSize = static_cast<int>(tmpImageSizeBytes);

	// serial number
	os << "Serial Number" << std::endl;
	if (AT_SUCCESS != AT_GetString(m_hCamera, L"Serial Number", m_strSerialNumber, 64)) return false;
	std::wcout << L"Serial Number:\t\t" << m_strSerialNumber << std::endl;
	m_bStatusOK = true;
	return true;
}
#ifdef USE_OPENCV
cv::Mat ZylaCamera::accquisitionMat(bool bAutoscale){
	// (nType >> 8) is the bit count per pixel

	if (UNKNOWN == m_nPixelEncoding) return cv::Mat();
	//int nBytesAllocated = m_nHeight * m_nWidth * (nType >> 8) + 8;
	//std::cout << "Bytes to allocate:\t " << m_nHeight * m_nWidth
	//	<< "\t" << m_nImageSize << std::endl;
	//int nBytesRead = m_nImageSize * (nType >> 8) + ((1 == (nType & 0xF)) ? m_nImageSize * (nType >> 8) : 0 + 8);
	unsigned char * pBuffer = new  unsigned char[m_nImageSize];	// Add 8 to allow data alignment 
																// acquisition data
																//unsigned char * pBuffer;
																//Allocate a memory buffer to store one frame
	AT_QueueBuffer(m_hCamera, pBuffer, m_nImageSize);

	auto imageType = (1 == (m_nPixelEncoding >> 8) ? CV_8UC1 :
		2 == (m_nPixelEncoding >> 8) ? CV_16UC1 :
		4 == (m_nPixelEncoding >> 8) ? CV_32FC1 : 0);

	//cout << "Image Type:\t"<< imageType << "\t" << CV_32SC1 << endl;

	//Start the Acquisition running
	//std::cout << "Acquisition Start" << std::endl;
	AT_Command(m_hCamera, L"Acquisition Start");
	//Sleep in this thread until data is ready, in this case set
	//the timeout to infinite for simplicity



	//cv::Mat mat = cv::Mat::ones(m_nHeight, m_nWidth, imageType) * 0x0;		// todo:Verify: decide which type the image belongs to
	cv::Mat mat = cv::Mat(m_nHeight, m_nWidth, imageType);
	unsigned char* buffer;
	static int num = 0;

	//std::cout <<"nBytesAllocated"<< nBytesAllocated << std::endl;
	if (AT_SUCCESS == AT_WaitBuffer(m_hCamera, &buffer, &m_nImageSize, 10000)) {
		//std::cout << "Get buffer successfully" << std::endl;
		uchar * data = mat.data;
		if (MONO16 == m_nPixelEncoding || MONO12 == m_nPixelEncoding) {	// Mono16 || Mono12
			short * ps = reinterpret_cast<short *>(buffer);
			float maximum = ps[0]; // 16bit
			for (int i = 0; i < m_nImageSize / 2; ++i) maximum = maximum < ps[i] ? ps[i] : maximum;

			short * data = reinterpret_cast<short *>(mat.data);
			for (int i = 0; i < m_nImageSize / 2; ++i) data[i] = ps[i] / maximum * 0xFFFF;
		}
		else if (MONO32 == m_nPixelEncoding) { // Mono32
			float * data = reinterpret_cast<float *>(mat.data);
			unsigned int * pInt = reinterpret_cast<unsigned  int *>(buffer);
			float maximum = pInt[0];
			for (int i = 0; i < mat.rows * mat.cols; ++i) maximum = maximum > pInt[i] ? maximum : pInt[i];
			for (int i = 0; i < mat.rows * mat.cols; ++i) data[i] = pInt[i] / maximum;

		}
		else if (MONO12PACKED == m_nPixelEncoding) {// Mono12Packed
			unsigned char * psrc = buffer;
			float maximum = psrc[0]; // 16bit
			if (bAutoscale)
				for (int i = 0; i < m_nImageSize; i += 3) {
					int lower = (int(psrc[i]) << 4) | (psrc[i + 1] & 0x0F);
					int higher = (int(psrc[i + 2] << 4)) | (psrc[i + 1] >> 4);
					maximum = maximum < lower ? lower : maximum;
					maximum = maximum < lower ? lower : maximum;
				}
			float scale = 0xFFFF / maximum;

			for (int i = 0; i < m_nImageSize; i += 3, data += 4) {
				int lower = (int(psrc[i]) << 4) | (psrc[i + 1] & 0x0F);
				int higher = (int(psrc[i + 2] << 4)) | (psrc[i + 1] >> 4);
				if (bAutoscale)
				{
					lower = lower * scale;
					higher = higher * scale;
				}

				data[0] = lower & 0xFF;
				data[1] = lower >> 8;
				data[2] = higher & 0xFF;
				data[3] = higher >> 8;
			}

		}
	}
	else {
		std::cout << "Error get buffer..." << std::endl;
		mat = cv::Mat();	// error happens
	}
	//std::cout << "AcquisitionStop" << std::endl;
	AT_Command(m_hCamera, L"AcquisitionStop");		//Stop the Acquisition
	AT_Flush(m_hCamera);

	delete[] pBuffer;
	return mat;
}
#endif

bool ZylaCamera::accquisition(unsigned char * pdata, int & nBytesRead, std::ostream & os)
{
	// Special note: the release of resouces should be handled out of this function
	// @[inout]		pdata	- nullptr if error(s) happens(s), otherwise the address 
	//						  of image data
	// @[return]	false	- if acquisition of data fails, and pdata is nullptr in 
	//						  this case

	if (UNKNOWN == m_nPixelEncoding) return false;
	std::cout << "Bytes to allocate:\t " << m_nHeight * m_nWidth
		<< "\t" << m_nImageSize << std::endl;
	unsigned char * pBuffer = new  unsigned char[m_nImageSize];	// Add 8 to allow data alignment 
																// acquisition data
																//unsigned char * pBuffer;
																//Allocate a memory buffer to store one frame
	AT_QueueBuffer(m_hCamera, pBuffer, m_nImageSize);

	auto imageType = (1 == (m_nPixelEncoding >> 8) ? CV_8UC1 :
		2 == (m_nPixelEncoding >> 8) ? CV_16UC1 :
		4 == (m_nPixelEncoding >> 8) ? CV_32FC1 : 0);

	//cout << "Image Type:\t"<< imageType << "\t" << CV_32SC1 << endl;

	//Start the Acquisition running
	std::cout << "Acquisition Start" << std::endl;
	AT_Command(m_hCamera, L"Acquisition Start");
	//Sleep in this thread until data is ready, in this case set
	//the timeout to infinite for simplicity

	unsigned char* buffer;

	//std::cout <<"nBytesAllocated"<< nBytesAllocated << std::endl;
	if (AT_SUCCESS == AT_WaitBuffer(m_hCamera, &buffer, &m_nImageSize, 10000)) {
		//uchar * data = pdata;
		if (MONO16== m_nPixelEncoding || MONO12 == m_nPixelEncoding) {	// Mono16 || Mono12
			short * ps = reinterpret_cast<short *>(buffer);
			float maximum = ps[0]; // 16bit
			for (int i = 0; i < m_nImageSize / 2; ++i) maximum = maximum < ps[i] ? ps[i] : maximum;

			short * data = reinterpret_cast<short *>(pdata);
			for (int i = 0; i < m_nImageSize / 2; ++i) data[i] = ps[i] / maximum * 0xFFFF;
		}
		else if (MONO32== m_nPixelEncoding) { // Mono32
			float * data = reinterpret_cast<float *>(pdata);
			unsigned int * pInt = reinterpret_cast<unsigned  int *>(buffer);
			float maximum = pInt[0];
			int size = m_nHeight * m_nWidth;
			for (int i = 0; i < size; ++i) maximum = maximum > pInt[i] ? maximum : pInt[i];
			for (int i = 0; i < size; ++i) data[i] = pInt[i] / maximum;

		}
		else if (MONO12PACKED == m_nPixelEncoding) {// Mono12Packed
			unsigned char * psrc = buffer;
			float maximum = psrc[0]; // 16bit
			for (int i = 0; i < m_nImageSize; i += 3) {
				int lower = (int(psrc[i]) << 4) | (psrc[i + 1] & 0x0F);
				int higher = (int(psrc[i + 2] << 4)) | (psrc[i + 1] >> 4);
				maximum = maximum < lower ? lower : maximum;
				maximum = maximum < lower ? lower : maximum;
			}
			float scale = 0xFFFF / maximum;
			uchar * data = pdata;
			for (int i = 0; i < m_nImageSize; i += 3, data += 4) {
				int lower = (int(psrc[i]) << 4) | (psrc[i + 1] & 0x0F);
				int higher = (int(psrc[i + 2] << 4)) | (psrc[i + 1] >> 4);
				lower = lower * scale;
				higher = higher * scale;


				data[0] = lower & 0xFF;
				data[1] = lower >> 8;
				data[2] = higher & 0xFF;
				data[3] = higher >> 8;
			}

		}
	}
	else {
		std::cout << "Error get buffer..." << std::endl;
		pdata = nullptr;
	}
	std::cout << "AcquisitionStop" << std::endl;
	AT_Command(m_hCamera, L"AcquisitionStop");		//Stop the Acquisition
	AT_Flush(m_hCamera);

	delete[] pBuffer;
	return true;
}

bool ZylaCamera::accquisition2file(char * pFilename)
{
	if (!m_bStatusOK) return false;
	/**
	 * Dumping the memory to a file
	 * When using this function, time is of great importance, so 
	 * no output should be direct to the console since the update
	 * of console are extremely slow.
	 */
	bool bRet = true;
	bool bProvidedFilename = (pFilename != nullptr);
	static int nNum = 0;
	if (!bProvidedFilename) {
		pFilename = new char[64];
		sprintf(pFilename, "data-%05d.bin", ++nNum);
	}
	//std::cout << pFilename << std::endl;
	unsigned char * pBuffer = new  unsigned char[m_nImageSize];	// Add 8 to allow data alignment 
																// acquisition data
																//unsigned char * pBuffer;
																//Allocate a memory buffer to store one frame
	AT_QueueBuffer(m_hCamera, pBuffer, m_nImageSize);

	AT_Command(m_hCamera, L"Acquisition Start");
	unsigned char* buffer;
	if (AT_SUCCESS == AT_WaitBuffer(m_hCamera, &buffer, &m_nImageSize, 10000)) {
		DataHeader header{ m_nHeight, m_nWidth, m_nImageSize, m_nPixelEncoding };

		std::ofstream fout(pFilename, std::ios::binary);
		fout.write((char*)&header, sizeof(DataHeader));
		fout.write((char*)buffer, m_nImageSize);
		fout.close();
	}else{
		std::cout << "Error get buffer..." << std::endl;
		bRet = false;
	}
	AT_Command(m_hCamera, L"AcquisitionStop");		//Stop the Acquisition
	AT_Flush(m_hCamera);

	delete[] pBuffer;
	
	if (!bProvidedFilename) delete pFilename;
	return bRet;
}

cv::Mat ZylaCamera::loadFromFile(char * pFilename)
{
	std::ifstream fin(pFilename, std::ios::binary);
	if (!fin.is_open()) {
		std::cerr<< "I can't get access to the file, please check!" << std::endl
			<< "Current path is ";
		system("echo %cd%");
		return cv::Mat();
	}
	//DataHeader header{ 2160, 2560, 22118400, m_nPixelEncoding };
	 DataHeader header;
	 fin.read((char*)&header, sizeof(DataHeader));

	PixelEncoding pe = header.m_nPixelEncoding;
	//PixelEncoding pe = m_nPixelEncoding;
	auto imageType = (1 == (pe >> 8) ? CV_8UC1 :
		2 == (pe >> 8) ? CV_16UC1 :
		4 == (pe >> 8) ? CV_32FC1 : 0);
	//std::cout << "m_nPixelEncoding = " << std::hex << pe << std::endl;
	//std::cout << header.m_nImageSize << std::endl;
	cv::Mat mat = cv::Mat(header.m_nHeight, header.m_nWidth, imageType);
	char * buffer = new char[header.m_nImageSize];
	fin.read(buffer, header.m_nImageSize);
	//std::cout << "Read file finished" << std::endl
		 //<< "fin.bad() = " << fin.bad() << std::endl;
	if (!fin.bad()) {
		uchar * data = mat.data;
		if (MONO16 == pe || MONO12 == pe) {	// Mono16 || Mono12
			short * ps = reinterpret_cast<short *>(buffer);
			float maximum = ps[0]; // 16bit
			for (int i = 0; i < header.m_nImageSize / 2; ++i) maximum = maximum < ps[i] ? ps[i] : maximum;

			short * data = reinterpret_cast<short *>(mat.data);
			for (int i = 0; i < header.m_nImageSize / 2; ++i) data[i] = ps[i] / maximum * 0xFFFF;
		}
		else if (MONO32 == pe) { // Mono32
			float * data = reinterpret_cast<float *>(mat.data);
			unsigned int * pInt = reinterpret_cast<unsigned  int *>(buffer);
			float maximum = pInt[0];
			for (int i = 0; i < mat.rows * mat.cols; ++i) maximum = maximum > pInt[i] ? maximum : pInt[i];
			for (int i = 0; i < mat.rows * mat.cols; ++i) data[i] = pInt[i] / maximum;

		}
		else if (MONO12PACKED == pe) {// Mono12Packed
			unsigned char * psrc = reinterpret_cast<unsigned char *>(buffer);
			float maximum = psrc[0]; // 16bit
		
			for (int i = 0; i < header.m_nImageSize; i += 3) {
				int lower = (int(psrc[i]) << 4) | (psrc[i + 1] & 0x0F);
				int higher = (int(psrc[i + 2] << 4)) | (psrc[i + 1] >> 4);
				maximum = maximum < lower ? lower : maximum;
				maximum = maximum < lower ? lower : maximum;
			}
			float scale = 0xFFFF / maximum;

			for (int i = 0; i < header.m_nImageSize; i += 3, data += 4) {
				int lower = (int(psrc[i]) << 4) | (psrc[i + 1] & 0x0F);
				int higher = (int(psrc[i + 2] << 4)) | (psrc[i + 1] >> 4);
				lower = lower * scale;
				higher = higher * scale;

				data[0] = lower & 0xFF;
				data[1] = lower >> 8;
				data[2] = higher & 0xFF;
				data[3] = higher >> 8;
			}

		}
	}else{
		mat = cv::Mat();
	}

	fin.close();
	return mat;
}

void ZylaCamera::setROI(RECT rectROI, int nSuperPixel)
{
	this->m_rectROI = rectROI;
	m_nSuperPixel = nSuperPixel;
}

ZylaCamera::RECT::RECT(int left, int top, int width, int height)
{
	this->m_nLeft = left;
	this->m_nWidth = abs(height);
	this->m_nTop = top;
	this->m_nHeight = abs(height);
}

int ZylaCamera::RECT::width()
{
	return m_nWidth;
}

int ZylaCamera::RECT::height()
{
	return m_nHeight;
}

int ZylaCamera::RECT::left()
{
	return m_nLeft;
}

int ZylaCamera::RECT::right()
{
	return m_nLeft+m_nWidth;
}

int ZylaCamera::RECT::top()
{
	return m_nTop;
}

int ZylaCamera::RECT::bottom()
{
	return m_nTop+m_nHeight;
}
