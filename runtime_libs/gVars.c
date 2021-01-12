/*
 * This file is part of sfsg.
 *
 * Author: Bahram Yarahmadi
 * Copyright (c) 2020  Inria
 *
 */


unsigned int nst = 0;  // Shows the nesting when a faulure happens
unsigned int firstTimeInLoop = 0; 
unsigned int lh = 0;
unsigned int callInstAddr = 0;
unsigned int volatile_trace_arr[100];  // 100 is the maximum trace number
