/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HARVESTER_MLO_H
#define _HARVESTER_MLO_H

#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>
#include "ansc_platform.h"
#include <wifi_hal.h>
#include "harvester_associated_devices.h"

/*
 * MLO RFC Enable/Disable Feature
 */
#define HARVESTER_MLO_RFC_PARAM "Device.DeviceInfo.X_RDKCENTRAL-COM_Report.InterfaceDevicesWifi.MloRfcEnable"

/**
 * @brief Parse MLO format JSON into structures
 * @param[in] jsonVal cJSON object
 * @param[out] associated_dev Array of MLO device structures
 * @param[out] assocDevCount Number of devices
 * @return 0 for success, 1 for failure
 */
int mlo_parseAssociatedDeviceDiagnostics(void *jsonVal, wifi_associated_dev_t **associated_dev, bool **mld_enable_list, char ***band_list, uint32_t *assocDevCount);

/**
 * @brief Get MLO RFC enable status
 * @return true if MLO harvesting is enabled, false otherwise
 */
bool get_HarvesterMLORfcEnable(void);

/**
 * @brief Set MLO RFC enable status and persist to PSM
 */
int set_HarvesterMLORfcEnable(bool bValue);

/**
 * @brief Initialize and register MLO RFC RBUS data elements
 * @return 0 for success, 1 for failure
 */
int regHarvesterDataModel(void);

/**
 * @brief Unregister MLO RFC RBUS data elements
 */
void harvesterMLO_RfcUninit(void);

#endif /* _HARVESTER_MLO_H */
