//*****************************************************************************
// IPCam3DS
// jc38x (jcds38x@gmail.com)
// https://github.com/jc38x/IPCam3DS
// 2017
//*****************************************************************************
// This project is based on
// https://gist.github.com/Tryptich/2a15909e384b582c51b5
// https://github.com/MrNbaYoh/3DSCameraExample
// https://github.com/kbowen99/3DS-KurtScan
// ctrulib camera/image example
// ctrulib network/sockets example
//*****************************************************************************

#pragma once

#include <3ds.h>

void init_server(uint16_t port, int backlog);
void exit_server();
void send_frames(CAMU_Size size);
