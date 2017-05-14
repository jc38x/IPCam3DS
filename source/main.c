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

static char const * const str_rate[] = {
    "FRAME_RATE_15",
	"FRAME_RATE_15_TO_5",
	"FRAME_RATE_15_TO_2",
	"FRAME_RATE_10",
	"FRAME_RATE_8_5",
	"FRAME_RATE_5",
	"FRAME_RATE_20",
	"FRAME_RATE_20_TO_5",
	"FRAME_RATE_30",
	"FRAME_RATE_30_TO_5",
	"FRAME_RATE_15_TO_10",
	"FRAME_RATE_20_TO_10",
	"FRAME_RATE_30_TO_10"
};

static char const * const str_format[] = {
    "OUTPUT_YUV_422",
	"OUTPUT_RGB_565"
};

int main(int argc, char **argv) {
	const CAMU_Size         camsize = SIZE_CTR_TOP_LCD;
	const CAMU_OutputFormat format  = OUTPUT_RGB_565;
	const CAMU_FrameRate    rate    = FRAME_RATE_10;

	const uint16_t  port    = 80;
	const int       backlog = 4;

	gfxInitDefault();
    gfxSetDoubleBuffering(GFX_TOP,    false);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);

	PrintConsole con_top;
	PrintConsole con_bot;

	consoleInit(GFX_TOP,    &con_top);
	consoleInit(GFX_BOTTOM, &con_bot);

    consoleSelect(&con_top);

    printf("IPCam 3DS\n");
    printf("https://github.com/jc38x/IPCam3DS\n");
    printf("\n");

    s16 width;
	s16 height;
	u32 imsize;

    get_camera_size(camsize, &width, &height, &imsize);

    printf("Camera configuration\n");
    printf("Size %dx%d (%d)\n", (int)width, (int)height, (int)imsize);
    printf("Output Format %s\n", str_format[format]);
    printf("Frame Rate %s\n", str_rate[rate]);
    printf("\n");

    init_capture(camsize, format, rate);
	init_server(port, backlog);

	printf("\n");
	printf("Press Start to quit\n");

	consoleSelect(&con_bot);

    while (aptMainLoop()) {
        hidScanInput();

        u32 kDown = hidKeysDown();
		if ((kDown & KEY_START) != 0) {break;}

		send_frames(camsize);

        gspWaitForVBlank();
        gfxSwapBuffers();
    }

    exit_server();
    exit_capture();

    printf("Cleanup completed\n");

    gfxExit();

    return 0;
}
