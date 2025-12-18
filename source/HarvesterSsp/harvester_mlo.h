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

#define MAX_MLO_LINKS 3  /* 2G, 5G, 6G */

/**
 * @brief Per-link data for MLO device
 */
typedef struct _mlo_link_data {
    char band[8];                          /* "2G", "5G", "6G" */
    bool associationLink;                  /* Primary association link */
    uint32_t cli_LastDataDownlinkRate;
    uint32_t cli_LastDataUplinkRate;
    uint64_t cli_BytesSent;
    uint64_t cli_BytesReceived;
    uint64_t cli_PacketsSent;
    uint64_t cli_PacketsReceived;
    uint32_t cli_Errors;
    uint32_t cli_RetransCount;
    uint64_t cli_DataFramesSentAck;
    int cli_SignalStrength;
    int cli_SNR;
    char cli_OperatingStandard[64];
    char cli_OperatingChannelBandwidth[64];
    uint32_t cli_AuthenticationFailures;
    bool cli_AuthenticationState;
    bool cli_Active;
    char cli_InterferenceSources[64];
    uint64_t cli_DataFramesSentNoAck;
    int cli_RSSI;
    int cli_MinRSSI;
    int cli_MaxRSSI;
    uint32_t cli_Disassociations;
    uint32_t cli_Retransmissions;
} mlo_link_data_t;

/**
 * @brief MLO device - one MAC with multiple links
 */
typedef struct _mlo_assoc_dev {
    unsigned char cli_MACAddress[6];
    int numLinks;
    mlo_link_data_t links[MAX_MLO_LINKS];
} mlo_assoc_dev_t;

/**
 * @brief MLO linked list node
 */
struct mlo_associated_device_data {
    struct timeval timestamp;
    char* vapIndex;
    unsigned long numAssocDevices;
    mlo_assoc_dev_t* devicedata;
    struct mlo_associated_device_data *next;
};

/**
 * @brief Parse MLO format JSON into structures
 * @param[in] jsonVal cJSON object
 * @param[out] associated_dev Array of MLO device structures
 * @param[out] assocDevCount Number of devices
 * @param[out] vapIndex VAP index string (caller must free)
 * @return 0 for success, 1 for failure
 */
int mlo_parseAssociatedDeviceDiagnostics(void *jsonVal, 
                                         mlo_assoc_dev_t **associated_dev, 
                                         uint32_t *assocDevCount,
                                         char **vapIndex);

/**
 * @brief Add MLO device data to linked list
 */
void add_to_mlo_list(struct mlo_associated_device_data **headnode, 
                     char* vapIndex, 
                     unsigned long devices, 
                     mlo_assoc_dev_t* devicedata);

/**
 * @brief Print MLO linked list for debugging
 */
void print_mlo_list(struct mlo_associated_device_data *head);

/**
 * @brief Delete and free MLO linked list
 */
void delete_mlo_list(struct mlo_associated_device_data *head);

/*
 * MLO RFC Enable/Disable Feature
 */
#define HARVESTER_MLO_RFC_PARAM "Device.DeviceInfo.X_RDKCENTRAL-COM_Report.InterfaceDevicesWifi.MloRfcEnable"

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
