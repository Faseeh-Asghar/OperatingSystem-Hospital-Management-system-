#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<time.h>

int extractNum(char* source)
{
    int temp = 0;
    for(int i = 0; source[i]!='\0'; i++)
    {
        char current = source[i] - '0';
        temp = temp*10;
        temp += current;
    }

    return temp;
}

int main(int argc, char*argv[])
{
    if (argc!=4)
    {
        printf("Invalid Arguments!\n");
        printf("Correct Usage: ./patient_simulator <patientName, bedIndex, bedType>\n");
        return 1;
    }

    char* patientName = argv[1];
    int assignedBed = extractNum(argv[2]); // getPatientID from the arguments
    char* bedType = argv[3]; //second arg will have the bed type

    printf("Patient %s: Admitted to %s Bed %d\n", patientName, bedType, assignedBed);


    int sleepTime = 0;
    if(strcmp(bedType, "ICU")==0)
    {
        sleepTime = (rand()%11)+5;
    }
    else if(strcmp(bedType, "ISOLATION")==0)
    {
        sleepTime = (rand()%8)+3;
    }
    else
    {
        sleepTime = (rand()%7)+2;
    }

    sleepTime = 3; // for manual overriding

    printf("Patient %s: Treatment started for %d seconds...\n", patientName, sleepTime);
    sleep(sleepTime); // simulating the treatment...awein 

    int fd = open("/tmp/dischargeFifo", O_RDWR);
    if (fd==-1)
    {
        printf("Patient simulator failed to open discharge pipe for bed %d", assignedBed);
    }
    else 
    {
        char buffer[256];
        sprintf(buffer, "%d, %s\n", assignedBed, bedType); // i had added ',' between d and s..... messed up alot bruhhh...the discharger was expecting without coma
        write(fd, buffer, strlen(buffer));
        close(fd);
    }

    printf("Patient %s: Treatment complete. Now freeing bed %d\n", patientName, assignedBed);
    return 0;
}