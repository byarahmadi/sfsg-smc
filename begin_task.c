/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */



#include <msp430.h>

void
begin_task ()
{
  WDTCTL = WDTPW | WDTHOLD;
  PM5CTL0 &= ~LOCKLPM5;
  P4DIR |= BIT6 + BIT3;
  P4OUT |= BIT6 + BIT3;
}
