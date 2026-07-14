
#ifndef DM02_TEST_IMUTASK_H
#define DM02_TEST_IMUTASK_H

/**
  ******************************************************************************
  * @FileName			    IMUTask.h
  * @Description            this file contains the common defines and functions prototypes for
 *             				the IMUTask.cpp driver
  * @author                 Zang Chenrui
  * @note
  ******************************************************************************
  *
  * Copyright (c) 2026 Team TsingYun-Tsinghua University
  * All rights reserved.
  *
  ******************************************************************************
**/

#include <stdint.h>
#include <sys/cdefs.h>
#include "stm32h7xx_hal.h"


#define IMU_UART                huart10

/**
 * @brief IMU HI91数据帧定义
 */
typedef  struct
{
    uint8_t frame_header_1; ///< 0x5A
    uint8_t frame_header_2; ///< 0xA5
    uint16_t payload_length; ///< 76
    uint16_t CRC16;

    uint8_t tag; ///< 0x91
    uint16_t main_status;
    int8_t temperature; ///< 温度 (°C)
    float air_pressure; ///< 气压 (Pa)
    uint32_t system_time; ///< 时间戳 (ms)
    float acc_b_X; ///< 加速度X (G)
    float acc_b_Y; ///< 加速度Y (G)
    float acc_b_Z; ///< 加速度Z (G)
    float gyr_b_X; ///< 角速度X (deg/s)
    float gyr_b_Y; ///< 角速度Y (deg/s)
    float gyr_b_Z; ///< 角速度Z (deg/s)
    float mag_b_X; ///< 磁强度X (μT)
    float mag_b_Y; ///< 磁强度Y (μT)
    float mag_b_Z; ///< 磁强度Z (μT)
    float roll; ///< 横滚角 (deg)
    float pitch; ///< 俯仰角 (deg)
    float yaw; ///< 航向角 (deg)
    float quat_W; ///< 四元数W
    float quat_X; ///< 四元数X
    float quat_Y; ///< 四元数Y
    float quat_Z; ///< 四元数Z
} IMU_HI91_t;

void IMU_CRC16_Update(uint16_t *currectCrc, const uint8_t *src, uint32_t lengthInBytes); ///< CRC16计算函数 (IMU厂商提供)

constexpr size_t IMU_RX_LEN = 82; ///< IMU数据接收长度 (与数据帧长度相同)
constexpr size_t IMU_WATCHDOG_THRESHOLD = 200; ///< IMU看门狗阈值 (超过该次数主循环未接收到数据，将被重置)

typedef struct
{
	float ax;
	float ay;
	float az;

	// float mx;
	// float my;
	// float mz;

	// float temp;

	float wx;
	float wy;
	float wz;

	// float vx;
	// float vy;
	// float vz;

	float rol;
	float pit;
	float yaw;

	float negative_wx;
	float negative_wy;
	float negative_wz;

	// float negative_vx;
	// float negative_vy;
	// float negative_vz;

	float negative_rol;
	float negative_pit;
	float negative_yaw;

	// uint8_t FirstEnter;
	// uint16_t InitCount;
	// uint8_t InitFinish;
} imu_t;

typedef enum
{
    wx,wy,wz,vx,vy,vz,rol,pit,yaw,negative_wx,negative_wy,negative_wz,negative_vx,negative_vy,negative_vz,negative_rol,negative_pit,negative_yaw
} DataName_e;

/**
 * @brief IMU类
 */
class IMU
{
private:
    uint8_t rx_data[IMU_RX_LEN];
    uint8_t rx_data_copy[IMU_RX_LEN];

    bool rx_available;
    uint32_t rc_count;
    bool online_flag; ///< IMU在线状态

    bool Process(uint8_t *data);
    void ProcessData();

public:
    IMU_HI91_t Data; ///< 原始数据
    imu_t ProcessedData; ///< 处理后的数据

    void Reset(); ///< 重置IMU
    void Handle(); ///< IMU主循环函数
    void UartRxCpltCallback(uint8_t *Buffer, uint16_t Length); ///< UART Rx回调函数
    bool Online() const; ///< 判断IMU是否在线

    const imu_t &GetData(); ///< 获得处理后的数据
    const float *GetpData(DataName_e data); ///< 获得指向处理后数据的指针
};

extern IMU imu; ///< IMU全局静态对象

#endif //DM02_TEST_IMUTASK_H