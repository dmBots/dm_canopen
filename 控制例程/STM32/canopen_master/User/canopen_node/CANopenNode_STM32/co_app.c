#include "co_app.h"
#include "co_bsp.h"
#include "301/CO_SDOclient.h"
#include "OD.h"
#include "fdcan.h"
#include "tim.h"
#include "delay.h"
#include "stdio.h"
#include "dm4310_drv.h"

canopen_node_t canopen_node;


extern CO_t* CO;

/* control and rx data processs  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == canopenNodeSTM32->timerHandle) {
		canopen_app_interrupt();
	}
	if (htim == &htim4) {
	
		dm4310_ctrl_send(&motor[M1]);
		dm4310_fbdata(&motor[M1]);

	}
}

/* para init  */
void canopen_init(void)
{
	uint32_t time;

	/* 初始化CANopen */
	canopen_node.CANHandle = &hfdcan1;              /* 使用CANFD1接口 */
	canopen_node.HWInitFunction = MX_FDCAN1_Init;   /* 初始化CANFD1 */
	canopen_node.timerHandle = &htim3;              /* 使用的定时器句柄 */
	canopen_node.desiredNodeID = 0x1A;                /* Node-ID */
	canopen_node.baudrate = 1000;                    /* 波特率，单位KHz */
	canopen_app_init(&canopen_node);
	HAL_TIM_Base_Start_IT(&htim4);	
	
	motor[M1].id = 1;
	motor[M1].ctrl.word = 0x0F;
	motor[M1].ctrl.mode = 0x02;
	motor[M1].ctrl.pos_set = 1000.0f;
	motor[M1].ctrl.vel_set = 10.0f;
	motor[M1].ctrl.tor_set = 0.0f;
}

/* read sdo data */
CO_SDO_abortCode_t read_sdo(uint8_t nodeId, uint16_t index, uint8_t subIndex, uint8_t *buf, size_t bufSize, size_t *readSize)
{
    CO_SDO_return_t SDO_ret;

    // setup client (this can be skipped, if remote device don't change)
    SDO_ret = CO_SDOclient_setup(CO->SDOclient,
                                 CO_CAN_ID_SDO_CLI + nodeId,
                                 CO_CAN_ID_SDO_SRV + nodeId,
                                 nodeId);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd)
    {
        return CO_SDO_AB_GENERAL;
    }

    // initiate upload
    SDO_ret = CO_SDOclientUploadInitiate(CO->SDOclient, index, subIndex, 1000, false);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd)
    {
        return CO_SDO_AB_GENERAL;
    }

    // upload data
    do
    {
        uint32_t timeDifference_us = 10000;
        CO_SDO_abortCode_t abortCode = CO_SDO_AB_NONE;

        SDO_ret = CO_SDOclientUpload(CO->SDOclient,
                                     timeDifference_us,
                                     false,
                                     &abortCode,
                                     NULL, NULL, NULL);
        if (SDO_ret < 0)
        {
            return abortCode;
        }

        HAL_Delay(10);
    } while (SDO_ret > 0);

    *readSize = CO_SDOclientUploadBufRead(CO->SDOclient, buf, bufSize);

    return CO_SDO_AB_NONE;
}

/* write sdo data */
CO_SDO_abortCode_t write_sdo(uint8_t nodeId, uint16_t index, uint8_t subIndex, uint8_t *data, size_t dataSize)
{
    CO_SDO_return_t SDO_ret;
    bool_t bufferPartial = false;

    // setup client (this can be skipped, if remote device is the same)
    SDO_ret = CO_SDOclient_setup(CO->SDOclient,
                                 CO_CAN_ID_SDO_CLI + nodeId,
                                 CO_CAN_ID_SDO_SRV + nodeId,
                                 nodeId);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd)
    {
        return -1;
    }

    // initiate download
    SDO_ret = CO_SDOclientDownloadInitiate(CO->SDOclient, index, subIndex,
                                           dataSize, 1000, false);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd)
    {
        return -1;
    }

    // fill data
    size_t nWritten = CO_SDOclientDownloadBufWrite(CO->SDOclient, data, dataSize);
    if (nWritten < dataSize)
    {
        bufferPartial = true;
        // If SDO Fifo buffer is too small, data can be refilled in the loop.
    }

    // download data
    do
    {
        uint32_t timeDifference_us = 10000;
        CO_SDO_abortCode_t abortCode = CO_SDO_AB_NONE;

        SDO_ret = CO_SDOclientDownload(CO->SDOclient,
                                       timeDifference_us,
                                       false,
                                       bufferPartial,
                                       &abortCode,
                                       NULL, NULL);
        if (SDO_ret < 0)
        {
            return abortCode;
        }

        HAL_Delay(10);
    } while (SDO_ret > 0);

    return CO_SDO_AB_NONE;
}


/* Timestamp, not currently used */
const uint8_t mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static uint8_t isleap_year(uint16_t _year)
{
    if (_year % 4 == 0) 
    {
        if (_year % 100 == 0)
        {
            if (_year % 400 == 0)
                return 1; 
            else
                return 0;
        }
        else
            return 1;
    }
    else
        return 0;
}

uint8_t canopen_write_clock(uint16_t _year, uint8_t _mon, uint8_t _day, uint8_t _hour, uint8_t _min, uint8_t _sec, uint32_t _ms, uint32_t _Interval_ms)
{
    uint16_t t;
    uint32_t seccount = 0;
    uint32_t days;

    if (_year < 2000 || _year > 2099)
        return 0; /* _year范围1970-2099，此处设置范围为2000-2099 */

    for (t = 1984; t < _year; t++) /* 把所有年份的秒钟相加 */
    {
        if (isleap_year(t))       /* 判断是否为闰年 */
            seccount += 31622400; /* 闰年的秒钟数 */
        else
            seccount += 31536000; /* 平年的秒钟数 */
    }

    _mon -= 1;

    for (t = 0; t < _mon; t++) /* 把前面月份的秒钟数相加 */
    {
        seccount += (uint32_t)mon_table[t] * 86400; /* 月份秒钟数相加 */

        if (isleap_year(_year) && t == 1)
            seccount += 86400; /* 闰年2月份增加一天的秒钟数 */
    }

    seccount += (uint32_t)(_day - 1) * 86400; /* 把前面日期的秒钟数相加 */
    seccount += (uint32_t)_hour * 3600; /* 小时秒钟数 */
    seccount += (uint32_t)_min * 60; /* 分钟秒钟数 */
    seccount += _sec; /* 最后的秒钟加上去 */
    _ms = _ms + (seccount % (60 * 60 * 24)) * 1000;
    days = seccount / (60 * 60 * 24);
    CO_TIME_set(CO->TIME, _ms, days, _Interval_ms);

    return 1;
}
