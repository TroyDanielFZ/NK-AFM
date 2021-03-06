// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ZylaCamera.h"

int main()
{
	ZylaCamera camera;
	camera.setPixelEncoding(ZylaCamera::PixelEncoding::MONO32);
	try{
//#define SAVE_DATA
#ifdef SAVE_DATA
		camera.openCamera();
		for(int i = 0; i< 3; ++i) camera.accquisition2file();
#else

		char strFilename[32]{ "data-00001.bin" };
		cv::Mat mat = camera.loadFromFile(strFilename);
		if (!mat.empty()) {
			cv::imshow("Mono32", mat);
			cv::waitKey(0);
		}
		else {
			std::cout << "Error read parsing the file" << std::endl;
			system("pause");
		}
#endif // SAVE_DATA

	
	}
	catch (...) {
		std::cout << "Some error happens!" << std::endl;
		system("pause");
	}
    return 0;
}

