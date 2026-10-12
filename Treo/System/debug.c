#include "debug.h"

#include "drv_crc32.h"

#include "app_flash_simple.h"

void Debug_Init(void) {
    Drv_Crc32_Init();
    App_Flash_Init();
    
}


void Debug_RunOnce(void) {
    App_Flash_DeviceConfig_Load();
    App_Flash_DeviceConfig_Prf();
}









