
/*
 */

#ifndef EEPROM_24C32_H
#define EEPROM_24C32_H

#include "I2C_Driver.h"

// Define the I2C address for 24C32 (A2, A1, A0 = 0)
#define EEPROM_24C32_ADDR 0x50  // 7-bit address (0x50 << 1) for write, (0x51 << 1) for read
#define EEPROM_READ_QUEUE_SIZE 30
// Function prototypes
void    eeprom_init(uint32_t frequency);
uint8_t eeprom_writeByte(uint16_t addr, uint8_t data);                              // Write a byte to the EEPROM
uint8_t eeprom_readByte(uint16_t addr, uint8_t* CallBackData );                     // Read a byte from the EEPROM
uint8_t eeprom_writeArray(uint16_t addr, uint8_t length, uint8_t *data);            // Write an array of bytes to the EEPROM
uint8_t eeprom_readArray(uint16_t addr, uint8_t length,uint8_t* CallBackData  );    // Read an array of bytes from the EEPROM
uint8_t eeprom_read_uint16_t(uint16_t addr ,uint16_t* CallBackData ) ;
uint8_t eeprom_write_uint16_t(uint16_t addr, uint16_t data);
void eeprom_Update() ;                                                              //called periodically to Update i2c and update if eeprom data is ready !

#endif // EEPROM_24C32_H
/* for the read request it returns 1 on success on putting a request
 * after the request is done 
 *
 */