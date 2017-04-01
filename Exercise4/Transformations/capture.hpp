#ifndef __CAPTURE__
#define __CAPTURE__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>
#include <pthread.h>
#include <syslog.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define HRES                1280
#define VRES                960
#define RATIO               3
#define KERNEL_SIZE         3
#define MAX_LOW_THRESHOLD   100
#define NUM_TRANS           4
#define MAX_STRING_LENGTH   25

uint8_t transformationName[ NUM_TRANS ][ MAX_STRING_LENGTH ] = 
{
	"Raw Video",
	"Canny Threshold",
	"Hough Lines",
	"HoughCircles"
};

void *ShowRaw( void * );
void *CannyThreshold( void * );
void *HoughLines( void * );
void *HoughCircles( void * );

#endif // __CAPTURE__
