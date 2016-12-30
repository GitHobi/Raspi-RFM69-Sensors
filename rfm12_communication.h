#ifndef RFM12_COMMUNICATION
#define RFM12_COMMUNICATION

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

uint16_t rfm12_send_word(uint16_t command);

int init_rfm12();
void rfm12_select ( int value);
int rfm12_send_cmd(uint8_t value);

uint8_t readTemperature(uint8_t userCal);
void rfm69_writeReg ( uint8_t reg, uint8_t value);
uint8_t rfm69_readReg ( uint8_t reg);

void rfm_fifo_clear(void);
void rfm_rxoff(void);
void rfm_rxon(void);

#endif // RFM12_COMMUNICATION
