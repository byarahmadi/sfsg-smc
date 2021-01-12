/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */

#include <msp430.h>
void
end_task ()
{
  PM5CTL0 &= ~LOCKLPM5;		// Disable the GPIO power-on default high-impedance mode
  WDTCTL = WDTPW | WDTHOLD;
  P1DIR |= BIT0 + BIT5;
  P1OUT |= BIT0 + BIT5;

#ifndef VERIFY
  while (1);
#else
  unsigned long int i = 0;
  while (i < 100000)
    {
      asm volatile ("NOP");
      i++;
    }

#endif
}
