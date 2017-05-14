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

void get_camera_size(CAMU_Size size, s16 *width, s16 *height, u32 *imsize);
void init_capture(CAMU_Size size, CAMU_OutputFormat format, CAMU_FrameRate rate);
void exit_capture();
bool load_capture(u16 **cam1, u16 **cam2);
void read_camera(CAMU_Size size, u16 *cam1, u16 *cam2);
