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

#define CONFIG_3D_SLIDERSTATE (*(volatile float*)0x1FF81080)

static char const * const g_str_format[] = {
    "OUTPUT_YUV_422",
	"OUTPUT_RGB_565"
};

static char const * const g_str_rate[] = {
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

static const CAMU_Size         g_camsize = SIZE_CTR_TOP_LCD;
static const CAMU_OutputFormat g_format  = OUTPUT_RGB_565;
static const CAMU_FrameRate    g_rate    = FRAME_RATE_10;

static char     g_remoteip[16] = "0.0.0.0";
static uint16_t g_remoteport   = 81;
static uint16_t g_localport    = 80;
static int      g_backlog      = 4;

void init(void);
void logo(void);
void print_calibration_data(void);
void take_picture(void);
void draw_picture(void);
void cleanup(void);

int main(int argc, char **argv) {
    u32 kDown;

    init();
    begin_capture();

	while (aptMainLoop()) {
        hidScanInput();
        kDown = hidKeysDown();

        receive_capture();

		     if ((kDown & KEY_START) != 0) {break;}
		else if ((kDown & KEY_Y)     != 0) {print_calibration_data();}
        else if ((kDown & KEY_R)     != 0) {take_picture();}

        draw_picture();

        gspWaitForVBlank();
        gfxSwapBuffers();
    }

    end_capture();
    cleanup();

    return 0;
}

void load_settings(void) {
    FILE *f = fopen("settings.txt", "r");
    if (f != NULL) {
    fscanf(f, "%16s",  g_remoteip);
    fscanf(f, "%hd", &g_remoteport);
    fscanf(f, "%hd", &g_localport);
    fscanf(f, "%d",  &g_backlog);
    fclose(f);
    }
    else {
    printf("Failed to open settings.txt\n");
    }
}

void cleanup(void) {
    printf("Cleaning up (might take a few minutes)\n");
    exit_server();
    exit_capture();
    gfxExit();
}

void logo(void) {
    printf("IPCam3DS\n");
    printf("https://github.com/jc38x/IPCam3DS\n");
    printf("\n");

    s16 width;
	s16 height;
	u32 imsize;

    get_camera_size(g_camsize, &width, &height, &imsize);

    printf("Camera configuration\n");
    printf("Size %dx%d (%d)\n", (int)width, (int)height, (int)imsize);
    printf("Output Format %s\n", g_str_format[g_format]);
    printf("Frame Rate %s\n", g_str_rate[g_rate]);
    printf("\n");
}

void print_calibration_data(void) {
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
    printf("\n");
}

void writefb_RGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height) {
	u8 *fb_8 = (u8*) fb;
	u16 *img_16 = (u16*) img;
	int i, j, draw_x, draw_y;
	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			draw_y = y + height - j;
			draw_x = x + i;
			u32 v = (draw_y + draw_x * height) * 3;
			u16 data = img_16[j * width + i];
			uint8_t b = ((data >> 11) & 0x1F) << 3;
			uint8_t g = ((data >> 5) & 0x3F) << 2;
			uint8_t r = (data & 0x1F) << 3;
			fb_8[v] = r;
			fb_8[v+1] = g;
			fb_8[v+2] = b;
		}
	}
}

void draw_picture(void) {
    u16 *cam1;
    u16 *cam2;
    u32  imsize;
    s16  width;
	s16  height;

    get_camera_size(g_camsize, &width, &height, &imsize);
    bool ready = load_capture(&cam1, &cam2, &imsize);
    if (!ready) {return;}

    gfxSet3D(CONFIG_3D_SLIDERSTATE > 0.0f);
    writefb_RGB565(gfxGetFramebuffer(GFX_TOP, GFX_LEFT,  NULL, NULL), cam1, 0, 0, width, height);
    writefb_RGB565(gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), cam2, 0, 0, width, height);
}

void init(void) {
    gfxInitDefault();
    gfxSetDoubleBuffering(GFX_TOP,    true);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);
	consoleInit(GFX_BOTTOM, NULL);

	logo();
	load_settings();
    init_capture(g_camsize, g_format, g_rate);
    init_server(g_localport, g_backlog);

    printf("\n");
    printf("Press R to take a picture\n");
    printf("Press Y to show calibration data\n");
	printf("Press Start to quit\n");
	printf("\n");
}

void take_picture(void) {
    printf("Sending to %s:%d\n", g_remoteip, (int)g_remoteport);
    end_capture();
    send_frames_to_server(g_remoteip, g_remoteport);
    begin_capture();
    printf("Done\n");
}
