#include "dm4310_drv.h"
#include "fdcan.h"
#include "co_app.h"
#include "co_bsp.h"
#include "301/CO_SDOclient.h"
#include "OD.h"

extern CO_t* CO;
motor_t motor[4];

/**
************************************************************************
* @brief:      	dm4310_ctrl_send: 发送DM4310电机控制命令函数
* @param[in]:   motor: 指向motor_t结构的指针，包含电机相关信息和反馈数据
* @param[in]:   num:   电机编号
* @retval:     	void
* @details:    	根据电机控制模式发送相应的命令到DM4310电机
*               支持的控制模式包括扭矩模式、位置速度控制模式和速度控制模式
************************************************************************
**/
void dm4310_ctrl_send(motor_t *motor, uint8_t num)
{
	uint16_t s_int = motor->ctrl.mode;
	uint16_t p_int = float_to_uint(motor->ctrl.pos_set,  -P_MAX, P_MAX, 16);
	uint16_t v_int = float_to_uint(motor->ctrl.vel_set,  -V_MAX, V_MAX, 16);
	uint16_t t_int = float_to_uint(motor->ctrl.tor_set,  -T_MAX, T_MAX, 16);
	
	OD_RAM.x2000_fast_tpdo[num] = 	((uint64_t)s_int << 48) |
									((uint64_t)p_int << 32) |
									((uint64_t)v_int << 16) |
									((uint64_t)t_int << 0 );
	
		
	CO_TPDOsendRequest(&CO->TPDO[num]);
}


/**
************************************************************************
* @brief:      	dm4310_fbdata: 获取DM4310电机反馈数据函数
* @param[in]:   motor:    指向motor_t结构的指针，包含电机相关信息和反馈数据
* @param[in]:   num:   电机编号
* @retval:     	void
* @details:    	从接收到的数据中提取DM4310电机的反馈信息，包括电机ID、
*               状态、位置、速度、扭矩以及相关温度参数
************************************************************************
**/
void dm4310_fbdata(motor_t *motor, uint8_t num)
{
	motor->para.s_int = (uint16_t)(OD_RAM.x2001_fast_rpdo[num] >> 48);
	motor->para.p_int = (uint16_t)(OD_RAM.x2001_fast_rpdo[num] >> 32);
	motor->para.v_int = (uint16_t)(OD_RAM.x2001_fast_rpdo[num] >> 16);
	motor->para.t_int = (uint16_t)(OD_RAM.x2001_fast_rpdo[num] >> 0 );
	
	motor->para.id 	  = motor->para.s_int&0xFF;
	motor->para.mode  = (motor->para.s_int>>8)&0xF;
	motor->para.state = (motor->para.s_int>>12)&0xF;
	motor->para.pos = uint_to_float(motor->para.p_int, P_MIN, P_MAX, 16); // (-12.5,12.5)
	motor->para.vel = uint_to_float(motor->para.v_int, V_MIN, V_MAX, 16); // (-45.0,45.0)
	motor->para.tor = uint_to_float(motor->para.t_int, T_MIN, T_MAX, 16); // (-18.0,18.0)
}

void dm4310_enable(motor_t *motor)
{
	uint8_t data_t = 0x01;
	write_sdo(motor->id, 0x200B, 0x02, (uint8_t *)&data_t, 1);
}

void dm4310_disable(motor_t *motor)
{
	uint8_t data_t = 0x02;
	write_sdo(motor->id, 0x200B, 0x02, (uint8_t *)&data_t, 1);
}

void dm4310_setub(motor_t *motor)
{
	uint8_t data_t = 0x03;
	write_sdo(motor->id, 0x200B, 0x02, (uint8_t *)&data_t, 1);
}

void dm4310_clear_err(motor_t *motor)
{
	uint8_t data_t = 0x01;
	write_sdo(motor->id, 0x200B, 0x03, (uint8_t *)&data_t, 1);
}

void save_pos_zero(motor_t *motor)
{
	uint8_t data_t = 0x01;
	write_sdo(motor->id, 0x200B, 0x04, (uint8_t *)&data_t, 1);
}

void dm4310_save_para(motor_t *motor)
{
	uint32_t data_t = 0x00;
	write_sdo(motor->id, 0x1010, 0x01, (uint8_t *)&data_t, 4);
}

void write_htime(motor_t *motor, uint16_t h_time)
{
	write_sdo(motor->id, 0x1017, 0x00, (uint8_t *)&h_time, 2);
}

void write_canpara(motor_t *motor, uint8_t node_id, uint16_t baud)
{
	if(node_id > 127) node_id = 1;
	
	write_sdo(motor->id, 0x2003, 0x01, (uint8_t *)&node_id, 1);
	write_sdo(motor->id, 0x2003, 0x02, (uint8_t *)&baud, 2);
}

void write_currbw(motor_t *motor, uint16_t currrbw)
{
	write_sdo(motor->id, 0x2004, 0x03, (uint8_t *)&currrbw, 2);
}

void write_velpi(motor_t *motor, float kp, float ki, uint8_t damp)
{
	uint32_t kp_int = float_to_uint32(kp, 0, 1, 32);
	uint32_t ki_int = float_to_uint32(ki, 0, 1, 32);
	
	write_sdo(motor->id, 0x2005, 0x01, (uint8_t *)&kp_int, 4);
	write_sdo(motor->id, 0x2005, 0x02, (uint8_t *)&ki_int, 4);
	write_sdo(motor->id, 0x2005, 0x03, (uint8_t *)&damp, 1);
}

void write_pospi(motor_t *motor, float kp, float ki)
{
	uint32_t kp_int = float_to_uint32(kp, 0, 200, 32);
	uint32_t ki_int = float_to_uint32(ki, 0, 200, 32);
	
	write_sdo(motor->id, 0x2006, 0x01, (uint8_t *)&kp_int, 4);
	write_sdo(motor->id, 0x2006, 0x02, (uint8_t *)&ki_int, 4);
}

void write_limit(motor_t *motor, float pmax, float vmax, float tmax)
{
	uint32_t pmax_int = float_to_uint32(pmax, 0, 100, 32);
	uint32_t vmax_int = float_to_uint32(vmax, 0, 100, 32);
	uint32_t tmax_int = float_to_uint32(tmax, 0, 100, 32);
	
	write_sdo(motor->id, 0x2007, 0x01, (uint8_t *)&pmax_int, 4);
	write_sdo(motor->id, 0x2007, 0x02, (uint8_t *)&vmax_int, 4);
	write_sdo(motor->id, 0x2007, 0x03, (uint8_t *)&tmax_int, 4);
}

void write_oc(motor_t *motor, float curr)
{
	uint32_t curr_int = float_to_uint32(curr, 0, 1, 16);
	
	write_sdo(motor->id, 0x2008, 0x01, (uint8_t *)&curr_int, 16);
}

void write_ov(motor_t *motor, uint8_t ov)
{
	write_sdo(motor->id, 0x2009, 0x01, (uint8_t *)&ov, 1);
}

void write_uv(motor_t *motor, uint8_t uv)
{
	write_sdo(motor->id, 0x2009, 0x02, (uint8_t *)&uv, 1);
}

void write_ot(motor_t *motor, float temp)
{
	uint16_t temp_int = float_to_uint32(temp, -200, 200, 16);
	
	write_sdo(motor->id, 0x200A, 0x01, (uint8_t *)&temp_int, 2);
}

void write_vel(motor_t *motor, float limit_v, float acc, float dec)
{
	uint32_t limit_v_int = float_to_uint32(limit_v, 0, 3000, 32);
	uint32_t acc_int = float_to_uint32(acc, 0, 100, 32);
	uint32_t dec_int = float_to_uint32(dec, -100, 0, 32);
	
	write_sdo(motor->id, 0x6080, 0x00, (uint8_t *)&limit_v_int, 4);
	write_sdo(motor->id, 0x6083, 0x00, (uint8_t *)&acc_int, 4);
	write_sdo(motor->id, 0x6084, 0x00, (uint8_t *)&dec_int, 4);
}
/* */
void write_tpdo(motor_t *motor, uint8_t num, uint8_t bit)
{
	uint32_t data;
	
	if((num == 0) && (bit == 0))
		data = 0xC0000180 | motor->id;
	if((num == 0) && (bit == 1))
		data = 0x180 | motor->id;
	
	if((num == 1) && (bit == 0))
		data = 0xC0000280 | motor->id;
	if((num == 1) && (bit == 1))
		data = 0x280 | motor->id;
	
	if((num == 2) && (bit == 0))
		data = 0xC0000380 | motor->id;
	if((num == 2) && (bit == 1))
		data = 0x380 | motor->id;
	
	if((num == 3) && (bit == 0))
		data = 0xC0000480 | motor->id;
	if((num == 3) && (bit == 1))
		data = 0x480 | motor->id;
	
	write_sdo(motor->id, 0x1800|num, 0x01, (uint8_t *)&data, 4);
}

/**
************************************************************************
* @brief:      	float_to_uint: 浮点数转换为无符号整数函数
* @param[in]:   x_float:	待转换的浮点数
* @param[in]:   x_min:		范围最小值
* @param[in]:   x_max:		范围最大值
* @param[in]:   bits: 		目标无符号整数的位数
* @retval:     	无符号整数结果
* @details:    	将给定的浮点数 x 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个指定位数的无符号整数
************************************************************************
**/
int float_to_uint(float x_float, float x_min, float x_max, int bits)
{
	/* Converts a float to an unsigned int, given range and number of bits */
	float span = x_max - x_min;
	float offset = x_min;
	return (int) ((x_float-offset)*((float)((1<<bits)-1))/span);
}
/**
************************************************************************
* @brief:      	uint_to_float: 无符号整数转换为浮点数函数
* @param[in]:   x_int: 待转换的无符号整数
* @param[in]:   x_min: 范围最小值
* @param[in]:   x_max: 范围最大值
* @param[in]:   bits:  无符号整数的位数
* @retval:     	浮点数结果
* @details:    	将给定的无符号整数 x_int 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个浮点数
************************************************************************
**/
float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
	/* converts unsigned int to float, given range and number of bits */
	float span = x_max - x_min;
	float offset = x_min;
	return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}

/**
************************************************************************
* @brief:      	float_to_uint: 浮点数转换为无符号整数函数
* @param[in]:   x_float:	待转换的浮点数
* @param[in]:   x_min:		范围最小值
* @param[in]:   x_max:		范围最大值
* @param[in]:   bits: 		目标无符号整数的位数
* @retval:     	无符号整数结果
* @details:    	将给定的浮点数 x 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个指定位数的无符号整数
************************************************************************
**/
uint32_t float_to_uint32(float x_float, float x_min, float x_max, int bits)
{
    if (bits <= 0 || bits > 32)
        return 0; // Or handle error as needed

    float span = x_max - x_min;
    float offset = x_min;
    uint32_t max_int = (1ULL << bits) - 1;
    return (uint32_t)((x_float - offset) * max_int / span);
}
/**
************************************************************************
* @brief:      	uint_to_float: 无符号整数转换为浮点数函数
* @param[in]:   x_int: 待转换的无符号整数
* @param[in]:   x_min: 范围最小值
* @param[in]:   x_max: 范围最大值
* @param[in]:   bits:  无符号整数的位数
* @retval:     	浮点数结果
* @details:    	将给定的无符号整数 x_int 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个浮点数
************************************************************************
**/
float uint32_to_float(uint32_t x_uint, float x_min, float x_max, int bits)
{
    if (bits <= 0 || bits > 32)
        return 0.0f; // Or handle error as needed

    float span = x_max - x_min;
    uint32_t max_int = (1ULL << bits) - 1;
    return ((float)x_uint / max_int) * span + x_min;
}


