extern float ble_pitch;
extern float ble_roll;
extern float ble_temp;
void update_loop(char *ble_addr, float *pitch, float *roll, float *temp);
void start_ble_temp_update_thread(const char *ble_addr);
void stop_ble_temp_update_thread(void);
