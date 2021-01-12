/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */

/* This program estimates the execution time
 */
#include <msp430.h>

#include <stdio.h>

void main(void) {
  WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer
  PM5CTL0 &=
      ~LOCKLPM5;  // Disable the GPIO power-on default high-impedance mode
  TA0CCR0 = 65535;
  unsigned int value = 0;

  P3DIR &= ~(BIT0 + BIT4);
  P3OUT &= ~(BIT0 + BIT4);
  P3REN |= BIT0 + BIT4;

  P4DIR = BIT6;
  P4OUT = BIT6;

  unsigned int *locToSave = 0xE760;
  if (*(locToSave) != 0xFFFF)
    while (1)
      ;

  while (!(P3IN & BIT0))
    ;

  TA0CTL = 0x0110;
  unsigned int interval = 0;

  while (1) {
    if (P3IN & BIT4) {
      value = *((unsigned int *)TA0R);
      TA0CTL = 0;  // Stop counting
      //   value = *((unsigned int *)TA0R);
      break;
    }

    if (TA0CTL & 0x0001) {
      TA0CTL = TA0CTL & 0xFFFE;
      interval++;
    }
  }

  if (*(locToSave) == 0xFFFF) {
    *locToSave = interval;
    locToSave = 0xE762;
    *locToSave = value;
  }

  P1DIR |= BIT0;
  P1OUT |= BIT0;

  while (1)
    ;
}
