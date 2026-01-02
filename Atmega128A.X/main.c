#define F_CPU 8000000UL
#define _XTAL_FREQ 8000000  // 8 MHz clock frequency

#include <avr/io.h>
#include <util/delay.h>
#include "i2c_driver.h"
#include "EEPROM_24C32.h"
#include "ProgramDataHandler.h"
#include "rtc_ds1307.h"
#define SUCCESS 1
#define ERROR 0

uint16_t read16 = 0xAB;
uint8_t readbyte = 0xAB;
uint8_t arr[] = {0xAB , 0xCD , 0xEF , 0x12 , 0x34};
uint8_t read_arr[sizeof(arr)] ;

    uint8_t init_data[7] = {DS1307_REGISTER_SECONDS_DEFAULT, DS1307_REGISTER_MINUTES_DEFAULT, DS1307_REGISTER_HOURS_DEFAULT, 
                            DS1307_REGISTER_DAY_OF_WEEK_DEFAULT, DS1307_REGISTER_DATE_DEFAULT, 
                            DS1307_REGISTER_MONTH_DEFAULT, DS1307_REGISTER_YEAR_DEFAULT};
    
       uint8_t time_data[3]; // 0 = seconds, 1 = minutes, 2 = hours
void init_portb() {
    // Set PORTB as output
    DDRB = 0xFF;
    PORTB = 0x00; // Initialize PORTB to low
}

int main() {

    _delay_ms(1000);
    // Initialize I2C and EEPROM
    i2c_Init(I2C_STANDARD_MODE);
    eeprom_init(I2C_STANDARD_MODE);
    init_portb();
    // Set the DS1307 to run and reset state
    DS1307_init(init_data, CLOCK_RUN, NO_FORCE_RESET);
    DS1307_read(TIME, time_data);
    while(1){
    DS1307_update();
    eeprom_Update();
    _delay_ms(10);}
    return 0;
}
