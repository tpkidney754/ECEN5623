





/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/

/* Set the following option to 1 to include the WEB server in the build.  By
default the WEB server is excluded to keep the compiled code size under the 32K
limit imposed by the KickStart version of the IAR compiler.  The graphics
libraries take up a lot of ROM space, hence including the graphics libraries
and the TCP/IP stack together cannot be accommodated with the 32K size limit. */
#define mainINCLUDE_WEB_SERVER		1


/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "sysctl.h"
#include "gpio.h"
#include "grlib.h"

#include "rit128x96x4.h"
#include "osram128x64x4.h"
#include "formike128x128x16.h"
#include "interrupt.h"
#include "timer.h"

/* Demo app includes. */
//#include "BlockQ.h"
//#include "death.h"
#include "integer.h"
//#include "blocktim.h"
#include "flash.h"
//#include "partest.h"
//#include "semtest.h"
//#include "PollQ.h"
#include "lcd_message.h"
#include "bitmap.h"
#include "GenQTest.h"
//#include "QPeek.h"
//#include "recmutex.h"
//#include "IntQueue.h"

/*-----------------------------------------------------------*/

/* The time between cycles of the 'check' functionality (defined within the
tick hook. */
#define mainCHECK_DELAY						( ( portTickType ) 5000 / portTICK_RATE_MS )

/* Size of the stack allocated to the uIP task. */
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 3 )

/* The OLED task uses the sprintf function so requires a little more stack too. */
#define mainOLED_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE + 50 )

/* Task priorities. */

#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY +1)


/* The maximum number of message that can be waiting for display at any one
time. */
#define mainOLED_QUEUE_SIZE					( 3 )

/* Dimensions the buffer into which the jitter time is written. */
#define mainMAX_MSG_LEN						25

/* The period of the system clock in nano seconds.  This is used to calculate
the jitter time in nano seconds. */
#define mainNS_PER_CLOCK					( ( unsigned portLONG ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/* Constants used when writing strings to the display. */
#define mainCHARACTER_HEIGHT				( 9 )
#define mainMAX_ROWS_128					( mainCHARACTER_HEIGHT * 14 )
#define mainMAX_ROWS_96						( mainCHARACTER_HEIGHT * 10 )
#define mainMAX_ROWS_64						( mainCHARACTER_HEIGHT * 7 )
#define mainFULL_SCALE						( 15 )
#define ulSSI_FREQUENCY						( 3500000UL )

/*-----------------------------------------------------------*/

/*
 * The task that handles the uIP stack.  All TCP/IP processing is performed in
 * this task.
 */
extern void vuIP_Task( void *pvParameters );

/*
 * The display is written two by more than one task so is controlled by a
 * 'gatekeeper' task.  This is the only task that is actually permitted to
 * access the display directly.  Other tasks wanting to display a message send
 * the message to the gatekeeper.
 */
static void vOLEDTask( void *pvParameters );

static void vOLED_Print_Task( void *pvParameters );


/*
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void );

/*
 * Configures the high frequency timers - those used to measure the timing
 * jitter while the real time kernel is executing.
 */
extern void vSetupHighFrequencyTimer( void );

/*
 * Hook functions that can get called by the kernel.
 */
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );
void vApplicationTickHook( void );


/* The queue used to send messages to the OLED task. */
xQueueHandle xOLEDQueue;

/* The welcome text. */
const portCHAR * const pcWelcomeMessage = "   www.FreeRTOS.org";

int set_up_LCD=0;

/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/



int main( void )
{
	
	prvSetupHardware();
	
	timer_init();
	uart_init();

  /* Create the queue used by the OLED task.  Messages for display on the OLED
	are received via this queue. */
	xOLEDQueue = xQueueCreate( mainOLED_QUEUE_SIZE, sizeof( xOLEDMessage ) );
	xTaskCreate( vOLEDTask, ( signed portCHAR * ) "OLED", mainOLED_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY+10, NULL );

	xTaskCreate( vOLED_Print_Task, ( signed portCHAR * ) "OLED_PRINT", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+9, NULL );

  vStartFibTasks( tskIDLE_PRIORITY+20 );

	/* Start the scheduler. */
	vTaskStartScheduler();
	
  /* Will only get here if there was insufficient memory to create the idle
    task. */
	return 0;
}
/*-----------------------------------------------------------*/

void prvSetupHardware( void )
{
    /* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    a workaround to allow the PLL to operate reliably. */
    if( DEVICE_IS_REVA2 )
    {
        SysCtlLDOSet( SYSCTL_LDO_2_75V );
    }
	
	/* Set the clocking to run from the PLL at 50 MHz */
	SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );
	
	/* 	Enable Port F for Ethernet LEDs
		LED0        Bit 3   Output
		LED1        Bit 2   Output */
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF );
	GPIODirModeSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3), GPIO_DIR_MODE_HW );
	GPIOPadConfigSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3 ), GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );	
	
	//vParTestInitialise();
}

/*-----------------------------------------------------------*/
void vApplicationTickHook( void )
{
static unsigned portLONG ulTicksSinceLastDisplay = 0;
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

}
/*-----------------------------------------------------------*/



void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
}

void vOLEDTask( void *pvParameters )
{
xOLEDMessage xMessage;
unsigned portLONG ulY, ulMaxY;
static portCHAR cMessage[ mainMAX_MSG_LEN ];
extern volatile unsigned portLONG ulMaxJitter;
unsigned portBASE_TYPE uxUnusedStackOnEntry;
const unsigned portCHAR *pucImage;

/* Functions to access the OLED.  The one used depends on the dev kit
being used. */
void ( *vOLEDInit )( unsigned portLONG ) = NULL;
void ( *vOLEDStringDraw )( const portCHAR *, unsigned portLONG, unsigned portLONG, unsigned portCHAR ) = NULL;
void ( *vOLEDImageDraw )( const unsigned portCHAR *, unsigned portLONG, unsigned portLONG, unsigned portLONG, unsigned portLONG ) = NULL;
void ( *vOLEDClear )( void ) = NULL;

	/* Just for demo purposes. */
	uxUnusedStackOnEntry = uxTaskGetStackHighWaterMark( NULL );

	/* Map the OLED access functions to the driver functions that are appropriate
	for the evaluation kit being used. */	
	switch( HWREG( SYSCTL_DID1 ) & SYSCTL_DID1_PRTNO_MASK )
	{
		case SYSCTL_DID1_PRTNO_6965	:	
		case SYSCTL_DID1_PRTNO_2965	:	vOLEDInit = OSRAM128x64x4Init;
										vOLEDStringDraw = OSRAM128x64x4StringDraw;
										vOLEDImageDraw = OSRAM128x64x4ImageDraw;
										vOLEDClear = OSRAM128x64x4Clear;
										ulMaxY = mainMAX_ROWS_64;
										pucImage = pucBasicBitmap;
										break;
										
		case SYSCTL_DID1_PRTNO_1968	:	
		case SYSCTL_DID1_PRTNO_8962 :	vOLEDInit = RIT128x96x4Init;
										vOLEDStringDraw = RIT128x96x4StringDraw;
										vOLEDImageDraw = RIT128x96x4ImageDraw;
										vOLEDClear = RIT128x96x4Clear;
										ulMaxY = mainMAX_ROWS_96;
										pucImage = pucBasicBitmap;
										break;
										
		default						:	vOLEDInit = vFormike128x128x16Init;
										vOLEDStringDraw = vFormike128x128x16StringDraw;
										vOLEDImageDraw = vFormike128x128x16ImageDraw;
										vOLEDClear = vFormike128x128x16Clear;
										ulMaxY = mainMAX_ROWS_128;
										pucImage = pucGrLibBitmap;
										break;
										
	}

	ulY = ulMaxY;
	
	/* Initialise the OLED and display a startup message. */
	vOLEDInit( ulSSI_FREQUENCY );	
	vOLEDStringDraw( "POWERED BY FreeRTOS", 0, 0, mainFULL_SCALE );
	vOLEDImageDraw( pucImage, 0, mainCHARACTER_HEIGHT + 1, bmpBITMAP_WIDTH, bmpBITMAP_HEIGHT );

	for( ;; )
	{
		/* Wait for a message to arrive that requires displaying. */
		xQueueReceive( xOLEDQueue, &xMessage, portMAX_DELAY );
	
		/* Write the message on the next available row. */
		ulY += mainCHARACTER_HEIGHT;
		if( ulY >= ulMaxY )
		{
			ulY = mainCHARACTER_HEIGHT;
			vOLEDClear();
			vOLEDStringDraw( pcWelcomeMessage, 0, 0, mainFULL_SCALE );			
		}

		/* Display the message along with the maximum jitter time from the
		high priority time test. */
		sprintf( cMessage, "%s ", xMessage.pcMessage);
		vOLEDStringDraw( cMessage, 0, ulY, mainFULL_SCALE );
	}
}
/*-----------------------------------------------------------*/
void vOLED_Print_Task( void *pvParameters )
{
	//static xOLEDMessage xMessage = { "JAY" };
  static unsigned portLONG ulTicksSinceLastDisplay = 0;
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  int flag=0;
	//uart_send("Here1",5);
  for (; ;)
  {
		
			if( flag != 1 )
				{
					//xMessage.pcMessage = "ECEN";
					//uart_send("Here2",5);
					//xOLEDMessage xMessage = { "ECEN" };
					flag=1;
					//xHigherPriorityTaskWoken = pdFALSE;
		      //xQueueSendFromISR( xOLEDQueue, &xMessage, &xHigherPriorityTaskWoken );
					
				}
			else 
				{
					//xMessage.pcMessage ="5623";
					//uart_send("Here3",5);
					//xOLEDMessage xMessage = { "5623" };
					flag=0;
					//xHigherPriorityTaskWoken = pdFALSE;
		      //xQueueSendFromISR( xOLEDQueue, &xMessage, &xHigherPriorityTaskWoken );
				}
				  //xHigherPriorityTaskWoken = pdFALSE;
		     // xQueueSendFromISR( xOLEDQueue, &xMessage, &xHigherPriorityTaskWoken );
		}
 }



