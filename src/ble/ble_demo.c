#include <stdio.h>
#include <unistd.h>
#include "ble_update.h"
#include <sys/shm.h>

#define SHM_KEY 1234
#define SHM_KEY2 1244

// Usage: ./ble_demo <ble_address>

int main(int argc, const char* argv[]) {

    float *ble_temp, *ble_pitch, *ble_roll;

    //update_loop("BC:6A:29:AB:41:36", &ble_temp);
    //update_loop("BC:6A:29:C3:3C:79", &ble_temp);

    // Write pitch-angle to shared memory
    int shmid = shmget((key_t) SHM_KEY, sizeof(float),
                       0666 | IPC_CREAT);
    if (shmid == -1) {
        printf("shmget failed\n");
    }

    ble_pitch = shmat(shmid, (void *) 0, 0);

    // Write roll-angle to shared memory
    int shmid2 = shmget((key_t) SHM_KEY2, sizeof(float),
                       0666 | IPC_CREAT);
    if (shmid2 == -1) {
        printf("shmget failed\n");
    }

    ble_roll = shmat(shmid2, (void *) 0, 0);

    *ble_pitch = 0;
    *ble_roll = 0;

    update_loop((char *) argv[1], ble_pitch, ble_roll);

    // never reached

    return 0;
}
