/**
 * @file shell.h
 * @author Dorian Benech
 * @brief
 * @version 1.0
 * @date 2023-10-11
 *
 * @copyright Copyright (c) 2023 GammaTroniques
 *
 */

#ifndef SHELL_H
#define SHELL_H

/*==============================================================================
 Local Include
===============================================================================*/
#include "config.h"
// #include "mqtt.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "string.h"
#include "esp_log.h"
#include "argtable3/argtable3.h"
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-guides/usb-serial-jtag-console.html

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
/*==============================================================================
 Public Defines
==============================================================================*/

/*==============================================================================
 Public Macro
==============================================================================*/

/*==============================================================================
 Public Type
==============================================================================*/

/*==============================================================================
 Public Variables Declaration
==============================================================================*/

/*==============================================================================
 Public Functions Declaration
==============================================================================*/

extern void shell_init();

extern esp_console_repl_t *shell_repl;

uint8_t tuya_stop();
uint8_t tuya_restart();

void shell_deinit();
void shell_reinit();

extern esp_reset_reason_t shell_wake_reason();
#endif /* SHELL_H */