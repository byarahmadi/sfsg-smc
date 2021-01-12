#!/bin/sh
rm /tmp/*.dot
rm ../outputs/*.pdf
opt -view-cfg ../outputs/after_bb_id.bc -o cfg.bc &> /tmp/opt-cfg-output.txt

ls /tmp/*.dot | while read line
do 
	SUBSTR=$(echo $line | cut -d'/' -f 3)
	NAME=$(echo $SUBSTR | cut -d'-' -f 1)
	dot -Tpdf  $line -o ../outputs/$NAME.pdf	
done

#rm /tmp/*.dot
#RAW="_RAW"
#opt -view-cfg ../outputs/raw.bc -o cfg.bc &> /tmp/opt-cfg-output.txt

#ls /tmp/*.dot | while read line
#do
	
#        SUBSTR=$(echo $line | cut -d'/' -f 3)
#        NAME=$(echo $SUBSTR | cut -d'-' -f 1)
#	var=$NAME$RAW
#        dot -Tpdf  $line -o ../outputs/$var.pdf
#done



exit 0

