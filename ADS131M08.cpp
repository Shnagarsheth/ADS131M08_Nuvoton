/**
 * @file ADS131M08.cpp
 * @brief  Adapted for Nuvoton Microcontrollers from --> https://github.com/moothyknight/ADS131M08_Arduino
 *        Original library link --> https://github.com/icl-rocketry/ADS131M04-Lib (LGPL)
 */

#include <stdint.h>
#include <stdbool.h>
#include "stdio.h"
#include "M451Series.h"
#include "math.h"
#include <ADS131M08.h>

/*------------------------------------*/
/*-------Private Functions------------*/
/*------------------------------------*/

/**
 * @brief Creates and sends a Communication Frame to get the values from ADS131M08
 * @param outPtr -- pass the pointer to the output variable
 * @param command -- pass the command word 
 */
void ADS131M08::spiCommFrame(uint32_t * outPtr, uint16_t command){
    uint8_t i=0;
	SPI_SET_SS_LOW(SPI0);

    // Send the command in the first word
    *outPtr = spiTransferWord(command);

    // For the next 8 words, just read the data
    for (i=1; i < 9; i++) {
        outPtr++;
        *outPtr = spiTransferWord(0x0000) >> 8;
    }

    // Save CRC bits
    outPtr++;
    *outPtr = spiTransferWord(0x0000);
    SPI_SET_SS_HIGH(SPI0);
}

/**
 * @brief spiTransferWord
 * 
 * @param inputData 
 * @return uint32_t 
 */
uint32_t ADS131M08::spiTransferWord(uint16_t inputData){
    /* Transfer a 24 bit word
        Data returned is MSB aligned*/ 

    uint32_t receivedData=0;
	
    SPI_WRITE_TX(SPI0, inputData >> 8);
    while(SPI_IS_BUSY(SPI0));
    while(!SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0)){
        receivedData = SPI_READ_RX(SPI0);
    }
    
    receivedData <<=8;
    SPI_WRITE_TX(SPI0,((inputData<<8) >> 8));
    while(SPI_IS_BUSY(SPI0));
    while(!SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0)){
        receivedData |= SPI_READ_RX(SPI0);
    }
    
    receivedData <<=8;
    SPI_WRITE_TX(SPI0, 0x00);
    while(SPI_IS_BUSY(SPI0));
    
    while(!SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0)){
        receivedData |= SPI_READ_RX(SPI0);
    }

    return receivedData << 8;
}

/**
 * @brief reading channels 
 * @param channelArrPtr 
 * @param channelArrLen 
 * @param outputArrPtr 
 */
void ADS131M08::readChannels(int8_t * channelArrPtr, int8_t channelArrLen, int32_t * outputArrPtr){
    
    uint32_t rawDataArr[10]; uint8_t i=0;

    // Get data by simply sending 0x0000 on SPI transfer
    spiCommFrame(&rawDataArr[0],0x0000);
    
    // Save the decoded data for each of the channels
    for (i = 0; i<channelArrLen; i++) {
        *outputArrPtr = twoCompDeco(rawDataArr[*channelArrPtr+1]);
        outputArrPtr++;
        channelArrPtr++;
    }
}


/**
 * @brief get two's complemnt of the data 
 * @param data 
 * @return int32_t 
 */

int32_t ADS131M08::twoCompDeco2(uint32_t data) {
    // Take the two's complement of the data
	
    int32_t dataInt=0;
    data <<= 8;
    dataInt = (int)data;

    return dataInt/pow(2,8);
}


/*------------------------------------*/
/*-------Public Functions------------*/
/*------------------------------------*/

ADS131M08::ADS131M08(int clkIN)
{
    //PWM COnfiguration - Nuvoton Specific
    PWM_ConfigOutputChannel(PWM1, 0, clkIN, 50);

    /* Enable PWM Output path for PWM1 channel 0 */
    PWM_EnableOutput(PWM1, PWM_CH_0_MASK);

    /* Enable Timer for PWM1 channel 0 */
    PWM_Start(PWM1, PWM_CH_0_MASK);

    //Setting IO mode for 3 pins DRDY, SyncReset and ChipSelect - Nuvoton Specific
    GPIO_SetMode(PB, BIT4, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PD, BIT3, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PD, BIT2, GPIO_MODE_QUASI);
}

/** Initialization
 * @brief initializies the external ADC, configures PWM generation for providing clock pulses to the ADS131M08, 
 * sets 3 GPIOs for ADS DataReady Pin (DRDY), Chip Select (CS) and SyncReset 
 * @param clkIN 
 */

void ADS131M08::ads131m08_init(uint8_t DRDY, uint8_t SYNCRST, uint8_t CHIPSELECT){

    //Initializing the ADS131M08
	DRDY = 1;   //DRDY - pin
	delay(10);
    SYNCRST = 0;  //SYNCRESET - pin
	CHIPSELECT = 0; //CS - pin
	delay(10);
    CHIPSELECT = 1;
	SYNCRST = 1;
    while(DRDY == 0);
}

/** PGA
 * @brief   Full-Scale Range [Programmable Gain Amplifier (PGA)]
            GAIN SETTING FSR
            1    ±1.2 V
            2    ±600 mV
            4    ±300 mV
            8    ±150 mV
            16   ±75 mV
            32   ±37.5 mV
            64   ±18.75 mV
            128  ±9.375 mV
 * @param gainRegNumber -- there are 2 registers 
 * @param gain -- ranges (1 to 128) -- (1, 2, 4, 8, 16, 32, 64, 128)
 * @return true if gain set successfully
 * @return false if gain set unsuccessfully
 */
bool ADS131M08::setGain(uint8_t gainRegNumber, uint8_t gain){
    uint16_t writegain = 0;
    if(gain == 1){
        writegain =     0x0000;//    0b0000000000000000;
    }  
    else if (gain == 2){
        writegain = 0x1111;//0b0001000100010001;
    }
    else if (gain == 4) {
        writegain = 0x2222;//0b0010001000100010;
    }
    else if (gain == 8) { 
        writegain = 0x3333;//0b0011001100110011;
    }
    else if (gain == 16) { 
        writegain = 0x4444;//;0b0100010001000100;
    }
    else if (gain == 32) {
        writegain = 0x5555;//0b0101010101010101;
    }
    else if (gain == 64) {
        writegain = 0x6666;//0b0110011001100110;
    }
    else if (gain == 128) {
        writegain = 0x7777;//0b0111011101110111;
    }
    else {
        return false;
    }
    
    switch (gainRegNumber){
    case 1:
        writeReg(ADS131_GAIN1, writegain); 
        return true;   
        break;
    case 2:
        writeReg(ADS131_GAIN2, writegain);
        return true;    
        break;     
    default:
        return false;
        break;
    }

}

/* Registers
ADDRESS	   REGISTER     RESET VALUE	

DEVICE SETTINGS AND INDICATORS (Read-Only Registers)
00h         ID	         28xxh
01h	      STATUS	     0500h		

GLOBAL SETTINGS ACROSS CHANNELS
02h	      MODE	         0510h
03h	      CLOCK          FF0Eh
04h	      GAIN1          0000h
05h       GAIN2          0000h
06h       CFG            0600h
07h       THRSHLD_MSB    0000h
08h	      THRSHLD_LSB	 0000h

CHANNEL-SPECIFIC SETTINGS
09h	      CH0_CFG	     0000h
0Ah	      CH0_OCAL_MSB	 0000h
0Bh	      CH0_OCAL_LSB	 0000h
0Ch       CH0_GCAL_MSB	 8000h
0Dh	      CH0_GCAL_LSB   0000h

0Eh	      CH1_CFG        0000h
0Fh	      CH1_OCAL_MSB   0000h
10h       CH1_OCAL_LSB   0000h
11h       CH1_GCAL_MSB   8000h
12h       CH1_GCAL_LSB   0000h
.
.
.
Similarly fot CH2 to CH7
.
.
.
3Eh	      REGMAP_CRC	 0000h
3Fh	      RESERVED	     0000h
*/
/**
 * @brief Adresses of register ranges from 00h to 3Fh
 * @param reg 
 * @param data 
 * @return true 
 * @return false 
 */
bool ADS131M08::writeReg(uint8_t reg, uint16_t data){
    /* Writes the content of data to the register reg
        Returns 1 if successful
    */    
    uint8_t commandPref = 0x06, i=0;    uint32_t respArr[10];

    // Make command word using syntax found in data sheet
    uint16_t commandWord = (commandPref<<12) + (reg<<7);

    SPI_SET_SS_LOW(SPI0);
    spiTransferWord(commandWord);    
    spiTransferWord2(data);

    // Send 4 empty words 
    for (i=0; i<4; i++) {
        spiTransferWord(0x0000);
    }
    SPI_SET_SS_HIGH(SPI0);
    // Get response
    spiCommFrame(&respArr[0],0x0000);
    if ( ( (0x04<<12) + (reg<<7) ) == respArr[0]) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Provides 16 bit unsigned int - value of the particular register
 * 
 * @param reg 
 * @return uint16_t 
 */
uint16_t ADS131M08::readReg(uint8_t reg) {
    /* Reads the content of single register found at address reg
        Returns register value
    */
    uint8_t commandPref = 0x0A;

    // Make command word using syntax found in data sheet
    uint16_t commandWord = (commandPref << 12) + (reg << 7);

   // uint32_t resArr[10];
    // Use first frame to send command
    spiCommFrame(&ressArr[0], commandWord);

    // Read response
    spiCommFrame(&ressArr[0], 0x0000);

    return ressArr[0] >> 16;
}

/**
 * @brief Provides 32-bit signed readings of a particular channel (0 to 7)
 * @param channelNumber (0 to 7)
 * @return int32_t  Raw value from a single channelinput from 0-7
 */
int32_t ADS131M08::readSingleChannel(uint8_t channelNumber) {
   
    int32_t outputArr[1];
    int8_t *channelArr =&channel;

    readChannels(channelArr, 1, &outputArr[0]);

    return outputArr[0];
}

/**
 * @brief to be called for reading all channel readings
 * 
 * @param inputArr - provide array to store values of all channels
 */
void ADS131M08::readAllChannels(int32_t inputArr[8]) {
    uint32_t rawDataArr[10];
    int8_t channelArrPtr = 0;
    int8_t channelArrLen = 8;
	uint8_t i=0;

    // Get data
    spiCommFrame(&rawDataArr[0], 0x0000);
    // Save the decoded data for each of the channels
    for (i = 0; i<8; i++) {
        inputArr[i] = twoCompDeco(rawDataArr[channelArrPtr+1]);
        channelArrPtr++;
    }
}

/**
 * @brief Get the All Channel Readings and specific samples.  
 * @param sampleCount - Specify how many samples you want at an interval of 1ms (1KSPS- default OSR) 
 * @return int32_t - return a 2D array with with each row denoting channel no. and each column is the sampel number. 
 */
int32_t getSpecificSamples(uint8_t sampleCount){
    uint8_t channelNum=0; int32_t allChRawData[8][sampleCount];
    //int32_t allChProsData[8][sampleCount];
    
	for(channelNum=0;channelNum<8;channelNum++){  //Rows - Channel Number and Columns -- Samples from ADC Channels
		
		allChRawData[channelNum][sampleCount]=readSingleChannel(channelNum);
        
        // to be uncommented for Calibration of Full-Scale +/- 1.2V 
		//allChProsData[channelNum][sampleCount]=(((float)(allChRawData[channelNum][sampleCount])*1.2)/8388608);
	}
	return allChRawData;
}

/**
 * @brief loop to generate 1 milisecond delay with Cclk = 72MHz
 * @param milliseconds 
 */
void delay(uint16_t milliseconds)
{
      uint16_t x,i;
      for(i=0;i<milliseconds;i++)
      {
          for(x=0; x<7200; x++);    
      }
}
