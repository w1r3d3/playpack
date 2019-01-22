#ifndef __XMODEM_H__
#define __XMODEM_H__


/////// Global Constants //////////////////////////////////////////////////////////////////////////

#define XMODEM_PACKET_SIZE  128   //128 bytes of data
#define XMODEM_HEADER_SIZE  3     //packet id, sequence number and sequence number checksum
#define XMODEM_FOOTER_SIZE  2     //attached crc16

#define XMODEM_BUF_SIZE \
  ((XMODEM_PACKET_SIZE)+(XMODEM_HEADER_SIZE)+(XMODEM_FOOTER_SIZE))

#define XMODEM_SOH    0x01
#define XMODEM_EOT    0x04
#define XMODEM_ACK    0x06
#define XMODEM_NAK    0x15
#define XMODEM_CRCCHR 'C'

#define XMODEM_PACKET_GOOD  0     //packet is valid
#define XMODEM_PACKET_BAD   1     //packet is damaged
#define XMODEM_PACKET_DUP   2     //packet is duplicated
#define XMODEM_PACKET_ERR   3     //packet receive error
#define XMODEM_PACKET_OUT   4     //packet receive timeout
#define XMODEM_PACKET_END   5     //end packet was detected
#define XMODEM_PACKET_NC    6     //no connection established

#define XMODEM_CRC_INITIALIZE 0   //adjust crc calculation
#define XMODEM_CRC_FINALIZE   0   //

#define XMODEM_MAX_RETRIES 16


/////// Function Prototypes ///////////////////////////////////////////////////////////////////////

void xmodem_init(); //init xmodem library
void xmodem_rx();   //receive file (implement function: xmodem_rx_write)

uint8 xmodem_read_getc();   //called from inside playback to directly read samples
void  xmodem_read_rewind(); //
void  xmodem_read_open();   //
void  xmodem_read_close();  //


/////// Externally Implemented Function Prototypes ////////////////////////////////////////////////

void xmodem_rx_write(uint8 *buf, uint16 size);  //gets called on every received and validated block


#endif // __XMODEM_H__
