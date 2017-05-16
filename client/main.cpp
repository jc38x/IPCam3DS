/**
* OpenCV video streaming over TCP/IP
* Client: Receives video from server and display it
* by Steve Tuenkam
*/

/*
Edited 5/15/2007
jc38x
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <opencv2/opencv.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define CAM1_WIN_NAME "Image cam1"
#define CAM2_WIN_NAME "Image cam2"

void take_picture(cv::Mat &img1_16, cv::Mat &img2_16, cv::Mat &img1_24, cv::Mat &img2_24, int imgSize, sockaddr_in const *serverAddr, socklen_t const &addrLen);

int main(int argc, char** argv) {
    //--------------------------------------------------------
    //networking stuff: socket , connect
    //--------------------------------------------------------

	// TODO: Don't hardcode this stuff
    char* serverIP   = "192.168.100.4";
    int   serverPort = 80;
	int   height     = 240;
	int   width      = 400;
	//---------------------------------
    
    WSADATA wsaData = { 0 };
    int iResult = 0;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
		std::cerr << "WSAStartup failed " << iResult << std::endl;
        return 1;
    }

	struct  sockaddr_in serverAddr;
	socklen_t           addrLen = sizeof(struct sockaddr_in);

	serverAddr.sin_family = PF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(serverIP);
	serverAddr.sin_port = htons(serverPort);

	//----------------------------------------------------------
	//OpenCV Code
	//----------------------------------------------------------
	cv::Mat img1_24 = cv::Mat::zeros(height, width, CV_8UC3);
	cv::Mat img2_24 = cv::Mat::zeros(height, width, CV_8UC3);
	cv::Mat img1_16 = cv::Mat::zeros(height, width, CV_16UC1);
	cv::Mat img2_16 = cv::Mat::zeros(height, width, CV_16UC1);

	int imgSize = height * width * sizeof(uint16_t);
	int key;

	std::cout << "Configuration" << std::endl;
	std::cout << "Server IP: " << serverIP << std::endl;
	std::cout << "Port: " << serverPort << std::endl;
	std::cout << "Image width: " << width << std::endl;
	std::cout << "Image height: " << height << std::endl;
	std::cout << "Image size: " << imgSize << std::endl;
	std::cout << std::endl;

	std::cout << "Press SPACE to take a picture" << std::endl;
	std::cout << "Press ENTER to save pictures" << std::endl;
	std::cout << "Press ESC to quit" << std::endl;
	
	cv::namedWindow(CAM1_WIN_NAME);
	cv::namedWindow(CAM2_WIN_NAME);

    while (true) {
		key = cv::waitKey();
		     if (key == VK_ESCAPE) { break; }
		else if (key == VK_SPACE)  { take_picture(img1_16, img2_16, img1_24, img2_24, imgSize, &serverAddr, addrLen); }
		else if (key == VK_RETURN) {
			cv::imwrite("im1.png", img1_24);
			cv::imwrite("im2.png", img2_24);
		}

		cv::imshow(CAM1_WIN_NAME, img1_24);
		cv::imshow(CAM2_WIN_NAME, img2_24);
	}

	WSACleanup();
    return 0;
}

bool recv_picture(cv::Mat &img1, cv::Mat &img2, int imgSize, sockaddr_in const *serverAddr, socklen_t const &addrLen) {
	SOCKET sokt = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sokt >= 0) {
	int ret = connect(sokt, (sockaddr *)serverAddr, addrLen);
	if (ret >= 0) {
	int bytes1 = recv(sokt, (char *)img1.data, imgSize, MSG_WAITALL);
	int bytes2 = recv(sokt, (char *)img2.data, imgSize, MSG_WAITALL);
	bool ok1 = bytes1 == imgSize;
	bool ok2 = bytes2 == imgSize;
	if (!ok1) { std::cerr << "recv (1) failed" << std::endl; }
	if (!ok2) { std::cerr << "recv (2) failed" << std::endl; }
	return ok1 && ok2;
	}
	else {
	std::cerr << "connect() failed" << std::endl;
	}
	}
	else {
	std::cerr << "socket() failed" << std::endl;
	return false;
	}

	closesocket(sokt);
	return false;
}

void conv_RGB565_picture(cv::Mat const &img_565, cv::Mat &img_888) {
	uint16_t *s = (uint16_t *)img_565.data;
	uint8_t  *d = (uint8_t  *)img_888.data;
	uint16_t data;

	for (int row = 0; row < img_565.rows; ++row) {
	for (int col = 0; col < img_565.cols; ++col) {
		data = *(s++);
		d[0] = ( data        & 0x1F) << 3;
		d[1] = ((data >>  5) & 0x3F) << 2;
		d[2] = ((data >> 11) & 0x1F) << 3;
		d += 3;
	}
	}
}

void take_picture(cv::Mat &img1_16, cv::Mat &img2_16, cv::Mat &img1_24, cv::Mat &img2_24, int imgSize, sockaddr_in const *serverAddr, socklen_t const &addrLen) {
	bool ok = recv_picture(img1_16, img2_16, imgSize, serverAddr, addrLen);
	if (!ok) { return; }
	conv_RGB565_picture(img1_16, img1_24);
	conv_RGB565_picture(img2_16, img2_24);
}
