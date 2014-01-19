#include <stdio.h>
#include "gatt_att.h"

unsigned short read_uint16_le(unsigned char *buff, int offset)
{
    unsigned short value = 0;

    value = buff[offset];
    value |= buff[offset + 1] << 8;

    return value;
}

void write_uint16_le(unsigned short value, unsigned char *buff, int offset)
{
    buff[offset + 1] = (value & 0xff00) >> 8;
    buff[offset] = value & 0x00ff;
}

short convert_int16_le(unsigned char *buff, int offset)
{
    // two bytes in the buffer (starting from the offset-index) encoded in little endian format
    // convert to a signed 16 bit integer in big-endian

    short val = 0;

    val = (short) ( (buff[offset + 1] << 8) | buff[offset] ); // signed integer16-casting (short) is MUST !

/*    if (val & 0x8000) {
       //val = (0xffff - val + 1) * -1;
	val = ( (val ^ 0xffff) + 1 ) & 0xffff;  // undo two's complement
	val *= -1; // make native negative
    }
*/

    return val;
}

int DecReadByGroupResponse(unsigned char *data, unsigned int length, unsigned short *end_hdl) // decode read-response PDU
{
    int i;
    int type = data[1];
    unsigned int num = (length - 2) / data[1];

    unsigned short start_handle, end_handle;
    long uuid;

    for (i = 0; i < num; i++) {
	start_handle = read_uint16_le(data, 2 + i * type + 0);
	end_handle = read_uint16_le(data, 2 + i * type + 2);
        uuid = read_uint16_le(data, 2 + i * type + 4);
	printf("start_hdl: 0x%x, end_handle: 0x%x, uuid: 0x%x\n", start_handle, end_handle, (unsigned int) uuid);
    }
    *end_hdl = end_handle;

    return 0;
}

void EncReadByGroupRequest(unsigned char *buff, int *length, unsigned short start_hdl, unsigned short end_hdl, unsigned short groupUuid) // encode request PDU
{
    buff[0] = ATT_OP_READ_BY_GROUP_REQ;

    write_uint16_le(start_hdl, buff, 1);
    write_uint16_le(end_hdl, buff, 3);
    write_uint16_le(groupUuid, buff, 5);

    *length = 7;
}

void EncWriteCommand(unsigned char *buff, unsigned short hdl, unsigned char data, int *length) // encode write-request PDU
{
    int i;

    buff[0] = ATT_OP_WRITE_CMD;
    write_uint16_le(hdl, buff, 1);

//    for (i = 0; i < length; i++)
//        buff[3 + i] = * (data ++);

    buff[3] = data;
    *length = 4;
}

void EncReadRequest(unsigned char *buff, unsigned short hdl, int *length) // encode read-request PDU
{
    buff[0] = ATT_OP_READ_REQ;
    write_uint16_le(hdl, buff, 1);

    *length = 3;
}

int DecReadResponse(unsigned char *buff, unsigned int *dec_buff_index, int *length) // decode read-response PDU
{
    if (buff[0] == ATT_OP_READ_RESP && *length >=2) // simple check: OP-code valid and length >= (OP-code + one attribute-octet)
    {
	// just assign buff-pointer index to start from first attribute-octet
        // (there could be '0 to (ATT_MTU–1)' octets, and if the attribute value is longer than
        // (ATT_MTU–1) then the first (ATT_MTU–1) octets shall be included in this response.
	// The Read Blob Request would be used to read the remaining octets of a long attribute value:
	// NOT supported...)

        *dec_buff_index = 1;
	(*length) --;
	return 0;
    }
    else
        return -1; // bad response op-code
}
