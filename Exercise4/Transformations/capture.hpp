#ifndef __CAPTURE__
#define __CAPTURE__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define HRES                640
#define VRES                480
#define RATIO               3
#define KERNEL_SIZE         3
#define MAX_LOW_THRESHOLD   100
#define LOW_THRESHOLD       0

void CannyThreshold( int, void* frame );
void ShowRaw( int, void* frame );

#endif // __CAPTURE__
