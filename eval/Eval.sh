#!/bin/bash

CSV=${1:-2}
s=${2:-"*_dump_matched_*.json"}
BASEDIR=$(dirname $0)
eval=$BASEDIR/Eval

if [ $CSV -eq 1 ]
then
    echo "name, correct, missing, wrong, realtime, cputime, matcher, architecture, numpatterns"
elif [ $CSV -eq 2 ]
then
    echo "name	correct	missing	wrong	realtime	cputime	matcher	architecture	numpatterns"
fi

for f in $(find . -name "$s" -not -name "*_dump_matched_comments.json"); do
    if [ $CSV -eq 0 ]
    then    
        echo $f
    fi
result=$($eval -csv $CSV -matched $f -reference ${f/_dump_matched_*.json/_dump_matched_comments.json})
result=${result/ELF for PowerPC (Executable); CPU-ID: 15/PPC}
result=${result/ELF for ARM (Executable); CPU-ID: 13/ARM}
result=${result/ELF for MIPS (Executable); CPU-ID: 12/MIPS}
result=${result/ELF for Intel 386 (Executable); CPU-ID: 0/x86}
result=${result/DependenceGraph/PDG}
result=${result/ControlFlowGraph/CFG}
result=${result/.elf/}

if [[ -n $result ]]
then
    echo $result
fi
done
