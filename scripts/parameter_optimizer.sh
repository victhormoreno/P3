#!/bin/bash

DIR_P3=$HOME/PAV/P3
DB=$DIR_P3/pitch_db/train
CMD="get_pitch "
BESTSCORE=0
A=0
# rmax=0.406 r1=0.5 pow=-46 center=0

#apt install dc

for rmax in $(seq 0.38 .01 0.41); do 
for r1 in $(seq 0.52 .01 0.52); do
for pow in $(seq -48 1 -44); do
for center in $(seq 0 0.01 0.02); do
    echo -e  "\rPitchAnalyzer with rmax=$rmax, r1=$r1, pow=$pow, center=$center"
    for filewav in pitch_db/train/*.wav; do
        ff0=${filewav/.wav/.f0}
        echo -n "-"
        $CMD $filewav $ff0 --rmax $rmax --r1 $r1 --pow $pow --center $center|| (echo -e "\nError in $CMD $filewav $ff0" && exit 1) 
    done
    A=$(pitch_evaluate_noverb pitch_db/train/*.f0ref)
    if ! echo "$A $BESTSCORE -p" | dc | grep > /dev/null ^-; then
        BESTSCORE=$A
        clear
        echo -e "New Best score $BESTSCORE with parameters  rmax=$rmax, r1=$r1, pow=$pow, center=$center"
    fi
done done done done
exit 0