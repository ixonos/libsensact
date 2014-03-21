#include <errno.h>
#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <math.h>
#include <regex.h>
#include <sys/time.h>
#include <stdlib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <sys/ioctl.h>

#include "ble.h"
#include "gatt_att.h"
#include "TI_sensortag.h"

#define BLE_CONN_WAIT_TIMEOUT  10 // seconds

struct ble_t ble;

struct characteristics_table_t characteristics_table;

struct sockaddr_l2 {
  sa_family_t    l2_family;
  unsigned short l2_psm;
  bdaddr_t       l2_bdaddr;
  unsigned short l2_cid;
  uint8_t        l2_bdaddr_type;
};

inline double getFractionalSeconds(void) {
   struct timeval tv;
   double time_stamp;

   gettimeofday(&tv, NULL);
   time_stamp = (double) tv.tv_sec + (double) 1e-6 * tv.tv_usec; 

   // return seconds.microseconds since epoch 
   return(time_stamp);
}

// Notice: _l2cap_socket-functions are based on Sandeep Mistry's code published in 'https://github.com/sandeepmistry/noble/tree/master/src'

int ble_read_l2cap_socket(/*int handle,*/ unsigned char *data, int buff_len, int timeout, double *receive_time)
{
    fd_set rfds;
    struct timeval tv;

    int len = -1;
    int i, result;
    double time_stamp = 0; // time stapm of the recievd message

    FD_ZERO(&rfds);
    FD_SET(ble.l2capSock, &rfds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    result = select(ble.l2capSock + 1, &rfds, NULL, NULL, &tv);

    if (result <= 0) {
	if ( (result == -1) || (result == 0) ) {
	    printf("socket read error: %s\n", (result == -1) ? strerror(errno) : "timeout");
	    syslog(LOG_ERR, "socket read error: %s\n", (result == -1) ? strerror(errno) : "timeout");
	}
	else {
	    printf("socket read error: %s\n", "undefined");
	    syslog(LOG_ERR, "socket read error: %s\n", "undefined");
	}
	return -1;
    }

    if (FD_ISSET(ble.l2capSock, &rfds)) {
	// received data from the socket
	// read data-octets (maximum number of octets to be read = buff_len)
	len = read(ble.l2capSock, data, buff_len);

	if (len < 0) {
	  printf("Bad data from socket, error: %s\n", (len == -1) ? strerror(errno) : "undefined");
	  syslog(LOG_ERR, "Bad data from socket, error: %s\n", (len == -1) ? strerror(errno) : "undefined");
	  return -1;
	}
	
	time_stamp = getFractionalSeconds();
	
	if (receive_time != NULL)
	    *receive_time = time_stamp;

/*
	printf("Received data, %d bytes (BLE): ", len);
	//syslog(LOG_ERR, "Received data (BLE):\n");
	for(i = 0; i < len; i++) {
	    printf( "0x%x ", ((int)data[i]) );
	    //syslog( LOG_ERR, "0x%x", ((int)data[i]) );
	}
	printf("         time stamp: %lf [seconds.microseconds]\n", time_stamp);
	printf("\n");
*/
	return len;
    }
    else // this condition shouldn't be reached
        return -1;

    return len;
}

int ble_write_l2cap_socket(/*int handle,*/ unsigned char *data, int length, int timeout)
{
    int len;
    int i;
/*
    printf("Sending data,  %d bytes (BLE): ", length);
    for (i = 0; i < length; i++)
        printf("0x%x ", (unsigned char) data[i]);
    printf("\n");
*/
    len = write(ble.l2capSock, data, length);
    if (len < 0) {
	printf("error in writing on socket\n");
	syslog(LOG_ERR, "error in writing on socket");
	return -1;
    }
    return len;
}

int ble_disconnect_l2cap_socket(/*int handle*/void)
{
    printf("Disconnecting BLE device...\n");

    close(ble.l2capSock);
    printf("disconnected\n");
    syslog(LOG_ERR, "disconnected");

    return 0;
}

int ble_connect_l2cap_socket(int *ble_device_handle, char *addr)
{
    struct sockaddr_l2 sockAddr;
    int sock_flags;
    fd_set rfds;
    struct timeval tv;
    int l2capSock;
    int result;

    printf("Connect to address %s\n", addr);

    // create socket
    l2capSock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // bind
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.l2_family = AF_BLUETOOTH;
    bacpy(&sockAddr.l2_bdaddr, BDADDR_ANY);
    sockAddr.l2_cid = htobs(ATT_CID);

    result = bind(l2capSock, (struct sockaddr*)&sockAddr, sizeof(sockAddr));

    printf("bind %s\n", (result == -1) ? strerror(errno) : "success");
    syslog(LOG_ERR, "bind %s\n", (result == -1) ? strerror(errno) : "success");

    // connect
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.l2_family = AF_BLUETOOTH;


    str2ba(addr, &sockAddr.l2_bdaddr);
    sockAddr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    sockAddr.l2_cid = htobs(ATT_CID);


    /* Wait connection until BLE_CONN_WAIT_TIMEOUT */
    sock_flags = fcntl(l2capSock, F_GETFL);
    result = fcntl(l2capSock, F_SETFL, sock_flags | O_NONBLOCK);
    if (result == -1) {
      fprintf(stderr, "fcntl failed\n");
    }

    result = connect(l2capSock, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
    if (result == -1) {
      if (errno != EINPROGRESS) {
	printf("connect error: %s\n", strerror(errno));
	syslog(LOG_ERR, "connect error: %s\n", strerror(errno));
	close(l2capSock);
	printf("error\n");
	return -1;
      }
    }

    FD_ZERO(&rfds);
    FD_SET(l2capSock, &rfds);
    tv.tv_sec = BLE_CONN_WAIT_TIMEOUT;
    tv.tv_usec = 0;

    result = select(l2capSock + 1, NULL, &rfds, NULL, &tv);

    if (result == 1) {
      int so_error;
      socklen_t len = sizeof(so_error);
      result = getsockopt(l2capSock, SOL_SOCKET, SO_ERROR, &so_error, &len);
      if (so_error == 0) { // socket is open
	// socket back to blocking state
	fcntl(l2capSock, F_SETFL, sock_flags & ~O_NONBLOCK);
	printf("socket is open\n");
      } else if (result == -1) {
	result = errno;
	close(l2capSock);
	printf("socket timeout (error %s)\n", strerror(result));
	return -1;
      }
    }
    else {  // result = 0 -> timeout; result < 0 -> error case
	printf("socket not open: %s\n", (result == -1) ? strerror(errno) : "timeout");
	syslog(LOG_ERR, "socket not open: %s\n", (result == -1) ? strerror(errno) : "timeout");
        close(l2capSock);
	return -1;
    }


    // Store BLE device handle (socket handle) for session
    *ble_device_handle = l2capSock;

    printf("connect success\n");
    syslog(LOG_ERR, "connect success");

    return 0;
}

int discoverServices(void)
{
    // services not supported otherwise, but you can get those FYI by enabling printf in DecReadByGroupResponsePrimary()
    int length = 0;
    int ret = 0;
    unsigned char buff[30];

    unsigned short start_hdl = 0x0001;
    unsigned short end_hdl = 0xFFFF;

    while (start_hdl != 0xFFFF) {
	EncReadByGroupRequest(buff, &length, start_hdl, end_hdl, GATT_PRIM_SVC_UUID);
	ret = ble_write_l2cap_socket(/*int handle,*/ buff, length, 10);
	if (ret < 0)
	    return -1;
	
	length = ble_read_l2cap_socket(/*int handle,*/ buff, sizeof(buff), 10, NULL);
	if (ret < 0)
	    return -1;
	
	ret = DecReadByGroupResponsePrimary(buff, length, &start_hdl);
	if (ret < 0)
	    return -1;
    }

    return 0; // success
}

int discoverCharacteristics(void)
{
    int length = 0;
    int i, num_elems = 0, total_num_attr = 0, ret = 0;
    unsigned char buff[30];
    unsigned short start_hdl = 0x0001;
    unsigned short end_hdl = 0xFFFF;
    struct dynamic_char_t *char_ptr = &characteristics_table.dyn_attr[0];

    memset(&characteristics_table, 0, sizeof(characteristics_table));

    while (start_hdl != 0xFFFF) {
	EncReadByTypeRequest(buff, &length, start_hdl, end_hdl, GATT_CHARAC_UUID);
	ret = ble_write_l2cap_socket(/*int handle,*/ buff, length, 10);
	if (ret < 0)
	    return -1;
	
	length = ble_read_l2cap_socket(/*int handle,*/ buff, sizeof(buff), 10, NULL);
	if (ret < 0)
	    return -1;
	
	num_elems = DecReadByTypeResponseCharacteristics(buff, length, &start_hdl, char_ptr);
	if (num_elems < 0)
	    break;

	char_ptr += num_elems;
	total_num_attr += num_elems;

	if (total_num_attr > sizeof(characteristics_table.dyn_attr)/sizeof(characteristics_table.dyn_attr[0]))
	    break; // don't request over supported number of attributes
    }

    characteristics_table.nbr_attr = total_num_attr;

    //for (i = 0, char_ptr = &characteristics_table.dyn_attr[0]; i < characteristics_table.nbr_attr; i++, char_ptr++)
    //    printf("uuid: 0x%x, char_handle: 0x%x, properties: 0x%x\n", char_ptr->UUID, char_ptr->handle, char_ptr->property);

    return 0; // success
}

int ble_read_val(unsigned short handle, unsigned char *buff, unsigned int buff_len, unsigned int *index, int *length, int timeout, double *time_stamp)
{
    int len = 0;
    int ret;

    EncReadRequest(buff, handle, &len);
    ret = ble_write_l2cap_socket(/*int handle,*/ buff, len, timeout);

    if (ret < 0) // problems with writing to the device
	return -1;

    len = ble_read_l2cap_socket(/*int handle,*/ buff, buff_len, timeout, time_stamp);

    if (len <= 0) // problems with reading from the device
	return -1;

    ret = DecReadResponse(buff, index, &len);
    *length = len;

    return ret;
}

int ble_enable_sensor(unsigned short handle, unsigned char flag, int timeout)
{
    int total_length = 0, payload_length = 0;
    unsigned char buff[10]; // buffer for encoded request frame (corresponding to ATT-protocol)
    unsigned char payload_data_buff[5];

    memset(buff, 0, sizeof(buff));
    memset(payload_data_buff, 0, sizeof(payload_data_buff));

    payload_data_buff[0] = flag; // TRUE/FALSE
    payload_length = 1;

    EncWriteCommand(buff, handle, payload_data_buff, &total_length, payload_length);

    return ble_write_l2cap_socket(/*int handle,*/ buff, total_length, timeout);
}

int ble_enable_notifications(unsigned short handle, unsigned char flag, int timeout)
{
    int total_length = 0, payload_length = 0;
    unsigned char notif_buff[10]; // buffer for encoded request frame (corresponding to ATT-protocol)
    unsigned char payload_data_buff[5];

    memset(notif_buff, 0, sizeof(notif_buff));
    memset(payload_data_buff, 0, sizeof(payload_data_buff));

    // sensorTag can be enabled to send notifications for every sensor by writing “01 00” / disabled by “00 00”
    payload_data_buff[0] = flag; // TRUE/FALSE
    payload_data_buff[1] = 0x00;
    payload_length = 2;

    EncWriteCommand(&notif_buff[0], handle, &payload_data_buff[0], &total_length, payload_length);

    return ble_write_l2cap_socket(/*int handle,*/ &notif_buff[0], total_length, timeout);
}

int ble_set_measuring_period(unsigned short handle, unsigned char input, int timeout)
{
    int total_length = 0, payload_length = 0;
    unsigned char mp_buff[10]; // buffer for encoded request frame (corresponding to ATT-protocol)
    unsigned char payload_data_buff[3];

    memset(mp_buff, 0, sizeof(mp_buff));
    memset(payload_data_buff, 0, sizeof(payload_data_buff));

    payload_data_buff[0] = input; // period = [input*10]ms
    payload_length = 1;

    EncWriteCommand(&mp_buff[0], handle, &payload_data_buff[0], &total_length, payload_length);

    return ble_write_l2cap_socket(/*int handle,*/ &mp_buff[0], total_length, timeout);
}

int ble_get_string(char *feature, char *value, int timeout, double *time_stamp)
{
   unsigned int data_handle;
   int length = 0, ret = 0;
   unsigned char buff[100];
   unsigned index = 0; // start-address after response-decoding
   unsigned char property;
   struct characteristics_table_t *char_ptr = &characteristics_table;

   memset(buff, 0, sizeof(buff));

   if (find_att_handle(/*session_manuf,*/ feature, READ_SENSOR, char_ptr, &data_handle, &property, NULL) < 0)
        return -1; // handle not found

    ret = ble_read_val(data_handle, buff, sizeof(buff), &index, &length, timeout, time_stamp);

    if (ret < 0)
        return -1;

    memcpy(value, buff, length + 1);

    return 0;
}

int ble_sensor_status(char *feature, unsigned int read_write)
{
    // Status-flag is used for checking status of the sensor.
    // So it makes it possible to avoid sensor to be enabled every time, sensor is accessed.
    if (read_write == READ_SENSOR) // read status-flag of the sensor
    {
	if (!strcmp(feature, "Temp")) {
	    if (ble.sensor_status & SENSOR_TEMP)
	        return 1; // temperature-sensor already enabled
	    else
	        return 0;
	}
	else if (!strcmp(feature, "accelero")) {
	    if (ble.sensor_status & SENSOR_ACCELER)
	        return 1; //accelerometer-sensor already enabled
	    else
	        return 0;
	}
	else if (!strcmp(feature, "magnetom")) {
	    if (ble.sensor_status & SENSOR_MAGNETO)
	        return 1; //magnetometer-sensor already enabled
	    else
	        return 0;
	}
	else if (!strcmp(feature, "gyroscope")) {
	    if (ble.sensor_status & SENSOR_GYRO)
	        return 1; //gyroscope-sensor already enabled
	    else
	        return 0;
	}
	else
	    return -1; // requested sensor not supported
    }
    else if (read_write == ENABLE_SENSOR) // enable status-flag of the sensor by raising the corresponding bit
    {
	if (!strcmp(feature, "Temp"))
	    ble.sensor_status |= SENSOR_TEMP;
	else if (!strcmp(feature, "accelero"))
	    ble.sensor_status |= SENSOR_ACCELER;
	else if (!strcmp(feature, "magnetom"))
	    ble.sensor_status |= SENSOR_MAGNETO;
	else if (!strcmp(feature, "gyroscope"))
	    ble.sensor_status |= SENSOR_GYRO;
	else
	    return -1; // requested sensor not supported
    }

    return -1; // requested sensor not supported
}

int ble_get_float(char *feature, double *value1, double *value2, double *value3, int timeout, double *time_stamp)
{
    unsigned int data_handle, conf_handle;
    unsigned char property;
    int length = 0;
    unsigned char buff[30];
    unsigned index = 0; // start-address after response-decoding
    int ret = 0;
    Msg_Hdl msg_handler_ptr;
    struct characteristics_table_t *char_ptr = &characteristics_table;

    if (find_att_handle(/*session_manuf,*/ feature, READ_SENSOR, char_ptr, &data_handle, &property, &msg_handler_ptr) < 0)
        return -1; // handle not found

    ret = ble_sensor_status(feature, READ_SENSOR);
    if (ret < 0)
        return -1;

    if (ret == 0) // sensor is disabled
    {
	if (find_att_handle(/*session_manuf,*/ feature, ENABLE_SENSOR, char_ptr, &conf_handle, &property, NULL) < 0)
	    return -1; // handle not found

        if (!strcmp(feature, "gyroscope")) {
            //ret = ble_enable_notifications(0x0F, 1, timeout); // general notifications seem not to be needed enabled specifically
	    ret = ble_enable_notifications(data_handle + 0x01, 1, timeout);
	    if (ret < 0)
                return -1;

	    usleep(100000);

	    // Trigger sensor-measurement by enabling 'x/y/z'-axis sensors.
	    // 1 to enable X axis only, 2 to enable Y axis only, 3 = X and Y, 4 = Z only, 5 = X and Z, 6 = Y and Z, 7 = X, Y and Z
	    ret = ble_enable_sensor(conf_handle, 7, timeout);
	}
	else if (!strcmp(feature, "magnetom")) {
	    ret = ble_enable_notifications(data_handle + 0x01, 0x01, timeout);
	    if (ret < 0)
                return -1;

	    usleep(100000);

	    ret = ble_enable_sensor(conf_handle, 1, timeout); // trigger sensor-measurement by enabling the sensor
	    if (ret < 0)
                return -1;

	    usleep(100000);

	    ret = ble_set_measuring_period(conf_handle + 0x03, 0x0A, timeout); // set 100 ms (default period: 0xc8 = 200 -> 200 ms * 10 = 2 sec)
    	    if (ret < 0)
                return -1;
	}
	else if (!strcmp(feature, "accelero")) {
/*	    ret = ble_enable_notifications(data_handle + 0x01, 0x01, timeout);
	    if (ret < 0)
                return -1;

	    usleep(100000);
*/
	    ret = ble_enable_sensor(conf_handle, 1, timeout); // trigger sensor-measurement by enabling the sensor
	    if (ret < 0)
                return -1;

	    usleep(100000);

	    ret = ble_set_measuring_period(conf_handle + 0x03, 0x0A, timeout); // set 100 ms (default period: 1s)
    	    if (ret < 0)
                return -1;
	}
	else
	    ret = ble_enable_sensor(conf_handle, 1, timeout); // trigger sensor-measurement by enabling the sensor

        if (ret < 0)
            return -1;

        sleep(1); // set some time for the enabling of the sensor to finalize
    }

    ret = ble_read_val(data_handle, buff, sizeof(buff), &index, &length, timeout, time_stamp);

//    ble_enable_sensor(conf_handle, 0); // disable the sensor after the measurement
//    sleep(1);
    if (ret < 0)
        return -1;

    ble_sensor_status(feature, ENABLE_SENSOR);

    // call valid manufacturer specific response-handler
    ret = ( (*msg_handler_ptr)(&buff[index], length, (void*) value1, (void*) value2, (void*) value3, time_stamp) );

    return 0;

}



// Idea for functions below (get_hci_handle and ble_conn_update ) is taken from /linux/bluetooth/bluez/tools/hcitool.c

//int conn_list(int s, int dev_id, long arg)
int get_hci_handle(int dev_id)
{
	struct hci_conn_list_req *cl;
	struct hci_conn_info *ci;
	int i;

	int sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	
	if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
		perror("Can't allocate memory");
		return -1;
	}
	cl->dev_id = dev_id;
	cl->conn_num = 10;
	ci = cl->conn_info;

	if (ioctl(sk, HCIGETCONNLIST, (void *) cl)) {
		perror("Can't get connection list");
		free(cl);
		return -1;
	}

	for (i = 0; i < cl->conn_num; i++, ci++) {
		char bt_addr[18];
		ba2str(&ci->bdaddr, bt_addr);
		//TODO: validate that handle is according to our BT-address...
		printf("\t%s handle %d state %d\n", (char *) bt_addr, ci->handle, ci->state);
		return ci->handle;
	}

	free(cl);
	close(sk);
	return 0;
}

int ble_conn_update(void)
{
    int dd, err, dev_id;
    unsigned int handle, min_conn_interval = 6, max_conn_interval = 6, slave_latency = 0, supervision_timeout = 0x0C80;

    // notice: max_conn_interval/min_conn_interval in seconds is: multiple by 1.25 (e.g. 6*1.25 = 7.5 s). 
    // This is the minimum interval value which can be set!
    dev_id = hci_get_route(NULL);

    dd = hci_open_dev(dev_id);
    if (dd < 0) {
        fprintf(stderr, "HCI device open failed\n");
        return -1;
    }

    handle = get_hci_handle(dev_id);
    if (handle < 0)
        return -1;

    if (hci_le_conn_update(dd, htobs(handle), htobs(min_conn_interval), htobs(max_conn_interval),
            htobs(slave_latency), htobs(supervision_timeout), 5000) < 0)
    {
        err = -errno;
        fprintf(stderr, "Could not change connection params: %s(%d)\n", strerror(-err), -err);
	return -1;
    }

    hci_close_dev(dd);

    return 0;
}

int ble_connect(char *ble_addr)
{
    ble.sensor_status = 0; // clean first all status-indicators of the sensors

    return ble_connect_l2cap_socket(&ble.l2capSock, ble_addr);
}

int ble_disconnect(void)
{
    ble.sensor_status = 0; // clean all status-indicators of the sensors
    return ble_disconnect_l2cap_socket();
}

int ble_validate_address(const char* address) {
	regex_t regex;
	int reti, verdict = 0;
	char buffer[100];

	/* Compile regular expression */
	reti = regcomp(&regex, "^([0-9A-F]{2}[:]){5}[0-9A-F]{2}", REG_ICASE | REG_EXTENDED);
	if (reti) {
		fprintf(stderr, "Could not compile regex\n");
	}

	/* Execute regular expression */
	reti = regexec(&regex, address, 0, NULL, 0);
	if (!reti) {
		verdict = 1;
	} else if (reti == REG_NOMATCH) {
	} else {
		regerror(reti, &regex, buffer, sizeof(buffer));
		fprintf(stderr, "Regex match failed: %s\n", buffer);
	}

	regfree(&regex);

	return verdict;
}


