#ifndef __CAPTURE__
#define __CAPTURE__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <syslog.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define RATIO               3
#define KERNEL_SIZE         3
#define MAX_LOW_THRESHOLD   100
#define NUM_TRANS           4
#define MAX_STRING_LENGTH   25
#define NUM_RES             3

uint8_t transformationName[ NUM_TRANS ][ MAX_STRING_LENGTH ] = 
{
	"Raw Video",
	"Canny Threshold",
	"Hough Lines",
	"HoughCircles"
};

uint32_t hres[ NUM_RES ] = { 320, 640, 1280 };
uint32_t vres[ NUM_RES ] = { 240, 480, 960  };

void *ShowRaw( void * );
void *CannyThreshold( void * );
void *HoughLines( void * );
void *HoughCircles( void * );

#endif // __CAPTURE__
