#include <stdio.h>
#include <unistd.h>
#include "ble_update.h"
#include <sys/shm.h>

#define SHM_KEY1 1234
#define SHM_KEY2 1244
#define SHM_KEY3 1254

// Small test utility to read out temperature

int main(int argc, const char* argv[]) {

    float *ble_temp, *ble_pitch, *ble_roll;
/*
    // Write temperature to shared memory
    int shmid = shmget((key_t) SHM_KEY, sizeof(float),
                       0666 | IPC_CREAT);
    if (shmid == -1) {
        printf("shmget failed\n");
    }

    ble_temp = shmat(shmid, (void *) 0, 0);

    printf("ble_temp =%f\n", *ble_temp);
    // never reached
*/

    // read pitch-angle from shared memory
    int shmid = shmget((key_t) SHM_KEY1, sizeof(float),
                       0666 | IPC_CREAT);
    if (shmid == -1) {
        printf("shmget1 (shmid1) failed\n");
    }

    ble_pitch = shmat(shmid, (void *) 0, 0);

    // read roll-angle from shared memory
    int shmid2 = shmget((key_t) SHM_KEY2, sizeof(float),
                       0666 | IPC_CREAT);
    if (shmid2 == -1) {
        printf("shmget (shmid2) failed\n");
    }

    ble_roll = shmat(shmid2, (void *) 0, 0);

    // read temperature from shared memory
    int shmid3 = shmget((key_t) SHM_KEY3, sizeof(float),
                       0666 | IPC_CREAT);
    if (shmid3 == -1) {
        printf("shmget (shmid3) failed\n");
    }

    ble_temp = shmat(shmid3, (void *) 0, 0);
    
    printf("ble temperature = %f [°C]\n", *ble_temp);
    printf("ble pitch-angle = %f°, ble roll-angle: %f°\n", *ble_pitch, *ble_roll);
    
    return 0;
}
