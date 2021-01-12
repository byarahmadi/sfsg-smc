/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */

#include <msp430.h>
void sense(unsigned int time) {
  TA0CCR0 = time; // 10000;
  TA0CTL = 0x0110;
  for (;;) {
    if (TA0CTL & 0x0001) {
      TA0CTL = 0;
      break;
    }
  }
}

void led(unsigned int time) {
  asm volatile("NOP");
  TA0CCR0 = time; // 10000;
  TA0CTL = 0x0110;
  for (;;) {
    if (TA0CTL & 0x0001) {
      TA0CTL = 0;
      break;
    }
  }
}
