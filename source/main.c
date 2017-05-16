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

#include <3ds.h>
#include <stdio.h>

#include "camera.h"
#include "server.h"

static char const * const str_format[] = {
    "OUTPUT_YUV_422",
	"OUTPUT_RGB_565"
};

static const CAMU_Size         camsize = SIZE_CTR_TOP_LCD;
static const CAMU_OutputFormat format  = OUTPUT_RGB_565;

static const uint16_t port    = 80;
static const int      backlog = 4;

void print_calibration_data() {
    CAMU_StereoCameraCalibrationData data;

    CAMU_GetStereoCameraCalibrationData(&data);

    printf("Calibration data\n");
    printf("tx: %f | ty: %f\n", data.translationX, data.translationY);
    printf("rx: %f | ry: %f | v: %d\n", data.rotationX, data.rotationY, (int)data.isValidRotationXY);
    printf("rz: %f\n", data.rotationZ);
    printf("s:  %f\n", data.scale);
    printf("w:  %d | h:  %d\n", (int)data.imageWidth, (int)data.imageHeight);
    printf("al: %f | ar: %f\n", data.angleOfViewLeft, data.angleOfViewRight);
    printf("dc: %f | dt: %f\n", data.distanceCameras, data.distanceToChart);
}

int main(int argc, char **argv) {
    PrintConsole con_top;
	PrintConsole con_bot;

	gfxInitDefault();
    gfxSetDoubleBuffering(GFX_TOP,    false);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);

	consoleInit(GFX_TOP,    &con_top);
	consoleInit(GFX_BOTTOM, &con_bot);

    consoleSelect(&con_top);

    printf("IPCam3DS\n");
    printf("https://github.com/jc38x/IPCam3DS\n");
    printf("\n");

    s16 width;
	s16 height;
	u32 imsize;

    get_camera_size(camsize, &width, &height, &imsize);

    printf("Camera configuration\n");
    printf("Size %dx%d (%d)\n", (int)width, (int)height, (int)imsize);
    printf("Output Format %s\n", str_format[format]);
    printf("\n");

    init_capture(camsize);
    init_server(port, backlog);

    configure_camera(camsize, format);

    printf("\n");
	printf("Press Start to quit\n");

	consoleSelect(&con_bot);

	while (aptMainLoop()) {
        hidScanInput();

        u32 kDown = hidKeysDown();

		if ((kDown & KEY_START)  != 0) {break;}
		if ((kDown & KEY_SELECT) != 0) {print_calibration_data();}

		send_frames(camsize);

        gspWaitForVBlank();
        gfxSwapBuffers();
    }

    printf("Cleaning up (might take a few minutes)\n");

    exit_server();
    exit_capture();
    gfxExit();

    return 0;
}
