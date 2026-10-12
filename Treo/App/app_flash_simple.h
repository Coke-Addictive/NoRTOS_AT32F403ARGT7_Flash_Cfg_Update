/**
 * @file app_flash_simple.h
 * @brief 固定容量用户配置结构及 Flash 加载、保存、复位和打印接口。
 * @author Treo
 * @version 1.0.0 (V1 配置格式)
 * @date 2026-10-12
 *
 * V1：配置共 512 字节，有效字段 63 字节，预留 449 字节，按 4 字节对齐。
 * CRC32 只计算完整 payload，包含预留区。
 *
 * 后续扩展：
 * 1. 修改配置结构体（Device_CFG_Params_t）：在 Offset 前追加字段，保持已发布字段的类型、长度、顺序和含义不变。
 * 2. 缩减宏 DEVICE_CONFIG_RESERVED_SIZE ：计入对齐填充，确保结构体仍为 512 字节。
 * 3. 增加宏 DEVICE_CONFIG_SCHEMA_VERSION 版本号：在 app_flash_simple.c 中递增 ，载荷大小保持 512U。（还够用的话）
 * 4. 补充默认值：DEFAULT_DEVICE_CFG
 * 5. 补充打印函数：App_Flash_DeviceConfig_Prf
 * 6. 更新这里的注释
 * 
 * 有注意：
 * 效长度由 offsetof 自动计算；参数检查暂留 TODO，后续同步补充。
 * 升级时保留旧字段，新字段使用新版默认值；保持原有端序、布局和 CRC 算法。
 * 旧固件遇到新版记录会写回默认配置，不支持降级保留配置。
 */
#ifndef APP_FLASH_SIMPLE_H_
#define APP_FLASH_SIMPLE_H_

#include "stdint.h"

#define DEVICE_CONFIG_REMOTE_ID_BUFFER_SIZE             6U              // 5位遥控器ID和字符串结束符
#define DEVICE_CONFIG_RESERVED_SIZE                     449U            // 后续版本扩展预留空间，单位字节


/**
 * @brief 设备运行配置。
 * @note 结构体固定为 512 字节，按 4 字节对齐，后续增加字段时须相应缩减 Offset 预留区，不能调整已有字段顺序。
 */
typedef struct __attribute__((aligned(4))) {
    uint8_t Device_Hard_SW;                             // 系统硬开关标志位
    uint8_t Device_Soft_SW;                             // 系统软开关标志位
    uint8_t Device_Silent_SW;                           // 系统静默开关标志位（什么功能都不起作用，只存活）

    uint8_t Device_Night_Mute_SW;                       // 夜间静音 - 开关
    uint8_t Device_Night_Mute_Start_Hour;               // 夜静开始时间-小时
    uint8_t Device_Night_Mute_Start_Minute;             // 夜静开始时间-分钟
    uint8_t Device_Night_Mute_End_Hour;                 // 夜静结束时间-小时
    uint8_t Device_Night_Mute_End_Minute;               // 夜静结束时间-分钟

    uint8_t Device_Fragment_Mute_SW;                    // 时间段静音 - 总开关
    uint8_t Device_Fragment_Mute_1_SW;                  // 时间段1静音 - 开关
    uint8_t Device_Fragment_Mute_1_Start_Hour;          // 时间段1开始时间-小时
    uint8_t Device_Fragment_Mute_1_Start_Minute;        // 时间段1开始时间-分钟
    uint8_t Device_Fragment_Mute_1_End_Hour;            // 时间段1结束时间-小时
    uint8_t Device_Fragment_Mute_1_End_Minute;          // 时间段1结束时间-分钟

    uint8_t Device_Fragment_Mute_2_SW;                  // 时间段2静音 - 开关
    uint8_t Device_Fragment_Mute_2_Start_Hour;          // 时间段2开始时间-小时
    uint8_t Device_Fragment_Mute_2_Start_Minute;        // 时间段2开始时间-分钟
    uint8_t Device_Fragment_Mute_2_End_Hour;            // 时间段2结束时间-小时
    uint8_t Device_Fragment_Mute_2_End_Minute;          // 时间段2结束时间-分钟

    uint8_t Device_Fragment_Mute_3_SW;                  // 时间段3静音 - 开关
    uint8_t Device_Fragment_Mute_3_Start_Hour;          // 时间段3开始时间-小时
    uint8_t Device_Fragment_Mute_3_Start_Minute;        // 时间段3开始时间-分钟
    uint8_t Device_Fragment_Mute_3_End_Hour;            // 时间段3结束时间-小时
    uint8_t Device_Fragment_Mute_3_End_Minute;          // 时间段3结束时间-分钟

    uint8_t Device_Ultrasonic_SW;                       // 超声波 - 开关
    uint8_t Device_Ultrasonic_Mode;                     // 超声波模式（正常/特殊）（对应阶梯/扫频）
    uint8_t Device_Ultrasonic_Start_Hz;                 // 超声波开始频率
    uint8_t Device_Ultrasonic_End_Hz;                   // 超声波结束频率
    uint8_t Device_Ultrasonic_Schedule_SW;              // 超声波定时 - 开关
    uint8_t Device_Ultrasonic_Schedule_Span;            // 超声波定时跨度，分钟

    uint8_t Device_Simulated_SW;                        // 模拟声 - 开关
    uint8_t Device_Simulated_Mode;                      // 模拟声模式(1.顺序  2.随机)
    uint8_t Device_Simulated_Select_Number_Max;         // 模拟声音频最多选取数量
    uint8_t Device_Simulated_Schedule_SW;               // 模拟声定时 - 开关
    uint8_t Device_Simulated_Schedule_Span;             // 模拟声定时跨度，分钟

    uint8_t Device_R_B_LED_SW;                          // 红蓝光 - 开关
    uint8_t Device_R_B_LED_Mode;                        // 红蓝光模式(正常/特殊)(对应交替闪烁/同频闪烁)
    uint8_t Device_R_B_LED_Frequency;                   // 红蓝光频率
    uint8_t Device_R_B_LED_Schedule_SW;                 // 红蓝光定时 - 开关
    uint8_t Device_R_B_LED_Schedule_Span;               // 红蓝光定时跨度，分钟

    uint8_t Device_Laser_SW;                            // 激光 - 开关
    uint8_t Device_Laser_Mode;                          // 激光模式（正常/特殊）（对应轮询/花样预设数组）
    uint8_t Device_Laser_Frequency;                     // 激光频率
    uint8_t Device_Laser_Schedule_SW;                   // 激光定时 - 开关
    uint8_t Device_Laser_Schedule_Span;                 // 激光定时跨度，分钟

    uint8_t Device_Soft_Schedule_SW;                    // 软定时 - 开关
    uint8_t Device_Soft_Schedule_Span;                  // 软定时跨度，分钟

    uint8_t Device_Hard_Schedule_Mode;                  // 硬定时
    uint8_t Device_Hard_Schedule_Span;                  // 硬定时跨度，分钟

    uint8_t Device_Logic_Volume;                        // 逻辑音量等级：0关、1低、2中、3高、4最大
    uint8_t Device_Real_Volume;                         // 真实物理音量值：

    uint8_t Device_Cooling_Mode;                        // 设备冷却模式(固定/递增)
    uint8_t Device_Cooling_Initial_Duration;            // 设备初始冷却时间，分钟
    uint8_t Device_Cooling_Increase_Time;               // 设备连续触发递增冷却时间，分钟

    uint8_t Device_Location_SW;                         // 系统定位开关
    uint8_t Device_Location_Report_SW;                  // 系统定位上报开关
    uint8_t Device_Location_Frequency;                  // 系统定位频率，小时

    uint8_t Device_R5a_Id[DEVICE_CONFIG_REMOTE_ID_BUFFER_SIZE]; // 5位遥控器ID和结束符

    uint8_t Offset[DEVICE_CONFIG_RESERVED_SIZE]; // 后续版本扩展预留字段
} Device_CFG_Params_t;



int32_t App_Flash_Init(void);









//----------------------------------------用户配置相关-----------------------------------------
int32_t App_Flash_DeviceConfig_Load(void);
int32_t App_Flash_DeviceConfig_Get(Device_CFG_Params_t *config);
int32_t App_Flash_DeviceConfig_Save(const Device_CFG_Params_t *config);
int32_t App_Flash_DeviceConfig_Reset(void);
void App_Flash_DeviceConfig_Prf(void);








#endif // APP_FLASH_SIMPLE_H_




// 版本更新记录
/*
//-------------------------------------V1.0.0--------------------------------------
当前问题：堆设置2048字节（2K=0X800） 栈给了4096字节（4K=0X1000）。主动扩大了
因为App_Flash_DeviceConfig_Load 和 Device_Config_Record_Write这两个函数内部都定义了很大的
临时DeviceConfigRecord_t record;                //整个大结构体

如果首次运行，Load检测到无效，就会LOAD 调用  Device_Config_Record_Write 导致所需的栈深度超过1600
暂时有想过只维护一个全局局部变量，问了GPT但是给的方案没有经过仔细琢磨，暂时决定不改了。










*/
