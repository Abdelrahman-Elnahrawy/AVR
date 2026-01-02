#include "rtc_ds1307.h"
#include"i2c_driver.h"

// Local Variables
static uint8_t DS1307ReadWaitingFlag;
static uint8_t DS1307DataReadyFlag;

static uint8_t  CurrentReadReg;
static uint8_t  CurrentLength;
static void*    CurrentDataPtr;

// Arrays to hold the read request details
static uint16_t DS1307ReadRegisterQueue[DS1307_READ_QUEUE_SIZE];     // Array to hold DS1307 addresses
static void*    DS1307ReadDataPtrQueue[DS1307_READ_QUEUE_SIZE];     // Array to hold pointers to data buffers
static uint8_t  DS1307ReadLengthQueue [DS1307_READ_QUEUE_SIZE];      // Array to hold the lengths of data to be read

// Pointers to the head and tail of the FIFO queue
static uint8_t queueHead = 0;
static uint8_t queueTail = 0;
static uint8_t DS1307ReadQueueSize;
static uint8_t DS1307QueueADD(uint16_t adr , uint8_t length,void* DataPtr){
    if (DS1307ReadQueueSize<DS1307_READ_QUEUE_SIZE){
        DS1307ReadRegisterQueue[queueHead]  =adr;
        DS1307ReadDataPtrQueue[queueHead]  =DataPtr;
        DS1307ReadLengthQueue [queueHead]  =length;
        queueHead=(queueHead+1)%DS1307_READ_QUEUE_SIZE;
        DS1307ReadQueueSize++;
        return 1;
    } else{
    return 0;
    }
}
static uint8_t DS1307QueueGet(){
    if(DS1307ReadQueueSize>0){
        CurrentReadReg  = DS1307ReadRegisterQueue[queueTail];
        CurrentDataPtr  = DS1307ReadDataPtrQueue [queueTail];
        CurrentLength   = DS1307ReadLengthQueue  [queueTail];
        return 1;
    } else {
            return 0;
        }
}
/*function to transmit one byte of data to register_address on DS1307*/
void time_i2c_write_single(uint8_t device_address, uint8_t register_address, uint8_t *data_byte)
{
   i2c_SendArray(device_address, 2, (uint8_t[]) {register_address , *data_byte});   
}

/*function to transmit an array of data to device_address, starting from start_register_address*/
void time_i2c_write_multi(uint8_t device_address, uint8_t start_register_address, uint8_t *data_array, uint8_t data_length)
{
    uint8_t DataArr[data_length + 1];
    DataArr[0] = start_register_address;
    for(uint8_t i = 0 ; i < data_length ; i ++){
        DataArr[i+1] = data_array[i];
    }
    i2c_SendArray(device_address, (data_length+1) ,DataArr);
}

/*function to read one byte of data from register_address on DS1307*/
void time_i2c_read_single(uint8_t device_address, uint8_t register_address, uint8_t *data_byte)
{
DS1307QueueADD(register_address ,1,data_byte);
}

/*function to read an array of data from device_address*/
void time_i2c_read_multi(uint8_t device_address, uint8_t start_register_address, uint8_t *data_array, uint8_t data_length)
{
    DS1307QueueADD(start_register_address ,data_length,data_array);
}

/*function to initialize I2C peripheral in 100khz*/
void DS1307_update()
{
        // Check if data in the I2C read buffer is ready to be processed
    if(DS1307ReadWaitingFlag){
        if (i2cReadDataReadyFlag ) {
            // Data is ready, set the flag (you can define a flag variable in your program)
            DS1307DataReadyFlag = 1;
            DS1307ReadWaitingFlag = 0;
            i2c_ReadFromRxBuffer(CurrentDataPtr, CurrentLength);
            DS1307DataReadyFlag = 0;
        }
    } else{
        // make sure there is enough size for the request in the write buffer , also if the i2c has another read request then wait 
        // and for the first condition is that , make sure that there is a request for data
        if(DS1307ReadQueueSize>0 && i2cReadBusyFlag == 0 &&((i2c_WriteBufferCurrentSize + 6) < I2C_WRITE_BUFFER_SIZE) ){  

        DS1307QueueGet();
        i2c_SendArray(DS1307_I2C_ADDRESS, 1, &CurrentReadReg);
        if(i2c_GetData(DS1307_I2C_ADDRESS, CurrentLength)){
            queueTail=(queueTail+1)%DS1307_READ_QUEUE_SIZE;
            DS1307ReadQueueSize--;
            DS1307ReadWaitingFlag = 1;
        } 
        }
    }
    
}
