#!/bin/bash
cd llvmBasicBlockBreakerPass/ && make clean && cd .. 
cd llvmAddGlobalsPass/ && make clean && cd ..
cd llvmRenameMainPass/ && make clean && cd ..
cd llvmCheckpointPlacerPass/ && make clean && cd ..
cd llvmAddBasicBlockAddrsPass/ && make clean && cd ..
cd llvmBBIdPass/ && make clean && cd ..




rm -f *.bc *.txt *.pdf *.pyc
