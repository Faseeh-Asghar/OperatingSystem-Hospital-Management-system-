#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <semaphore.h>
#include <time.h>
#include "hospital.h"

#define TOTAL_BEDS 20
#define QUEUE_CAP 32
#define SHM_KEY 530

Bed *bedList;
PatientRecord patientQueue[QUEUE_CAP];
int queueSize = 0;

sem_t *semICU;
sem_t *semISO;
sem_t *semGEN;

pthread_mutex_t bedListMutex;
pthread_mutex_t queueMutex;
sem_t semQueueFull;

FILE *logFile; // schedule_log.txt
pthread_mutex_t logMutex; // so two threads dont write at the same time

// reap zombie child processes so they dont pile up
void sigchld_handler(int sig)
{
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int extractNumber(char *source)
{
    int temp = 0;
    for (int i = 0; source[i] != '\0'; i++)
    {
        char current = source[i] - '0';
        temp = temp * 10;
        temp += current;
    }

    return temp;
}

void extractPatientData(char *buffer, char *name, int *age, int *sev, int *prio)
{
    char temp[64];
    int i = 0;
    int j = 0;
    int field = 0;
    while (buffer[i] != '\0' && buffer[i] != '\n')
    {
        if (buffer[i] == ' ')
        {
            temp[j] = '\0';

            if (field == 0)
                strcpy(name, temp);
            else if (field == 1)
                *age = extractNumber(temp);
            else if (field == 2)
                *sev = extractNumber(temp);

            field++;
            j = 0;
        }
        else
        {
            temp[j] = buffer[i];
            j++;
        }
        i++;
    }
    if (j > 0)
    {
        temp[j] = '\0';
        *prio = extractNumber(temp);
    }
}

void *receptionistFunc(void *arg)
{
    (void)arg;
    printf("Receptionist thread started\n");

    int patientCounter = 1;
    char buffer[256];

    int pipeFD = open("/tmp/triageFifo", O_RDWR);

    if (pipeFD < 0)
    {
        perror("[Receptionist] Error: Failed to open Triage Fifo");
        pthread_exit(NULL);
    }

    printf("Waiting for patients...\n");
    FILE *stream = fdopen(pipeFD, "r");

    while (1)
    {
        if (fgets(buffer, sizeof(buffer), stream) == NULL)
        {
            sleep(1);
        }
        else
        {
            PatientRecord newPat;
            newPat.patientId = patientCounter;
            patientCounter++;

            newPat.arrivalTime = time(NULL);

            extractPatientData(buffer, newPat.name, &newPat.age, &newPat.severity, &newPat.priority);

            newPat.careUnits = 1;

            pthread_mutex_lock(&queueMutex);

            int i = queueSize - 1;
            while (i >= 0 && patientQueue[i].priority > newPat.priority)
            {
                patientQueue[i + 1] = patientQueue[i];
                i--;
            }
            patientQueue[i + 1] = newPat;
            queueSize++;

            printf("Admitted %s, Priority %d to Queue. Total Waiting: %d\n", newPat.name, newPat.priority, queueSize);
            pthread_mutex_unlock(&queueMutex);
            sem_post(&semQueueFull);
        }
    }
    return NULL;
}

void *schedulerFunc(void *arg)
{
    (void)arg;
    printf("Scheduler thread started. Waiting for patients in the queue...\n");

    while (1)
    {
        sem_wait(&semQueueFull);

        pthread_mutex_lock(&queueMutex);
        PatientRecord pat = patientQueue[0];

        for (int i = 0; i < queueSize - 1; i++)
        {
            patientQueue[i] = patientQueue[i + 1];
        }
        queueSize = queueSize - 1;

        pthread_mutex_unlock(&queueMutex);

        printf("Scheduler extracted patient %s, Priority %d | Waiting Queue: %d\n", pat.name, pat.priority, queueSize);

        int start, end;
        char *wardName = "";
        if (pat.priority == 1)
        {
            sem_wait(semICU);
            start = 0;
            end = 4;
            wardName = "ICU";
        }
        else if (pat.priority == 2)
        {
            sem_wait(semISO);
            start = 4;
            end = 8;
            wardName = "ISOLATION";
        }
        else
        {
            sem_wait(semGEN);
            start = 8;
            end = 20;
            wardName = "GENERAL";
        }

        pthread_mutex_lock(&bedListMutex);
        int assignedBed = -1;
        for (int i = start; i < end; i++)
        {
            if (bedList[i].isFree == 1)
            {
                bedList[i].isFree = 0;
                bedList[i].patientId = pat.patientId;
                assignedBed = i;
                break;
            }
        }
        pthread_mutex_unlock(&bedListMutex);

        if (assignedBed == -1)
        {
            printf("Error: Scheduler extracted patient. But no bed available at the moment\n");
        }
        else
        {
            printf("Sheduler assigned %s to %s Bed %d\n", pat.name, wardName, assignedBed);

            // log this admission event to the schedule log file
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            pthread_mutex_lock(&logMutex);
            if (logFile != NULL)
            {
                fprintf(logFile, "[%02d:%02d:%02d] Patient %s (Priority %d) -> %s Bed %d | Queue remaining: %d\n",
                    t->tm_hour, t->tm_min, t->tm_sec,
                    pat.name, pat.priority, wardName, assignedBed, queueSize);
                fflush(logFile); // write it immediately dont buffer
            }
            pthread_mutex_unlock(&logMutex);

            pid_t pid = fork();
            if (pid < 0)
            {
                printf("Scheduler Error: fork failed for Bed %d\n", assignedBed);
            }
            else if (pid == 0)
            {
                char bedString[16];
                sprintf(bedString, "%d", assignedBed);

                execl("./patient_simulator", "patient_simulator", pat.name, bedString, wardName, NULL);

                printf("Scheduler Error: Failed to launch patient_simulator for bed %d", assignedBed);
                exit(1);
            }
        }
    }
    return NULL;
}

void *dischargerFunc(void *arg)
{
    (void)arg;

    int pipeFD = open("/tmp/dischargeFifo", O_RDWR);
    if (pipeFD < 0)
    {
        perror("Discharger failed to open pipe");
        pthread_exit(NULL);
    }

    FILE *stream = fdopen(pipeFD, "r");
    char buffer[256];

    printf("Discharger is Live. Waiting for patients to discharge\n");

    while (1)
    {
        if (fgets(buffer, sizeof(buffer), stream) != NULL)
        {
            int bedIndex;
            char bedType[16];

            if (sscanf(buffer, "%d %s", &bedIndex, bedType))
            {
                pthread_mutex_lock(&bedListMutex);
                bedList[bedIndex].isFree = 1;
                bedList[bedIndex].patientId = -1;
                pthread_mutex_unlock(&bedListMutex);
            }

            if (strcmp(bedType, "ICU") == 0)
            {
                sem_post(semICU);
            }
            else if (strcmp(bedType, "ISOLATION") == 0)
            {
                sem_post(semISO);
            }
            else
            {
                sem_post(semGEN);
            }
            printf("Discharger freed the bed %d\n", bedIndex);
        }
    }
    return NULL;
}

int main()
{
    printf("Starting Hospital Admissions Manager...\n");

    // register SIGCHLD handler so we dont get zombie processes piling up
    signal(SIGCHLD, sigchld_handler);

    // open the schedule log file
    logFile = fopen("logs/schedule_log.txt", "a");
    if (logFile == NULL)
    {
        printf("Warning: Could not open logs/schedule_log.txt, continuing without logging\n");
    }
    pthread_mutex_init(&logMutex, NULL);

    int shmID = shmget(SHM_KEY, TOTAL_BEDS * sizeof(Bed), IPC_CREAT | 0666);
    if (shmID < 0)
    {
        printf("Error: Failed to initalize shared memory (The Ward)\n");
        exit(1);
    }
    void *temp = shmat(shmID, NULL, 0);
    bedList = (Bed *)temp;

    for (int i = 0; i < TOTAL_BEDS; i++)
    {
        bedList[i].bedId = i;
        bedList[i].index = i;
        bedList[i].size = 1;
        bedList[i].isFree = 1;
        bedList[i].patientId = -1;
    }

    for (int i = 0; i < TOTAL_BEDS; i++)
    {
        if (i < 4)
            strcpy(bedList[i].bedType, "ICU");
        else if (i < 8)
            strcpy(bedList[i].bedType, "ISOLATION");
        else
            strcpy(bedList[i].bedType, "GENERAL");
    }

    printf("Shared memory initialized\n");

    semICU = sem_open("/semICUlimit", O_CREAT, 0644, 4);
    semISO = sem_open("/semISOlimit", O_CREAT, 0644, 4);
    semGEN = sem_open("/semGENlimit", O_CREAT, 0644, 12);
    sem_init(&semQueueFull, 0, 0);
    printf("Semaphores initialized\n");

    mkfifo("/tmp/triageFifo", 0666);
    mkfifo("/tmp/dischargeFifo", 0666);

    pthread_t receptionist, scheduler, discharger;
    pthread_create(&receptionist, NULL, receptionistFunc, NULL);
    pthread_create(&scheduler, NULL, schedulerFunc, NULL);
    pthread_create(&discharger, NULL, dischargerFunc, NULL);

    printf("Hospital is now open.\nWaiting for patients...\n");
    while (1)
    {
        sleep(1);
    }

    printf("Admissions Manager shutting down\n");
    return 0;
}