#!/bin/bash

echo "=== Starting Hospital System ==="

# start the main admissions manager in background
./admissions &
echo "Admissions Manager started (PID: $!)"

echo "Hospital is now running. Use stop_hospital.sh to shut it down."