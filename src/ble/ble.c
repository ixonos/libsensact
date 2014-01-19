#include <errno.h>
#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <math.h>
#include <regex.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "ble.h"
#include "gatt_att.h"
#include "TI_sensortag.h"

#define BLE_CONN_WAIT_TIMEOUT  10 // seconds

struct ble_t ble;

struct sockaddr_l2 {
  sa_family_t    l2_family;
  unsigned short l2_psm;
  bdaddr_t       l2_bdaddr;
  unsigned short l2_cid;
  uint8_t        l2_bdaddr_type;
};


// Notice: _l2cap_socket-functions are based on Sandeep Mistry's code published in 'https://github.com/sandeepmistry/noble/tree/master/src'

int ble_read_l2cap_socket(/*int handle,*/ unsigned char *data, int buff_len, int timeout)
{
    fd_set rfds;
    struct timeval tv;

    int len = 0;
    int i, result;

    while(1) {
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	FD_SET(ble.l2capSock, &rfds);

	tv.tv_sec = timeout; // CHECK GOOD VALUE ?????
	tv.tv_usec = 0;

	result = select(ble.l2capSock + 1, &rfds, NULL, NULL, &tv);

	if ( (result == -1) || (result == 0) ) {
	    printf("socket read error: %s\n", (result == -1) ? strerror(errno) : "timeout");
	    return -1;
	}
	
	if (FD_ISSET(ble.l2capSock, &rfds)) {
	    // received data from the socket
	    // read data-octets (maximum number of octets to be read ==buff_len)
	    len = read(ble.l2capSock, data, buff_len);

	    if (len < 0) {
	      printf("Bad data from socket\n");
	      syslog(LOG_ERR, "Bad data from socket");
	      return -1;
	    }

	    printf("Received data, %d bytes (BLE): ", len);
	    syslog(LOG_ERR, "Received data (BLE):\n");
	    for(i = 0; i < len; i++) {
		printf( "0x%x ", ((int)data[i]) );
		syslog( LOG_ERR, "0x%x", ((int)data[i]) );
	    }
	    printf("\n");
	    return len;
	}
    }

    return len;

}

int ble_write_l2cap_socket(/*int handle,*/ unsigned char *data, int length, int timeout)
{
    int len;
    int i;

    printf("Sending data,  %d bytes (BLE): ", length);
    for (i = 0; i < length; i++)
        printf("0x%x ", (unsigned char) data[i]);
    printf("\n");

    len = write(ble.l2capSock, data, length);
    if (len < 0) {
	printf("error in writing on socket\n"); // TODO: close the socket ??
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
    syslog(LOG_ERR, "bind");

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
	printf("connect %s\n", strerror(errno));
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
        close(l2capSock);
	return -1;
    }


    // Store BLE device handle (socket handle) for session
    *ble_device_handle = l2capSock;

    printf("connect success\n");
    syslog(LOG_ERR, "connect success");

    return 0;
}

void discoverServices(void)
{
    int length=0;
    unsigned char buff[30];

    unsigned short start_hdl = 0x0001;
    unsigned short end_hdl = 0xFFFF;

    while (start_hdl != 0xFFFF) {
	EncReadByGroupRequest(buff, &length, start_hdl, end_hdl, GATT_PRIM_SVC_UUID);
	ble_write_l2cap_socket(/*int handle,*/ buff, length, 10);

	length = ble_read_l2cap_socket(/*int handle,*/ buff, sizeof(buff), 10);
	DecReadByGroupResponse(buff, length, &start_hdl);
    }
}

int ble_read_val(unsigned short handle, unsigned char *buff, unsigned int buff_len, unsigned int *index, int *length, int timeout)
{
    int len = 0;
    int ret;

    EncReadRequest(buff, handle, &len);
    ret = ble_write_l2cap_socket(/*int handle,*/ buff, len, timeout);

    if (ret < 0) // problems with writing to the device
	return -1;

    len = ble_read_l2cap_socket(/*int handle,*/ buff, buff_len, timeout);

    if (len <= 0) // problems with reading from the device
	return -1;

    ret = DecReadResponse(buff, index, &len);
    *length = len;

    return ret;
}

int ble_enable_sensor(unsigned short handle, unsigned char flag, int timeout)
{
    int length = 0;
    unsigned char buff[30];

    EncWriteCommand(buff, handle, flag, &length);

    return ble_write_l2cap_socket(/*int handle,*/ buff, length, timeout);
}

int ble_get_float(char *feature, double *value, int timeout)
{
    unsigned int data_handle, conf_handle;
    unsigned char property;
    int length = 0;
    unsigned char buff[30];
    unsigned index = 0; // start-address after response-decoding
    int ret;
    Msg_Hdl msg_handler_ptr;

    if (find_att_handle(/*session_manuf,*/ feature, ENABLE_SENSOR, 0, &conf_handle, &property, NULL) < 0)
	return -1; // handle not found
	
    ret = ble_enable_sensor(conf_handle, 1, timeout); // trigger sensor-measurement by enabling the sensor
    if (ret < 0)
        return -1;

    sleep(1); // set some time for the measurement to finalize

    if (find_att_handle(/*session_manuf,*/ feature, READ_SENSOR, 0, &data_handle, &property, &msg_handler_ptr) < 0)
	return -1; // handle not found

    ret = ble_read_val(data_handle, buff, sizeof(buff), &index, &length, timeout);

//    ble_enable_sensor(conf_handle, 0); // disable the sensor after the measurement
//    sleep(1);
    if (ret < 0)
        return -1;

    // call valid manufacturer specific response-handler
    ret = ( (*msg_handler_ptr)(&buff[index], length, (void*) value) );

    return 0;

}

int ble_connect(char *ble_addr)
{
    return ble_connect_l2cap_socket(&ble.l2capSock, ble_addr);
}

int ble_disconnect(void)
{
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


