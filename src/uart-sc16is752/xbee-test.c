/* ------------------------------------------------------------ *
 * file:        xbee-test.c                                     *
 * purpose:     Sample program tests XBEE module communication  *
 *              sends +++ for CMD mode and expects OK returned  *
 *                                                              *
 *              rpi0w <-- ttySC1 RX ---\/--- RX XBEE            *
 *              rpi0w --> ttySC1 TX ---/\--- TX XBEE            *
 *                                                              *
 * return:      0 on success, and -1 on errors.                 *
 * compile:     see Makefile                                    *
 * example:     ./xbee-test                                     *
 *              XBee DEV open: /dev/ttySC1 9600B                *
 *              XBee CMD mode: OK                               *
 *              Xbee cmd ATSL: [417D5111]                       *
 *                                                              *
 * author:      06/10/2020 Frank4DD                             *
 * ------------------------------------------------------------ */
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "serial.h"

/* ------------------------------------------------------------ *
 * global variables                                             *
 * ------------------------------------------------------------ */
int verbose  = 0;              // 0 = off, 1 = on
char *port   = "/dev/ttySC1";  // serial port device
int speed    = 115200;         // XBee modified speed
//int speed    = 9600;         // XBee default speed

int main(void) {
   int fd;                      // serial port file descriptor
   int i;                       // loop counter
   int wait;                    // serail bytes waiting
   char response[1024] = "";    // serial response string

   printf("XBee DEV open: %s %dB\n", port, speed);

/* ------------------------------------------------------------ *
 * open serial port                                             *
 * ------------------------------------------------------------ */
   if((fd=getserial(port, speed)) < 0) {
      printf("Error opening port %s %d Baud\n", port, speed);
      return -1;
   }
   if(verbose == 1) printf("Debug: %s %d Baud connected\n", port, speed);

/* ------------------------------------------------------------ *
 * wait guard time, and send Xbee cmd char sequence (cc): +++   *
 * ------------------------------------------------------------ */
   sleep(1);              // wait guard time 1 second
   strserial(fd, "+++");
   if(verbose == 1) printf("Debug: %s send +++\n", port);

/* ------------------------------------------------------------ *
 * wait guard time, and check if 3 bytes response is received   *
 * ------------------------------------------------------------ */
   i = 0; wait = 0;
   while((wait = checkserial(fd)) == 0) {
      usleep(250000);     // waith another quarter second
      if(i>12) break;     // stop waiting after 3 seconds
      i++;
   }
   if(verbose == 1) printf("Debug: %s got %d bytes after %d ms\n", port, wait, i*250);

/* ------------------------------------------------------------ *
 * If there is no response, we have XBee communication failure  *
 * Either no XBee is connected, or XBee is on different speed.  *
 * ------------------------------------------------------------ */
   if(wait == 0) {
      printf("Error: No XBee responding\n");
      return -1;
   }

/* ------------------------------------------------------------ *
 * retrieve the reply chars and save them into response string  *
 * ------------------------------------------------------------ */
   i = 0;
   for (i=0; i<1024; i++) {
      if(checkserial(fd)>0) response[i] = getcharserial(fd);
      else break;
   }
/* ------------------------------------------------------------ *
 * Eliminate '\r' carriage return from response last char       *
 * ------------------------------------------------------------ */
   i--;                                        // back to last char
   if(response[i] == '\r') response[i] = '\0'; // eliminate '\r'

/* ------------------------------------------------------------ *
 * display response string                                      *
 * ------------------------------------------------------------ */
   if(verbose == 1) printf("Debug: port %s reply: %s (%d bytes)\n", port, response, i);
   if(strcmp(response, "OK") == 0) printf("XBee CMD mode: OK\n");

/* ------------------------------------------------------------ *
 * Send ATSL command to get lower device address                *
 * ------------------------------------------------------------ */
   strserial(fd, "ATSL\r");
   wait = checkserial(fd);
   if(verbose == 1) printf("Debug: %s send ATSL\\r\n", port);

/* ------------------------------------------------------------ *
 * Check if a response is received, wait up to 3 seconds        *
 * ------------------------------------------------------------ */
   i = 0;
   while((wait = checkserial(fd)) == 0) {
      usleep(250000);     // waith another quarter second
      if(i>12) break;     // stop waiting after 3 seconds
      i++;
   }
   if(verbose == 1) printf("Debug: %s got %d bytes after %d ms\n", port, wait, i*250);

/* ------------------------------------------------------------ *
 * retrieve the reply chars and save them into response string  *
 * ------------------------------------------------------------ */
   response[0] = '\0';
   for (i=0; i<1024; i++) {
      if(checkserial(fd)>0) response[i] = getcharserial(fd);
      else break;
   }

/* ------------------------------------------------------------ *
 * Eliminate '\r' carriage return from response last char       *
 * ------------------------------------------------------------ */
   i--;                                        // back to last char
   if(response[i] == '\r') response[i] = '\0'; // eliminate '\r'

/* ------------------------------------------------------------ *
 * display response string                                      *
 * ------------------------------------------------------------ */
   if(verbose == 1) printf("Debug: port %s reply: %s (%d bytes)\n", port, response, i);
   printf("Xbee cmd ATSL: [%s]\n", response);

/* ------------------------------------------------------------ *
 * close serial connection                                      *
 * ------------------------------------------------------------ */
   closeserial(fd);
   if(verbose == 1) printf("Debug: %s closed\n", port);
   return 0;
}
