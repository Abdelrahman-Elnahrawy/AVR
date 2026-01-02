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
 Brief : {PROJECT_NAME}               /:/  /       \::/  /        /:/  /
                                     \/__/         \/__/         \/__/
 _________________________________________________________________________________________*/
#include "EEPROM_24C32.h"

// Global variables

// Local Variables
static uint8_t eepromReadWaitingFlag;
 void* eppromCallbackDataPointer;
static uint8_t eepromDataReadyFlag;

static uint16_t CurrentAdr;
static uint8_t  CurrentLength;
static void*    CurrentDataPtr;

// Arrays to hold the read request details
static uint16_t eepromReadAddressQueue[EEPROM_READ_QUEUE_SIZE];     // Array to hold EEPROM addresses
static void*    eepromReadDataPtrQueue[EEPROM_READ_QUEUE_SIZE];     // Array to hold pointers to data buffers
static uint8_t  eepromReadLengthQueue[EEPROM_READ_QUEUE_SIZE];      // Array to hold the lengths of data to be read

// Pointers to the head and tail of the FIFO queue
static uint8_t queueHead = 0;
static uint8_t queueTail = 0;
static uint8_t eepromReadQueueSize;
static uint8_t eepromQueueADD(uint16_t adr , uint8_t length,void* DataPtr){
    if (eepromReadQueueSize<EEPROM_READ_QUEUE_SIZE){
        eepromReadAddressQueue[queueHead] =adr;
        eepromReadDataPtrQueue[queueHead] =DataPtr;
        eepromReadLengthQueue [queueHead]  =length;
        queueHead=(queueHead+1)%EEPROM_READ_QUEUE_SIZE;
        eepromReadQueueSize++;
        return 1;
    } else{
    return 0;
    }
}
static uint8_t eepromQueueGet(){
    if(eepromReadQueueSize>0){
        CurrentAdr     = eepromReadAddressQueue[queueTail];
        CurrentDataPtr = eepromReadDataPtrQueue[queueTail];
        CurrentLength  = eepromReadLengthQueue[queueTail];
        return 1;
    } else {
            return 0;
        }
}


uint8_t eeprom_readByte(uint16_t addr ,uint8_t* CallBackData ) {
    return eepromQueueADD(addr ,1,CallBackData);
}
uint8_t eeprom_readArray(uint16_t addr, uint8_t length, uint8_t * CallBackData) {
    return eepromQueueADD(addr  ,length,CallBackData);
}
/**
 * @brief Writes a single byte to the EEPROM at the specified address.
 * 
 * This function writes one byte of data to a specific memory address in the 
 * EEPROM (24C32). It splits the 16-bit memory address into two bytes (high 
 * and low byte) to be compatible with the I2C protocol and sends the address 
 * followed by the data to be written.
 * 
 * @param addr  The 16-bit address in the EEPROM where the data will be written.
 * @param data  The byte of data to write to the specified address.
 * 
 * @return uint8_t
 *         Returns the result of the I2C operation (1 for success, 0 for failure).
 */
uint8_t eeprom_write_uint16_t(uint16_t addr, uint16_t data) {
    uint8_t result;
    uint8_t addr_high = (addr >> 8);  // Extract high byte of the 16-bit address
    uint8_t addr_low  = addr;         // Extract low byte of the 16-bit address

    // Send the high byte, low byte, and data to the EEPROM using I2C
    result = i2c_SendArray(EEPROM_24C32_ADDR, 4, (uint8_t[]){addr_high, addr_low,(uint8_t) (data),(uint8_t) (data >> 8)});
    
    return result; // Return the status of the write operation
}

/**
 * @brief Writes a single byte to the EEPROM at the specified address.
 * 
 * This function writes one byte of data to a specific memory address in the 
 * EEPROM (24C32). It splits the 16-bit memory address into two bytes (high 
 * and low byte) to be compatible with the I2C protocol and sends the address 
 * followed by the data to be written.
 * 
 * @param addr  The 16-bit address in the EEPROM where the data will be written.
 * @param data  The byte of data to write to the specified address.
 * 
 * @return uint8_t
 *         Returns the result of the I2C operation (1 for success, 0 for failure).
 */
uint8_t eeprom_writeByte(uint16_t addr, uint8_t data) {
    uint8_t result;
    uint8_t addr_high = (addr >> 8);  // Extract high byte of the 16-bit address
    uint8_t addr_low  = addr;         // Extract low byte of the 16-bit address

    // Send the high byte, low byte, and data to the EEPROM using I2C
    result = i2c_SendArray(EEPROM_24C32_ADDR, 3, (uint8_t[]){addr_high, addr_low, data});
    
    return result; // Return the status of the write operation
}




/**
 * @brief Reads two bytes from the EEPROM at the specified address.
 * 
 * This function reads two bytes of data from a specific memory address in the
 * EEPROM (24C32). It first sends the 16-bit address to the EEPROM in a write 
 * operation (without stopping the I2C connection), then requests to read the 
 * byte stored at that address. When the data is ready eepromDataReadyFlag
 * will read as one , the eepromDataReadyFlag needs to be cleared manually
 * 
 * @param addr  The 16-bit address in the EEPROM from which the data will be read.
 * 
 * @return uint8_t
 *         Returns 1 if the read operation was successful and data is in queue,
 *         or 0 if the read operation failed.
 */
uint8_t eeprom_read_uint16_t(uint16_t addr ,uint16_t* CallBackData ) { 
    return eepromQueueADD(addr ,2,CallBackData);
}

/**
 * @brief Writes an array of bytes to the EEPROM starting from the specified address.
 * 
 * This function writes a sequence of bytes to the EEPROM (24C32) starting at a given 
 * 16-bit address. The function first sends the 16-bit address (high and low bytes), 
 * followed by the array of data bytes to be written into the EEPROM.
 * 
 * @param addr   The 16-bit starting address in the EEPROM where data will be written.
 * @param data   Pointer to the array of bytes to be written into the EEPROM.
 * @param length The number of bytes to be written from the array.
 * 
 * @return uint8_t
 *         Returns 1 if the write operation was successful, or 0 if the operation failed.
 */
uint8_t eeprom_writeArray(uint16_t addr, uint8_t length, uint8_t *data) {
    uint8_t result;
    uint8_t addr_high = (addr >> 8); // Extract high byte of the 16-bit address
    uint8_t addr_low = addr;         // Extract low byte of the 16-bit address
    
    // Create a buffer to hold the address and data together
    uint8_t buffer[length + 2];
    
    // Add the high and low address bytes to the buffer
    buffer[0] = addr_high;
    buffer[1] = addr_low;
    
    // Copy the data to be written into the buffer, starting after the address
    for (uint8_t i = 0; i < length; i++) {
        buffer[i + 2] = data[i];
    }
    
    // Send the buffer (address + data) to the EEPROM
    result = i2c_SendArray(EEPROM_24C32_ADDR, length + 2, buffer);
    
    return result; // Return status of the operation (1 for success, 0 for failure)
}




/**
 * @brief Periodically checks if the EEPROM data is ready to be read.
 * 
 * This function is meant to be called in a periodic task or main loop. It checks the 
 * I2C read buffer and sets a flag when data from the EEPROM is ready to be fetched.
 * The flag indicates that the EEPROM data has been successfully read and can now be
 * processed or stored in the appropriate variable.
 * 
 * @return uint8_t
 *         Returns 1 if data is ready to be fetched, 0 if data is not ready yet.
 */
void eeprom_Update() {

    // Check if data in the I2C read buffer is ready to be processed
    if(eepromReadWaitingFlag){
        if (i2cReadDataReadyFlag ) {
            // Data is ready, set the flag (you can define a flag variable in your program)
            eepromDataReadyFlag = 1;
            eepromReadWaitingFlag = 0;
            i2c_ReadFromRxBuffer(CurrentDataPtr, CurrentLength);
            eepromDataReadyFlag = 0;
        }
    } else{
        // make sure there is enough size for the request in the write buffer , also if the i2c has another read request then wait 
        // and for the first condition is that , make sure that there is a request for data
        if(eepromReadQueueSize>0 && i2cReadBusyFlag == 0 &&((i2c_WriteBufferCurrentSize + 6) < I2C_WRITE_BUFFER_SIZE) ){  

        eepromQueueGet();
        i2c_SendArray(EEPROM_24C32_ADDR, 2, (uint8_t[]){(CurrentAdr >> 8), (uint8_t) CurrentAdr});
        if(i2c_GetData(EEPROM_24C32_ADDR, CurrentLength)){
            queueTail=(queueTail+1)%EEPROM_READ_QUEUE_SIZE;
            eepromReadQueueSize--;
            eepromReadWaitingFlag = 1;
        } 
        }
    }
 i2c_Update();
}


void eeprom_init(uint32_t frequency){
    i2c_Init( frequency);
}