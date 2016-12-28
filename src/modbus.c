#include "modbus.h"
#include "algblock.h"



uint8_t Chk_mb_packet(uint8_t* buf, uint8_t type, uint8_t addr, uint8_t ln)
{
	uint16_t crc;
	
	if (buf[0] != addr)
	{
		return 1;
	}
	crc = Chk_CRC(buf,ln-2);
	if ((buf[ln-2]!= (uint8_t)(crc & 0xff))||(buf[ln-1]!=(crc >> 8)))
	{
		return 2;
	}
	
	//Check from ad16
	if (type==1){
		if ((buf[1]!=0x04) || (buf[2]!=0x10))
			return 3;
	}
	//Check from ai200
	if (type==2){
		if ((buf[1]!=0x43) || (buf[2]!=0) || (buf[3]!=0x18))
			return 3;
	}
	//Check from dd16
	if (type==3){
		if ((buf[1]!=0x02) || (buf[2]!=0x10))
			return 3;
	}
	//Check from do16
	if (type==11){
		if ((buf[1]!=0x51) || (buf[2]!=0x10))
			return 3;
	}
	return 0;
}




uint8_t Create_mb_packet(uint8_t* buf, uint8_t type, uint8_t addr)
{
	uint16_t crc;
	
	buf[0]=addr;
	//ad16
	if (type==1)
	{
		buf[1]=0x04;buf[2]=0;buf[3]=0;buf[4]=0x10;
		crc = Chk_CRC(buf,5);
		buf[5] = crc & 0xff;
		buf[6] = crc >> 8;
		return 7;
	}
	//ai200
	if (type==2)
	{
		buf[1]=0x43;buf[2]=0;buf[3]=0;buf[4]=0x02;buf[5]=0x18;
		crc = Chk_CRC(buf,6);
		buf[6] = crc & 0xff;
		buf[7] = crc >> 8;
		return 8;
	}
	//dd16
  if (type==3)
	{
		buf[1]=0x02;buf[2]=0;buf[3]=0;buf[4]=0x10;
		crc = Chk_CRC(buf,5);
		buf[5] = crc & 0xff;
		buf[6] = crc >> 8;
		return 7;
	}
	//do16
	if (type==11)
	{
		buf[1]=0x51;buf[2]=0;buf[3]=0;buf[4]=0x10;
		crc = Chk_CRC(buf,39);
		buf[39] = crc & 0xff;
		buf[40] = crc >> 8;
		return 41;
	}
	
	return 0;
}


uint16_t  Chk_CRC(uint8_t* buf, uint8_t len)
{
	volatile uint16_t crc = 0xffff;
	volatile uint16_t pos,i,d;
	
	for (pos = 0; pos < len; pos++)
  {
		d = 0x006f ^ 0x00;
		crc =crc ^ buf[pos];          // XOR byte into least sig. byte of crc
		for (i = 8; i != 0; i--)
		{    // Loop over each bit
			if ((crc & 0x0001) != 0)
      {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
      }
			else                            // Else LSB is not set
				crc >>= 1;                    // Just shift right
    }
	}
	return crc;
}
