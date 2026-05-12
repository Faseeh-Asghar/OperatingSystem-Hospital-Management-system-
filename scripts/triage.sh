#!/bin/bash

#Reading tjhe scipt in puts here
NAME=$1
AGE=$2
SEVERITY=$3

if [ "$#" -ne 3 ];
then 
    echo "Error: Incorrect number of arguments"
    echo "expected arguments: <name> <age> <severity 1-10>"
    exit 1
fi

if [ "$SEVERITY" -lt 1 ] || [ "$SEVERITY" -gt 10 ]; then 
    echo "Error: Severity must be between 1 and 10."
    exit 1
fi

if [ "$AGE" -lt 1 ] || [ "$AGE" -gt 100 ];
then 
    echo "Error: Age must be between 1 and 100."
    exit 1
fi

#converting severity to priority equivalent

if [ "$SEVERITY" -ge 9 ]; 
then PRIORITY=1
elif [ "$SEVERITY" -ge 7 ]; 
then PRIORITY=2
elif [ "$SEVERITY" -ge 5 ]; 
then PRIORITY=3
elif [ "$SEVERITY" -ge 3 ];
then PRIORITY=4
else
    PRIORITY=5
fi 

#pass the data to the C program here
echo "$NAME $AGE $SEVERITY $PRIORITY" > /tmp/triageFifo  
#we're sending the data in [A B C D \n] format to the pipe

echo "Sent: Patient: $NAME | Age: $AGE | Severity: $SEVERITY | Priority: $PRIORITY"