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

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "camera.h"

#define SOC_ALIGN      0x00001000
#define SOC_BUFFERSIZE 0x00100000

static u32 *g_soc_buffer = NULL;
static int  g_ssckt      = -1;

static int init_soc(u32 **soc_buffer, u32 context_size) {
    *soc_buffer = (u32 *)memalign(SOC_ALIGN, context_size);
    if (*soc_buffer == NULL) {return -1;}

    Result ret = socInit(*soc_buffer, context_size);
    if (ret == 0) {return 0;}

    free(*soc_buffer);
    printf("socInit failed with result 0x%X\n", (int)ret);
	return -2;
}

static void exit_soc(u32 *soc_buffer) {
    socExit();
    free(soc_buffer);
}

static int create_server_socket(uint16_t port, int backlog) {
    int ssckt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ssckt < 0) {return -1;}

    struct sockaddr_in server;
    int ret;
    memset(&server, 0, sizeof(server));

    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = gethostid();

    printf("Server %s:%d\n", inet_ntoa(server.sin_addr), (int)port);

    ret = bind(ssckt, (struct sockaddr *)&server, sizeof(server));
    if (ret ==  0) {
    ret = fcntl(ssckt, F_SETFL, fcntl(ssckt, F_GETFL, 0) | O_NONBLOCK);
    if (ret != -1) {
    ret = listen(ssckt, backlog);
    if (ret ==  0) {
    return ssckt;
    }
    else {ret = -4;}
    }
    else {ret = -3;}
    }
    else {ret = -2;}

    close(ssckt);
    return ret;
}

static int send_client_frames(int ssckt, CAMU_Size size, u16 *cam1, u16 *cam2) {
    struct sockaddr_in client;
    int ret;
    memset(&client, 0, sizeof(client));

    u32 clientlen = sizeof(client);
    int csckt = accept(ssckt, (struct sockaddr *)&client, &clientlen);
    if (csckt < 0) {return -1;}

    printf("Client %s:%d\n", inet_ntoa(client.sin_addr), (int)ntohs(client.sin_port));

    read_camera(size, cam1, cam2);

    s16 width;
    s16 height;
    u32 imsize;

    get_camera_size(size, &width, &height, &imsize);

    ret = fcntl(csckt, F_SETFL, fcntl(csckt, F_GETFL, 0) & ~O_NONBLOCK);
    if (ret != -1) {
    ret = send(csckt, cam1, imsize, 0);
    if (ret != -1) {
    ret = send(csckt, cam2, imsize, 0);
    if (ret != -1) {
    ret = 0;
    }
    else {ret = -4;}
    }
    else {ret = -3;}
    }
    else {ret = -2;}

    close(csckt);
    return ret;
}

void init_server(uint16_t port, int backlog) {
    if (g_ssckt >= 0) {return;}
    int ret = init_soc(&g_soc_buffer, SOC_BUFFERSIZE);
    if (ret == 0) {
    g_ssckt = create_server_socket(port, backlog);
    if (g_ssckt >= 0) {
    printf("Server ready\n");
    }
    else {
    printf("create_server_socket failed (%d)\n", g_ssckt);
    exit_soc(g_soc_buffer);
    }
    }
    else {
    printf("init_soc failed (%d)\n", ret);
    }
}

void exit_server() {
    if (g_ssckt < 0) {return;}
    close(g_ssckt);
    exit_soc(g_soc_buffer);
    g_ssckt = -1;
    printf("Server shutdown\n");
}

void send_frames(CAMU_Size size) {
    if (g_ssckt < 0) {return;}

    u16 *cam1;
    u16 *cam2;
    bool ready = load_capture(&cam1, &cam2);
    if (!ready) {return;}

    int ret = send_client_frames(g_ssckt, size, cam1, cam2);
    if (ret < -1) {printf("send_client_frames failed (%d)\n", ret);}
}
