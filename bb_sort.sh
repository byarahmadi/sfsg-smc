#!/bin/bash
nbbsArrayAddr=`grep "<nbbs>" ../outputs/a.objdump|awk '{print $1}'`
#nbbsArrayAddr=$(echo $nbbsArrayAddr | cut -d'0' -f 5)
echo "nbbs address = " $nbbsArrayAddr
echo $nbbsArrayAddr > nbbsArrayAddr.txt
checkpointAddr=`grep "<_checkpoint>" ../outputs/a.objdump|awk '{print $1}'`
echo "Checkpoint address = " $checkpointAddr
safepointAddr=`grep  "<_safepoint>" ../outputs/a.objdump|awk '{print $1}'`
echo "Safepoint address = " $safepointAddr
pref=";#0x"
addr=$(echo $safepointAddr | cut -d'0' -f 5)
fullAddr=$pref$addr

echo $fullAddr
grep $fullAddr ../outputs/a.objdump |grep  "call"|awk '{print $1}' > so_far_addrs.txt


