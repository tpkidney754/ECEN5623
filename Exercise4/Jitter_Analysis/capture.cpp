#include "capture.hpp"

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_MICROSEC (1000)


// Transform display window
static void* ( *Transformation[ NUM_TRANS ] )( void * ) =
               { ShowRaw,
                 CannyThreshold,
                 HoughLines,
                 HoughCircles };

IplImage* frame;

int lowThreshold = 0;
struct timespec currentTime;
struct timespec startTime;
struct timespec calculateTime;
CvCapture* capture;

int avg_raw, cnt_raw, avg_canny, cnt_canny, avg_hough, cnt_hough, avg_houghE, cnt_houghE;

struct timespec startTimeRaw, startTimeCanny, startTimeHough, startTimeHoughE;
struct timespec endTimeRaw, endTimeCanny, endTimeHough, endTimeHoughE;

sem_t sem_raw, sem_canny, sem_hough, sem_houghE;

int32_t main( int argc, char** argv )
{
    uint32_t dev = 0;
    uint32_t index = 0;
    pthread_t thread_raw, thread_canny, thread_hough, thread_houghE;
    pthread_attr_t th0, th1, th2, th3, main_attr;
    struct sched_param th0_param, th1_param, th2_param, th3_param, main_param;
    int thread_args = 0;
    int result_code;
    char q;
    uint32_t frames = 0;
    uint32_t totalFrames = 0;
    uint32_t seconds = 0;
    float average = 0.0;
    uint32_t res = 0;
    int max_prio = 0;

    sem_init(&sem_raw, 0, 0);
    sem_init(&sem_canny, 0, 0);
    sem_init(&sem_hough, 0, 0);
    sem_init(&sem_houghE, 0, 0);

    pthread_attr_init(&th0);
    pthread_attr_init(&th1);
    pthread_attr_init(&th2);
    pthread_attr_init(&th3);
    pthread_attr_init(&main_attr);

    pthread_attr_setinheritsched(&th0, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&th0, SCHED_FIFO);

    pthread_attr_setinheritsched(&th1, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&th1, SCHED_FIFO);

    pthread_attr_setinheritsched(&th2, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&th2, SCHED_FIFO);

    pthread_attr_setinheritsched(&th3, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&th3, SCHED_FIFO);

    pthread_attr_setinheritsched(&main_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&main_attr, SCHED_FIFO);

    max_prio = sched_get_priority_max(SCHED_FIFO);

    main_param.sched_priority = max_prio;
    th0_param.sched_priority = max_prio - 1;
    th1_param.sched_priority = max_prio - 1;
    th2_param.sched_priority = max_prio - 1;

    int check = sched_setscheduler(getpid(), SCHED_FIFO, &main_param);

    if(check)
    {
        printf("Error while setting scheduler, value of check is %d \n",check);
        perror(NULL);
        exit(-1);
    }

    pthread_attr_setschedparam(&th0, &th0_param);
    pthread_attr_setschedparam(&th1, &th1_param);
    pthread_attr_setschedparam(&th2, &th2_param);
    pthread_attr_setschedparam(&th3, &th3_param);
    pthread_attr_setschedparam(&main_attr, &main_param);


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

    namedWindow( "Canny Window", WINDOW_AUTOSIZE );
    namedWindow( "Hough Window", WINDOW_AUTOSIZE );
    namedWindow( "HoughE Window", WINDOW_AUTOSIZE );


    cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, hres[0] );
    cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, vres[0] );

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
	    printf("Breaking\n");
            break;
        }

        pthread_create( &thread_raw,
                        NULL,
                        Transformation[0],
                        &thread_args );

        pthread_create( &thread_canny,
                        NULL,
                        Transformation[1],
                        &thread_args );

        pthread_create( &thread_hough,
                        NULL,
                        Transformation[2],
                        &thread_args );

        pthread_create( &thread_houghE,
                        NULL,
                        Transformation[3],
                        &thread_args );

	sem_post(&sem_raw);

        pthread_join( thread_raw, NULL );
        pthread_join( thread_canny, NULL );
        pthread_join( thread_hough, NULL );
        pthread_join( thread_houghE, NULL );

//        frames++;
//        clock_gettime( CLOCK_REALTIME, &currentTime );
//        if( currentTime.tv_sec > startTime.tv_sec && currentTime.tv_nsec > startTime.tv_nsec )
//        {
//           totalFrames += frames;
//            seconds += ( currentTime.tv_sec - startTime.tv_sec );
//            average = 1.0 * totalFrames / seconds;

//            startTime.tv_sec += 1;
//            frames = 0;
//            syslog( LOG_MAKEPRI( LOG_USER, LOG_INFO ),
//              "%s FPS: %f for resolution: %dx%d at time %ld:%ld\n",
//              transformationName[ index ], average, hres[ res ], vres[ res ], currentTime.tv_sec, currentTime.tv_nsec );
//            if( seconds >= 10 )
//            {
//                frames = 0;
//                seconds = 0;
//                totalFrames = 0;
//                clock_gettime( CLOCK_REALTIME, &currentTime );
//                startTime = currentTime;
//                if( index == NUM_TRANS - 1 )
//                {
//                    if( res == NUM_RES - 1 )
//                    {
//                        break;
//                    }
//                    else
//                    {
//                        cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, hres[ ++res ] );
//                        cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, vres[ res ] );
//                        index = 0;
//                    }
//                }
//                else
//                {
//                    index++;
//               }
//            }

//        }

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
            cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, hres[index] );
            cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, vres[index] );

        }
    }
    closelog();
    cvReleaseCapture( &capture );
}

int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
	int dt_sec=stop->tv_sec - start->tv_sec;
	int dt_nsec=stop->tv_nsec - start->tv_nsec;

	if(dt_sec >= 0)
	{
		if(dt_nsec >= 0)
		{
			delta_t->tv_sec=dt_sec;
		 	delta_t->tv_nsec=dt_nsec;
		}
		else
		{
			delta_t->tv_sec=dt_sec-1;
			delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
		}
	}
	else
	{
		if(dt_nsec >= 0)
		{
			delta_t->tv_sec=dt_sec;
			delta_t->tv_nsec=dt_nsec;
		}
		else
		{
			delta_t->tv_sec=dt_sec-1;
			delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
		}
	}

	return(1);
}



void *ShowRaw( void * )
{
    cpu_set_t aff;
    CPU_SET(0, &aff);
    if(pthread_setaffinity_np(pthread_self(), sizeof(aff), &aff) < 0)
    {
        perror("setting affinity");
    }

    int deadlineRaw = 50000000;

    sem_wait(&sem_raw);

    clock_gettime( CLOCK_REALTIME, &currentTime );
    startTimeRaw = currentTime;


    Mat raw( frame );

    imshow( "Main Window", raw );

    clock_gettime( CLOCK_REALTIME, &currentTime );
    endTimeRaw = currentTime;

    delta_t(&endTimeRaw, &startTimeRaw, &calculateTime);
    printf("calculateTime is %ld & %ld\n", calculateTime.tv_sec, calculateTime.tv_nsec);

    avg_raw += deadlineRaw - calculateTime.tv_nsec;
    cnt_raw++;

    printf("Jitter for Raw is: %f\n", (double)(avg_raw/cnt_raw));

    sem_post(&sem_canny);
}

void *CannyThreshold( void * )
{
    int deadlineCanny = 70000000;
    cpu_set_t aff;
    CPU_SET(0, &aff);
    if(pthread_setaffinity_np(pthread_self(), sizeof(aff), &aff) < 0)
    {
        perror("setting affinity");
    }


    sem_wait(&sem_canny);

    clock_gettime( CLOCK_REALTIME, &currentTime );
    startTimeCanny = currentTime;


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

    imshow( "Canny Window", timg_grad );

    clock_gettime( CLOCK_REALTIME, &currentTime );
    endTimeCanny = currentTime;

    delta_t(&endTimeCanny, &startTimeCanny, &calculateTime);
    printf("calculateTime for canny is %ld & %ld\n", calculateTime.tv_sec, calculateTime.tv_nsec);

    avg_canny += deadlineCanny - calculateTime.tv_nsec;
    cnt_canny++;

    printf("Jitter for Canny is: %f\n", (double)(avg_canny/cnt_canny));


    sem_post(&sem_hough);

}

void *HoughLines( void * )
{
    int deadlineHough = 120000000;
    cpu_set_t aff;
    CPU_SET(0, &aff);
    if(pthread_setaffinity_np(pthread_self(), sizeof(aff), &aff) < 0)
    {
        perror("setting affinity");
    }

    frame = cvQueryFrame( capture );
    if( frame == NULL )
    {
	printf("Breaking\n");
        exit(-1);
    }

    sem_wait(&sem_hough);

    clock_gettime( CLOCK_REALTIME, &currentTime );
    startTimeHough = currentTime;


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

    imshow( "Hough Window", mat_frame );

    clock_gettime( CLOCK_REALTIME, &currentTime );
    endTimeHough = currentTime;

    delta_t(&endTimeHough, &startTimeHough, &calculateTime);
    printf("calculateTime for hough is %ld & %ld\n", calculateTime.tv_sec, calculateTime.tv_nsec);

    avg_hough += deadlineHough - calculateTime.tv_nsec;
    cnt_hough++;

    printf("Jitter for Hough is: %f\n", (double)(avg_hough/cnt_hough));

    sem_post(&sem_houghE);
}

void *HoughCircles( void * )
{
    int deadlineHoughE = 150000000;
    cpu_set_t aff;
    CPU_SET(0, &aff);
    if(pthread_setaffinity_np(pthread_self(), sizeof(aff), &aff) < 0)
    {
        perror("setting affinity");
    }

    sem_wait(&sem_houghE);

    clock_gettime( CLOCK_REALTIME, &currentTime );
    startTimeHoughE = currentTime;

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

    imshow( "HoughE Window", mat_frame );

    clock_gettime( CLOCK_REALTIME, &currentTime );
    endTimeHoughE = currentTime;

    delta_t(&endTimeHoughE, &startTimeHoughE, &calculateTime);
    printf("calculateTime for HoughE is %ld & %ld\n", calculateTime.tv_sec, calculateTime.tv_nsec);

    avg_houghE += deadlineHoughE - calculateTime.tv_nsec;
    cnt_houghE++;

    printf("Jitter for HoughE is: %f\n", (double)(avg_houghE/cnt_houghE));


    sem_post(&sem_raw);
}

