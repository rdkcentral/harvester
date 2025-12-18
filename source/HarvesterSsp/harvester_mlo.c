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
int mlo_parseAssociatedDeviceDiagnostics(void *jsonVal, mlo_assoc_dev_t **associated_dev, uint32_t *assocDevCount, char **vapIndex)
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
  mlo_assoc_dev_t *dev = NULL;
  int i = 0;
  int j = 0;
  errno_t rc = -1;

  CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, %s: Entered\n", __FUNCTION__));

  if (json == NULL || associated_dev == NULL || assocDevCount == NULL || vapIndex == NULL)
  {
    CcspHarvesterTrace(("RDK_LOG_ERROR, mlo_parseAssociatedDeviceDiagnostics: NULL parameter\n"));
    return 1;
  }

  *associated_dev = NULL;
  *assocDevCount = 0;
  *vapIndex = NULL;

  outerArr = cJSON_GetObjectItem(json, "AssociatedClientsDiagnostics");
  if (outerArr == NULL)
  {
    CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: no associated mlo devices clients are connected\n", __FUNCTION__));
    return 1;
  }
  CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, %s: Found AssociatedClientsDiagnostics\n", __FUNCTION__));

  item = cJSON_GetArrayItem(outerArr, 0);
  if (item == NULL)
  {
    CcspHarvesterTrace(("RDK_LOG_ERROR, mlo_parse: No items in AssociatedClientsDiagnostics array\n"));
    return 1;
  }

  /* Get VapIndex */
  vapItem = cJSON_GetObjectItem(item, "VapIndex");
  if (vapItem != NULL && vapItem->valuestring != NULL)
  {
    *vapIndex = strdup(vapItem->valuestring);
    CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, %s: Parsed VapIndex: %s\n", __FUNCTION__, *vapIndex));
  }
  else
  {
    CcspHarvesterTrace(("RDK_LOG_WARN, %s: VapIndex not found or NULL\n", __FUNCTION__));
  }

  /* Get clients array */
  clientsArr = cJSON_GetObjectItem(item, "AssociatedClientDiagnostics");
  if (clientsArr == NULL)
  {
    CcspHarvesterTrace(("RDK_LOG_INFO, Harvester %s: no associated mlo devices are found\n", __FUNCTION__));
    return 0;
  }

  *assocDevCount = cJSON_GetArraySize(clientsArr);
  CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, %s: Found %d associated mlo clients\n", __FUNCTION__, *assocDevCount));
  
  if (*assocDevCount == 0)
  {
    CcspHarvesterTrace(("RDK_LOG_INFO, Harvester %s: no associated mlo devices are connected\n", __FUNCTION__));
    return 0;
  }

  dev = (mlo_assoc_dev_t *)calloc(*assocDevCount, sizeof(mlo_assoc_dev_t));
  if (dev == NULL)
  {
    CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: Memory allocation failed for %d devices\n", __FUNCTION__, *assocDevCount));
    return 1;
  }
  *associated_dev = dev;

  for (i = 0; i < (int)*assocDevCount; i++)
  {
    client = cJSON_GetArrayItem(clientsArr, i);
    if (client == NULL)
    {
      CcspHarvesterTrace(("RDK_LOG_WARN, %s: Client item %d is NULL\n", __FUNCTION__, i));
      continue;
    }

    /* Parse MAC */
    jsonItem = cJSON_GetObjectItem(client, "MAC");
    if (jsonItem != NULL && jsonItem->valuestring != NULL) {
      sscanf(jsonItem->valuestring, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
             &dev[i].cli_MACAddress[0], &dev[i].cli_MACAddress[1],
             &dev[i].cli_MACAddress[2], &dev[i].cli_MACAddress[3],
             &dev[i].cli_MACAddress[4], &dev[i].cli_MACAddress[5]);
      CcspHarvesterConsoleTrace(
          ("RDK_LOG_DEBUG, MLO Device[%d] MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           i, dev[i].cli_MACAddress[0], dev[i].cli_MACAddress[1],
           dev[i].cli_MACAddress[2], dev[i].cli_MACAddress[3],
           dev[i].cli_MACAddress[4], dev[i].cli_MACAddress[5]));
    }
    else
    {
        CcspHarvesterTrace(("RDK_LOG_WARN, %s: MAC not found for device %d\n", __FUNCTION__, i));
    }

    /* Parse NumLinks -- Confirm whether its NUM or STRING */
    jsonItem = cJSON_GetObjectItem(client, "NumLinks");
    if (jsonItem != NULL && jsonItem->valuestring != NULL) {
      dev[i].numLinks = atoi(jsonItem->valuestring);
    }
    CcspHarvesterConsoleTrace(
        ("RDK_LOG_DEBUG, MLO Device[%d] NumLinks: %d\n", i, dev[i].numLinks));

    /* Parse Links array */
    linksArr = cJSON_GetObjectItem(client, "Links");
    if (linksArr == NULL)
    {
      CcspHarvesterTrace(("RDK_LOG_WARN, %s: Links array not found for device %d\n", __FUNCTION__, i));
      continue;
    }

    for (j = 0; j < dev[i].numLinks && j < MAX_MLO_LINKS; j++)
    {
      mlo_link_data_t *link_data = &dev[i].links[j];
      link = cJSON_GetArrayItem(linksArr, j);
      if (link == NULL)
      {
        CcspHarvesterTrace(("RDK_LOG_WARN, %s: Link item %d not found for device %d\n", __FUNCTION__, j, i));
        continue;
      }
      
      CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, %s: Parsing Link %d for Device %d\n", __FUNCTION__, j, i));

      /* Band */
      jsonItem = cJSON_GetObjectItem(link, "Band");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        rc = strcpy_s(link_data->band, sizeof(link_data->band), jsonItem->valuestring);
        ERR_CHK(rc);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tBand: %s\n", link_data->band));
      }

      /* AssociationLink (boolean in JSON) */
      jsonItem = cJSON_GetObjectItem(link, "AssociationLink");
      if (jsonItem != NULL)
      {
        link_data->associationLink = cJSON_IsTrue(jsonItem);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tAssociationLink: %d\n", link_data->associationLink));
      }

      /* DownlinkDataRate */
      jsonItem = cJSON_GetObjectItem(link, "DownlinkDataRate");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_LastDataDownlinkRate = atoi(jsonItem->valuestring);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tDownlink: %d\n", link_data->cli_LastDataDownlinkRate));
      }

      /* UplinkDataRate */
      jsonItem = cJSON_GetObjectItem(link, "UplinkDataRate");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_LastDataUplinkRate = atoi(jsonItem->valuestring);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tUplink: %d\n", link_data->cli_LastDataUplinkRate));
      }

      /* BytesSent */
      jsonItem = cJSON_GetObjectItem(link, "BytesSent");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_BytesSent = strtoull(jsonItem->valuestring, NULL, 10);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tBytesSent: %llu\n", (unsigned long long)link_data->cli_BytesSent));
      }

      /* BytesReceived */
      jsonItem = cJSON_GetObjectItem(link, "BytesReceived");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_BytesReceived = strtoull(jsonItem->valuestring, NULL, 10);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tBytesReceived: %llu\n", (unsigned long long)link_data->cli_BytesReceived));
      }

      /* PacketsSent */
      jsonItem = cJSON_GetObjectItem(link, "PacketsSent");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_PacketsSent = strtoull(jsonItem->valuestring, NULL, 10);
      }

      /* PacketsRecieved */
      jsonItem = cJSON_GetObjectItem(link, "PacketsRecieved");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_PacketsReceived = strtoull(jsonItem->valuestring, NULL, 10);
      }

      /* Errors */
      jsonItem = cJSON_GetObjectItem(link, "Errors");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_Errors = atoi(jsonItem->valuestring);
      }

      /* RetransCount */
      jsonItem = cJSON_GetObjectItem(link, "RetransCount");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_RetransCount = atoi(jsonItem->valuestring);
      }

      /* Acknowledgements */
      jsonItem = cJSON_GetObjectItem(link, "Acknowledgements");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_DataFramesSentAck = strtoull(jsonItem->valuestring, NULL, 10);
      }

      /* SignalStrength */
      jsonItem = cJSON_GetObjectItem(link, "SignalStrength");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_SignalStrength = atoi(jsonItem->valuestring);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tSignalStrength: %d\n", link_data->cli_SignalStrength));
      }

      /* SNR */
      jsonItem = cJSON_GetObjectItem(link, "SNR");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_SNR = atoi(jsonItem->valuestring);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tSNR: %d\n", link_data->cli_SNR));
      }

      /* OperatingStandard */
      jsonItem = cJSON_GetObjectItem(link, "OperatingStandard");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        rc = strcpy_s(link_data->cli_OperatingStandard, sizeof(link_data->cli_OperatingStandard), jsonItem->valuestring);
        ERR_CHK(rc);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tStandard: %s\n", link_data->cli_OperatingStandard));
      }

      /* OperatingChannelBandwidth */
      jsonItem = cJSON_GetObjectItem(link, "OperatingChannelBandwidth");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        rc = strcpy_s(link_data->cli_OperatingChannelBandwidth,
                      sizeof(link_data->cli_OperatingChannelBandwidth),
                      jsonItem->valuestring);
        ERR_CHK(rc);
        CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tBW: %s\n", link_data->cli_OperatingChannelBandwidth));
      }

      /* AuthenticationFailures */
      jsonItem = cJSON_GetObjectItem(link, "AuthenticationFailures");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_AuthenticationFailures = atoi(jsonItem->valuestring);
      }

      /* AuthenticationState */
      jsonItem = cJSON_GetObjectItem(link, "AuthenticationState");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_AuthenticationState = (strcmp(jsonItem->valuestring, "1") == 0);
      }

      /* Active */
      jsonItem = cJSON_GetObjectItem(link, "Active");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_Active = (strcmp(jsonItem->valuestring, "1") == 0);
      }

      /* InterferenceSources */
      jsonItem = cJSON_GetObjectItem(link, "InterferenceSources");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        rc = strcpy_s(link_data->cli_InterferenceSources,
                      sizeof(link_data->cli_InterferenceSources),
                      jsonItem->valuestring);
        ERR_CHK(rc);
      }

      /* DataFramesSentNoAck */
      jsonItem = cJSON_GetObjectItem(link, "DataFramesSentNoAck");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_DataFramesSentNoAck = strtoull(jsonItem->valuestring, NULL, 10);
      }

      /* RSSI */
      jsonItem = cJSON_GetObjectItem(link, "RSSI");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_RSSI = atoi(jsonItem->valuestring);
         CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, \tRSSI: %d\n", link_data->cli_RSSI));
      }

      /* MinRSSI */
      jsonItem = cJSON_GetObjectItem(link, "MinRSSI");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_MinRSSI = atoi(jsonItem->valuestring);
      }

      /* MaxRSSI */
      jsonItem = cJSON_GetObjectItem(link, "MaxRSSI");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_MaxRSSI = atoi(jsonItem->valuestring);
      }

      /* Disassociations */
      jsonItem = cJSON_GetObjectItem(link, "Disassociations");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_Disassociations = atoi(jsonItem->valuestring);
      }

      /* Retransmissions */
      jsonItem = cJSON_GetObjectItem(link, "Retransmissions");
      if (jsonItem != NULL && jsonItem->valuestring != NULL)
      {
        link_data->cli_Retransmissions = atoi(jsonItem->valuestring);
      }

    }
  }

  CcspHarvesterTrace(("RDK_LOG_INFO, mlo_parseAssociatedDeviceDiagnostics: Successfully Parsed %u MLO devices\n", *assocDevCount));
  return 0;
}

/**
 * @brief Add MLO device data to linked list
 */
void add_to_mlo_list(struct mlo_associated_device_data **headnode,
                     char *vapIndex, unsigned long devices,
                     mlo_assoc_dev_t *devicedata) {
  struct mlo_associated_device_data *ptr = NULL;
  struct mlo_associated_device_data *curr = NULL;
  errno_t rc = -1;

  CcspHarvesterConsoleTrace(
      ("RDK_LOG_DEBUG, Harvester %s ENTER\n", __FUNCTION__));

  if (headnode == NULL || vapIndex == NULL) {
    CcspHarvesterTrace(("RDK_LOG_ERROR, add_to_mlo_list: NULL parameter\n"));
    return;
  }

  ptr = (struct mlo_associated_device_data *)malloc(sizeof(struct mlo_associated_device_data));
  if (ptr == NULL)
  {
    CcspHarvesterTrace(("RDK_LOG_ERROR, add_to_mlo_list: Memory allocation failed\n"));
    return;
  }

  rc = memset_s(ptr, sizeof(struct mlo_associated_device_data), 0, sizeof(struct mlo_associated_device_data));
  ERR_CHK(rc);

  ptr->vapIndex = strdup(vapIndex);
  ptr->numAssocDevices = devices;
  ptr->devicedata = devicedata;
  ptr->next = NULL;
  gettimeofday(&(ptr->timestamp), NULL);

  if (*headnode == NULL)
  {
    *headnode = ptr;
  }
  else
  {
    curr = *headnode;
    while (curr->next != NULL)
    {
      curr = curr->next;
    }
    curr->next = ptr;
  }

  CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s EXIT\n", __FUNCTION__));
}

/**
 * @brief Print MLO linked list for debugging
 */
void print_mlo_list(struct mlo_associated_device_data *head)
{
  struct mlo_associated_device_data *ptr = head;
  int nodeNum = 0;

  CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s ENTER\n", __FUNCTION__));

  while (ptr != NULL)
  {
    CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, MLO Node[%d]: VapIndex=%s "
                               "NumDevices=%lu Timestamp=%ld\n",
                               nodeNum, ptr->vapIndex ? ptr->vapIndex : "NULL",
                               ptr->numAssocDevices,
                               (long)ptr->timestamp.tv_sec));
    ptr = ptr->next;
    nodeNum++;
  }

  CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s EXIT\n", __FUNCTION__));
}

/**
 * @brief Delete and free MLO linked list
 */
void delete_mlo_list(struct mlo_associated_device_data *head)
{
  struct mlo_associated_device_data *curr = head;
  struct mlo_associated_device_data *next = NULL;

  CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s ENTER\n", __FUNCTION__));

  while (curr != NULL)
  {
    next = curr->next;
    CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Deleting MLO Node VapIndex=%s\n",   curr->vapIndex ? curr->vapIndex : "NULL"));
    if (curr->vapIndex != NULL)
    {
      free(curr->vapIndex);
    }
    if (curr->devicedata != NULL)
    {
      free(curr->devicedata);
    }
    free(curr);
    curr = next;
  }

  CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s EXIT\n", __FUNCTION__));
}

