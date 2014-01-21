#include <stdio.h>
#include <unistd.h>
#include "ble_update.h"

int main(void) {

    //update_loop("BC:6A:29:AB:41:36", &ble_temp);
//    update_loop("BC:6A:29:C3:3C:79", &ble_temp);
    // never reached

    printf("Starting thread\n");
    start_ble_temp_update_thread("BC:6A:29:C3:3C:79");

    while (1)
    {
        printf("TEMPERATURE: %f\n", ble_temp);
        sleep(1);
    }

    return 0;
}
