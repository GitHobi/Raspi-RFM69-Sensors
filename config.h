#ifndef CONFIG
#define CONFIG


#define	TRUE	(1==1)
#define	FALSE	(!TRUE)

#define	SPI_CHAN		1
#define SPI_SPEED		100
#define	NUM_TIMES		100
#define	MAX_SIZE		(1024*1024)

#define RFM12_CE        11
#define RFM12_nIRQ      6
#define RFM12_SDI       12
#define RFM12_SDO       13
#define RFM12_SCK       14


/* Carrierfrequenz in Hz */
#define FREQUENCY 433920000LL

/* Bitrate in bps (1200 ... 300000) and shall it be calculated as for RFM12
 * in order to get equal bitrate for RFM12 and RFM69 if used together? */
#define BITRATE				19200L
#define RFM12COMP			1

// Don't change anything from here
#define XTALFREQ			32000000UL

#define FRF					((FREQUENCY*524288LL + (XTALFREQ/2)) / XTALFREQ)
#define FRF_MSB				((FRF>>16) & 0xFF)
#define FRF_MID				((FRF>>8) & 0xFF)
#define FRF_LSB				((FRF) & 0xFF)

#if RFM12COMP
#define BR_VAL				(((((( 344828L - (BITRATE<2694L)*301725L ) + (BITRATE/2))/BITRATE) - 1) | ( (BITRATE<2694) * 0x80 )))
#if BR_VAL<128
#define BR                  ((10000000L + (29*BR_VAL/2))/(29*(BR_VAL+1)))
#else
#define BR_VAL_S			((BR_VAL-127)*8)
#define BR					((10000000L + (29*BR_VAL_S/2))/(29*(BR_VAL_S)))
#endif
#else
#define BR                  BITRATE
#endif

// Value for input timeout
#ifndef TIMEOUTVAL
#define TIMEOUTVAL			(100000)
#endif

#define DATARATE			((XTALFREQ + (BR/2)) / BR)
#define DATARATE_MSB		(DATARATE>>8)
#define DATARATE_LSB		(DATARATE & 0xFF)

#define MAX_ARRAYSIZE 27

#define RegOpMode 0x01
#define RegDataModul 0x02
#define RegBitrateMsb 0x03
#define RegBitrateLsb 0x04
#define REG_FRFMSB 0x07

#define RegIrqFlags1 0x27
#define RegIrqFlags2 0x28

#define RegSyncConfig 0x2e
#define RegSyncValue1 0x2f
#define RegPacketConfig1 0x37
#define RegPayloadLength 0x38

#define wiringPi

#endif // CONFIG

