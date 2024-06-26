/**
 * @file ota.h
 * @author Dorian Benech
 * @brief
 * @version 1.0
 * @date 2023-10-27
 *
 * @copyright Copyright (c) 2023 GammaTroniques
 *
 */

#ifndef OTA_H
#define OTA_H

/*==============================================================================
 Local Include
===============================================================================*/
#include <stdint.h>
#include <stdbool.h>
/*==============================================================================
 Public Defines
==============================================================================*/

/*==============================================================================
 Public Macro
==============================================================================*/

/*==============================================================================
 Public Type
==============================================================================*/
typedef struct
{
    char target[16];
    char version[16];
    char currentVersion[16];
    char hwVersion[16];
    char app_url[256];
    char storage_url[256];
    char md5[33];
} ota_version_t;

typedef enum
{
    OTA_IDLE,
    OTA_AVAILABLE,
    OTA_INSTALLING,
    OTA_OK,
    OTA_REBOOT,
    OTA_ERROR
} ota_state_t;
/*==============================================================================
 Public Variables Declaration
==============================================================================*/

extern ota_state_t ota_state;
extern bool ota_available;

/*==============================================================================
 Public Functions Declaration
==============================================================================*/
int ota_get_latest(ota_version_t *version);
void ota_perform_task(void *pvParameter);
#endif /* OTA_H */
