/**
* OpenCV video streaming over TCP/IP
* Client: Receives video from server and display it
* by Steve Tuenkam
*/

/*
Edited 5/28/2017
jc38x
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <opencv2/opencv.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define CAM1_WIN_NAME "CAM1"
#define CAM2_WIN_NAME "CAM2"

static const int      g_width = 400;
static const int      g_height = 240;
static const uint16_t g_localport = 81;
static const int      g_backlog = 4;

const int g_imsize = g_height * g_width * sizeof(uint16_t);

bool init_sockets() {
	WSADATA wsaData = { 0 };
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	return iResult == 0;
}

void exit_sockets(SOCKET ssckt) {
	closesocket(ssckt);
	WSACleanup();
}

int create_server_socket(uint16_t localport, int backlog, SOCKET &ssckt) {
	ssckt = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (ssckt == INVALID_SOCKET) { return -1; }

	struct sockaddr_in server;

	server.sin_family      = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port        = htons(localport);

	int ret;

	ret = bind(ssckt, (struct sockaddr *)&server, sizeof(server));
	if (ret == 0) {
	ret = listen(ssckt, backlog);
	if (ret == 0) {
	//u_long iMode = 1;
	//ioctlsocket(ssckt, FIONBIO, &iMode);
	return 0;
	}
	else {
	ret = -3;
	}
	}
	else {
	ret = -2;
	}

	closesocket(ssckt);
	return ret;
}

int receive_frames(SOCKET ssckt, char *data1, char *data2, int imsize) {
	int clientlen = sizeof(struct sockaddr_in);
	struct sockaddr_in client;

	SOCKET csckt = accept(ssckt, (struct sockaddr *)&client, &clientlen);
	if (csckt == INVALID_SOCKET) { return -1; }

	std::cout << "Client " << inet_ntoa(client.sin_addr) << ":" << (int)ntohs(client.sin_port) << std::endl;

	int ret;
	int bytes1 = recv(csckt, data1, imsize, MSG_WAITALL);
	if (bytes1 == imsize) {
	int bytes2 = recv(csckt, data2, imsize, MSG_WAITALL);
	if (bytes2 == imsize) {
	ret = 0;
	}
	else {
	ret = -3;
	}
	}
	else {
	ret = -2;
	}

	closesocket(csckt);
	return ret;
}

void convert_RGB565_picture(uint16_t *s, int width, int height, uint8_t *d) {
	int numel = width * height;
	uint16_t data;

	for (int n = 0; n < numel; ++n) {
		data = *(s++);
		d[0] = ( data        & 0x1F) << 3;
		d[1] = ((data >>  5) & 0x3F) << 2;
		d[2] = ((data >> 11) & 0x1F) << 3;
		d += 3;
	}
}

int init(uint16_t localport, int backlog, SOCKET &ssckt) {
	if (!init_sockets()) {
		std::cout << "init_sockets failed" << std::endl;
		return -1;
	}

	int ret = create_server_socket(localport, backlog, ssckt);
	if (ret != 0) {
		std::cout << "create_server_socket failed (" << ret << ")" << std::endl;
		return -2;
	}

	return 0;
}

int main(int argc, char** argv) {
	SOCKET ssckt;
	int    ret;
	int    key;

	ret = init(g_localport, g_backlog, ssckt);
	if (ret != 0) { return ret; }
	
	cv::Mat img1_565(g_height, g_width, CV_16UC1);
	cv::Mat img2_565(g_height, g_width, CV_16UC1);
	cv::Mat img1_888(g_height, g_width, CV_8UC3);
	cv::Mat img2_888(g_height, g_width, CV_8UC3);

	//cv::namedWindow(CAM1_WIN_NAME);
	//cv::namedWindow(CAM2_WIN_NAME);

	std::string basename;
	
	for (;;) {
		key = cv::waitKey(1);
		if (key == VK_ESCAPE) { break; }

		ret = receive_frames(ssckt, (char *)img1_565.data, (char *)img2_565.data, g_imsize);
		if (ret != 0) {
			std::cout << "receive_frames failed (" << ret << ")" << std::endl;
			continue;
		}

		convert_RGB565_picture((uint16_t *)img1_565.data, g_width, g_height, img1_888.data);
		convert_RGB565_picture((uint16_t *)img2_565.data, g_width, g_height, img2_888.data);

		//cv::imshow(CAM1_WIN_NAME, img1_888);
		//cv::imshow(CAM2_WIN_NAME, img2_888);

		basename = std::to_string(time(NULL));

		cv::imwrite(basename + "_CAM1.png", img1_888);
		cv::imwrite(basename + "_CAM2.png", img2_888);
	}

	exit_sockets(ssckt);
	return 0;
}
