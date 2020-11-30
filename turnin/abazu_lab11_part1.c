/*	Author: Andrew Bazua [abazu001]
 *  Partner(s) Name:
 *	Lab Section: 024
 *	Assignment: Lab #11  Exercise #1
 *	Exercise Description: [Design a system where a ‘char’ variable can be
    incremented or decremented based on specific button presses. The value of
    the variable is then transmitted to the shift register and displayed on a
    bank of eight LEDs.]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link:
 *  https://drive.google.com/file/d/1dwrF6QmGY8r61g3MJQG1bEvzlVNF4igf/view?usp=sharing
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "tasks.h"
#include "transmit_shift_data.h"
#include "timer.h"

typedef enum counter_states {init, wait, increment, decrement, reset,
                             buttonPress} counter;

int counterTick(int state);

int main(void) {
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    // Task 1 (counterTick)
    task1.state = init;
    task1.period = 50;
    task1.elapsedTime = task1.period;
    task1.TickFct = &counterTick;

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

int counterTick(int state) {
    static unsigned char shiftOutput;
    unsigned char A0 = ~PINA & 0x01;
    unsigned char A1 = ~PINA & 0x02;

    switch (state) {
        case init: state = wait; shiftOutput = 0xF5; break;

        case wait:
            if (A0 && A1) { state = reset; }
            else if (A0 && !A1) { state = increment; }
            else if (!A0 && A1) { state = decrement; }
            else { state = wait; }
            break;

        case increment:
            if (A0 && A1) { state = reset; }
            else if (A0 && !A1) { state = buttonPress; }
            else if (!A0 && A1) { state = decrement; }
            else { state = wait; }
            break;

        case decrement:
            if (A0 && A1) { state = reset; }
            else if (!A0 && A1) { state = buttonPress; }
            else if (A0 && !A1) { state = increment; }
            else { state = wait; }
            break;

        case reset:
            if (A0 || A1) { state = buttonPress; }
            else { state = wait; }
            break;

        case buttonPress:
            if (!A0 && !A1) { state = wait; }
            else if (A0 && A1) { state = reset; }
            else { state = buttonPress; }
            break;

        default:
            state = init;
            break;
    }

    switch (state) {
        case init:
            // Set default output to 0xFF - 0x0A
            shiftOutput = 0xF5;
            break;

        case wait: break;

        case increment:
            if (shiftOutput < 0xFF) { ++shiftOutput; }
            break;

        case decrement:
            if (shiftOutput > 0x00) { --shiftOutput;}
            break;

        case reset:
            shiftOutput = 0x00;
            break;

        case buttonPress: break;

    }
    // pass counter value to shift register
    transmit_data(shiftOutput);
    return state;
}
