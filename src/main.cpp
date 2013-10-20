#include <stdio.h>
#include <math.h>
#include "SDL.h"
#include "SDL_image.h"
#include <GLES/gl.h>
#include <GLES/glext.h>

#include "PDL.h"
#include <linux/input.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <linux/types.h>
#define ROOT_PATH "/media/cryptofs/apps/usr/palm/applications/com.nizovn.androidchroot/"
#define UINPUT_LOCATION "/media/internal/AndroidChroot/root/dev/uinput"
#define SLEEP_FILE_NAME "/media/internal/AndroidChroot/root/AndroidChroot/wait_for_fb_sleep"
#define WAKE_FILE_NAME "/media/internal/AndroidChroot/root/AndroidChroot/wait_for_fb_wake"

#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <linux/fb.h>
#include <stdlib.h>
int w,h;
bool shutdown=true;
int fd;
struct fb_var_screeninfo vi;
void set_fb1(void)
{
	ioctl(fd, FBIOGET_VSCREENINFO, &vi);
	vi.xres_virtual=vi.xres;
	vi.yres_virtual=vi.yres;
	ioctl(fd, FBIOPUT_VSCREENINFO, &vi);
}
bool Init(void) 
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
	SDL_SetVideoMode(0, 0, 32, SDL_OPENGL);
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	w = info->current_w;
	h = info->current_h;
	fd = open("/dev/fb1", O_RDONLY);
	set_fb1();
	return true;
}

int uinput_fd;
int send_uevent(int fd, __u16 type, __u16 code, __s32 value)
{
	struct input_event event;
	memset(&event, 0, sizeof(event));
	event.type = type;
	event.code = code;
	event.value = value;
	gettimeofday(&event.time, NULL);
	if (write(fd, &event, sizeof(event)) != sizeof(event)) {
		fprintf(stderr, "Error on send_event %d", sizeof(event));
		return -1;
	}
	return 0;
}

void open_uinput()
{
	struct uinput_user_dev device;
	struct input_event myevent;
	int i,ret = 0;

	memset(&device, 0, sizeof device);

	uinput_fd=open(UINPUT_LOCATION,O_WRONLY);
	strcpy(device.name,"chroot_input");

	device.id.bustype=BUS_USB;
	device.id.vendor=1;
	device.id.product=1;
	device.id.version=1;

	for (i=0; i < ABS_MAX; i++) {
		device.absmax[i] = -1;
		device.absmin[i] = -1;
		device.absfuzz[i] = -1;
		device.absflat[i] = -1;
	}
	device.absmin[ABS_MT_POSITION_X]=0;
	device.absmax[ABS_MT_POSITION_X]=w;
	device.absfuzz[ABS_MT_POSITION_X]=2;
	device.absflat[ABS_MT_POSITION_X]=0;
	device.absmin[ABS_MT_POSITION_Y]=0;
	device.absmax[ABS_MT_POSITION_Y]=h;
	device.absfuzz[ABS_MT_POSITION_Y]=1;
	device.absflat[ABS_MT_POSITION_Y]=0;

	if (write(uinput_fd,&device,sizeof(device)) != sizeof(device))
		fprintf(stderr, "error setup %d\n",uinput_fd);

	if (ioctl(uinput_fd,UI_SET_EVBIT,EV_KEY) < 0)
		fprintf(stderr, "error evbit key\n");

	if (ioctl(uinput_fd,UI_SET_EVBIT, EV_SYN) < 0)
		fprintf(stderr, "error evbit key\n");

	if (ioctl(uinput_fd,UI_SET_EVBIT,EV_ABS) < 0)
		fprintf(stderr, "error evbit rel\n");

	if (ioctl(uinput_fd,UI_SET_KEYBIT, KEY_BACK) < 0)
		fprintf(stderr, "error keybit key\n");
	if (ioctl(uinput_fd,UI_SET_KEYBIT, KEY_HOME) < 0)
		fprintf(stderr, "error keybit key\n");
	if (ioctl(uinput_fd,UI_SET_KEYBIT, KEY_POWER) < 0)
		fprintf(stderr, "error keybit key\n");

	if (ioctl(uinput_fd,UI_SET_ABSBIT,ABS_MT_POSITION_X) < 0)
		fprintf(stderr, "error tool rel\n");

	if (ioctl(uinput_fd,UI_SET_ABSBIT,ABS_MT_POSITION_Y) < 0)
		fprintf(stderr, "error tool rel\n");

	if (ioctl(uinput_fd,UI_SET_KEYBIT,BTN_TOUCH) < 0)
		fprintf(stderr, "error evbit rel\n");

	if (ioctl(uinput_fd,UI_DEV_CREATE) < 0)
		fprintf(stderr, "error create\n");

}
void send_sleep_or_wake(){
	send_uevent(uinput_fd, EV_KEY, KEY_POWER, 1);
	send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
	printf("Sending screen\n");
	send_uevent(uinput_fd, EV_KEY, KEY_POWER, 0);
	send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
	int fd_wait=open(SLEEP_FILE_NAME,O_NONBLOCK|O_WRONLY);
	if(fd_wait>0)system("echo sleeping > " SLEEP_FILE_NAME);
	else system("echo awake > " WAKE_FILE_NAME);
	close(fd_wait);
}


int main(int argc, char** argv)
{

	if (Init() == false)
		return -1;
	system(ROOT_PATH "start.sh");
	open_uinput();
	SDL_Event Event;
	bool active=true;
	while (1) {
		SDL_Delay(10);
		if (active) {
			set_fb1();
			SDL_GL_SwapBuffers();
		}
		else{
			SDL_Delay(1000);
		} 
		int x,y,k;
		while (SDL_PollEvent(&Event)) {
			switch (Event.type) {
				case SDL_MOUSEBUTTONDOWN:
					if(SDL_GetMultiMouseState(3,NULL,NULL)&SDL_BUTTON(1)){
						SDL_Delay(500);
						SDL_PumpEvents();
						if(SDL_GetMultiMouseState(4,NULL,NULL)&SDL_BUTTON(1)){
							send_uevent(uinput_fd, EV_KEY, KEY_POWER, 1);
							send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
							printf("Sending power on\n");
							SDL_Delay(1000);
							send_uevent(uinput_fd, EV_KEY, KEY_POWER, 0);
							send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
							printf("Sending power off\n");
							if(shutdown){
								system(ROOT_PATH "shutdown.sh &");
								shutdown=false;
							}
						}
						else
							if(SDL_GetMultiMouseState(3,NULL,NULL)&SDL_BUTTON(1)){
								send_uevent(uinput_fd, EV_KEY, KEY_HOME, 1);
								send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
								printf("Sending home\n");
								send_uevent(uinput_fd, EV_KEY, KEY_HOME, 0);
								send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
							}
					}
				case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONUP:
					k=0;
					for(int i=0;i<3;i++){
						if(SDL_GetMultiMouseState(i,&x,&y)&SDL_BUTTON(1)){
							send_uevent(uinput_fd, EV_ABS, ABS_MT_POSITION_X, x);
							send_uevent(uinput_fd, EV_ABS, ABS_MT_POSITION_Y, y);
							send_uevent(uinput_fd, EV_KEY, BTN_TOUCH, 1);
							send_uevent(uinput_fd, EV_SYN, SYN_MT_REPORT, 0);
							k++;
						}
					}
					if(k==0){
						send_uevent(uinput_fd, EV_KEY, BTN_TOUCH, 0);
						send_uevent(uinput_fd, EV_SYN, SYN_MT_REPORT, 0);
					}
					send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
					break;
				case SDL_KEYDOWN:
					switch (Event.key.keysym.sym) {
						case PDLK_GESTURE_BACK:
							send_uevent(uinput_fd, EV_KEY, KEY_BACK, 1);
							send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
							printf("Sending back\n");
							send_uevent(uinput_fd, EV_KEY, KEY_BACK, 0);
							send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
							break;
						case PDLK_GESTURE_FORWARD:
							send_uevent(uinput_fd, EV_KEY, KEY_HOME, 1);
							send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
							printf("Sending home\n");
							send_uevent(uinput_fd, EV_KEY, KEY_HOME, 0);
							send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
							break;
						default:
						    break;
					}
					break;
				case SDL_ACTIVEEVENT:
					if (Event.active.state==SDL_APPACTIVE) {
						active=Event.active.gain;
						send_sleep_or_wake();
					}
					break;
				case SDL_QUIT:
					close(fd);
					close(uinput_fd);
					SDL_Quit();
					exit(0);
					break;
				default:
					break;
			}
		}
	}
	close(fd);
	close(uinput_fd);
	SDL_Quit();
	return 0;
}
