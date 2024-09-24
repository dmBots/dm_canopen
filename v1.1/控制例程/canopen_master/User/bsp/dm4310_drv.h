#ifndef __DM4310_DRV_H__
#define __DM4310_DRV_H__
#include "main.h"
#include "fdcan.h"
#include "can_bsp.h"


typedef union
{
	float f_val;
	uint32_t u32_val;
	int32_t i32_val;
	uint8_t u8_val[4];
} data_u;

typedef enum
{
	M1,
	M2,
	M3,
	M4,
	num
} motor_num;


// 电机回传信息结构体
typedef struct 
{
	uint16_t status_word;
	uint16_t mode;
	
	float pos;
	float vel;
	float tor;
	
	float Tmos;
	float Tcoil;
}motor_fbpara_t;

// 电机参数设置结构体
typedef struct 
{
	uint16_t word;
	uint16_t mode;
	
	float pos_set;
	float vel_set;
	float tor_set;

}motor_ctrl_t;

typedef struct
{
	uint8_t id;
	motor_fbpara_t para;
	motor_ctrl_t ctrl;
	motor_ctrl_t cmd;
}motor_t;

extern motor_t motor[4];

void dm4310_ctrl_send(motor_t *motor);
void dm4310_fbdata(motor_t *motor);

void open_tpdo(motor_t *motor);

void write_od_f(uint16_t mainindex, uint16_t subindex, float data);
float read_od_f(uint16_t mainindex, uint16_t subindex);
void write_od_u32(uint16_t mainindex, uint16_t subindex, uint32_t data);
uint16_t *get_od_16addr(uint16_t mainindex, uint16_t subindex);
void write_od_16(uint16_t mainindex, uint16_t subindex, uint16_t data);
int16_t read_od_16(uint16_t mainindex, uint16_t subindex);

#endif /* __DM4310_DRV_H__ */









