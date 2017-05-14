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

#include <stdio.h>
#include <malloc.h>
#include <3ds.h>

#define WAIT_TIMEOUT (1000ULL * 1000ULL * 300ULL)

static u16  *g_cam1;
static u16  *g_cam2;
static bool  g_ready;

void get_camera_size(CAMU_Size size, s16 *width, s16 *height, u32 *imsize) {
    static const s16 dim[][2] = {
        {640, 480},
        {320, 240},
        {160, 120},
        {352, 288},
        {176, 144},
        {256, 192},
        {512, 384},
        {400, 240}
    };

    *width  = dim[size][0];
    *height = dim[size][1];
    *imsize = ((u32)*width * (u32)*height) * sizeof(u16);
}

static void init_camera(CAMU_Size size, CAMU_OutputFormat format, CAMU_FrameRate rate) {
    camInit();

    CAMU_SetSize(SELECT_OUT1_OUT2, size, CONTEXT_A);
    CAMU_SetOutputFormat(SELECT_OUT1_OUT2, format, CONTEXT_A);

    CAMU_SetFrameRate(SELECT_OUT1_OUT2, rate);
    CAMU_SetNoiseFilter(SELECT_OUT1_OUT2, true);
    CAMU_SetAutoExposure(SELECT_OUT1_OUT2, true);
    CAMU_SetAutoWhiteBalance(SELECT_OUT1_OUT2, true);

    //CAMU_SetLensCorrection(SELECT_OUT1_OUT2, LENS_CORRECTION_NORMAL);
    //CAMU_SetContrast(SELECT_OUT1_OUT2, CONTRAST_NORMAL);
    //CAMU_SetPhotoMode(SELECT_OUT1_OUT2, PHOTO_MODE_NORMAL);
    //CAMU_SetEffect(SELECT_OUT1_OUT2, EFFECT_NONE, CONTEXT_A);
    //CAMU_SetSharpness(u32 select, s8 sharpness);

    /*
    CAMU_StereoCameraCalibrationData data;
    CAMU_GetStereoCameraCalibrationData(&data);
    printf("tx: %f | ty: %f\n", data.translationX, data.translationY);
    printf("rx: %f | ry: %f | rz: %f\n", data.rotationX, data.rotationY, data.rotationZ);
    printf("s: %f\n", data.scale);
    printf("VXY: %d\n", data.isValidRotationXY);
    */

    CAMU_SetTrimming(PORT_CAM1, false);
    CAMU_SetTrimming(PORT_CAM2, false);

    CAMU_SetBrightnessSynchronization(true);
}

static bool create_camera_buffers(CAMU_Size size, u16 **cam1, u16 **cam2) {
    s16 width;
    s16 height;
    u32 imsize;

    get_camera_size(size, &width, &height, &imsize);

    *cam1 = (u16 *)malloc(imsize);
    *cam2 = (u16 *)malloc(imsize);

    bool ok1 = *cam1 != NULL;
    bool ok2 = *cam2 != NULL;

    if (ok1 && ok2) {return true;}

    if (ok1) {free(*cam1);}
    if (ok2) {free(*cam2);}

    return false;
}

static void exit_camera(u16 *cam1, u16 *cam2) {
    free(cam1);
    free(cam2);
}

void init_capture(CAMU_Size size, CAMU_OutputFormat format, CAMU_FrameRate rate) {
    if (g_ready) {return;}
    g_ready = create_camera_buffers(size, &g_cam1, &g_cam2);
    if (g_ready) {
    init_camera(size, format, rate);
    printf("Camera ready\n");
    }
    else {
    printf("create_camera_buffers failed\n");
    }
}

void exit_capture() {
    if (!g_ready) {return;}
    camExit();
    exit_camera(g_cam1, g_cam2);
    g_ready = false;
    printf("Camera shutdown\n");
}

bool load_capture(u16 **cam1, u16 **cam2) {
    *cam1 = g_cam1;
    *cam2 = g_cam2;
    return g_ready;
}

void read_camera(CAMU_Size size, u16 *cam1, u16 *cam2) {
    s16 width;
    s16 height;
    u32 imsize;
    u32 bufsize;

    get_camera_size(size, &width, &height, &imsize);

    CAMU_GetMaxBytes(&bufsize, width, height);
    CAMU_SetTransferBytes(PORT_BOTH, bufsize, width, height);

    CAMU_Activate(SELECT_OUT1_OUT2);
    CAMU_ClearBuffer(PORT_BOTH);
    CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2);

    CAMU_StartCapture(PORT_BOTH);

    Handle event1;
    Handle event2;

    CAMU_SetReceiving(&event1, (void *)cam1, PORT_CAM1, imsize, bufsize);
    CAMU_SetReceiving(&event2, (void *)cam2, PORT_CAM2, imsize, bufsize);

    svcWaitSynchronization(event1, WAIT_TIMEOUT);
    svcWaitSynchronization(event2, WAIT_TIMEOUT);

    CAMU_StopCapture(PORT_BOTH);

    svcCloseHandle(event1);
    svcCloseHandle(event2);

    CAMU_Activate(SELECT_NONE);
}
