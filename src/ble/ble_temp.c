#include <stdio.h>
#include <unistd.h>
#include "ble_update.h"
#include <sys/shm.h>

#define SHM_KEY 1234

// Small test utility to read out temperature

int main(int argc, const char* argv[]) {

    float *ble_temp;

    // Write temperature to shared memory
    int shmid = shmget((key_t) SHM_KEY, sizeof(float),
                       0666 | IPC_CREAT);
    if (shmid == -1) {
        printf("shmget failed\n");
    }

    ble_temp = shmat(shmid, (void *) 0, 0);

    printf("ble_temp =%f\n", *ble_temp);
    // never reached

    return 0;
}
