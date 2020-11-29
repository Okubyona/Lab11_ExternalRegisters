#ifndef __transmit_shift_data_h__
#define __transmit_shift_data_h__

// must have <avr/io.h> included

void transmit_data(unsigned char data) {
    for (int i = 0; i < 8; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTC = 0x08;
        // set SER = next bit of data to be sent.
        PORTC |= ((data >> i) & 0x01);
        // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
        PORTC |= 0x02;
    }
    // set RCLK = 1. Rising edge copies data from "Shift" register to "Storage" register
    PORTC |= 0x04;
    // clears all lines in preparation of new transmission
    PORTC = 0x00;
}

#endif
