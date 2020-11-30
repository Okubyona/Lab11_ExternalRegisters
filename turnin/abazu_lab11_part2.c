/*	Author: Andrew Bazua [abazu001]
 *  Partner(s) Name:
 *	Lab Section: 024
 *	Assignment: Lab #11  Exercise #2
 *	Exercise Description: [Design a system where one of three festive light
    displays is displayed on the shift register’s LED bank. The choice of
    festive light displays is determined by button presses.]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link:
 *  https://drive.google.com/file/d/1xhtyeiNha00Q6zb63rLT-y-lN-Ikw0W0/view?usp=sharing
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "tasks.h"
#include "transmit_shift_data.h"
#include "timer.h"

typedef enum lightControl_states {init_lc, wait_lc, increment, decrement, onOff,
                                  buttonPress} lightControl;

typedef enum festiveLights1_states {wait_1, lightShow_1} Lights1;
typedef enum festiveLights2_states {wait_2, lightShow_2} Lights2;
typedef enum festiveLights3_states {wait_3, lightShow_3} Lights3;
typedef enum output_states {output} Output;


int lightControlTick (int state);
int festiveLights1 (int state);
int festiveLights2 (int state);
int festiveLights3 (int state);
int outputTick (int state);

int main(void) {
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;

    static task task1, task2, task3, task4, task5;
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    // Task 1 (lightControlTick)
    task1.state = init_lc;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &lightControlTick;

    task2.state = wait_1;
    task2.period = 200;
    task2.elapsedTime = task2.period;
    task2.TickFct = &festiveLights1;

    task3.state = wait_2;
    task3.period = 400;
    task3.elapsedTime = task3.period;
    task3.TickFct = &festiveLights2;

    task4.state = wait_3;
    task4.period = 200;
    task4.elapsedTime = task4.period;
    task4.TickFct = &festiveLights3;

    task5.state = output;
    task5.period = 100;
    task5.elapsedTime = task5.period;
    task5.TickFct = &outputTick;

    unsigned long GCD = tasks[0]->period;
    for (unsigned char i = 0; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();

    while (1) {
        for (unsigned char i = 0; i < numTasks; i++) {
            if (tasks[i]->elapsedTime == tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }

        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}


// Shared variable to distinguish which light pattern is on

unsigned char lightsOnDisplay;
unsigned char shiftOutput;

// ----------

int lightControlTick(int state) {
    unsigned char A0 = ~PINA & 0x01;
    unsigned char A1 = ~PINA & 0x02;

    switch (state) {
        case init_lc: state = wait_lc; lightsOnDisplay = 0x01;  break;

        case wait_lc:
            if (A0 && A1) { state = onOff; }
            else if (A0 && !A1) { state = increment; }
            else if (!A0 && A1) { state = decrement; }
            else { state = wait_lc; }
            break;

        case increment:
            if (A0 && A1) { state = onOff; }
            else if (A0 && !A1) { state = buttonPress; }
            else if (!A0 && A1) { state = decrement; }
            else { state = wait_lc; }
            break;

        case decrement:
            if (A0 && A1) { state = onOff; }
            else if (!A0 && A1) { state = buttonPress; }
            else if (A0 && !A1) { state = increment; }
            else { state = wait_lc; }
            break;

        case onOff:
            if (A0 || A1) { state = buttonPress; }
            else { state = wait_lc; }
            break;

        case buttonPress:
            if (!A0 && !A1) { state = wait_lc; }
            else { state = buttonPress; }
            break;

        default:
            state = init_lc;
            break;
    }

    switch (state) {
        case init_lc:
            // Set default light pattern to the first of three
            lightsOnDisplay = 0x01;
            break;

        case wait_lc: break;

        case increment:
            // Maximum of 3 light patterns
            if (lightsOnDisplay < 3 && lightsOnDisplay != 0) { ++lightsOnDisplay; }
            break;

        case decrement:
            if (lightsOnDisplay > 1) { --lightsOnDisplay;}
            break;

        case onOff:
            // If lights are on, turn them off
            if (lightsOnDisplay >= 1) { lightsOnDisplay = 0x00; shiftOutput = 0x00; }
            else { lightsOnDisplay = 0x01; }
            break;

        case buttonPress: break;

    }

    return state;
}

int festiveLights1 (int state) {
    // This festive light display has the following pattern:
            /* 10000001
               01000010
               00100100
               00011000
               00100100
               01000010
               10000001
            */
    static unsigned char shifter;

    switch (state) {
        case wait_1:
            if (lightsOnDisplay == 1) { state = lightShow_1; }
            else { state = wait_1; }
            break;

        case lightShow_1:
            if (lightsOnDisplay == 1) { state = lightShow_1; }
            else { state = wait_1; }
            break;
    }

    switch (state) {
        case wait_1:
            // Initial light s
            shifter = 0;
            break;

        case lightShow_1:
            shiftOutput = ((0x01 << shifter ) | (0x80 >> shifter));

            if (shifter < 7) { ++shifter; }
            else { shifter = 0;}
            break;
    }

    return state;
}

int festiveLights2 (int state) {
    static unsigned char flip;

    switch (state) {
        case wait_2:
            if (lightsOnDisplay == 2) { state = lightShow_2; }
            else { state = wait_2; }
            break;

        case lightShow_2:
            if (lightsOnDisplay == 2) { state = lightShow_2; }
            else { state = wait_2; }
            break;
    }

    switch (state) {
        case wait_2:
            // Initial light s
            flip = 0;
            break;

        case lightShow_2:
            if (flip) {
                shiftOutput = 0xAA;
                flip = 0x00;
            }
            else {
                shiftOutput = 0x55;
                flip = 0x01;
            }
            break;
    }

    return state;
}

int festiveLights3 (int state) {
    static int shifter;
    switch (state) {
        case wait_3:
            if (lightsOnDisplay == 3) { state = lightShow_3; }
            else { state = wait_3; }
            break;

        case lightShow_3:
            if (lightsOnDisplay == 3) { state = lightShow_3; }
            else { state = wait_3; }
            break;
    }

    switch (state) {
        case wait_3:
            // Initial light s
            shifter = 0;
            break;

        case lightShow_3:
            if (shifter == 0) {
                shiftOutput = 0x00;
            }
            if (shifter < 8) {
                shiftOutput = (shiftOutput | 0x01 << shifter); }
            else if (shifter < 16 ) { shiftOutput >>= 1; }
            ++shifter;

            if (shifter >= 16) { shifter = 0;}
            break;
    }
    return state;
}

int outputTick (int state) {
    switch (state) {
        case output:
            transmit_data(shiftOutput);
            break;

    }

    return state;
}
