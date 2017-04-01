#include "capture.hpp"

// Transform display window
static void* ( *Transformation[ NUM_TRANS ] )( void * ) =
               { ShowRaw,
                 CannyThreshold,
                 HoughLines,
                 HoughCircles };

int lowThreshold = 0;
IplImage* frame;
struct timespec currentTime;
struct timespec startTime;
struct timespec calculateTime;

int32_t main( int argc, char** argv )
{
    CvCapture* capture;
    uint32_t dev = 0;
    uint32_t index = 0;
    pthread_t thread;
    int thread_args = 0;
    int result_code;
    char q;
    uint32_t frames = 0;
    uint32_t totalFrames = 0;
    uint32_t seconds = 0;
    float average = 0.0;
    uint32_t res = 0;

    openlog( NULL, LOG_CONS, LOG_USER );

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

    namedWindow( "Main Window", WINDOW_AUTOSIZE );
    capture = (CvCapture *)cvCreateCameraCapture( dev );

    cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, hres[ res ] );
    cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, vres[ res ] );

    clock_gettime( CLOCK_REALTIME, &currentTime );
    startTime = currentTime;
    calculateTime = currentTime;
    calculateTime.tv_sec += 10;

    syslog( LOG_MAKEPRI( LOG_USER, LOG_INFO ),
              "Starting Log\n" );

    while( 1 )
    {

        frame = cvQueryFrame( capture );

        if( frame == NULL )
        {
            break;
        }


        pthread_create( &thread,
                        NULL,
                        Transformation[ index ],
                        &thread_args );

        pthread_join( thread, NULL );

        frames++;
        clock_gettime( CLOCK_REALTIME, &currentTime );
        if( currentTime.tv_sec > startTime.tv_sec && currentTime.tv_nsec > startTime.tv_nsec )
        {
            totalFrames += frames;
            seconds += ( currentTime.tv_sec - startTime.tv_sec );
            average = 1.0 * totalFrames / seconds;
        
            startTime.tv_sec += 1;
            frames = 0;
            syslog( LOG_MAKEPRI( LOG_USER, LOG_INFO ),
              "%s FPS: %f for resolution: %dx%d at time %ld:%ld\n",
              transformationName[ index ], average, hres[ res ], vres[ res ], currentTime.tv_sec, currentTime.tv_nsec );
            if( seconds >= 10 )
            {
                frames = 0;
                seconds = 0;
                totalFrames = 0;
                clock_gettime( CLOCK_REALTIME, &currentTime );
                startTime = currentTime;
                if( index == NUM_TRANS - 1 )
                {
                    if( res == NUM_RES - 1 )
                    {
                        break;
                    }
                    else
                    {
                        cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, hres[ ++res ] );
                        cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, vres[ res ] );
                        index = 0;
                    }
                }
                else
                {
                    index++;
                }
            }

        }

        q = cvWaitKey( 33 );
        if( q == 'q' )
        {
            printf("got quit\n");
            break;
        }
        else if( q >= '0' && q < ( NUM_TRANS + '0' ) )
        {
            index = q - '0';
            totalFrames = 0;
            frames = 0;
            seconds = 0;
            clock_gettime( CLOCK_REALTIME, &currentTime );
            startTime = currentTime;
        }
    }
    closelog();
    cvReleaseCapture( &capture );
}

void *ShowRaw( void * )
{
    Mat raw( frame );

    imshow( "Main Window", raw );
}

void *CannyThreshold( void * )
{
    Mat mat_frame( frame );
    Mat canny_frame, timg_gray, timg_grad;

    cvtColor( mat_frame, timg_gray, CV_RGB2GRAY );

    /// Reduce noise with a kernel 3x3
    blur( timg_gray, canny_frame, Size( 3,3 ) );

    /// Canny detector
    Canny( canny_frame, canny_frame, lowThreshold, lowThreshold * RATIO, KERNEL_SIZE );

    /// Using Canny's output as a mask, we display our result
    timg_grad = Scalar::all( 0 );

    mat_frame.copyTo( timg_grad, canny_frame );

    imshow( "Main Window", timg_grad );

}

void *HoughLines( void * )
{
    Mat mat_frame(frame);
    Mat canny_frame;
    vector<Vec4i> lines;

    Canny( mat_frame, canny_frame, 50, 200, 3 );

    //cvtColor(canny_frame, cdst, CV_GRAY2BGR);
    //cvtColor(mat_frame, gray, CV_BGR2GRAY);

    HoughLinesP( canny_frame, lines, 1, CV_PI / 180, 50, 50, 10 );

    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        line( mat_frame, Point( l[ 0 ], l[ 1 ] ), Point( l[ 2 ], l[ 3 ] ), Scalar( 0, 0, 255 ), 3, CV_AA );
    }

    imshow( "Main Window", mat_frame );
}

void *HoughCircles( void * )
{
    Mat mat_frame( frame );
    Mat gray;
    vector<Vec3f> circles;

    cvtColor( mat_frame, gray, CV_BGR2GRAY );
    GaussianBlur( gray, gray, Size( 9, 9 ), 2, 2 );

    HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows / 8, 100, 50, 0, 0);

    for( size_t i = 0; i < circles.size( ); i++ )
    {
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      // circle center
      circle( mat_frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
      // circle outline
      circle( mat_frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
    }

    imshow( "Main Window", mat_frame );

}
