#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<signal.h>
#include"hospital.h"

#define SHM_KEY 530
#define TOTAL_BEDS 20

int running = 1;
int runTime = 0;

void printBed(Bed bed)
{
    if (bed.isFree)
    {
        printf("Bed %d: EMPTY", bed.index);
    }
    else
    {
        printf("Bed %d: Patient %d", bed.index, bed.patientId);
    }

}

int main()
{
    printf("Connecting to hospital...\n");
    int shmID = shmget(SHM_KEY, TOTAL_BEDS * sizeof(Bed), 0666);
    
    if (!running) //overried condition
        return 0;

    void* temp = shmat(shmID, NULL,0);
    Bed *bedList = (Bed *)temp;

    while(running) // printing loop here
    {
        system("clear");
        printf("Dashboard Run Time: %d seconds\n", runTime);
        printf("== LIVE HOSPITAL DASHBOARD ==\n\n");

        printf("--ICU WARDS\n");
        for(int i=0;i<4;i++)
        {
            printBed(bedList[i]);
            printf("\n");
        }
        printf("\n\n");

        printf("--ISOLATION WARDS\n");
        for(int i=4;i<8;i++)
        {
            printBed(bedList[i]);
            printf("\n");
        }
        printf("\n\n");

        printf("--GENERAL WARDS\n");
        for(int i=8;i<20;i++)
        {
            printBed(bedList[i]);
            printf("\n");
        }
        printf("\n\n");

        runTime++;
        sleep(1);
    }

    printf("\n");
    printf("Disconnecting dashboard from the main process...\n");
    shmdt(bedList);

    return 0;
}