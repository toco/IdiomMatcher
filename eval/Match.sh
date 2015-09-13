#!/bin/bash

s=${1:-"*_dump.json"}
BASEDIR=$(dirname $0)
m="--matcher Naive --matcher SimpleGraph --matcher DependenceGraph"
#m="--dumpSwitches"
p="--patterns $BASEDIR/IdiomMatcher_arm.json --patterns $BASEDIR/IdiomMatcher_x86.json --pattern $BASEDIR/IdiomMatcher_ppc.json --pattern $BASEDIR/IdiomMatcher_mips.json"

echo "patterns are $p";

for f in $(find . -name "$s"); do
echo "Processing $f file..";
$BASEDIR/../build/src/Standalone/IdiomMatcherStandalone $p  --file $f $m
done
