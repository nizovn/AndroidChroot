/*=============================================================================
 Copyright (C) 2013 Nikolay Nizov <nizovn@gmail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 =============================================================================*/

#include <stdio.h>
#include "SDL.h"
#include "luna_methods.h"

#include "PDL.h"
#include <linux/input.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <linux/types.h>
#include <stdbool.h>

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
bool sigexit=false;
bool sendshutdown=false;
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
void send_sleep_or_wake(bool active){
	int fd_wake=open(WAKE_FILE_NAME,O_NONBLOCK|O_WRONLY);
	int fd_sleep=open(SLEEP_FILE_NAME,O_NONBLOCK|O_WRONLY);
	if((fd_wake>0)&&(active)){
		send_uevent(uinput_fd, EV_KEY, KEY_POWER, 1);
		send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
		printf("Sending screen\n");
		send_uevent(uinput_fd, EV_KEY, KEY_POWER, 0);
		send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
		system("echo awake > " WAKE_FILE_NAME);
	}
	if((fd_sleep>0)&&(!active)){
		send_uevent(uinput_fd, EV_KEY, KEY_POWER, 1);
		send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
		printf("Sending screen\n");
		send_uevent(uinput_fd, EV_KEY, KEY_POWER, 0);
		send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
		system("echo sleeping > " SLEEP_FILE_NAME);
	}
	close(fd_wake);
	close(fd_sleep);
}
void cleanup(){
	close(fd);
	ioctl(uinput_fd, UI_DEV_DESTROY);
	close(uinput_fd);
	SDL_Quit();
}
void handler1(int sig){
	sigexit=true;
	return;
}
void handler2(int sig){
	sendshutdown=true;
	return;
}
int main(int argc, char** argv)
{

	if (Init() == false)
		return -1;
	struct sigaction act1,act2;
	memset(&act1,0,sizeof(act1));
	memset(&act2,0,sizeof(act2));
	act1.sa_handler=handler1;
	act2.sa_handler=handler2;
	sigset_t set1,set2;
	sigemptyset(&set1);
	sigemptyset(&set2);
	sigaddset(&set1,SIGUSR1);
	sigaddset(&set2,SIGUSR2);
	act1.sa_mask=set1;
	act2.sa_mask=set2;
	sigaction(SIGUSR1, &act1,0);
	sigaction(SIGUSR2, &act2,0);
	open_uinput();
	SDL_Event Event;
	bool active=true;
	send_sleep_or_wake(active);
	while (1) {
		SDL_Delay(10);
		if (active) {
			set_fb1();
			SDL_GL_SwapBuffers();
		}
		else{
			SDL_Delay(1000);
		} 
		if(sigexit){
			cleanup();
			exit(1);
		};
		if((sendshutdown)&&(active)){
			sendshutdown=false;	
			send_uevent(uinput_fd, EV_KEY, KEY_POWER, 1);
			send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
			printf("Sending power on\n");
			SDL_Delay(1000);
			send_uevent(uinput_fd, EV_KEY, KEY_POWER, 0);
			send_uevent(uinput_fd, EV_SYN, SYN_REPORT, 0);
			printf("Sending power off\n");
 		};
		int x,y,k;
		while (SDL_PollEvent(&Event)) {
			switch (Event.type) {
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONUP:
					k=0;
					int i;
					for(i=0;i<5;i++){
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
						send_sleep_or_wake(active);
					}
					break;
				case SDL_QUIT:
					cleanup();
					exit(0);
					break;
				default:
					break;
			}
		}
	}
	cleanup();
	exit(0);
	return 0;
}
