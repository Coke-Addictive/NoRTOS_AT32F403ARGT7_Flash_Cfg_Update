#include "app_flash_simple.h"
#include "drv_crc32.h"
#include "drv_flash.h"
#include "log.h"

#include "stdbool.h"
#include "string.h"
#include <stddef.h>
#include <stdio.h>





// Flash记录格式
#define DEVICE_CONFIG_SCHEMA_VERSION                    1U                                      // 当前配置结构版本
#define DEVICE_CONFIG_PAYLOAD_SIZE_BYTES                512U                                    // Device_CFG_Params_t 载荷大小，单位字节
#define DEVICE_CONFIG_VALID_SIZE_BYTES      ((uint32_t)offsetof(Device_CFG_Params_t, Offset))   // 当前版本有效配置长度，单位字节






// 用户配置 Flash 记录，包含记录头和固定容量载荷
typedef struct __attribute__((aligned(4))) {
    uint32_t version;                                                                   // 配置结构版本
    uint32_t payload_valid_size;                                                        // 当前版本实际有效配置长度，单位字节
    uint8_t payload[DEVICE_CONFIG_PAYLOAD_SIZE_BYTES];                                  // 配置数据
    uint32_t crc32;                                                                     // CRC32校验码
} DeviceConfigRecord_t;




static bool s_app_flash_initialized = false;                // Flash服务初始化标志

static bool s_user_config_ready = false;                    // 用户配置加载就绪标志
static Device_CFG_Params_t s_device_config;                 // 当前用户配置RAM缓存


// 默认配置
static const Device_CFG_Params_t DEFAULT_DEVICE_CFG = {
    .Device_Hard_SW =                       0U,                                 // 系统硬开关默认关闭
    .Device_Soft_SW =                       0U,                                 // 系统软开关默认关闭
    .Device_Silent_SW =                     0U,                                 // 系统静默默认关闭

    .Device_Night_Mute_SW =                 0U,                                 // 夜间静音默认关闭
    .Device_Night_Mute_Start_Hour =         18U,                                // 夜静开始时间 18 点
    .Device_Night_Mute_Start_Minute =       0U,                                 // 夜静开始时间 0 分
    .Device_Night_Mute_End_Hour =           9U,                                 // 夜静结束时间 9 点
    .Device_Night_Mute_End_Minute =         0U,                                 // 夜静结束时间 0 分

    .Device_Fragment_Mute_SW =              0U,                                 // 时间段静音总开关默认关闭
    .Device_Fragment_Mute_1_SW =            0U,                                 // 时间段 1 静音默认关闭
    .Device_Fragment_Mute_1_Start_Hour =    0U,                                 // 时间段 1 开始小时占位，待业务设定
    .Device_Fragment_Mute_1_Start_Minute =  0U,                                 // 时间段 1 开始分钟占位，待业务设定
    .Device_Fragment_Mute_1_End_Hour =      0U,                                 // 时间段 1 结束小时占位，待业务设定
    .Device_Fragment_Mute_1_End_Minute =    0U,                                 // 时间段 1 结束分钟占位，待业务设定

    .Device_Fragment_Mute_2_SW =            0U,                                 // 时间段 2 静音默认关闭
    .Device_Fragment_Mute_2_Start_Hour =    0U,                                 // 时间段 2 开始小时占位，待业务设定
    .Device_Fragment_Mute_2_Start_Minute =  0U,                                 // 时间段 2 开始分钟占位，待业务设定
    .Device_Fragment_Mute_2_End_Hour =      0U,                                 // 时间段 2 结束小时占位，待业务设定
    .Device_Fragment_Mute_2_End_Minute =    0U,                                 // 时间段 2 结束分钟占位，待业务设定

    .Device_Fragment_Mute_3_SW =            0U,                                 // 时间段 3 静音默认关闭
    .Device_Fragment_Mute_3_Start_Hour =    0U,                                 // 时间段 3 开始小时占位，待业务设定
    .Device_Fragment_Mute_3_Start_Minute =  0U,                                 // 时间段 3 开始分钟占位，待业务设定
    .Device_Fragment_Mute_3_End_Hour =      0U,                                 // 时间段 3 结束小时占位，待业务设定
    .Device_Fragment_Mute_3_End_Minute =    0U,                                 // 时间段 3 结束分钟占位，待业务设定

    .Device_Ultrasonic_SW =                 1U,                                 // 超声波默认开启
    .Device_Ultrasonic_Mode =               0U,                                 // 超声波模式占位，待业务定义取值
    .Device_Ultrasonic_Start_Hz =           0U,                                 // 超声波开始频率占位，HZ,待确认取值
    .Device_Ultrasonic_End_Hz =             0U,                                 // 超声波结束频率占位，HZ,待确认取值
    .Device_Ultrasonic_Schedule_SW =        0U,                                 // 超声波定时默认关闭
    .Device_Ultrasonic_Schedule_Span =      0U,                                 // 超声波定时跨度占位，单位分钟

    .Device_Simulated_SW =                  1U,                                 // 模拟声默认开启
    .Device_Simulated_Mode =                0U,                                 // 模拟声默认模式
    .Device_Simulated_Select_Number_Max =   0U,                                 // 模拟声音频选取数量占位，待业务设定
    .Device_Simulated_Schedule_SW =         0U,                                 // 模拟声定时默认关闭
    .Device_Simulated_Schedule_Span =       0U,                                 // 模拟声定时跨度占位，单位分钟

    .Device_R_B_LED_SW =                    1U,                                 // 红蓝光默认开启
    .Device_R_B_LED_Mode =                  0U,                                 // 红蓝光模式占位，待业务定义取值
    .Device_R_B_LED_Frequency =             0U,                                 // 红蓝光频率占位，HZ,待确认取值
    .Device_R_B_LED_Schedule_SW =           0U,                                 // 红蓝光定时默认关闭
    .Device_R_B_LED_Schedule_Span =         0U,                                 // 红蓝光定时跨度占位，单位分钟

    .Device_Laser_SW =                      1U,                                 // 激光-开
    .Device_Laser_Mode =                    0U,                                 // 激光模式占位，待业务定义取值
    .Device_Laser_Frequency =               0U,                                 // 激光频率占位，HZ,待确认取值
    .Device_Laser_Schedule_SW =             0U,                                 // 激光定时默认关闭
    .Device_Laser_Schedule_Span =           0U,                                 // 激光定时跨度占位，单位分钟

    .Device_Soft_Schedule_SW =              0U,                                 // 软定时默认关闭
    .Device_Soft_Schedule_Span =            0U,                                 // 软定时时长，分钟，待确认取值

    .Device_Hard_Schedule_Mode =            0U,                                 // 硬定时默认关闭
    .Device_Hard_Schedule_Span =            0U,                                 // 硬定时时长，分钟，待确认取值

    .Device_Logic_Volume =                  1U,                                 // 默认使用低音量档位
    .Device_Real_Volume =                   0U,                                 // 物理音量占位，待确定低音量档位映射

    .Device_Location_SW =                   1U,                                 // 系统定位-开
    .Device_Location_Report_SW =            1U,                                 // 系统定位上报-开
    .Device_Location_Frequency =            24U,                                // 定位间隔占位，单位小时

    .Device_R5a_Id =                        {'0', '0', '0', '0', '0', '\0'},    // 默认遥控器 ID 为 00000
    .Offset =                               {0U}                                // 清零全部预留空间
};


// 函数声明-------------------------------
static bool Device_Config_Record_Is_Valid(const DeviceConfigRecord_t *record);
static bool Device_Config_Params_Is_Valid(const Device_CFG_Params_t *config);
static int32_t Device_Config_Record_Write(const Device_CFG_Params_t *config);
static bool Device_Config_Payload_Crc32_Calculate(const DeviceConfigRecord_t *record, uint32_t *crc32);

/**
 * @brief 计算配置记录完整 payload 的 CRC32，包括预留区。
 * @param record 待计算的只读配置记录，版本和有效长度不参与 CRC 计算。
 * @param crc32 输出计算结果的可写指针，失败时保持原值。
 * @return true 表示计算成功，false 表示空指针、CRC 驱动未初始化或追加数据失败。
 * @note 调用前须初始化 CRC 驱动，同一轮计算期间不得穿插其他 CRC 操作。
 * @note 目前只有内部payload参与CRC校验
 */
static bool Device_Config_Payload_Crc32_Calculate(const DeviceConfigRecord_t *record, uint32_t *crc32) {
    if ((record == NULL) || (crc32 == NULL)) {
        return false;
    }

    if (Drv_Crc32_Reset() == false) {
        return false;
    }

    if (Drv_Crc32_Update(record->payload, sizeof(record->payload)) == false) {
        return false;
    }

    *crc32 = Drv_Crc32_Get();

    return true;
}


/**
 * @brief 校验用户配置记录的格式、版本、有效长度和 CRC32。
 * @param record 已读取到 RAM 的完整用户配置记录。
 * @return true 表示校验通过，false 表示参数为空或记录无效。
 * @note 按新增字段追加到预留区、旧字段布局和含义保持不变的规则兼容旧版本。
 */
static bool Device_Config_Record_Is_Valid(const DeviceConfigRecord_t *record) {
    uint32_t crc32;

    if (record == NULL) {
        return false;
    }

    // 粗略的检查结构体版本号
    if ((record->version == 0U) || (record->version > DEVICE_CONFIG_SCHEMA_VERSION)) {
        return false;
    }

    // 检查有效数据长度
    if ((record->payload_valid_size == 0U) || (record->payload_valid_size > DEVICE_CONFIG_VALID_SIZE_BYTES) ||
        (record->payload_valid_size > sizeof(record->payload))) {
        return false;
    }


    // 当前版本的记录必须包含当前版本全部有效字段。
    if ((record->version == DEVICE_CONFIG_SCHEMA_VERSION) && (record->payload_valid_size != DEVICE_CONFIG_VALID_SIZE_BYTES)) {
        return false;
    }

    // 固定校验完整 payload，包括预留区；版本和有效长度由上面的条件单独检查。
    if (Device_Config_Payload_Crc32_Calculate(record, &crc32) == false) {
        return false;
    }

    return crc32 == record->crc32;
}


/**
 * @brief 设备配置参数合法性检查
 * @param config 
 * @return 
 */
static bool Device_Config_Params_Is_Valid(const Device_CFG_Params_t *config) {
    //[TODO]:这里应该做合法性检查，并且！把不合法的值全部改成默认值。
    return true;
}


/**
 * @brief 构造用户配置记录并整体写入 Flash。
 * @param config 待保存的用户配置。
 * @return RESULT_SUCCESS 表示写入成功，RESULT_FAIL 表示参数无效或 Flash 操作失败。
 * @note 调用方必须已获取 Flash 互斥量，本函数不更新 RAM 配置缓存。
 */
static int32_t Device_Config_Record_Write(const Device_CFG_Params_t *config) {
    DeviceConfigRecord_t record;            //整个大结构体
    Drv_Flash_Result_t result;              //Flash操作结果

    // 合法性检查
    if ((config == NULL) || (Device_Config_Params_Is_Valid(config) == false)) {
        return -1;
    }

     // 只复制有效配置，预留区统一清零。
    (void)memset(&record, 0, sizeof(record));
    record.version = DEVICE_CONFIG_SCHEMA_VERSION;
    record.payload_valid_size = DEVICE_CONFIG_VALID_SIZE_BYTES;
    (void)memcpy(record.payload, config, record.payload_valid_size);

    // 计算完整 payload 的 CRC32，成功后保存到记录中。
    if (Device_Config_Payload_Crc32_Calculate(&record, &record.crc32) == false) {
        return -2;    // CRC 计算失败，不执行 Flash 擦除和写入
    }

    // 擦除目标区域
    result = Drv_Flash_Erase_Sector_For_Address(DRV_FLASH_USERCFG_START_ADDR, DRV_FLASH_USERCFG_END_ADDR);
    if (result != DRV_FLASH_OK) {
        LOG_ERROR("擦除用户配置失败，错误码=%d", (int)result);
        return -3;
    }

    // 整体写入
    result = Drv_Flash_Write(DRV_FLASH_USERCFG_START_ADDR, &record, sizeof(record));
    if (result != DRV_FLASH_OK) {
        LOG_ERROR("写入用户配置主体失败，错误码=%d", (int)result);
        return -4;
    }

    return 0;
}


int32_t App_Flash_Init(void) {
    if (s_app_flash_initialized == true) {
        return 0;
    }

    Drv_Crc32_Init();

    s_app_flash_initialized = true;
    
    return 0;
}



int32_t App_Flash_DeviceConfig_Load(void) {
    DeviceConfigRecord_t record;                //整个大结构体
    Device_CFG_Params_t config;                 //用户配置表
    Drv_Flash_Result_t result;                  //Flash操作结果

    if (s_app_flash_initialized == false) {
        LOG_ERROR("Flash服务尚未初始化");
        return -1;
    }

    result = Drv_Flash_Read(DRV_FLASH_USERCFG_START_ADDR, &record, sizeof(record));
    if (result != DRV_FLASH_OK) {
        LOG_ERROR("读取用户配置失败，错误码=%d", (int)result);
        return -2;
    }

    // 先设置当前版本默认值，旧记录没有的新增字段会保留默认值。
    config = DEFAULT_DEVICE_CFG;

    if (Device_Config_Record_Is_Valid(&record) == true) {
        // 记录有效

        // 只覆盖旧版本有效配置，避免旧预留区覆盖新增字段的默认值。
        (void)memcpy(&config, record.payload, record.payload_valid_size);

        //[TODO]:参数合法性检查与修正

        // 是否存在版本变更检查
        if(record.version < DEVICE_CONFIG_SCHEMA_VERSION) {
            if (Device_Config_Record_Write(&config) != 0) {
                LOG_ERROR("写回升级后的用户配置失败");
                return -3;
            }

            LOG_INFO("用户配置升级完成，版本=%d->%d,有效长度=%d->%d", record.version, DEVICE_CONFIG_SCHEMA_VERSION, record.payload_valid_size, DEVICE_CONFIG_VALID_SIZE_BYTES);

        } else {
            LOG_INFO("用户配置加载成功，记录版本=%d,有效长度=%d", record.version,record.payload_valid_size);
        }




        
    } else {
        LOG_WARN("用户配置记录无效，使用默认配置");
        if (Device_Config_Record_Write(&config) != 0) {
            LOG_ERROR("写回默认用户配置失败");
            return -4;
        }
        LOG_INFO("默认用户配置已写回Flash");
    }

    // 配置准备完成后，统一更新 RAM 缓存和加载标志。
    s_device_config = config;
    s_user_config_ready = true;

    return 0;
}





int32_t App_Flash_DeviceConfig_Get(Device_CFG_Params_t *config) {
    if ((config == NULL) || (s_app_flash_initialized == false)) {
        return -1;
    }

    // 如果还没有有效数据
    if (s_user_config_ready == false) {
        return -2;
    }

    *config = s_device_config;

    return 0;
}

int32_t App_Flash_DeviceConfig_Save(const Device_CFG_Params_t *config) {
    if ((config == NULL) || (s_app_flash_initialized == false)) {
        return -1;
    }

    if (Device_Config_Record_Write(config) != 0) {
        return -2;
    }

    // Flash 保存成功后，更新 RAM 缓存和配置就绪标志。
    s_device_config = *config;
    s_user_config_ready = true;

    return 0;
}


int32_t App_Flash_DeviceConfig_Reset(void) {
    // Save 内部负责加锁、写入、更新缓存和解锁。
    return App_Flash_DeviceConfig_Save(&DEFAULT_DEVICE_CFG);
}


/**
 * @brief 获取当前用户配置快照，并按功能分组打印业务字段及配置容量。
 * @note 配置须已成功加载或保存，获取失败时只打印错误信息。
 */
void App_Flash_DeviceConfig_Prf(void) {
    Device_CFG_Params_t config;

    if (App_Flash_DeviceConfig_Get(&config) != 0) {
        LOG_ERROR("获取用户配置失败，无法打印");
        return;
    }
    LOG_INFO("========== 设备配置 ==========");
    LOG_INFO("系统开关: 硬=%u, 软=%u, 静默=%u", (unsigned int)config.Device_Hard_SW, (unsigned int)config.Device_Soft_SW, (unsigned int)config.Device_Silent_SW);

    LOG_INFO("夜间静音: 开关=%u, 时段=%02u:%02u-%02u:%02u", (unsigned int)config.Device_Night_Mute_SW, (unsigned int)config.Device_Night_Mute_Start_Hour, (unsigned int)config.Device_Night_Mute_Start_Minute, (unsigned int)config.Device_Night_Mute_End_Hour, (unsigned int)config.Device_Night_Mute_End_Minute);
    LOG_INFO("时间段静音: 总开关=%u", (unsigned int)config.Device_Fragment_Mute_SW);
    LOG_INFO("时间段 1: 开关=%u, 时段=%02u:%02u-%02u:%02u", (unsigned int)config.Device_Fragment_Mute_1_SW, (unsigned int)config.Device_Fragment_Mute_1_Start_Hour, (unsigned int)config.Device_Fragment_Mute_1_Start_Minute, (unsigned int)config.Device_Fragment_Mute_1_End_Hour, (unsigned int)config.Device_Fragment_Mute_1_End_Minute);
    LOG_INFO("时间段 2: 开关=%u, 时段=%02u:%02u-%02u:%02u", (unsigned int)config.Device_Fragment_Mute_2_SW, (unsigned int)config.Device_Fragment_Mute_2_Start_Hour, (unsigned int)config.Device_Fragment_Mute_2_Start_Minute, (unsigned int)config.Device_Fragment_Mute_2_End_Hour, (unsigned int)config.Device_Fragment_Mute_2_End_Minute);
    LOG_INFO("时间段 3: 开关=%u, 时段=%02u:%02u-%02u:%02u", (unsigned int)config.Device_Fragment_Mute_3_SW, (unsigned int)config.Device_Fragment_Mute_3_Start_Hour, (unsigned int)config.Device_Fragment_Mute_3_Start_Minute, (unsigned int)config.Device_Fragment_Mute_3_End_Hour, (unsigned int)config.Device_Fragment_Mute_3_End_Minute);

    LOG_INFO("超声波: 开关=%u, 模式=%u, 频率=%u-%u Hz", (unsigned int)config.Device_Ultrasonic_SW, (unsigned int)config.Device_Ultrasonic_Mode, (unsigned int)config.Device_Ultrasonic_Start_Hz, (unsigned int)config.Device_Ultrasonic_End_Hz);
    LOG_INFO("超声波定时: 开关=%u, 跨度=%u min", (unsigned int)config.Device_Ultrasonic_Schedule_SW, (unsigned int)config.Device_Ultrasonic_Schedule_Span);

    LOG_INFO("模拟声: 开关=%u, 模式=%u, 最多选取数量=%u", (unsigned int)config.Device_Simulated_SW, (unsigned int)config.Device_Simulated_Mode, (unsigned int)config.Device_Simulated_Select_Number_Max);
    LOG_INFO("模拟声定时: 开关=%u, 跨度=%u min", (unsigned int)config.Device_Simulated_Schedule_SW, (unsigned int)config.Device_Simulated_Schedule_Span);

    LOG_INFO("红蓝光: 开关=%u, 模式=%u, 频率=%u Hz", (unsigned int)config.Device_R_B_LED_SW, (unsigned int)config.Device_R_B_LED_Mode, (unsigned int)config.Device_R_B_LED_Frequency);
    LOG_INFO("红蓝光定时: 开关=%u, 跨度=%u min", (unsigned int)config.Device_R_B_LED_Schedule_SW, (unsigned int)config.Device_R_B_LED_Schedule_Span);

    LOG_INFO("激光: 开关=%u, 模式=%u, 频率=%u Hz", (unsigned int)config.Device_Laser_SW, (unsigned int)config.Device_Laser_Mode, (unsigned int)config.Device_Laser_Frequency);
    LOG_INFO("激光定时: 开关=%u, 跨度=%u min", (unsigned int)config.Device_Laser_Schedule_SW, (unsigned int)config.Device_Laser_Schedule_Span);

    LOG_INFO("软定时: 开关=%u, 跨度=%u min", (unsigned int)config.Device_Soft_Schedule_SW, (unsigned int)config.Device_Soft_Schedule_Span);
    LOG_INFO("硬定时: 模式=%u, 跨度=%u min", (unsigned int)config.Device_Hard_Schedule_Mode, (unsigned int)config.Device_Hard_Schedule_Span);
    LOG_INFO("音量: 逻辑等级=%u, 物理值=%u", (unsigned int)config.Device_Logic_Volume, (unsigned int)config.Device_Real_Volume);

    LOG_INFO("冷却: 模式=%u, 初始时间=%u min, 递增时间=%u min", (unsigned int)config.Device_Cooling_Mode, (unsigned int)config.Device_Cooling_Initial_Duration, (unsigned int)config.Device_Cooling_Increase_Time);
    LOG_INFO("定位: 开关=%u, 上报开关=%u, 间隔=%u h", (unsigned int)config.Device_Location_SW, (unsigned int)config.Device_Location_Report_SW, (unsigned int)config.Device_Location_Frequency);

    // 限定 ID 字符数，未保存字符串结束符时也不会越界读取。
    LOG_INFO("遥控器 ID: %.*s", (int)(sizeof(config.Device_R5a_Id) - 1U), (const char *)config.Device_R5a_Id);
    LOG_INFO("配置容量: 总计=%u bytes, 有效=%u bytes, 预留=%u bytes", (unsigned int)sizeof(config), (unsigned int)DEVICE_CONFIG_VALID_SIZE_BYTES, (unsigned int)sizeof(config.Offset));
    LOG_INFO("==============================");
}


