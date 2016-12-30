#include "rfm12_communication.h"

#include "config.h"

#ifdef wiringPi
#include <wiringPi.h>
#include <wiringPiSPI.h>
#else
#include <bcm2835.h>
#endif

#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>





// Turn Receiver on and off
void rfm_rxon(void) {
	uint32_t utimer;
	utimer = TIMEOUTVAL;
	rfm69_writeReg(0x01, 0x10); 									// RX on (set to receiver mode in RegOpMode)
	while (!(rfm69_readReg(0x27) & (1 << 7)) && --utimer)
		; 													// Wait for Mode-Ready-Flag
	utimer = TIMEOUTVAL;
	while (!(rfm69_readReg(0x27) & (1 << 6)) && --utimer)
		; 													// Wait for RX-Ready-Flag
}

void rfm_rxoff(void) {
	uint32_t utimer;
	utimer = TIMEOUTVAL;
	rfm69_writeReg(0x01, 0x04); 									// RX off (set to standby mode in RegOpMode)
	while (!(rfm69_readReg(0x27) & (1 << 7)) && --utimer)
		;													// Wait for Mode-Ready-Flag
}


static inline void rfm_setbit(uint32_t bitrate)
{
	uint8_t bw;
	uint16_t freqdev;

	switch (bitrate) {
    case 9600:
		bw = 1;
		freqdev = 1967;
		break;
    case 19200:
		bw = 1;
		freqdev = 1967;
		break;
	case 38400:
	case 57600:
		bw = 1;
		freqdev = 1475;
		break;
	case 115200:
		bw = 8;
		freqdev = 1966;
		break;
	default:
		bw = 2;
		freqdev = 737;
		break;
	}
	//Frequency Deviation
	rfm69_writeReg(0x05, (freqdev >> 8));
	rfm69_writeReg(0x06, (freqdev & 0xFF));

	//Data Rate
	rfm69_writeReg(0x03, DATARATE_MSB);
	rfm69_writeReg(0x04, DATARATE_LSB);

	//Receiver Bandwidth
	rfm69_writeReg(0x19, 80 | bw);
}

void rfm_fifo_clear(void)
{
	rfm69_writeReg(0x28, 0x10);
}

uint16_t rfm_status(void)
{
	uint16_t status = 0;
	status |= rfm69_readReg(0x27);
	status <<= 8;
	status |= rfm69_readReg(0x28);
	return status;
}

/*
 *
 * Init RFM12 chip.
 *
 */
int init_rfm12()
{
    //uint8_t  t = readTemperature ( -7);
    //printf ( "temp: %i\n", t);

	//uint16_t v = rfm12_send_word ( 0x8000 | 0x4E08 );


    //uint16_t b = rfm12_send_word ( 0x4F00 );

    uint8_t r = rfm69_readReg ( 0x58 );
    if ( r != 0x1b)
    {
        //fprintf (stderr, "RFM69 did not respond with expected value %02X\n", r) ;
        //exit (-1) ;
    }

    /*
    uint8_t w;

    rfm69_writeReg ( RegOpMode, 0 ); // Receiver aus
    w = rfm69_readReg(RegOpMode);
    printf ( "Radiostatus: %02X\n", w);


    //Bitrate 0x0D 0x05 ==> 9600
    //Bitrate 0x06 0x83 ==> 19200


    ///rfm69_writeReg( RegBitrateMsb, DATARATE_MSB);
    //rfm69_writeReg( RegBitrateLsb, DATARATE_LSB);

    rfm_setbit ( DATARATE);


    rfm69_writeReg( RegSyncConfig, 0b10011000); // SyncOn, SyncSize 2 byte
    //rfm69_writeReg( RegSyncConfig, 0b01000000); // no sync

    rfm69_writeReg( RegSyncValue1+0, 0x2D); // Syncword 1 ... 2D
    rfm69_writeReg( RegSyncValue1+1, 0xD4); // Syncword 2 ... D4

    rfm69_writeReg( RegPacketConfig1, 0b10000000); // variable length, no crc



    w = rfm69_readReg(RegSyncValue1+0);
    printf ( "SyncValue: %02X\n",w);
    w = rfm69_readReg(RegSyncValue1+1);
    printf ( "SyncValue: %02X\n",w);


    //uint32_t freq = 433*1000*1000LL;
    //uint32_t frf = ((freq << 2) / (32000000L >> 11)) << 6;
    //rfm69_writeReg(REG_FRFMSB, frf >> 16);
    //rfm69_writeReg(REG_FRFMSB+1, frf >> 8);
    //rfm69_writeReg(REG_FRFMSB+2, frf);
    rfm69_writeReg(REG_FRFMSB, FRF_MSB);
    rfm69_writeReg(REG_FRFMSB+1, FRF_MID);
    rfm69_writeReg(REG_FRFMSB+2, FRF_LSB);





    w = rfm69_readReg(0x07);
    printf ( "RF: %02X\n",w);
    w = rfm69_readReg(0x08);
    printf ( "RF: %02X\n",w);
    w = rfm69_readReg(0x09);
    printf ( "RF: %02X\n",w);


    w = rfm69_readReg(RegIrqFlags1);
    printf ( "RegIrqFlags1: %02X\n",w);
    w = rfm69_readReg(RegIrqFlags2);
    printf ( "RegIrqFlags2: %02X\n",w);

    rfm69_writeReg(RegIrqFlags2, 1<<4);


    // and go!
    //rfm69_writeReg ( RegOpMode, 0x10 ); // Receiver Mode (Rx)

*/

    rfm69_writeReg ( RegOpMode, 0 ); // Receiver aus

    rfm69_writeReg(0x02, 0x00); // FSK, Packet mode, No shaping

		//Bitrate + corresponding settings (Receiver bandwidth, frequency deviation)
		rfm_setbit(BITRATE);

		rfm69_writeReg(0x13, 0x1B); 					// OCP enabled, 100mA

		// DIO-Mapping
		rfm69_writeReg(0x25, 0x00 | (1<<6)); 			// Clkout, FifoFull, FifoNotEmpty, FifoLevel, PayloadReady
		rfm69_writeReg(0x26, 0x07); 					// Clock-Out off

		// Carrier frequency
		rfm69_writeReg(0x07, FRF_MSB);
		rfm69_writeReg(0x08, FRF_MID);
		rfm69_writeReg(0x09, FRF_LSB);

		// Packet config
		//rfm69_writeReg(0x37, 0x80); 					// Variable length, No DC-free encoding/decoding, No CRC-Check, No Address filter
		rfm69_writeReg(0x37, 0x00); 					// Fixed length, No DC-free encoding/decoding, No CRC-Check, No Address filter
		rfm69_writeReg(0x38, MAX_ARRAYSIZE); 	        // Max. Payload-Length
		rfm69_writeReg(0x3C, 0x80); 					// Tx-Start-Condition: FIFO not empty
		rfm69_writeReg(0x3D, 0x12); 					// Packet-Config2

		// Präambel length 3 bytes
		rfm69_writeReg(0x2C, 0x00);
		rfm69_writeReg(0x2D, 0x03);

		// Sync-Mode
		rfm69_writeReg(0x2E, 0x88); 					// set FIFO mode
		rfm69_writeReg(0x2F, 0x2D); 					// sync word MSB
		rfm69_writeReg(0x30, 0xD4); 					// sync word LSB

		// Receiver config
		rfm69_writeReg(0x18, 0x00); 					// LNA: 50 Ohm Input Impedance, Automatic Gain Control
		rfm69_writeReg(0x58, 0x2D); 					// High sensitivity mode
		rfm69_writeReg(0x6F, 0x30); 					// Improved DAGC
		rfm69_writeReg(0x29, 0xDC); 					// RSSI mind. -110 dBm
		rfm69_writeReg(0x1E, 0x2D); 					// Start AFC, Auto-On


		rfm69_writeReg(0x0A, 0x80); 						// Start RC-Oscillator
		uint32_t utimer = TIMEOUTVAL;
		while (!(rfm69_readReg(0x0A) & (1 << 6)) && --utimer)
		;



	rfm_fifo_clear();
    rfm_rxon();
    //printf ( "Receiver status: %04X\n", rfm_status());


/*
    uint8_t w = rfm69_readReg(0x07);
    printf ( "RF: %02X\n",w);
    w = rfm69_readReg(0x08);
    printf ( "RF: %02X\n",w);
    w = rfm69_readReg(0x09);
    printf ( "RF: %02X\n",w);


    w = rfm69_readReg(0x03);
    printf ( "DR: %02X\n",w);
    w = rfm69_readReg(0x04);
    printf ( "DR: %02X\n",w);

    w = rfm69_readReg(0x05);
    printf ( "DE: %02X\n",w);
    w = rfm69_readReg(0x06);
    printf ( "DE: %02X\n",w);
*/















/*
	rfm12_send_word(0xC080); // CLK-Frequenz / Batterie-Level
	rfm12_send_word(0x80D7); // FIFO ein, 433MHZ-Band, C = 12pF
	rfm12_send_word(0xC2AB); // Clock Recovery Auto, Filter digital, DQD-Thresold 3
	//rfm12_send_word(0xCA81); // FIFO-Level 8 Bit, Sync Pattern ein, High senity Reset aus
	rfm12_send_word(0xE000); // WakeUp aus
	rfm12_send_word(0xC800); // Low Duty Cycle aus
	rfm12_send_word(0xC4F3); // AFC immer, +7,5 / -10kHz, Add Freq-Offset zur PLL, Berechne Freq. Offset aus AFC                      // Status read
	rfm12_send_word(0xA620); // Frequenz 433,92MHz  // Status read
	rfm12_send_word(0x948C); // VDI Output, VDI Fast, Bandbreite 200kHz, LBA-Gain -6dB, RSSI-Thresold -79dB
	//rfm12_send_word(0xC610); // Baudrate 19200
	rfm12_send_word(0xC623);  // Baudrate 9600
	rfm12_send_word(0x8281); // Empfänger ein, Clock Out aus
	rfm12_send_word(0xCA80); // set FIFO mode
	rfm12_send_word(0xCAF3); // enable FIFO
*/



    return 1;
}

/*
 *
 ******************************************************************************
 *
*/
void rfm12_select ( int value)
{
    return;
/*
    if ( value == 0 )
    {
        digitalWrite (RFM12_CE, LOW) ;
    }
    else
    {
        digitalWrite (RFM12_CE, HIGH) ;
    }
return;
*/
}

/*
 *
 ******************************************************************************
 *
*/

#define REG_TEMP1           0x4E
#define REG_TEMP2           0x4F

#define COURSE_TEMP_COEF    -89 // starter callibration figure
#define RF_TEMP1_MEAS_START 0x08
#define RF_TEMP1_MEAS_RUNNING   0x04



void rfm69_writeReg ( uint8_t reg, uint8_t value)
{

    uint16_t command = (reg << 8) | value;
    command = command | 0x8000;

    //printf ( "W: $%02X=$%02X\n", reg, value);

    uint16_t data = (command >> 8) | ((command & 0xff) << 8);
    rfm12_select ( 0 );
    wiringPiSPIDataRW (SPI_CHAN, (unsigned char*)&data, 2);
    rfm12_select ( 1 );
}

uint8_t rfm69_readReg ( uint8_t reg)
{
    uint16_t command = (reg << 8);

    uint16_t data = (command >> 8) | ((command & 0xff) << 8);
    rfm12_select ( 0 );
    wiringPiSPIDataRW (SPI_CHAN, (unsigned char*)&data, 2);
    rfm12_select ( 1 );

    return data >> 8;
}



uint8_t readTemperature(uint8_t userCal)
{

  rfm69_writeReg(REG_TEMP1, RF_TEMP1_MEAS_START);
  while ((rfm69_readReg(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING));
  return ~rfm69_readReg(REG_TEMP2) + COURSE_TEMP_COEF + userCal;
}

uint16_t rfm12_send_word(uint16_t command)
{

    uint16_t data;

    rfm12_select ( 0 );


    data = (command >> 8) | ((command & 0xff) << 8);
    wiringPiSPIDataRW (SPI_CHAN, (unsigned char*)&data, 2);
    rfm12_select ( 1 );

    printf("Sent to SPI: 0x%04X. Read back from SPI: 0x%04X\n", command, data);

    return data >> 8;




}

