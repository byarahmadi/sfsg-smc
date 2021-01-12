/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */
/* This program generates reset signal in periodically
 * The pins on the main board and reset generator must be set before execution
 */

#include <msp430.h>

void main(void) {
  WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer
  PM5CTL0 &= ~LOCKLPM5;      // Disable the GPIO power-on default high-impedance
                         // mode to activate previously configured port settings
  int i = 0;
  P1DIR |= BIT0;
  P1OUT |= BIT0;
  P1DIR &= ~BIT3;
  P1REN &= ~BIT3;

  P4DIR &= ~BIT3;
  P4REN &= ~BIT3;
  unsigned int x = 0;
  TA0CCR0 = INTERVAL;
  unsigned int mode = MODE;
  unsigned int itr = ITR;

  while (!(P1IN & BIT3))
    ;
  P3DIR |= BIT4;
  P3OUT |= BIT4;  // by using &= ~BIT4, The first failure happens just after
                  // getting the signal
  TA0CTL = 0x0110;

  for (;;) {
    if (TA0CTL & 0x0001) {
      P1OUT ^= 0x01;
      i++;
      P3OUT ^= BIT4;
      TA0CTL = 0x0110;
    }

    if (P4IN & BIT3) {
      TA0CTL = 0;
      P3DIR &= ~BIT4;
      break;
    }
  }
}
