/*_____________________________{FILE_NAME}_____________________________________________________
                                      ___           ___           ___
 Author: Abdelrahman Selim           /\  \         /\  \         /\  \
                                    /::\  \       /::\  \       /::\  \
Created on: {DATE}                 /:/\:\  \     /:/\:\  \     /:/\:\  \
                                  /::\ \:\  \   _\:\ \:\  \   /::\ \:\  \
 Version: 01                     /:/\:\ \:\__\ /\ \:\ \:\__\ /:/\:\ \:\__\
                                 \/__\:\/:/  / \:\ \:\ \/__/ \/__\:\/:/  / 
                                      \::/  /   \:\ \:\__\        \::/  /
                                      /:/  /     \:\/:/  /        /:/  /
 Brief : {PROJECT_NAME}              /:/  /       \::/  /        /:/  /
                                     \/__/         \/__/         \/__/
 _________________________________________________________________________________________*/

#include "ProgramDataHandler.h"
// Global variables for the current program's parameters
uint16_t StandbyTemp;
uint16_t HoldTimeStandby;
uint16_t BurningTemp;
uint16_t BurningTime;
uint16_t CoolingTemp;
uint16_t CoolingTime;
uint16_t VaccumStartTemp;
uint16_t VaccumStopTemp;
uint8_t  RateOfHeatRise;
uint8_t  VaccumPercent;

// Function to load a program's parameters from EEPROM into global variables
void EEPROM_loadProgram(uint8_t groupIndex, uint8_t programIndex) {
    
    uint16_t CurrentMemoryAdress = (EEPROM_BASE_ADDR + ( groupIndex * PROGRAM_GROUP_SIZE ) +( programIndex * PROGRAM_DATA_SIZE ));
    
    eeprom_read_uint16_t((CurrentMemoryAdress + STANDBY_TEMP_OFFSET     ),&StandbyTemp      );
    eeprom_read_uint16_t((CurrentMemoryAdress + HOLD_TIME_STANDBY_OFFSET),&HoldTimeStandby  );
    eeprom_read_uint16_t((CurrentMemoryAdress + BURNING_TEMP_OFFSET     ),&BurningTemp      );
    eeprom_read_uint16_t((CurrentMemoryAdress + BURNING_TIME_OFFSET     ),&BurningTime      );
    eeprom_read_uint16_t((CurrentMemoryAdress + COOLING_TEMP_OFFSET     ),&CoolingTemp      );
    eeprom_read_uint16_t((CurrentMemoryAdress + COOLING_TIME_OFFSET     ),&CoolingTime      );
    eeprom_read_uint16_t((CurrentMemoryAdress + VACCUM_START_TEMP_OFFSET),&VaccumStartTemp  );
    eeprom_read_uint16_t((CurrentMemoryAdress + VACCUM_STOP_TEMP_OFFSET ),&VaccumStopTemp   );
    eeprom_readByte     ((CurrentMemoryAdress + RATE_OF_HEAT_RISE_OFFSET),&RateOfHeatRise   );
    eeprom_readByte     ((CurrentMemoryAdress + VACCUM_PERCENT_OFFSET   ),&VaccumPercent    );
}

// Function to save a program's parameters into EEPROM
void EEPROM_saveProgramData(uint8_t groupIndex, uint8_t programIndex, 
                        uint16_t standbyTemp    , uint16_t holdTimeStandby  , uint8_t rateOfHeatRise, 
                        uint16_t burningTemp    , uint16_t burningTime      , uint16_t coolingTemp, 
                        uint16_t coolingTime    , uint8_t vaccumPercent     , uint16_t vaccumStartTemp, 
                        uint16_t vaccumStopTemp) {
    
    
    uint16_t CurrentMemoryAdress = (EEPROM_BASE_ADDR + ( groupIndex * PROGRAM_GROUP_SIZE ) +( programIndex * PROGRAM_DATA_SIZE ));
    eeprom_write_uint16_t((CurrentMemoryAdress + STANDBY_TEMP_OFFSET     ),standbyTemp      );
    eeprom_write_uint16_t((CurrentMemoryAdress + HOLD_TIME_STANDBY_OFFSET),holdTimeStandby  );
    eeprom_write_uint16_t((CurrentMemoryAdress + BURNING_TEMP_OFFSET     ),burningTemp      );
    eeprom_write_uint16_t((CurrentMemoryAdress + BURNING_TIME_OFFSET     ),burningTime      );
    eeprom_write_uint16_t((CurrentMemoryAdress + COOLING_TEMP_OFFSET     ),coolingTemp      );
    eeprom_write_uint16_t((CurrentMemoryAdress + COOLING_TIME_OFFSET     ),coolingTime      );
    eeprom_write_uint16_t((CurrentMemoryAdress + VACCUM_START_TEMP_OFFSET),vaccumStartTemp  );
    eeprom_write_uint16_t((CurrentMemoryAdress + VACCUM_STOP_TEMP_OFFSET ),vaccumStopTemp   );
    eeprom_writeByte     ((CurrentMemoryAdress + RATE_OF_HEAT_RISE_OFFSET),rateOfHeatRise   );
    eeprom_writeByte     ((CurrentMemoryAdress + VACCUM_PERCENT_OFFSET   ),vaccumPercent    );
}
void EEPROM_saveCurrentSettings(uint8_t groupIndex, uint8_t programIndex){
        
    uint16_t CurrentMemoryAdress = (EEPROM_BASE_ADDR + ( groupIndex * PROGRAM_GROUP_SIZE ) +( programIndex * PROGRAM_DATA_SIZE ));
    
    eeprom_write_uint16_t((CurrentMemoryAdress + STANDBY_TEMP_OFFSET     ),StandbyTemp      );
    eeprom_write_uint16_t((CurrentMemoryAdress + HOLD_TIME_STANDBY_OFFSET),HoldTimeStandby  );
    eeprom_write_uint16_t((CurrentMemoryAdress + BURNING_TEMP_OFFSET     ),BurningTemp      );
    eeprom_write_uint16_t((CurrentMemoryAdress + BURNING_TIME_OFFSET     ),BurningTime      );
    eeprom_write_uint16_t((CurrentMemoryAdress + COOLING_TEMP_OFFSET     ),CoolingTemp      );
    eeprom_write_uint16_t((CurrentMemoryAdress + COOLING_TIME_OFFSET     ),CoolingTime      );
    eeprom_write_uint16_t((CurrentMemoryAdress + VACCUM_START_TEMP_OFFSET),VaccumStartTemp  );
    eeprom_write_uint16_t((CurrentMemoryAdress + VACCUM_STOP_TEMP_OFFSET ),VaccumStopTemp   );
    eeprom_writeByte     ((CurrentMemoryAdress + RATE_OF_HEAT_RISE_OFFSET),RateOfHeatRise   );
    eeprom_writeByte     ((CurrentMemoryAdress + VACCUM_PERCENT_OFFSET   ),VaccumPercent    );
}

// Function to read/write program variables (helper)
uint16_t EEPROM_readProgramVariable(uint8_t groupIndex, uint8_t programIndex, uint8_t variableOffset) {
    void *variablePtr = 0x00;
    uint8_t byte_flag = 0x00;
    uint16_t CurrentMemoryAdress = (EEPROM_BASE_ADDR + ( groupIndex * PROGRAM_GROUP_SIZE ) +( programIndex * PROGRAM_DATA_SIZE ));

    // Determine the memory address for the selected variable based on its offset
    switch (variableOffset) {
        case STANDBY_TEMP_OFFSET:
            variablePtr = &StandbyTemp;
            break;
        case HOLD_TIME_STANDBY_OFFSET:
            variablePtr = &HoldTimeStandby;
            break;
        case BURNING_TEMP_OFFSET:
            variablePtr = &BurningTemp;
            break;
        case BURNING_TIME_OFFSET:
            variablePtr = &BurningTime;
            break;
        case COOLING_TEMP_OFFSET:
            variablePtr = &CoolingTemp;
            break;
        case COOLING_TIME_OFFSET:
            variablePtr = &CoolingTime;
            break;
        case VACCUM_START_TEMP_OFFSET:
            variablePtr = &VaccumStartTemp;
            break;
        case VACCUM_STOP_TEMP_OFFSET:
            variablePtr = &VaccumStopTemp;
            break;
        case RATE_OF_HEAT_RISE_OFFSET:
            variablePtr = &RateOfHeatRise;
            byte_flag = 1;
            break;
        case VACCUM_PERCENT_OFFSET:
            variablePtr = &VaccumPercent;
            byte_flag = 1;
            break;
        default:
            // Invalid offset, return failure
            return 0;
    }
    if(byte_flag){
            eeprom_readByte     ((CurrentMemoryAdress + variableOffset   ),variablePtr    );
    } else{
            eeprom_read_uint16_t((CurrentMemoryAdress + variableOffset   ),variablePtr    );
    }
    return 1;
}

void EEPROM_writeProgramVariable(uint8_t groupIndex, uint8_t programIndex, uint8_t variableOffset, uint16_t value) {
    uint16_t CurrentMemoryAdress = (EEPROM_BASE_ADDR + ( groupIndex * PROGRAM_GROUP_SIZE ) +( programIndex * PROGRAM_DATA_SIZE ));
    if((variableOffset == VACCUM_PERCENT_OFFSET)||(variableOffset == RATE_OF_HEAT_RISE_OFFSET)){
            eeprom_writeByte     ((CurrentMemoryAdress + variableOffset   ),value    );
    } else{
            eeprom_write_uint16_t((CurrentMemoryAdress + variableOffset   ),value    );
    } 
}
