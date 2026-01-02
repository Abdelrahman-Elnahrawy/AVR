/*_____________________________{PROGRAM_DATA_HANDLER_H}_____________________________________________________
                                      ___           ___           ___
 Author: Abdelrahman Selim           /\  \         /\  \         /\  \
                                    /::\  \       /::\  \       /::\  \
Created on: {DATE}                 /:/\:\  \     /:/\:\  \     /:/\:\  \
                                  /::\ \:\  \   _\:\ \:\  \   /::\ \:\  \
 Version: 01                     /:/\:\ \:\__\ /\ \:\ \:\__\ /:/\:\ \:\__\
                                 \/__\:\/:/  / \:\ \:\ \/__/ \/__\:\/:/  / 
                                      \::/  /   \:\ \:\__\        \::/  /
                                      /:/  /     \:\/:/  /        /:/  /
 Brief : EEPROM Memory Management     /:/  /       \::/  /        /:/  /
                                     \/__/         \/__/         \/__/
 _________________________________________________________________________________________*/

#ifndef PROGRAM_DATA_HANDLER_H
#define PROGRAM_DATA_HANDLER_H

#include <stdint.h>
#include "EEPROM_24C32.h"
// Global variables for the current program's parameters
extern uint16_t StandbyTemp;
extern uint16_t HoldTimeStandby;
extern uint16_t BurningTemp;
extern uint16_t BurningTime;
extern uint16_t CoolingTemp;
extern uint16_t CoolingTime;
extern uint16_t VaccumStartTemp;
extern uint16_t VaccumStopTemp;
extern uint8_t  RateOfHeatRise;
extern uint8_t  VaccumPercent;



// Variable Offsets within each program
#define STANDBY_TEMP_OFFSET       0
#define HOLD_TIME_STANDBY_OFFSET  2
#define BURNING_TEMP_OFFSET       4
#define BURNING_TIME_OFFSET       6
#define COOLING_TEMP_OFFSET       8
#define COOLING_TIME_OFFSET       10
#define VACCUM_START_TEMP_OFFSET  12    
#define VACCUM_STOP_TEMP_OFFSET   14
#define RATE_OF_HEAT_RISE_OFFSET  16
#define VACCUM_PERCENT_OFFSET     17
// two bytes remain
// Function prototypes
#define EEPROM_BASE_ADDR    0x0000  // the base adress 0x0000 for the default settings   5A
#define EEPROM_MAX_ADDR     0x0FFF  // the 24c32 is 32 kilo byte
#define PROGRAM_GROUP_SIZE  200     //each group consists of 10 Programs 
#define PROGRAM_DATA_SIZE   20      //each program is 20 bytes
#define DEFULT_PROGRAM      0x0000  // the memory adress of the default settings
/* group range from   0 to 9
 * Program range from 0 to 9
 */
// Base address of the EEPROM
// Load a program's parameters from EEPROM into global variables
void EEPROM_loadProgram(uint8_t groupIndex, uint8_t programIndex);

// Save a program's parameters into EEPROM
void EEPROM_saveProgramData(uint8_t groupIndex, uint8_t programIndex, 
                        uint16_t standbyTemp, uint16_t holdTimeStandby, uint8_t rateOfHeatRise, 
                        uint16_t burningTemp, uint16_t burningTime, uint16_t coolingTemp, 
                        uint16_t coolingTime, uint8_t vaccumPercent, uint16_t vaccumStartTemp, 
                        uint16_t vaccumStopTemp);

// Helper functions to read/write program variables
uint16_t EEPROM_readProgramVariable(uint8_t groupIndex, uint8_t programIndex, uint8_t variableOffset);
void EEPROM_writeProgramVariable(uint8_t groupIndex, uint8_t programIndex, uint8_t variableOffset, uint16_t value);
void EEPROM_saveCurrentSettings(uint8_t groupIndex, uint8_t programIndex);
#endif // PROGRAM_DATA_HANDLER_H
