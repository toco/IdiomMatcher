#!/bin/bash
IDA=/Applications/idaq.app/Contents/MacOS/idaq

BASEDIR=$(dirname $0)

for f in *.elf; do
echo "Processing $f file to queue with command:";
COMMAND="ulimit -v 32000000; ulimit -f 3000000; ulimit -t 300; export TVHEADLESS=1; ($IDA -S$BASEDIR/dumpJSON.idc -B $f)" 
echo $COMMAND
echo $COMMAND | batch
done