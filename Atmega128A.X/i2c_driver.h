#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <avr/io.h>
#include <avr/interrupt.h>

// Define CPU frequency
#define F_CPU 8000000UL  // Define CPU frequency as 8 MHz

// Macros for setting I2C speed (Standard or Fast mode)
#define I2C_STANDARD_MODE 100000UL  // Standard I2C speed of 100 kHz
#define I2C_FAST_MODE 400000UL      // Fast I2C speed of 400 kHz

// Buffer sizes for I2C communication
#define I2C_WRITE_BUFFER_SIZE 100    // Size of the write buffer
#define I2C_READ_BUFFER_SIZE  100     // Size of the read buffer

// TWI Status Register (TWSR) status codes
#define TWI_START             0x08    // Start condition transmitted
#define TWI_REP_START         0x10    // Repeated start condition transmitted
#define TWI_MT_SLA_ACK        0x18    // SLA+W transmitted, ACK received
#define TWI_MT_SLA_NACK       0x20    // SLA+W transmitted, NACK received
#define TWI_MT_DATA_ACK       0x28    // Data byte transmitted, ACK received
#define TWI_MT_DATA_NACK      0x30    // Data byte transmitted, NACK received
#define TWI_MR_SLA_ACK        0x40    // SLA+R transmitted, ACK received
#define TWI_MR_SLA_NACK       0x48    // SLA+R transmitted, NACK received
#define TWI_MR_DATA_ACK       0x50    // Data byte received, ACK returned
#define TWI_MR_DATA_NACK      0x58    // Data byte received, NACK returned

// I2C error codes
#define I2C_ERROR_ADRESS_WRITE  0x01 // Address write error
#define I2C_ERROR_DATA_WRITE    0x02 // Data write error
#define I2C_ERROR_ADRESS_READ   0x03 // Address read error

// External variable to indicate I2C errors
extern uint8_t i2cErorrFlag;             
extern uint8_t i2cReadDataReadyFlag;
extern uint8_t i2cReadBusyFlag;
extern uint8_t i2c_WriteBufferCurrentSize;           // Tracks the number of bytes currently in the write buffer

// Function prototypes
void       i2c_Update();                                                // Update the I2C state
void       i2c_Init(uint32_t frequency);                                // Initialize the I2C interface with a specified frequency
uint8_t    i2c_SendByte(uint8_t adr, uint8_t data);                     // Send a single byte to an I2C device
uint8_t    i2c_SendArray(uint8_t adr, uint8_t length, uint8_t* data);   // Send an array of bytes to an I2C device
uint8_t    i2c_GetData(uint8_t adr, uint8_t length);                    // Prepare to read data from an I2C device
uint8_t    i2c_SendArraySr(uint8_t adr, uint8_t length, uint8_t* data); // Send an array with a repeated start condition
uint8_t    i2c_ReadFromRxBuffer(uint8_t* data, uint8_t length);         // Read data from the RX buffer

#endif // I2C_DRIVER_H
