#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include <dolramdisk.h>

#include "testramdisk.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void *Initialise();

int main(int argc, char **argv)
{
	xfb = Initialise();

	printf("\nHello World from libdolramdisk!\n");

	printf("initializing ramdisk...");

	if(dolramdiskInit(&testramdisk))
		printf("failed!");
	else
		printf("OK!\n");

	printf("Trying to open test.txt...");

	FILE *fp = fopen("testramdisk:/test.txt","r");

	if(!fp)
		printf("failed!");
	else
		printf("OK!\n");

	printf("Trying to read from test.txt. Content between lines:\n---\n");

	char line[80];
	while(fgets(line, 80, fp) != NULL)
		printf(line);

	printf ("---\nlikely end of file\n");

	printf("Trying to close test.txt...");

	if(fclose(fp))
		printf("failed!");
	else
		printf("OK!\n");


	printf("Press HOME to exit!\n");

	while(1)
	{
		VIDEO_WaitVSync();
		WPAD_ScanPads();

		int buttonsDown = WPAD_ButtonsDown(0);
		if (buttonsDown & WPAD_BUTTON_HOME)
			exit(0);
	}

	return 0;
}

void * Initialise()
{
	void *framebuffer;

	VIDEO_Init();
	WPAD_Init();
	
	rmode = VIDEO_GetPreferredMode(NULL);

	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(framebuffer,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	return framebuffer;
}
