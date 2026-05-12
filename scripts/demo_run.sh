#!/bin/bash
echo "========================================="
echo "   HOSPITAL OS - LIVE VIVA DEMO          "
echo "========================================="

# Start the hospital processes
./scripts/start_hospital.sh
sleep 2

echo ""
echo "========================================="
echo "   SENDING PATIENTS (STRESS TEST)        "
echo "========================================="
./scripts/stress_test.sh

echo ""
echo "========================================="
echo "   SYSTEM LIVE! TAKE YOUR SCREENSHOTS    "
echo "========================================="
echo "The dashboard is running in the background."
echo "I will keep this window open for 30 seconds..."

# Countdown
for i in {30..1}; do
    echo -ne "Time remaining: $i seconds...\r"
    sleep 1
done

echo ""
echo "========================================="
echo "   SHUTTING DOWN SYSTEM                  "
echo "========================================="
./scripts/stop_hospital.sh

echo ""
echo "Demo complete! Press [ENTER] to close this window."
read
