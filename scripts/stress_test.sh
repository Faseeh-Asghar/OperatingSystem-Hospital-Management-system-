#!/bin/bash

echo "INITIATING HOSPITAL STRESS TEST..."
echo ""

NUM_PATIENTS=40

echo "Generating an incoming wave of $NUM_PATIENTS patients..."
echo "Dispatching to triage..."

for i in $(seq 1 $NUM_PATIENTS); do
    AGE=$(( (RANDOM % 90) + 1 ))
    SEVERITY=$(( (RANDOM % 10) + 1 ))
    ./scripts/triage.sh "TestPat_$i" $AGE $SEVERITY &
    sleep 0.1
done

echo "==DONE=="