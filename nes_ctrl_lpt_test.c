/*
	Program to test NES controller functionality on parallel port
	-------------------------------------------------------------
	
	Author: Martin Albrecht <martin.albrecht@javacoffee.de>
	Homepage: http://code.javacoffee.de
	
	Copyright (C) 2008 Martin Albrecht <martin.albrecht@javacoffee.de>
	
	I used some code from the Linux joystick driver project, but extended and
	modified it.
	(joy-console.c  Version 0.14V)
	Copyright (c) 1998 Andree Borrmann
	Copyright (c) 1999 John Dahlstrom
	Copyright (c) 1999 David Kuder
	Copyright (c) 1999 Vojtech Pavlik
	
	This program is free software; you can redistribute it and/or modify 
	it under the terms of the GNU General Public License as published by 
	the Free Software Foundation; either version 2 of the License, or 
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful, but 
	WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
	or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
	for more details.
	
	You should have received a copy of the GNU General Public License along 
	with this program; if not, see http://www.gnu.org/licenses/
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/joystick.h>


/*
	Define our base address to 0x378 (the first parallel port on most systems)
	and set data, status and control port addresses.
	
	To use LPT2 or three, just edit the base address to your local address of
	the port.
*/
#define BASE 0x378
#define DATA BASE+0
#define STATUS BASE+1
#define CONTROL BASE+2


/*
	Write the value of a byte to a char pointer in binary format
*/
void binprint( unsigned char x, char *buf )
{
    int i;
    for( i=0; i<8; i++ )
        buf[7-i] = (x&(1<<i)) ? '1' : '0';
    buf[8] = 0;
    return;
}


/*
	Okay, here is our main loop...
*/
int main(int argc, char **argv)
{
    char bin_DTA[255];
    char bin_STA[255];
    char bin_CTR[255];
    int fd;
    int i;
    
    unsigned char buttons = 2;
    char *button;
    unsigned char axes = 2;
    int *axis;
    int version = 0x000800;
    char name[255] = "Unknown";
    uint16_t btnmap[KEY_MAX - BTN_MISC + 1];
    uint8_t axmap[ABS_MAX + 1];
    struct js_event js;
    
	/* Usage... */
    if( argc < 2 ){
        printf("Usage: %s <device>\n\nDevice could be: /dev/js0\n", argv[0]);
        return 0;
    }
    
	/* Open the device and read in data */
    if( (fd = open(argv[1], O_RDONLY)) < 0 ){
        printf("Error: Cannot open device %s!\n", argv[1]);
        exit(1);
    }
    if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
		perror("\njstest: error reading");
		return 1;
    }
    
    /* Set our variables with the joystick data */
    ioctl(fd, JSIOCGVERSION, &version);
    ioctl(fd, JSIOCGAXES, &axes);
    ioctl(fd, JSIOCGBUTTONS, &buttons);
    ioctl(fd, JSIOCGNAME(255), name);
    ioctl(fd, JSIOCGAXMAP, axmap);
    ioctl(fd, JSIOCGBTNMAP, btnmap);

    while(1){
        
		/* Get access to the ports */
        if (ioperm(BASE, 3, 1)){
            perror("ioperm"); exit(1);
        }
		
		/* Look for pressed buttons or axis on joystick */
        switch(js.type & ~JS_EVENT_INIT) {
            case JS_EVENT_BUTTON:
                button[js.number] = js.value;
                break;
            case JS_EVENT_AXIS:
                axis[js.number] = js.value;
                break;
            default:
                break;
        }
        
		/* Clear out the screen */
        system("clear");
	
		/* Status of the pins in binary format */
        binprint(inb(DATA), bin_DTA);
        binprint(inb(STATUS), bin_STA);
        binprint(inb(CONTROL), bin_CTR);
        
        /* Print out our vlues on the port */
        printf("Name: %s\nVersion %d\nButtons: %d\n", name, version, buttons);
        printf("\n------------ RUNNING SCAN ------------\n");
        printf("Data\t\tStatus\t\tControl\n");
        printf("----\t\t------\t\t-------\n");
        printf("%d  \t\t%d    \t\t%d     \n", inb(DATA), inb(STATUS), inb(CONTROL));
        printf("%s\t%s\t%s\n\n",bin_DTA , bin_STA, bin_CTR);
        
		/* Test area to test controller functionality */
		printf("Test area:\n----------\n\r");
        
		/* Check out axis and print out status */
        if (axes) {
            printf("Axes: ");
            for (i = 0; i < axes; i++)
                printf("%2d:%6d ", i, axis[i]);
        }
        
		/* Check out buttons and print out status */
        if (buttons) {
            printf("\n\nButtons: ");
            for (i = 0; i < buttons; i++)
                printf("%2d:%s", i, button[i] ? "on " : "off");
        }
        
		/* Exit message, flush screen and sleep for 10.000 microsekonds */
        printf("\n\nInterrupt (CTRL+C) to exit...\n");
        fflush(stdout);
        usleep(10000);
	
		/* Read in the device status again */
		if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
			perror("\njstest: error reading");
			return 1;
		}
        
        /* We don't need the ports anymore */
        if (ioperm(BASE, 3, 0)){
            perror("ioperm"); exit(1);
        }
    }
	
    return 0;
}
