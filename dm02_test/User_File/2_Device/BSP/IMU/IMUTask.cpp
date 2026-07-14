/**
  ******************************************************************************
  * @FileName			    IMUTask.cpp
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

#include "IMUTask.h"
#include <string.h>

/**
 * @brief 重置IMU
 * @details 重置状态 & 重新开启UART接收
 */
void IMU::Reset()
{

    rx_available = false;
    rc_count = 0;
    online_flag = false;

}

/**
 * @brief IMU主循环函数
 * @details 处理数据 & 看门狗
 */
void IMU::Handle()
{
    if (rx_available)
    {
        rx_available = false;

        if (Process(rx_data_copy))
        {
            online_flag = true;
            rc_count = 0;
        }
    }

    // 看门狗
		rc_count++;
    if (rc_count > IMU_WATCHDOG_THRESHOLD)
    {
        Reset();
    }
}

/**
 * @brief UART Rx回调函数
 * @details 拷贝数据 & 重新开启接收
 */
void IMU::UartRxCpltCallback(uint8_t *Buffer, uint16_t Length)
{
    rx_available = true;

    // 由于DMA直接对内存写入，因此这里先将内存里的数据写入缓存
    // SCB_InvalidateDCache_by_Addr((uint32_t *)rx_data, IMU_RX_LEN);
    // memcpy(rx_data_copy, rx_data, IMU_RX_LEN);
    // SCB_CleanDCache_by_Addr((uint32_t *)rx_data, IMU_RX_LEN); // 维护缓存同步
}

/**
 * @brief 解析IMU数据帧
 * @return 是否成功解析
 * @details 验证帧格式 & 验证CRC16 & 拷贝数据
 */
bool IMU::Process(uint8_t *data)
{
    const IMU_HI91_t *frame = (IMU_HI91_t *)data;

    // 验证帧格式正确性
    bool header_correct = frame->frame_header_1 == 0x5A && frame->frame_header_2 == 0xA5;
    bool length_correct = frame->payload_length == 76;
    bool tag_correct = frame->tag == 0x91;
    if (!(header_correct && length_correct && tag_correct))
    {
        return false;
    }

    // 验证全帧(不含CRC16自身)的CRC16校验值
    uint16_t actual_CRC16 = 0;
    IMU_CRC16_Update(&actual_CRC16, data, 4);
    IMU_CRC16_Update(&actual_CRC16, data + 6, frame->payload_length);
    if (actual_CRC16 != frame->CRC16)
    {
        return false;
    }

    // 拷贝数据
    memcpy(&Data, data, IMU_RX_LEN);

    ProcessData();

    return true;
}

/**
 * @brief 判断IMU是否在线
 */
bool IMU::Online() const
{
#ifdef ENABLE_IMU
    return online_flag;
#else
    return false;
#endif
}

/**
 * @brief CRC16计算函数 (IMU厂商提供)
 */
void IMU_CRC16_Update(uint16_t *currectCrc, const uint8_t *src, uint32_t lengthInBytes)
{
    uint32_t crc = *currectCrc;
    uint32_t j;
    for (j = 0; j < lengthInBytes; ++j)
    {
        uint32_t i;
        uint32_t byte = src[j];
        crc ^= byte << 8;
        for (i = 0; i < 8; ++i)
        {
            uint32_t temp = crc << 1;
            if (crc & 0x8000)
            {
                temp ^= 0x1021;
            }
            crc = temp;
        }
    }
    *currectCrc = crc;
}

/**
 * @brief 处理数据
 */
void IMU::ProcessData()
{
    ProcessedData.ax = Data.acc_b_X;
    ProcessedData.ay = Data.acc_b_Y;
    ProcessedData.az = Data.acc_b_Z;

    ProcessedData.wx = Data.gyr_b_X;
    ProcessedData.wy = Data.gyr_b_Y;
    ProcessedData.wz = Data.gyr_b_Z;

    ProcessedData.yaw = Data.yaw;
    ProcessedData.pit = Data.pitch;
    ProcessedData.rol = Data.roll;

    ProcessedData.negative_yaw = -ProcessedData.yaw;
    ProcessedData.negative_pit = -ProcessedData.pit;
    ProcessedData.negative_rol = -ProcessedData.rol;

    // ProcessedData.negative_vx = -ProcessedData.vx;
    // ProcessedData.negative_vy = -ProcessedData.vy;
    // ProcessedData.negative_vz = -ProcessedData.vz;

    ProcessedData.negative_wx = -ProcessedData.wx;
    ProcessedData.negative_wy = -ProcessedData.wy;
    ProcessedData.negative_wz = -ProcessedData.wz;
}

/**
 * @brief 获得处理后的数据
 */
const imu_t &IMU::GetData()
{
    return ProcessedData;
}

/**
 * @brief 获得指向处理后数据的指针
 */
const float *IMU::GetpData(DataName_e data)
{
    switch(data)
    {
        case wx: return &(ProcessedData.wx); break;
        case wy: return &(ProcessedData.wy); break;
        case wz: return &(ProcessedData.wz); break;
        // case vx: return &(ProcessedData.vx); break;
        // case vy: return &(ProcessedData.vy); break;
        // case vz: return &(ProcessedData.vz); break;
        case rol: return &(ProcessedData.rol); break;
        case yaw: return &(ProcessedData.yaw); break;
        case pit: return &(ProcessedData.pit); break;

        case negative_wz: { return &(ProcessedData.negative_wz); break;}
        case negative_wy: { return &(ProcessedData.negative_wy); break;}
        case negative_yaw: { return &(ProcessedData.negative_yaw); break;}
        case negative_pit: { return &(ProcessedData.negative_pit); break;}

        default: return nullptr; break;
    }
}

IMU imu;
