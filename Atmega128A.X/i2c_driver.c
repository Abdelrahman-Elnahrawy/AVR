/*_____________________________{FILE_NAME}_____________________________________________________
                                      ___           ___           ___
 Author: Abdelrahman Selim           /\  \         /\  \         /\  \
                                    /::\  \       /::\  \       /::\  \
Created on: 16/10/2024             /:/\:\  \     /:/\:\  \     /:/\:\  \
                                  /::\ \:\  \   _\:\ \:\  \   /::\ \:\  \
 Version: 01                     /:/\:\ \:\__\ /\ \:\ \:\__\ /:/\:\ \:\__\
                                 \/__\:\/:/  / \:\ \:\ \/__/ \/__\:\/:/  / 
                                      \::/  /   \:\ \:\__\        \::/  /
                                      /:/  /     \:\/:/  /        /:/  /
 Brief : {PROJECT_NAME}              /:/  /       \::/  /        /:/  /
                                     \/__/         \/__/         \/__/
 _________________________________________________________________________________________*/
#include "i2c_driver.h"

// Global Variables
uint8_t i2cErorrFlag;         // Error flag for I2C operations
uint8_t i2cReadDataReadyFlag;
uint8_t i2cReadBusyFlag;
uint8_t i2c_WriteBufferCurrentSize;           // Tracks the number of bytes currently in the write buffer

// Static Variables for Read Buffer Management
static uint8_t i2c_ReadDataLength;                          // Length of data to be read
static uint8_t i2c_ReadBuffer[I2C_READ_BUFFER_SIZE];    // 30-byte circular buffer for I2C read data
static uint8_t i2c_ReadBufferHead = 0;                  // Points to where the next byte will be written in the read buffer
static uint8_t i2c_ReadBufferTail = 0;                  // Points to where the next byte will be read from the read buffer
static uint8_t i2c_ReadBufferCurrentSize = 0;           // Tracks the number of bytes currently in the read buffer

// Static Variables for Write Buffer Management
static uint8_t WriteDataLength;                          // Length of data to be sent 
static uint8_t i2c_WriteBuffer[I2C_WRITE_BUFFER_SIZE];   // 30-byte circular buffer for I2C write data
static uint8_t i2c_WriteBufferHead = 0;                  // Points to where the next byte will be written in the write buffer
static uint8_t i2c_WriteBufferTail = 0;                  // Points to where the next byte will be read from the write buffer

// Static Variables for I2C State Management
static uint8_t RepeatStartFlag;               // Flag indicating if a repeated start is requested
static uint8_t RepeatStartPlace;              // Store the current place for repeated start

static uint8_t currentAddress;                 // Current I2C slave address
static uint8_t CurrentData;                    // Holder for the data to be transmitted currently


/**
 * @brief Adds data to the I2C write buffer.
 * 
 * This function is responsible for adding a given address, length of data, and the actual data 
 * to the I2C write buffer. It ensures that there is enough space in the buffer before adding 
 * the new data. The function maintains the circular nature of the buffer by wrapping around 
 * the buffer indices as needed.
 *
 * @param address  The I2C address to be sent, indicating the target slave device.
 * @param length   The length of the data to be sent (number of bytes).
 * @param data     A pointer to the data array that needs to be added to the write buffer.
 * 
 * @return uint8_t Returns 1 on success if the data was successfully added to the buffer, 
 *                 or 0 if there is not enough space in the buffer to accommodate the new data.
 */
static uint8_t i2c_AddToWriteBuffer(uint8_t address, uint8_t length, uint8_t *data) {
    // Ensure that the total size of the message does not exceed i2c_WriteBuffer space
    if (length > (I2C_WRITE_BUFFER_SIZE - i2c_WriteBufferCurrentSize - 2)) {  
        // Check for available space (2 bytes for address and length)
        return 0;  // Not enough space in i2c_WriteBuffer
    }

    // Add address to i2c_WriteBuffer
    i2c_WriteBuffer[i2c_WriteBufferHead] = address;
    i2c_WriteBufferHead = (i2c_WriteBufferHead + 1) % I2C_WRITE_BUFFER_SIZE;
    i2c_WriteBufferCurrentSize++;

    // Add length to i2c_WriteBuffer
    i2c_WriteBuffer[i2c_WriteBufferHead] = length;
    i2c_WriteBufferHead = (i2c_WriteBufferHead + 1) % I2C_WRITE_BUFFER_SIZE;
    i2c_WriteBufferCurrentSize++;

    // Add data to i2c_WriteBuffer
    for (uint8_t i = 0; i < length; i++) {
        i2c_WriteBuffer[i2c_WriteBufferHead] = data[i];
        i2c_WriteBufferHead = (i2c_WriteBufferHead + 1) % I2C_WRITE_BUFFER_SIZE;
        i2c_WriteBufferCurrentSize++;
    }

    return 1;  // Success
}

/**
 * @brief Retrieves the next address and length from the I2C write buffer.
 * 
 * This function reads the next I2C address and the length of the data to be transmitted 
 * from the I2C write buffer. It ensures that there is sufficient data available in the buffer 
 * before reading the values. The function also updates the tail index and the current size 
 * of the buffer accordingly. 
 * 
 * **Note:** The buffer size is reduced by the amount of data taken, two bytes 
 * one for the address and one for the length n the data byte is freed after 
 * reading from it in the TWI interrupt 
 * 
 * @param address Pointer to a variable where the retrieved I2C address will be stored.
 * @param length  Pointer to a variable where the retrieved length of the data will be stored.
 * 
 * @return uint8_t Returns 1 on success if the address and length were successfully read 
 *                 from the buffer, or 0 if there is not enough data available to read.
 */
static uint8_t i2c_getFromWriteBuffer(uint8_t *address, uint8_t *length) {
    // Ensure i2c_WriteBuffer is not empty
    if (i2c_WriteBufferCurrentSize < 2) {
        return 0;  // Not enough data to read (at least address + length needed)
    }

    // Read address
    *address = i2c_WriteBuffer[i2c_WriteBufferTail];
    i2c_WriteBufferTail = (i2c_WriteBufferTail + 1) % I2C_WRITE_BUFFER_SIZE;
    i2c_WriteBufferCurrentSize--;

    // Read length
    *length = i2c_WriteBuffer[i2c_WriteBufferTail];
    i2c_WriteBufferTail = (i2c_WriteBufferTail + 1) % I2C_WRITE_BUFFER_SIZE;
    i2c_WriteBufferCurrentSize--;

    // Ensure we have enough data for the length field
    if (i2c_WriteBufferCurrentSize < *length) {
        return 0;  // Not enough data in the i2c_WriteBuffer
    }

    return 1;  // Success
}


// TWI interrupt service routine

// TWI Status Register (TWSR) status codes
/**
 * @brief TWI (Two Wire Interface) interrupt service routine.
 * 
 * This ISR handles various TWI events during I2C communication. 
 * It processes the status of the TWI operation by checking the 
 * TWI status register (TWSR) and executing actions based on the 
 * received status code. This includes managing the transmission 
 * and reception of data, handling errors, and controlling the 
 * I2C bus state (start, repeat start, stop conditions).
 * 
 * The behavior of this ISR can be summarized as follows:
 * - Handles start and repeated start conditions by reading the 
 *   next address and data length from the write buffer.
 * - Manages data transmission, including ACK/NACK responses 
 *   for transmitted and received bytes.
 * - Updates the write buffer status (tail and current size) 
 *   after each transmission.
 * - Maintains the read buffer for received data.
 * - Sets error flags for various failure conditions.
 * 
 * **Note:** Proper handling of data lengths and buffer management 
 * is crucial to ensure accurate communication over the I2C bus.
 */
ISR(TWI_vect) {
    switch (TWSR & 0xF8) { // TWI Status Register (TWSR) status codes
        case TWI_START:
            // Handle the start condition
            if (i2c_getFromWriteBuffer(&currentAddress, &WriteDataLength)) {
                TWDR = currentAddress; // Load slave address into data register
                TWCR = (TWCR & ~((1 << TWSTA) | (1 << TWSTO))) | (1 << TWINT); // Clear STA and ensure TWINT is set
            }
            break;

        case TWI_REP_START:
            // Handle the repeated start condition
            if (i2c_getFromWriteBuffer(&currentAddress, &WriteDataLength)) {
                TWDR = currentAddress; // Load slave address into data register
                TWCR = (TWCR & ~((1 << TWSTA) | (1 << TWSTO))) | (1 << TWINT); // Clear STA and ensure TWINT is set
            } else {
                TWCR |= (1 << TWINT) | (1 << TWSTO); // Send stop condition if no data available
            }
            break;

        case TWI_MT_SLA_NACK:
            // Master transmit, slave address not acknowledged
            i2c_WriteBufferTail += WriteDataLength; // Adjust the write buffer tail
            i2c_WriteBufferCurrentSize -= WriteDataLength; // Update the current size
            TWCR |= (1 << TWINT) | (1 << TWSTO); // Stop condition
            i2cErorrFlag = I2C_ERROR_ADRESS_WRITE; // Set error flag
            break;

        case TWI_MT_SLA_ACK:
            // Master transmit, slave address acknowledged
            CurrentData = i2c_WriteBuffer[i2c_WriteBufferTail++]; // Get the next byte to send
            TWDR = CurrentData; // Load the byte into the data register
            i2c_WriteBufferTail %= I2C_WRITE_BUFFER_SIZE; // Wrap around the buffer size
            i2c_WriteBufferCurrentSize--; // Decrease current size of the buffer
            WriteDataLength--; // Decrease data length to send
            TWCR |= (1 << TWINT); // Start transmission
            break;

        case TWI_MT_DATA_NACK:
            // Data not acknowledged by the slave
            i2c_WriteBufferTail += WriteDataLength; // Adjust the write buffer tail
            i2c_WriteBufferCurrentSize -= WriteDataLength; // Update the current size
            TWCR |= (1 << TWINT) | (1 << TWSTO); // Stop condition
            i2cErorrFlag = I2C_ERROR_DATA_WRITE; // Set error flag
            break;

        case TWI_MT_DATA_ACK:
            // Data acknowledged by the slave
            if (WriteDataLength > 0) {
                TWDR = i2c_WriteBuffer[i2c_WriteBufferTail++]; // Get the next byte to send
                i2c_WriteBufferTail %= I2C_WRITE_BUFFER_SIZE; // Wrap around the buffer size
                i2c_WriteBufferCurrentSize--; // Decrease current size of the buffer
                WriteDataLength--; // Decrease data length to send
                TWCR |= (1 << TWINT); // Start transmission
            } else {
                // Check if repeat start condition is needed
                if (RepeatStartFlag && (RepeatStartPlace == i2c_WriteBufferTail)) {
                    TWCR |= (1 << TWINT) | (1 << TWSTA); // Initiate repeat start
                    RepeatStartFlag = 0; // Reset repeat start flag
                } else {
                    TWCR |= (1 << TWINT) | (1 << TWSTO); // Send stop condition
                }
            }
            break;

        case TWI_MR_SLA_ACK: // SLA+R transmitted, ACK received
            // Check how many bytes to read
            if (i2c_ReadDataLength > 1) {
                // Expecting more than one byte, send ACK after receiving each byte
                TWCR |= (1 << TWINT) | (1 << TWEA); // Enable ACK
            } else {
                // Only one byte to read, do not send ACK after receiving it
                TWCR |= (1 << TWINT); // Do not send ACK after the last byte
            }
            break;

        case TWI_MR_SLA_NACK: // SLA+R transmitted, NACK received
            TWCR |= (1 << TWINT) | (1 << TWSTO); // Stop condition
            i2cErorrFlag = I2C_ERROR_ADRESS_READ; // Set error flag
            break;

        case TWI_MR_DATA_ACK: // Data byte received, ACK returned
            // Store received data in the read buffer
            i2c_ReadBuffer[i2c_ReadBufferHead] = TWDR; // Save the received byte
            i2c_ReadBufferHead = (i2c_ReadBufferHead + 1) % I2C_READ_BUFFER_SIZE; // Move head index
            i2c_ReadBufferCurrentSize++; // Increase current size
            i2c_ReadDataLength--; // Decrease remaining data length

            // Check if more bytes are expected
            if (i2c_ReadDataLength > 1) {
                TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWEA); // Send ACK to receive the next byte
            } else {
                // Prepare to receive the last byte without sending ACK
                TWCR = (TWCR & ~(1 << TWEA)) | (1 << TWINT) | (1 << TWEN);
            }
            break;

        case TWI_MR_DATA_NACK: // Data byte received, NACK returned
            // Store received data in the read buffer
            i2c_ReadBuffer[i2c_ReadBufferHead] = TWDR; // Save the received byte
            i2c_ReadBufferHead = (i2c_ReadBufferHead + 1) % I2C_READ_BUFFER_SIZE; // Move head index
            i2c_ReadBufferCurrentSize++; // Increase current size
            i2c_ReadDataLength--; // Decrease remaining data length
            TWCR |= (1 << TWINT) | (1 << TWSTO); // Stop condition
            break;

        default:
            // Handle other statuses as needed
            //TWCR |= (1 << TWINT) | (1 << TWSTO); // Send stop condition on error
            break;
    }
}

/**
 * @brief Sends a single byte of data to a specified I2C address.
 * 
 * This function prepares the data to be sent over the I2C bus by 
 * adding the data to the write buffer. It shifts the I2C address 
 * left by one bit to account for the read/write bit in the I2C 
 * protocol. The function assumes that the write operation is 
 * requested.
 * 
 * @param adr The I2C address of the target device (7-bit address).
 * @param data The byte of data to send to the specified address.
 * @return uint8_t Returns the result of the buffer addition, typically 
 * indicating success or failure.
 */
uint8_t i2c_SendByte(uint8_t adr, uint8_t data) {
    // Shift the address left by one bit to include the write bit
    // and add the data to the write buffer
    uint8_t result = i2c_AddToWriteBuffer((adr << 1), 1, &data);
    return result;
}


/**
 * @brief Initializes the I2C (TWI) interface with the specified SCL frequency.
 * 
 * This function configures the I2C interface by setting the bit rate register
 * (TWBR) to achieve the desired SCL frequency based on the provided input.
 * It allows the user to specify the desired I2C SCL frequency, making the function 
 * immune to changes in CPU frequency. The TWI and its interrupt are enabled, 
 * along with global interrupts.
 * 
 * @param frequency The desired I2C SCL frequency (in Hertz). 
 *                  Supported values are typically 100 kHz and 400 kHz.
 */
void i2c_Init(uint32_t frequency) {
    // Calculate TWBR value based on the desired frequency
    // TWBR = (F_CPU / (2 * SCL_FREQUENCY)) - 16
    uint8_t twbrValue;

    if (frequency == 100000) {
        twbrValue = (uint8_t)((F_CPU / (2 * frequency)) - 16); // Calculate TWBR for 100 kHz
    } else if (frequency == 400000) {
        twbrValue = (uint8_t)((F_CPU / (2 * frequency)) - 16); // Calculate TWBR for 400 kHz
    } else {
        // If an unsupported frequency is provided, default to 100 kHz
        twbrValue = (uint8_t)((F_CPU / (2 * 100000)) - 16); // Default to 100 kHz
    }

    // Set bit rate register for the desired frequency
    TWBR = twbrValue;

    // Set prescaler to 1 (no scaling)
    TWSR &= ~(1 << TWPS1) & ~(1 << TWPS0);

    // Enable TWI and TWI interrupt
    TWCR = (1 << TWEN) | (1 << TWIE); // Enable TWI and TWI Interrupt

    // Enable global interrupts
    sei(); 
}


/**
 * @brief Sends an array of bytes to the specified I2C address.
 * 
 * This function takes an I2C slave address, the length of the data to be sent,
 * and a pointer to the data array. It adds the data to the I2C write buffer
 * for transmission. The address is shifted left by one to comply with I2C protocol.
 * 
 * @param adr The I2C slave address (7-bit).
 * @param length The number of bytes to send from the data array.
 * @param data A pointer to the array of data to be sent.
 * 
 * @return 1 if the data was successfully added to the write buffer, 0 otherwise.
 */
uint8_t i2c_SendArray(uint8_t adr, uint8_t length, uint8_t* data) {
    // Add the address and data length to the I2C write buffer
    uint8_t result = i2c_AddToWriteBuffer((adr << 1), length, data);
    return result;
}


/**
 * @brief Sends an array of bytes to the specified I2C address with a repeat start condition.
 * 
 * This function sets a flag to indicate that a repeat start condition will be used for 
 * the next I2C transmission. It calculates the position in the write buffer where the 
 * data will be written, accounting for the address and data length. 
 * 
 * The address is shifted left by one to comply with the I2C protocol. 
 * 
 * @param adr The I2C slave address (7-bit).
 * @param length The number of bytes to send from the data array.
 * @param data A pointer to the array of data to be sent.
 * 
 * @return 1 if the data was successfully added to the write buffer, 0 otherwise.
 */
uint8_t i2c_SendArraySr(uint8_t adr, uint8_t length, uint8_t* data) {
    // Set the repeat start flag for the next transmission
    RepeatStartFlag = 1;
    
    // Calculate the position in the write buffer for the repeat start condition
    RepeatStartPlace = i2c_WriteBufferHead + (length + 2); // Include address and length
    uint8_t result = i2c_AddToWriteBuffer((adr << 1), length, data);
    // Add the address and data length to the I2C write buffer
    return result;
}


/**
 * @brief Initiates a start condition for I2C communication if data is available.
 * 
 * This function checks if there are any bytes in the write buffer. 
 * If the write buffer contains data, it sets the start condition 
 * in the TWI Control Register (TWCR) to begin the I2C communication.
 * 
 * This function should be called periodically to ensure that data is
 *  transmitted when the write buffer is not empty.
 */
void i2c_Update() {
    if (i2c_WriteBufferCurrentSize) {    // Check if there is data in the write buffer
    // Set the start condition for I2C communication
    TWCR |= (1 << TWSTA);
    }
    if(i2c_ReadBufferCurrentSize){
        i2cReadDataReadyFlag = 1;
    } else {
        i2cReadDataReadyFlag = 0;
    }
}


/**
 * @brief Initiates an I2C read operation from the specified device address.
 * 
 * This function prepares to read a specified number of bytes from an I2C device.
 * It ensures that no conflicting read operations are in progress by checking if 
 * the read buffer is empty and no previous read command is pending. If conditions 
 * are met, it sets the length of data to be read, prepares the slave address 
 * for a read operation, and adds it to the I2C write buffer. The read command 
 * is later handled during the I2C communication sequence.
 * 
 * @param adr    I2C address of the device (7-bit address, without the R/W bit).
 * @param length Number of bytes to be read from the device.
 * 
 * @return uint8_t
 *         Returns 1 if the read operation is successfully initiated.
 *         Returns 0 if a read operation is already pending or the buffer is not ready.
 */

uint8_t i2c_GetData(uint8_t adr, uint8_t length) {
    // Ensure the read buffer is empty and no read operation is already pending
    if(i2c_ReadBufferCurrentSize == 0 && i2c_ReadDataLength == 0) { 
        // Set the length of the data to be read
        i2c_ReadDataLength = length;
        // Prepare the address with the read bit (LSB = 1) and add to write buffer
        // with zero length and zero data
        uint8_t result = i2c_AddToWriteBuffer(((adr << 1) + 1), 0, 0);
        if(result){i2cReadBusyFlag = 1;} // if the write in buffer process succeded
        return result;
    } else {
        // Return 0 if a read operation is already in progress or buffer is not ready
        return 0;
    }
}


/**
 * @brief Reads data from the I2C read buffer.
 * 
 * This function retrieves a specified number of bytes from the 
 * I2C read buffer and returns them in the provided data array.
 * It updates the read buffer's tail pointer to the next position 
 * after reading the requested bytes. 
 * 
 * @param data   Pointer to the array where the read data will be stored.
 * @param length Number of bytes to read from the buffer.
 * @return       Returns the number of bytes successfully read; 
 *               returns 0 if the requested length exceeds the 
 *               current available data in the read buffer.
 */
uint8_t i2c_ReadFromRxBuffer(uint8_t* data, uint8_t length) {
    // Ensure there is enough data in the read buffer
    if (length > i2c_ReadBufferCurrentSize) {
        return 0; // Not enough data to read
    }

    // Read the requested number of bytes from the read buffer
    for (uint8_t i = 0; i < length; i++) {
        data[i] = i2c_ReadBuffer[i2c_ReadBufferTail]; // Retrieve byte
        i2c_ReadBufferTail = (i2c_ReadBufferTail + 1) % I2C_READ_BUFFER_SIZE; // Update tail pointer
        i2c_ReadBufferCurrentSize--; // Decrease the size of the read buffer
    }
    i2cReadBusyFlag = 0;
    return length; // Return the number of bytes read
}
