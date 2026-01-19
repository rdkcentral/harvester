/*
 * If not stated otherwise in this file or this component's Licenses.txt file
 * the following copyright and licenses apply:
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harvester_mlo.h"
#include <rbus/rbus.h>
#include "harvester_rbus_api.h"
#include "../../include/ccsp_harvesterLog_wrapper.h"
#include "safec_lib_common.h"
#include <cJSON.h>

/* Global variable for MLO RFC enable status */
static bool g_MLORfcEnabled = false;

/**
 * @brief Get MLO RFC enable status
 */
bool get_HarvesterMLORfcEnable(void)
{
    return g_MLORfcEnabled;
}

/**
 * @brief Set MLO RFC enable status and persist to PSM
 */
int set_HarvesterMLORfcEnable(bool bValue)
{
    // Updare global mlo rfc variable
    g_MLORfcEnabled = bValue;

    // Update PSM DB Value
    rbusError_t retPsmSet = RBUS_ERROR_SUCCESS;
    char *buf = NULL;

    buf = bValue ? strdup("true") : strdup("false");
    if (buf == NULL)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: strdup failed\n", __FUNCTION__));
        return 1;
    }

    retPsmSet = rbus_StoreValueIntoPsmDB(HARVESTER_MLO_RFC_PARAM, buf);
    if (retPsmSet != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: PSM set failed ret %d for parameter %s and value %s\n", __FUNCTION__, retPsmSet, HARVESTER_MLO_RFC_PARAM, buf));
        free(buf);
        return 1;
    }

    CcspHarvesterTrace(("RDK_LOG_INFO, %s: PSM set success for parameter %s and value %s\n", __FUNCTION__, HARVESTER_MLO_RFC_PARAM, buf));
    free(buf);
    return 0;
}

/**
 * @brief RBUS Get handler for MLO RFC parameter
 */
static rbusError_t harvesterMLO_RfcGetHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t* opts)
{
    (void)handle;
    (void)opts;

    const char *propertyName;
    propertyName = rbusProperty_GetName(property);
    if (propertyName == NULL)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: Unable to handle get request for property\n", __FUNCTION__));
        return RBUS_ERROR_INVALID_INPUT;
    }

    CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: Property Name is %s\n", __FUNCTION__, propertyName));

    if (strcmp(propertyName, HARVESTER_MLO_RFC_PARAM) != 0)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: Unexpected parameter %s\n", __FUNCTION__, propertyName));
        return RBUS_ERROR_ELEMENT_DOES_NOT_EXIST;
    }

    rbusError_t retPsmGet = RBUS_ERROR_SUCCESS;
    rbusValue_t value;
    rbusValue_Init(&value);
    char *tmpchar = NULL;

    /* Get value from PSM DB */
    retPsmGet = rbus_GetValueFromPsmDB(HARVESTER_MLO_RFC_PARAM, &tmpchar);
    if (retPsmGet == RBUS_ERROR_SUCCESS)
    {
      if (tmpchar != NULL)
      {
          if ((strcmp(tmpchar, "true") == 0) || (strcmp(tmpchar, "TRUE") == 0))
          {
            g_MLORfcEnabled = true;
          }
          else
          {
            g_MLORfcEnabled = false;
          }
          free(tmpchar);
      }
      CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: MLO RFC value from PSM = %d\n", __FUNCTION__, g_MLORfcEnabled));
    }
    else
    {
      CcspHarvesterTrace(("RDK_LOG_WARN, %s: PSM get failed ret %d, using cached value %d\n",__FUNCTION__, retPsmGet, g_MLORfcEnabled));
    }

    rbusValue_SetBoolean(value, g_MLORfcEnabled);
    rbusProperty_SetValue(property, value);
    rbusValue_Release(value);

    CcspHarvesterTrace(("RDK_LOG_INFO, %s: Mlo Rfc value fetched is %s\n", __FUNCTION__, g_MLORfcEnabled ? "true" : "false"));
    return RBUS_ERROR_SUCCESS;
}

/**
 * @brief RBUS Set handler for MLO RFC parameter
 */
static rbusError_t harvesterMLO_RfcSetHandler(rbusHandle_t handle, rbusProperty_t prop, rbusSetHandlerOptions_t* opts)
{
    (void)handle;
    (void)opts;
    const char *propertyName;
    propertyName = rbusProperty_GetName(prop);
    if (propertyName == NULL)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: Unable to handle get request for property\n", __FUNCTION__));
        return RBUS_ERROR_INVALID_INPUT;
    }

    CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: Property Name is %s\n", __FUNCTION__, propertyName));

    if (strcmp(propertyName, HARVESTER_MLO_RFC_PARAM) != 0)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: Unexpected parameter %s\n", __FUNCTION__, propertyName));
        return RBUS_ERROR_ELEMENT_DOES_NOT_EXIST;
    }
    
    rbusValue_t paramValue_t = NULL;
    rbusValueType_t type;

    paramValue_t = rbusProperty_GetValue(prop);
    if (paramValue_t == NULL)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: value is NULL\n", __FUNCTION__));
        return RBUS_ERROR_INVALID_INPUT;
    }

    type = rbusValue_GetType(paramValue_t);
    if (type != RBUS_BOOLEAN)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: Unexpected value type %d\n", __FUNCTION__, type));
        return RBUS_ERROR_INVALID_INPUT;
    }

    bool paramVal = rbusValue_GetBoolean(paramValue_t);
    CcspHarvesterTrace(("RDK_LOG_INFO, %s: Setting MLO RFC to %s\n", __FUNCTION__, paramVal ? "true" : "false"));

    if (set_HarvesterMLORfcEnable(paramVal) != 0) {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: set_HarvesterMLORfcEnable failed\n", __FUNCTION__));
        return RBUS_ERROR_BUS_ERROR;
    }

    CcspHarvesterTrace(("RDK_LOG_INFO, %s: MLO RFC set successfully to %s\n", 
                        __FUNCTION__, paramVal ? "true" : "false"));
    return RBUS_ERROR_SUCCESS;
}

/**
 * @brief Initialize and register MLO RFC RBUS data elements
 */
int regHarvesterDataModel()
{
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    rbusHandle_t handle = get_rbus_handle();


    if (handle == NULL)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: rbus handle is NULL\n", __FUNCTION__));
        return -1;
    }

    CcspHarvesterTrace(("RDK_LOG_INFO, %s: Registering MLO RFC parameter %s\n", __FUNCTION__, HARVESTER_MLO_RFC_PARAM));

    rbusDataElement_t dataElements[1] = {
      {HARVESTER_MLO_RFC_PARAM, RBUS_ELEMENT_TYPE_PROPERTY, {harvesterMLO_RfcGetHandler, harvesterMLO_RfcSetHandler, NULL, NULL, NULL, NULL}}
    };

    ret = rbus_regDataElements(handle, 1, dataElements);

    if (ret != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: rbus_regDataElements failed with error %d\n", __FUNCTION__, ret));
        return -1;
    }
    return 0;
}

/**
 * @brief Unregister MLO RFC RBUS data elements
 */
void harvesterMLO_RfcUninit(void)
{
    rbusHandle_t handle = get_rbus_handle();

    if (handle == NULL) {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: rbus handle is NULL\n", __FUNCTION__));
        return;
    }

    CcspHarvesterTrace(("RDK_LOG_INFO, %s: Unregistering MLO RFC parameter\n", __FUNCTION__));

    rbusDataElement_t dataElements[1] = {
        {HARVESTER_MLO_RFC_PARAM, RBUS_ELEMENT_TYPE_PROPERTY, {harvesterMLO_RfcGetHandler, harvesterMLO_RfcSetHandler, NULL, NULL, NULL, NULL}}
    };

    rbus_unregDataElements(handle, 1, dataElements);
    CcspHarvesterTrace(("RDK_LOG_INFO, %s: MLO RFC unregistration done\n", __FUNCTION__));
}

/**
 * @brief Parse MLO format JSON into structures
 */
int mlo_parseAssociatedDeviceDiagnostics(void *jsonVal, wifi_associated_dev_t **associated_dev, wifi_mlo_assoc_dev_data **mlo_data, uint32_t *mloAssocDevCount)
{
    cJSON *json = (cJSON *)jsonVal;
    cJSON *outerArr = NULL;
    cJSON *item = NULL;
    cJSON *vapItem = NULL;
    cJSON *clientsArr = NULL;
    cJSON *client = NULL;
    cJSON *linksArr = NULL;
    cJSON *link = NULL;
    cJSON *jsonItem = NULL;
    wifi_associated_dev_t *dev = NULL;
    wifi_mlo_assoc_dev_data *mlo_dev = NULL;
    int i = 0, j = 0, k = 0;
    uint32_t totalLinks = 0;
    errno_t rc = -1;

    CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, %s: Entered\n", __FUNCTION__));

    if (json == NULL || associated_dev == NULL || mloAssocDevCount == NULL || mlo_data == NULL)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, try_parse: NULL parameter\n"));
        return 1;
    }

    *associated_dev = NULL;
    *mlo_data = NULL;
    *mloAssocDevCount = 0;

    outerArr = cJSON_GetObjectItem(json, "AssociatedClientsDiagnostics");
    if (outerArr == NULL)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: no associated mlo devices clients are connected\n", __FUNCTION__));
        return 1;
    }
    
    int outerArrSize = cJSON_GetArraySize(outerArr);
    CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, %s: Found AssociatedClientsDiagnostics size %d\n", __FUNCTION__, outerArrSize));

    /* Count Total Links to allocate memory */
    for(i = 0; i < outerArrSize; i++)
    {
        item = cJSON_GetArrayItem(outerArr, i);
        if (item == NULL) continue;
        
        clientsArr = cJSON_GetObjectItem(item, "AssociatedClientDiagnostics");
        if (clientsArr != NULL)
        {
            int numClients = cJSON_GetArraySize(clientsArr);
            for(j = 0; j < numClients; j++)
            {
                client = cJSON_GetArrayItem(clientsArr, j);
                if(client != NULL)
                {
                    linksArr = cJSON_GetObjectItem(client, "Links");
                    if(linksArr != NULL)
                    {
                        totalLinks += cJSON_GetArraySize(linksArr);
                    }
                }
            }
        }
    }

    CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, %s: Total Links found: %u\n", __FUNCTION__, totalLinks));

    if (totalLinks == 0)
    {
        return 0;
    }

    /* Allocate Memory */
    dev = (wifi_associated_dev_t *)calloc(totalLinks, sizeof(wifi_associated_dev_t));
    if (dev == NULL)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: Memory allocation failed for dev %d devices\n", __FUNCTION__, totalLinks));
        return 1;
    }

    mlo_dev = (wifi_mlo_assoc_dev_data *)calloc(totalLinks, sizeof(wifi_mlo_assoc_dev_data));
    if (mlo_dev == NULL)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: Memory allocation failed for mlo_dev %d devices\n", __FUNCTION__, totalLinks));
        free(dev);
        return 1;
    }

    /* Parse and populate Data */
    uint32_t current_idx = 0;
    for(i = 0; i < outerArrSize; i++)
    {
        item = cJSON_GetArrayItem(outerArr, i);
        if (item == NULL) continue;

        clientsArr = cJSON_GetObjectItem(item, "AssociatedClientDiagnostics");
        if (clientsArr == NULL) continue;

        int numClients = cJSON_GetArraySize(clientsArr);
        for(j = 0; j < numClients; j++)
        {
            client = cJSON_GetArrayItem(clientsArr, j);
            if(client == NULL) continue;

            // Get Client MAC Address (Common for all links)
            unsigned char clientMac[6] = {0};
            bool macParsed = false;
            jsonItem = cJSON_GetObjectItem(client, "MAC");
            if (jsonItem != NULL && jsonItem->valuestring != NULL) {
                sscanf(jsonItem->valuestring, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
                       &clientMac[0], &clientMac[1], &clientMac[2], &clientMac[3],
                       &clientMac[4], &clientMac[5]);
                macParsed = true;
            }

            linksArr = cJSON_GetObjectItem(client, "Links");
            if (linksArr == NULL)
                continue;

            int numLinks = cJSON_GetArraySize(linksArr);
            for(k = 0; k < numLinks; k++)
            {
                link = cJSON_GetArrayItem(linksArr, k);
                if(link == NULL)
                {
                    continue;
                }

                if (current_idx >= totalLinks)
                {
                    break;
                }

                wifi_associated_dev_t *dst = &dev[current_idx];
                wifi_mlo_assoc_dev_data *mlo_dst = &mlo_dev[current_idx];

                // Set MAC
                if(macParsed) {
                    memcpy(dst->cli_MACAddress, clientMac, 6);
                }
                // Parse MLD Enable State
                if(cJSON_GetObjectItem(devData, "MLDEnable") != NULL)
                {
                    bool MLDState = (strcmp(cJSON_GetObjectItem(devData, "MLDEnable")->valuestring,"1") == 0) ? true : false;
                    mlo_dst->isMLDEnabled = MLDState;
                    CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] isMLDEnabled %s i %d\n", count, (mlo_dst->isMLDEnabled == 1)? "true": "false", current_idx);
                }                

                // Parse Band from Link
                jsonItem = cJSON_GetObjectItem(link, "Band");
                if (jsonItem != NULL && jsonItem->valuestring != NULL)
                {
                    rc = strcpy_s(mlo_dst->frequency_band[current_idx], 8, jsonItem->valuestring);
                    ERR_CHK(rc);
                }

                // Parse Data Rates
                jsonItem = cJSON_GetObjectItem(link, "DownlinkDataRate");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_LastDataDownlinkRate = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_LastDataDownlinkRate = jsonItem->valueint;
                }

                jsonItem = cJSON_GetObjectItem(link, "UplinkDataRate");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_LastDataUplinkRate = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_LastDataUplinkRate = jsonItem->valueint;
                }

                // Parse Signal Metrics
                jsonItem = cJSON_GetObjectItem(link, "RSSI");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_RSSI = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_RSSI = jsonItem->valueint;
                }

                jsonItem = cJSON_GetObjectItem(link, "MinRSSI");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_MinRSSI = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_MinRSSI = jsonItem->valueint;
                }

                jsonItem = cJSON_GetObjectItem(link, "MaxRSSI");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_MaxRSSI = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_MaxRSSI = jsonItem->valueint;
                }

                jsonItem = cJSON_GetObjectItem(link, "SignalStrength");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_SignalStrength = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_SignalStrength = jsonItem->valueint;
                }

                jsonItem = cJSON_GetObjectItem(link, "SNR");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_SNR = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_SNR = jsonItem->valueint;
                }

                jsonItem = cJSON_GetObjectItem(link, "BytesSent");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_BytesSent = strtoull(jsonItem->valuestring, NULL, 10);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_BytesSent = jsonItem->valuedouble;
                }

                jsonItem = cJSON_GetObjectItem(link, "BytesReceived");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_BytesReceived = strtoull(jsonItem->valuestring, NULL, 10);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_BytesReceived = jsonItem->valuedouble;
                }

                jsonItem = cJSON_GetObjectItem(link, "Retransmissions");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_Retransmissions = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_Retransmissions = jsonItem->valueint;
                }

                jsonItem = cJSON_GetObjectItem(link, "DataFramesSentNoAck");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_DataFramesSentNoAck = strtoull(jsonItem->valuestring, NULL, 10);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_DataFramesSentNoAck = jsonItem->valuedouble;
                }

                jsonItem = cJSON_GetObjectItem(link, "Acknowledgements");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_DataFramesSentAck = strtoull(jsonItem->valuestring, NULL, 10);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_DataFramesSentAck = jsonItem->valuedouble;
                }

                // Parse Authentication and State
                jsonItem = cJSON_GetObjectItem(link, "AuthenticationFailures");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_AuthenticationFailures = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_AuthenticationFailures = jsonItem->valueint;
                }

                jsonItem = cJSON_GetObjectItem(link, "AuthenticationState");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsBool(jsonItem)) dst->cli_AuthenticationState = cJSON_IsTrue(jsonItem);
                    else if(cJSON_IsString(jsonItem)) dst->cli_AuthenticationState = (atoi(jsonItem->valuestring) == 1);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_AuthenticationState = (jsonItem->valueint == 1);
                }

                jsonItem = cJSON_GetObjectItem(link, "Active");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsBool(jsonItem)) dst->cli_Active = cJSON_IsTrue(jsonItem);
                    else if(cJSON_IsString(jsonItem)) dst->cli_Active = (atoi(jsonItem->valuestring) == 1);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_Active = (jsonItem->valueint == 1);
                }

                jsonItem = cJSON_GetObjectItem(link, "Disassociations");
                if (jsonItem != NULL)
                {
                    if(cJSON_IsString(jsonItem)) dst->cli_Disassociations = atoi(jsonItem->valuestring);
                    else if(cJSON_IsNumber(jsonItem)) dst->cli_Disassociations = jsonItem->valueint;
                }

                jsonItem = cJSON_GetObjectItem(link, "OperatingStandard");
                if (jsonItem && jsonItem->valuestring) {
                    rc = strcpy_s(dst->cli_OperatingStandard, sizeof(dst->cli_OperatingStandard), jsonItem->valuestring);
                    ERR_CHK(rc);
                }

                jsonItem = cJSON_GetObjectItem(link, "OperatingChannelBandwidth");
                if (jsonItem && jsonItem->valuestring) {
                    rc = strcpy_s(dst->cli_OperatingChannelBandwidth, sizeof(dst->cli_OperatingChannelBandwidth), jsonItem->valuestring);
                    ERR_CHK(rc);
                }

                jsonItem = cJSON_GetObjectItem(link, "InterferenceSources");
                if (jsonItem && jsonItem->valuestring) {
                    rc = strcpy_s(dst->cli_InterferenceSources, sizeof(dst->cli_InterferenceSources), jsonItem->valuestring);
                    ERR_CHK(rc);
                }

                current_idx++;
            }
        }
    }
    
    *associated_dev = dev;
    *mlo_data = mlo_dev;
    *mloAssocDevCount = totalLinks;
    
    CcspHarvesterTrace(("RDK_LOG_INFO, mlo_parseAssociatedDeviceDiagnostics: Successfully Parsed %u MLO Links\n", totalLinks));
    return 0;
}
