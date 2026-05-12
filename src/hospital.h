#include<time.h>
#include<stdbool.h>

typedef struct{
    // severity is inputted (1-10), and priority be computed on the base of rules
    int patientId;
    char name[64];
    int age, severity, priority, careUnits;
    time_t arrivalTime;
} PatientRecord;

typedef struct{
    int bedId;
    int index, size, patientId;
    bool isFree;
    char bedType[16]; // ICU, GENERAL, ISOLATION
} Bed;
