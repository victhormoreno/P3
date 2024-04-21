#!/bin/bash

DIR_P3=$HOME/PAV/P3
DB=$DIR_P3/pitch_db/train
CMD="get_pitch "
BESTSCORE=0
A=0
param1=0.406 param2=0.5 param3=-46 ccparam=0

#apt install dc

for param1 in $(seq 0.38 .01 0.41); do 
for param2 in $(seq 0.52     .01 0.52); do
for param3 in $(seq -48 1 -44); do
#for ccparam in $(seq 0 0.01 0.02); do
    echo -e  "\rPitchAnalyzer with param1=$param1, param2=$param2, param3=$param3, ccth=$ccparam"
    for filewav in pitch_db/train/*.wav; do
        ff0=${filewav/.wav/.f0}
        #echo "$CMD --param1 $param1 --param2 $param2 --param3 $param3 $filewav $ff0"
        echo -n "-"
        $CMD -1 $param1 -2 $param2 -3 $param3 -c $ccparam $filewav $ff0 || (echo -e "\nError in $CMD $filewav $ff0" && exit 1) 
    done
    A=$(pitch_evaluate_noverb pitch_db/train/*.f0ref)
    if ! echo "$A $BESTSCORE -p" | dc | grep > /dev/null ^-; then
        BESTSCORE=$A
        clear
        echo -e "New Best score $BESTSCORE with parameters  param1=$param1, param2=$param2, param3=$param3, ccth=$ccparam"
    fi
    done     done  done # done
exit 0