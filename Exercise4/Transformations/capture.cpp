#include "capture.hpp"

// Transform display window


int32_t lowThreshold = LOW_THRESHOLD;
int32_t const max_lowThreshold = MAX_LOW_THRESHOLD;
char timg_window_name[] = "Edge Detector Transform";

int32_t main( int argc, char** argv )
{
    CvCapture* capture;
    uint32_t dev = 0;
    IplImage* frame;

    if( argc > 1 )
    {
        sscanf( argv[1], "%d", &dev );
        printf( "using %s\n", argv[ 1 ] );
    }
    else if( argc == 1 )
    {
        printf( "using default\n" );
    }
    else
    {
        printf( "usage: capture [dev]\n" );
        exit( -1 );
    }

    namedWindow( timg_window_name, CV_WINDOW_AUTOSIZE );
    // Create a Trackbar for user to enter threshold
    createTrackbar( "Min Threshold:", timg_window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

    capture = (CvCapture *)cvCreateCameraCapture( dev );
    cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, HRES );
    cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, VRES );


    while( 1 )
    {
        frame = cvQueryFrame( capture );

        if( frame == NULL )
        {
            break;
        }

        //CannyThreshold( 0, frame );
        ShowRaw( 0, frame );

        char q = cvWaitKey( 33 );
        if( q == 'q' )
        {
            printf("got quit\n");
            break;
        }
    }

    cvReleaseCapture( &capture );

}

void CannyThreshold( int, void* frame )
{
    Mat mat_frame( ( IplImage* ) frame );
    Mat canny_frame, timg_gray, timg_grad;

    cvtColor( mat_frame, timg_gray, CV_RGB2GRAY );

    /// Reduce noise with a kernel 3x3
    blur( timg_gray, canny_frame, Size( 3,3 ) );

    /// Canny detector
    Canny( canny_frame, canny_frame, lowThreshold, lowThreshold * RATIO, KERNEL_SIZE );

    /// Using Canny's output as a mask, we display our result
    timg_grad = Scalar::all( 0 );

    mat_frame.copyTo( timg_grad, canny_frame );

    imshow( timg_window_name, timg_grad );

}

void ShowRaw( int, void* frame )
{
    Mat mat_frame( ( IplImage* ) frame );
    Mat raw;

    //raw = Scalar::all( 0 );
    //mat_frame.copyTo( )
    imshow( timg_window_name, mat_frame );
}
