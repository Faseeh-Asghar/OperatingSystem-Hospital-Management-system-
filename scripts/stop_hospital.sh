#!/bin/bash

echo "Initiating Hospital Shutdown..."

#Kill the main C program
pkill admissions 2>/dev/null
echo "Admissions manager stopped."

#Clean up POSIX Semaphores
rm -f /dev/shm/sem.*

#Clean up the Named Pipe (FIFO)
rm -f /tmp/triageFifo
rm -f /tmp/dischargeFifo

# Clean up System V Shared Memory
ipcrm -M 530 2>/dev/null # SHM_KEY = 530

echo "IPC resources cleaned."
echo "HOSPITAL CLOSED..."
echo "All beds freed!"