/*
 * rfm12.c:
 *
 ***********************************************************************

 ***********************************************************************
 */

/*
char buffer[PAKET_LEN] =
{
		(0x50 | SENSOR_NR),  	// Sensornummer
		(0x60 | SENSOR_VALUES), 			// Status + Sensor Anzahl
		(0x50 | SUB_TYP_1), 0x35, 0x20, 0x20, 0x20, 0x20, 0x20, // Sub-Sensor 1
		(0x50 | SUB_TYP_2), 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, // Sub-Sensor 2
		(0x50 | SUB_TYP_3), 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, // Sub-Sensor 3
		0x00,  // sequence number
		0x00 }; // Checksumme
*/

/*
char buffer[PAKET_LEN] =
{
    (0x50 | SENSOR_NR),
     0x50,                  					// Sensornummer, Status
    (0x50 | SUB_TYP_1),0x35,0x20,0x20,0x20,0x20,0x20,		// Sub-Sensor 1
	(0x50 | SUB_TYP_2),0x20,0x20,0x20,0x20,0x20,0x20,		// Sub-Sensor 2
	(0x50 | SUB_TYP_3),0x20,0x20,0x20,0x20,0x20,0x20,		// Sub-Sensor 3
	 0x00};                                             // Checksumme
*/

#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <linux/spi/spidev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#include "config.h"

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "rfm12_communication.h"

char path[200] = "/var/www/html/wetter";

static volatile int keepRunning = 1;
static volatile int verbose = 0;
static volatile int foreground = 0;

void intHandler(int dummy) 
{
	syslog(LOG_NOTICE, "Interrupt detected ... will terminate!");
	keepRunning = 0;
}


#define GPIO_FN_MAXLEN	64
#define POLL_TIMEOUT	600000
#define RDBUF_LEN	5

static int myFd ;


typedef struct  {
      uint8_t SensorNr;
      uint8_t Values;
      uint8_t SensorType1;
      char SensorValue1[6];
      uint8_t SensorType2;
      char SensorValue2[6];
      uint8_t SensorType3;
      char SensorValue3[6];
      uint8_t SequenceNumber;
      uint8_t Checksum;

} DataGramm25;

typedef struct  {
      uint8_t SensorNr;
      uint8_t Values;
      uint8_t SensorType1;
      char SensorValue1[6];
      uint8_t SensorType2;
      char SensorValue2[6];
      uint8_t SensorType3;
      char SensorValue3[6];
      uint8_t Checksum;

} DataGramm24;

typedef union
{
    DataGramm25 dataGramm25;
    DataGramm24 dataGramm24;
    uint8_t buffer[27];
} DataPaketUnion;


/*
*
*/
  uint8_t
    _crc_ibutton_update(uint8_t crc, uint8_t data)
    {
        uint8_t i;

        crc = crc ^ data;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x01)
                crc = (crc >> 1) ^ 0x8C;
            else
                crc >>= 1;
        }

        return crc;
    }

/*
 */
uint8_t checkChecksum(uint8_t PAKET_LEN, uint8_t* buffer)
{
    uint8_t byte = 0;

    for (uint8_t i=0; i<(PAKET_LEN-2); i++)
    {
	 	byte = _crc_ibutton_update(byte, buffer[i]);
    }

    return byte;
}

/*
*/
double stringToDouble ( char* string, int len )
{
    char buffer[10];
    strncpy ( buffer, string, len);
    buffer[7] = 0;
    return atof(buffer);
}


/*
 *
 * Read data from RFM12
 *
 */
void readData ()
{
    int abort = 0;

        uint8_t d;
	//while (1)
    {
        //printf ("Reading for data ...\n");

        //while (digitalRead(6)==0)
        //    delay ( 1000 );

        rfm_rxoff();

        //while ( digitalRead(5)==1 )
        //{
        //    d = rfm69_readReg ( 0x00 );
        //    fprintf (stdout, "%02X \n", d);
        //}

        DataPaketUnion dataPaket;

        for (int i = 0; i<25; i++)
        {
            d = rfm69_readReg ( 0x00 );
            dataPaket.buffer[i] = d;
        }

        if ( (dataPaket.buffer[0] & 0xF0) != 0x50 )
        {
            //printf ("not my packet ....\n");
            abort = 1;
        }

        if ( !abort )
        {

            uint8_t paketLen = 25;
            if ( dataPaket.buffer[1] == 0x50 )
            {
				//fprintf(stdout, "Might temperature packet ...\n");
                paketLen = 24;
            }

            uint8_t cs = checkChecksum( paketLen, (uint8_t*)&dataPaket.buffer);
            //fprintf (stdout, "==> %02X \n", cs);
            if ( cs == dataPaket.buffer[paketLen-1])
            {
               for (int i = 0; i<paketLen; i++)
                {
                    //fprintf (stdout, "%02i: %02X %c\n", i, dataPaket.buffer[i], dataPaket.buffer[i]);
                }
				
                if (paketLen == 25 )
                {


					if (verbose) fprintf(stdout, "Send: %i\n", dataPaket.dataGramm25.SensorNr & 0x0F);
					if (verbose) fprintf(stdout, "Sequ: %i\n", dataPaket.dataGramm25.SequenceNumber);
                    //fprintf ( stdout, "Temp: %s\n", dataPaket.dataGramm25.SensorValue1);
					if (verbose) fprintf(stdout, "Temp: %2.1f\n", stringToDouble(dataPaket.dataGramm25.SensorValue1, 6));
                    //fprintf ( stdout, "Feuc: %s\n", dataPaket.dataGramm25.SensorValue2);
					if (verbose) fprintf(stdout, "Feuc: %2.1f\n", stringToDouble(dataPaket.dataGramm25.SensorValue2, 6));
                    //fprintf ( stdout, "Pwer: %s\n", dataPaket.dataGramm25.SensorValue3);
					if (verbose) fprintf(stdout, "Pwer: %2.1f\n", stringToDouble(dataPaket.dataGramm25.SensorValue3, 6));
					
					char tmp[2000];
					sprintf ( tmp, "rrdtool update %s/sensor-%it.rrd N:%f", path, dataPaket.dataGramm25.SensorNr & 0x0F, stringToDouble(dataPaket.dataGramm25.SensorValue1, 6));
					system ( tmp );
					if (verbose) syslog(LOG_NOTICE, tmp);

					sprintf ( tmp, "rrdtool update %s/sensor-%if.rrd N:%f", path, dataPaket.dataGramm25.SensorNr & 0x0F, stringToDouble(dataPaket.dataGramm25.SensorValue2, 6));
					system ( tmp );
					if (verbose) syslog(LOG_NOTICE, tmp);
                }
                else
                {
					if (verbose) fprintf(stdout, "Send: %i\n", dataPaket.dataGramm24.SensorNr & 0x0F);
					if (verbose)  fprintf(stdout, "Type: %i\n", dataPaket.dataGramm24.SensorType1 & 0x0F);
					if (verbose) fprintf(stdout, "Temp: %2.1f\n", stringToDouble(dataPaket.dataGramm24.SensorValue1, 6));
					
					char tmp[2000];
					sprintf ( tmp, "rrdtool update %s/sensor-%i.rrd N:%f",  path, dataPaket.dataGramm24.SensorNr & 0x0F, stringToDouble(dataPaket.dataGramm24.SensorValue1, 6));
					system ( tmp );

					if ( verbose) syslog(LOG_NOTICE, tmp);
                }

				if (verbose) fprintf(stdout, "---------------------\n");
            }
        }





        rfm_fifo_clear();
        rfm_rxon();
    }
}

static void skeleton_daemon()
{
	pid_t pid;

	/* Fork off the parent process */
	pid = fork();

	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* On success: The child process becomes session leader */
	if (setsid() < 0)
		exit(EXIT_FAILURE);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* Set new file permissions */
	umask(0);

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/tmp");

	/* Close all open file descriptors */
	int x;
	for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
	{
		close(x);
	}

	/* Catch, ignore and handle signals */
	//TODO: Implement a working signal handler */
	//signal(SIGCHLD, intHandler);
	signal(SIGHUP, intHandler);
}

int rfm_wait()
{
	if (verbose) syslog(LOG_NOTICE, "Waiting for data (wiringPi) ...");

    wiringPiISR (RFM12_nIRQ, INT_EDGE_RISING, &readData) ;

    return 1;
}

/*
 *
 * Configure SPI communication
 *
 */
void spiSetup ()
{

	if (verbose) syslog(LOG_NOTICE, "setting up SPI ...");
    if ((myFd = wiringPiSPISetup (SPI_CHAN, 500000)) < 0)
    {
		char msg[1024];
        sprintf (msg, "Can't open the SPI bus: %s", strerror (errno)) ;
		syslog(LOG_NOTICE, msg);
        exit (EXIT_FAILURE) ;
    }
}


/*
 *
 * Configure port settings ...
 *
 */
int init_ports()
{
	if (verbose) syslog(LOG_NOTICE, "setting up ports with wiringPi...");

    pinMode (6, INPUT);
    pinMode (5, INPUT);

	return 1;
}


int main(int argc, char *argv[])
{

	int opt;

	while ((opt = getopt(argc, argv, "fv")) != -1) 
	{
		switch (opt) 
		{
			case 'v':
				verbose = 1;
				break;
			case 'f':
				foreground = 1;
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
				exit(EXIT_FAILURE);
			}
	}

	if (foreground == 0)
	{
		skeleton_daemon();
	}

	/* Open the log file */
	openlog("RFM69-Receiver", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "Process started!");

	if (verbose) syslog(LOG_NOTICE, "Starting Receiver!");

	if (verbose) syslog(LOG_NOTICE, "Setting up wiringPi!\n");
	wiringPiSetup () ;
	spiSetup () ;
	init_ports();

	if (verbose) syslog(LOG_NOTICE, "setting up RFM12 ...\n");
	init_rfm12();


	rfm_wait();

	signal(SIGINT, intHandler);
	signal(SIGKILL, intHandler);

	while (keepRunning)
    {
        delay ( 1000 );
    }



	if (verbose) syslog(LOG_NOTICE, "Terminating on request!\n");

  	return 1 ;
}
