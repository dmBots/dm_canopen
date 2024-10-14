#include "dm4310_drv.h"
#include "fdcan.h"
#include "co_app.h"
#include "co_bsp.h"
#include "301/CO_SDOclient.h"
#include "OD.h"

extern CO_t* CO;
motor_t motor[4];



void dm4310_ctrl_send(motor_t *motor)
{
	write_od_16(0x2000, 0x01, motor->ctrl.word);
	write_od_16(0x2000, 0x03, motor->ctrl.mode);
	write_od_f(0x2000, 0x05, motor->ctrl.tor_set);
	write_od_f(0x2000, 0x06, motor->ctrl.vel_set);
	write_od_f(0x2000, 0x07, motor->ctrl.pos_set);
		
	CO_TPDOsendRequest(&CO->TPDO[0]);
	CO_TPDOsendRequest(&CO->TPDO[1]);
}



void dm4310_fbdata(motor_t *motor)
{
	motor->para.status_word = read_od_16(0x2000, 0x02);
	motor->para.mode = read_od_16(0x2000, 0x04);
	
	motor->para.tor = read_od_f(0x2000, 0x08);
	motor->para.vel = read_od_f(0x2000, 0x09);
	motor->para.pos = read_od_f(0x2000, 0x0A);
}

void open_tpdo(motor_t *motor)
{
	uint32_t val1 = 0x181;
	uint32_t val2 = 0x281;
	
	write_sdo(motor->id, 0x1800, 0x01, (uint8_t *)&val1, 4);
	write_sdo(motor->id, 0x1801, 0x01, (uint8_t *)&val2, 4);
}



static uint32_t nullvar;
uint32_t *get_od_addr(uint16_t mainindex, uint16_t subindex)
{
	void *ptr = &nullvar;
	switch (mainindex)
    {
		case 0x2000:
			switch (subindex)
			{
				case 0x05: ptr = &(OD_RAM.x2001_M1.tor_set); break;
				case 0x06: ptr = &(OD_RAM.x2001_M1.vel_set); break;
				case 0x07: ptr = &(OD_RAM.x2001_M1.pos_set); break;
				case 0x08: ptr = &(OD_RAM.x2001_M1.tor); 	 break;
				case 0x09: ptr = &(OD_RAM.x2001_M1.vel);     break;
				case 0x0A: ptr = &(OD_RAM.x2001_M1.pos);   	 break;
			}
			break;
    }
	return ptr;
}

void write_od_f(uint16_t mainindex, uint16_t subindex, float data)
{
	data_u d;
	
	d.f_val = data;
	*get_od_addr(mainindex, subindex) = d.u32_val;
}

float read_od_f(uint16_t mainindex, uint16_t subindex)
{
	data_u d;
	
	d.u32_val = *get_od_addr(mainindex, subindex);
	return d.f_val;
}

void write_od_u32(uint16_t mainindex, uint16_t subindex, uint32_t data)
{
	*get_od_addr(mainindex, subindex) = data;
}

static uint16_t nullvar16;
uint16_t *get_od_16addr(uint16_t mainindex, uint16_t subindex)
{
	void *ptr = &nullvar16;
	switch (mainindex)
    {
		case 0x2000:
			switch (subindex)
			{
				case 0x01: ptr = &(OD_RAM.x2001_M1.control_word); 			break;
				case 0x02: ptr = &(OD_RAM.x2001_M1.status_word); 			break;
				case 0x03: ptr = &(OD_RAM.x2001_M1.mode_operation); 		break;
				case 0x04: ptr = &(OD_RAM.x2001_M1.mode_operation_display); break;
			}
			break;
	}
	return ptr;
}

void write_od_16(uint16_t mainindex, uint16_t subindex, uint16_t data)
{
	*get_od_16addr(mainindex, subindex) = (uint16_t)(data);
}

int16_t read_od_16(uint16_t mainindex, uint16_t subindex)
{
	uint16_t temp = *get_od_16addr(mainindex, subindex);
	return ((int16_t)temp);
}

