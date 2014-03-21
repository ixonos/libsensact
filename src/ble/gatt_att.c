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

int DecReadByTypeResponseCharacteristics(unsigned char *data, unsigned int length, unsigned short *end_hdl, struct dynamic_char_t *char_ptr)
{
    // decode characteristics services

    int i;
    int elem;
    unsigned int num;

    unsigned short start_handle, char_handle, properties;

    /* PDU must contain at least:
         * - Attribute Opcode: 1 octet
         * - type (element length): 1 octet
         * - Attribute Data List (at least one entry):
         *   - Attribute start Handle: 2 octets
         *   - Attribute property: 1 octet
         *   - Char Characteristics Handle: 2 octets
         *   - Attribute Value (uuid): at least 2 octets */

    if (data[0] != ATT_OP_READ_BY_TYPE_RESP)
        // this Attribute Opcode is wrong response type.
        // This will also be stop-condition for getting next values.
        return -1;

    elem = data[1]; // number of bytes in one element (element includes one start-handle, end-handle and uuid)

    if (length < 9 || elem < 7 || ((length - 2) % elem))
        return -1;

    num = (length - 2) / elem; // number of elements

    for (i = 0; i < num; i++) {
	start_handle = read_uint16_le(data, 2 + i * elem + 0); // 2 octets
	
	char_ptr->property = data[2 + i * elem + 2]; // 1 octet
	char_ptr->handle = read_uint16_le(data, 2 + i * elem + 3); // 2 octets

	if(elem == 0x7) // element size = 7 octets
	    // e.g. 0x9 0x7    0x2 0x0 0x2 0x3 0x0 0x0 0x2a    0x4 0x0 0x2 0x5 0x0 0x1 0x2a    0x6 0x0 0xa 0x7 0x0 0x2 0x2a
            char_ptr->UUID = read_uint16_le(data, 2 + i * elem + 5); // this element has 16-bit uuid

	else if(elem == 0x15)// element size = 0x15 = 21 octets
	    // This element has 128-bit uuid containing base-UUID and sensor specific 16-bit part. Lets capture 16-bit part.
	    // e.g. TI-specific base UIID (128 bit): TI_BASE_UUID = 0xF000xxxx04514000B000000000000000
	    // So store 'xxxx'-octets.
	    // Notice the order of the bytes (LSB first): 
	    //     e.g. "0x9 0x15   0x24 0x0 0x12 0x25 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0xb0 0x0 0x40 0x51 0x4 0x1 0xaa 0x0 0xf0"
	    //          => UUID=0xaa01
	    char_ptr->UUID = read_uint16_le(data, 2 + i * elem + (elem - 4));
	else
	    return -1; // non-supported element-size

	printf("start_hdl: 0x%x, propertys: 0x%x, char_handle: 0x%x, uuid: 0x%x\n", start_handle, char_ptr->property, char_ptr->handle, char_ptr->UUID);
	*end_hdl = start_handle + 1; // end-handle of this response will be start-handle for next request (makes it possible to get other elements)
	char_ptr++;
    }

    return num;
}

int DecReadByGroupResponsePrimary(unsigned char *data, unsigned int length, unsigned short *end_hdl) // decode primary services
{
    int i;
    int elem;
    unsigned int num;

    unsigned short start_handle, end_handle;
    unsigned int uuid;

    /* PDU must contain at least:
         * - Attribute Opcode: 1 octet
         * - type (element length): 1 octet
         * - Attribute Data List (at least one entry):
         *   - Attribute start Handle: 2 octets
         *   - End Group Handle: 2 octets
         *   - Attribute Value (uuid): at least 1 octet */

    if (data[0] != ATT_OP_READ_BY_GROUP_RESP) // this Attribute Opcode is wrong response type
        return -1;

    elem = data[1]; // number of bytes in one element (element includes one start-handle, end-handle and uuid)

    if (length < 7 || elem < 5 || ((length - 2) % elem))
        return -1;

    num = (length - 2) / elem; // number of elements

    for (i = 0; i < num; i++) {
	start_handle = read_uint16_le(data, 2 + i * elem + 0);
	end_handle = read_uint16_le(data, 2 + i * elem + 2);

	if(elem == 0x6) // element size = 6 octets
	    // e.g. 0x11 0x6    0x1 0x0 0xb 0x0 0x0 0x18     0xc 0x0 0xf 0x0 0x1 0x18     0x10 0x0 0x22 0x0 0xa 0x18
            uuid = read_uint16_le(data, 2 + i * elem + 4); // this element has 16-bit uuid

	else if(elem == 0x14)// element size = 0x14 = 20 octets
	    // This element has 128-bit uuid containing base-UUID and sensor specific 16-bit part. Lets capture 16-bit part.
	    // e.g. TI-specific base UIID (128 bit): TI_BASE_UUID = 0xF000xxxx04514000B000000000000000
	    // So store 'xxxx'-octets.
	    // Notice the order of the bytes (LSB first):
	    //         e.g. "0x11 0x14     0x39 0x0 0x43 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0xb0 0x0 0x40 0x51 0x4 0x20 0xaa 0x0 0xf0 "
	    //               => UUID=0xaa20
	    uuid = read_uint16_le(data, 2 + i * elem + (20 - 4));
	else
	    return -1; // non-supported element-size

	printf("start_hdl: 0x%x, end_handle: 0x%x, uuid: 0x%x\n", start_handle, end_handle, uuid);
    }

    *end_hdl = end_handle; // end-handle of this response will be start-handle for next request (makes it possible to get other elements)

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

void EncReadByTypeRequest(unsigned char *buff, int *length, unsigned short start_hdl, unsigned short end_hdl, unsigned short groupUuid) // encode request PDU
{
    buff[0] = ATT_OP_READ_BY_TYPE_REQ;

    write_uint16_le(start_hdl, buff, 1);
    write_uint16_le(end_hdl, buff, 3);
    write_uint16_le(groupUuid, buff, 5);

    *length = 7;
}

void EncWriteCommand(unsigned char *buff, unsigned short hdl, unsigned char *data, int *total_length, int payload_length) // encode write-request PDU
{
    // parameters:
    // - buff: data encoded in this buffer (function calling EncWriteCommand MUST take care, buffer is properly allocated)
    // - hdl: attribute character handle
    // - data: payload data (function calling EncWriteCommand MUST take care, that buffer is properly allocated)
    // - total_length: total number of octets for encoded data
    // - payload_length: total number of octets for payload data
    int i, length = 0;

    buff[0] = ATT_OP_WRITE_CMD; // write commad
    length++;

    write_uint16_le(hdl, buff, 1); // attribute-handle in little endian-format (two bytes)
    length +=2;

    // payload data
    for (i = 0; i < payload_length; i++, length++, data++)
        buff[length + i] = data[i];

    *total_length = length;
}

void EncReadRequest(unsigned char *buff, unsigned short hdl, int *length) // encode read-request PDU
{
    buff[0] = ATT_OP_READ_REQ;
    write_uint16_le(hdl, buff, 1);

    *length = 3;
}

int DecReadResponse(unsigned char *buff, unsigned int *dec_buff_index, int *length) // decode read-response PDU
{
    if (buff[0] == ATT_OP_READ_RESP && *length >=2) // simple check: OP-code valid and length >= (OP-code + one payload-octet)
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
    else if (buff[0] == ATT_OP_HANDLE_NOTIFY && *length >=4) // simple check: OP-code valid and length >= (OP-code + 2-octet handle + one payload-octet)
    {
        printf("--------------------Notification-----------------\n");
	// For notifications, also attribute-character handle is received. So correct response message-handle could be selected.
	// BUT: NOT supported yet! Just skip the handle.
        *dec_buff_index = 3;
	*length -=3;
        return 0;
    }
    else
        return -1; // bad response op-code
}
