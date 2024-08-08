#ifndef __DM4310_DRV_H__
#define __DM4310_DRV_H__
#include "main.h"
#include "fdcan.h"
#include "can_bsp.h"

#define P_MIN -12.5f
#define P_MAX 12.5f
#define V_MIN -30.0f
#define V_MAX 30.0f
#define KP_MIN 0.0f
#define KP_MAX 500.0f
#define KD_MIN 0.0f
#define KD_MAX 5.0f
#define T_MIN -10.0f
#define T_MAX 10.0f

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
	uint8_t id;
	uint8_t mode;
	uint8_t state;
	uint16_t s_int;
	uint16_t p_int;
	uint16_t v_int;
	uint16_t t_int;
	int kp_int;
	int kd_int;
	float pos;
	float vel;
	float tor;
	float Kp;
	float Kd;
	float Tmos;
	float Tcoil;
}motor_fbpara_t;

// 电机参数设置结构体
typedef struct 
{
	int8_t mode;
	float pos_set;
	float vel_set;
	float tor_set;
	float kp_set;
	float kd_set;
}motor_ctrl_t;

typedef struct
{
	uint8_t id;
	motor_fbpara_t para;
	motor_ctrl_t ctrl;
	motor_ctrl_t cmd;
}motor_t;

extern motor_t motor[4];

float uint_to_float(int x_int, float x_min, float x_max, int bits);
int float_to_uint(float x_float, float x_min, float x_max, int bits);

float uint32_to_float(uint32_t x_uint, float x_min, float x_max, int bits);
uint32_t float_to_uint32(float x_float, float x_min, float x_max, int bits);

void dm4310_enable(motor_t *motor);
void dm4310_disable(motor_t *motor);
void dm4310_setub(motor_t *motor);
void dm4310_clear_err(motor_t *motor);
void save_pos_zero(motor_t *motor);
void dm4310_save_para(motor_t *motor);
void write_htime(motor_t *motor, uint16_t h_time);
void write_canpara(motor_t *motor, uint8_t node_id, uint16_t baud);
void write_currbw(motor_t *motor, uint16_t currrbw);
void write_velpi(motor_t *motor, float kp, float ki, uint8_t damp);
void write_pospi(motor_t *motor, float kp, float ki);
void write_limit(motor_t *motor, float pmax, float vmax, float tmax);
void write_oc(motor_t *motor, float curr);
void write_ov(motor_t *motor, uint8_t ov);
void write_uv(motor_t *motor, uint8_t uv);
void write_ot(motor_t *motor, float temp);
void write_vel(motor_t *motor, float limit_v, float acc, float dec);
void write_tpdo(motor_t *motor, uint8_t num, uint8_t bit);

void dm4310_ctrl_send(motor_t *motor, uint8_t num);


void dm4310_fbdata(motor_t *motor, uint8_t num);

#endif /* __DM4310_DRV_H__ */









