#ifndef GATT_ATT_H
#define GATT_ATT_H

#define ATT_OP_ERROR                0x01
#define ATT_OP_FIND_INFO_REQ        0x04
#define ATT_OP_FIND_INFO_RESP       0x05
#define ATT_OP_READ_BY_TYPE_REQ     0x08
#define ATT_OP_READ_BY_TYPE_RESP    0x09
#define ATT_OP_READ_REQ             0x0a
#define ATT_OP_READ_RESP            0x0b
#define ATT_OP_READ_BY_GROUP_REQ    0x10
#define ATT_OP_READ_BY_GROUP_RESP   0x11
#define ATT_OP_WRITE_REQ            0x12
#define ATT_OP_WRITE_RESP           0x13
#define ATT_OP_HANDLE_NOTIFY        0x1b
#define ATT_OP_WRITE_CMD            0x52

unsigned short read_uint16_le(unsigned char *buff, int offset);
void write_uint16_le(unsigned short value, unsigned char *buff, int offset);
short convert_int16_le(unsigned char *buff, int offset);

int DecReadByGroupResponse(unsigned char *data, unsigned int length, unsigned short *end_hdl);
void EncReadByGroupRequest(unsigned char *buff, int *length, unsigned short start_hdl, unsigned short end_hdl, unsigned short groupUuid);
void EncWriteCommand(unsigned char *buff, unsigned short hdl, unsigned char data, int *length);
void EncReadRequest(unsigned char *buff, unsigned short hdl, int *length);
int DecReadResponse(unsigned char *buff, unsigned int *dec_buff_index, int *length);

#endif
