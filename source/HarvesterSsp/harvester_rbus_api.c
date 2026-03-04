/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
/**
 * @file harvester_rbus_api.c
 *
 * @ To replace the WiFi HAL calls with RBUS API, when 'OneWifi' is enabled.
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <semaphore.h>
#include "ccsp_harvesterLog_wrapper.h"
#include "report_common.h"
#include "safec_lib_common.h"
#include "secure_wrapper.h"
#include "harvester_rbus_api.h"
#include <cJSON.h>
#include <pthread.h>

pthread_mutex_t mlorfc_mut= PTHREAD_MUTEX_INITIALIZER;

STATIC rbusHandle_t rbus_handle;

/* Global variable for MLO RFC enable status */
static bool g_MLORfcEnabled = false;

rbusHandle_t get_rbus_handle(void)
{
    return rbus_handle;
}

bool rbusInitializedCheck()
{
    return rbus_handle != NULL ? true : false;
}

int harvesterRbusInit(const char *pComponentName)
{
    int ret = RBUS_ERROR_SUCCESS;
    CcspHarvesterTrace(("RDK_LOG_INFO, rbus_open for component %s\n", pComponentName));

    ret = rbus_open(&rbus_handle, pComponentName);

    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: failed with error code %d\n", __FUNCTION__, ret));
        return 1;
    }

    CcspHarvesterTrace(("RDK_LOG_INFO, Harvester %s: is success. ret is %d\n", __FUNCTION__, ret));
    return 0;
}

void harvesterRbus_Uninit()
{
    rbus_close(rbus_handle);
}

int rbus_getBoolValue(BOOL * value, char * path)
{
    int rc = 0;
    rbusValue_t boolVal = NULL;

    if(!rbusInitializedCheck())
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_open failed\n", __FUNCTION__));
        return 1;
    }

    CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: calling rbus get for %s\n", __FUNCTION__, path));

    rc = rbus_get(rbus_handle, path, &boolVal);

    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_get failed for [%s] with error [%d]\n",  __FUNCTION__, path, rc));
        if(boolVal != NULL)
        {
            rbusValue_Release(boolVal);
        }
        return 1;
    }

    *value = rbusValue_GetBoolean(boolVal);
    CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: the value of %s is %s\n", __FUNCTION__, path, *value ? "true" : "false"));
    rbusValue_Release(boolVal);
    return 0;
}

int rbus_getStringValue(char * value, char * path)
{
    int rc = 0;
    rbusValue_t strVal = NULL;

    if(!rbusInitializedCheck())
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_open failed\n", __FUNCTION__));
        return 1;
    }

    CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: calling rbus get for %s\n", __FUNCTION__, path));

    rc = rbus_get(rbus_handle, path, &strVal);

    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_get failed for [%s] with error [%d]\n", __FUNCTION__, path, rc));
        if(strVal != NULL)
        {
            rbusValue_Release(strVal);
        }
        return 1;
    }

    snprintf(value, 128, (char *)rbusValue_GetString(strVal, NULL));
    CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: the value for %s is %s\n", __FUNCTION__, path, value));
    rbusValue_Release(strVal);
    return 0;
}

int rbus_getUInt32Value(ULONG * value, char * path)
{
    int rc = 0;
    rbusValue_t uintVal = NULL;

    if(!rbusInitializedCheck())
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_open failed\n", __FUNCTION__));
        return 1;
    }

    CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: calling rbus get for %s\n", __FUNCTION__, path));

    rc = rbus_get(rbus_handle, path, &uintVal);

    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_get failed for [%s] with error [%d]\n",  __FUNCTION__, path, rc));
        if(uintVal != NULL)
        {
            rbusValue_Release(uintVal);
        }
        return 1;
    }

    *value = rbusValue_GetUInt32(uintVal);
    CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: the value for %s is %lu\n", __FUNCTION__, path, *value));
    rbusValue_Release(uintVal);
    return 0;
}

int rbus_getApAssociatedDeviceDiagnosticResult(int index, wifi_associated_dev_t** associated_dev, wifi_mlo_associated_dev_t **mlo_associated_dev, uint32_t *assocDevCount)
{
    int rc, count = 0, i=0;
    wifi_associated_dev_t *dev=NULL;
    wifi_mlo_associated_dev_t *mlo_dev=NULL;
    rbusValue_t assocDevVal = NULL;

    if(!rbusInitializedCheck())
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_open failed\n", __FUNCTION__));
        return 1;
    }

    char path[128] = {'\0'};
    snprintf(path, sizeof(path), "Device.WiFi.AccessPoint.%d.X_RDK_DiagData", index);

    CcspHarvesterTrace(("RDK_LOG_INFO, Harvester %s: calling rbus get for %s\n", __FUNCTION__, path));
    if(!rbus_handle)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, rbus_getApAssociatedDeviceDiagnosticResult failed as rbus_handle is not initialized\n"));
        return 1;
    }
    rc = rbus_get(rbus_handle, path, &assocDevVal);

    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_get failed for [%s] with error [%d]\n", __FUNCTION__, path, rc));
        if(assocDevVal != NULL)
        {
            rbusValue_Release(assocDevVal);
        }
        return 1;
    }

    cJSON *jsonVal = NULL;
    cJSON *outerClientsArr = NULL;
    cJSON *innerClientArr = NULL;
    cJSON *item = NULL;
    cJSON *devData = NULL;
    int outerArrCount = 0;
    char *assocDevDataStr = (char *)rbusValue_GetString(assocDevVal, NULL);
    CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: assocDevDataStr is %s\n", __FUNCTION__, assocDevDataStr));
    CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s: assocDevDataStr is %s\n", __FUNCTION__, assocDevDataStr));
    jsonVal = cJSON_Parse(assocDevDataStr);

    if (jsonVal != NULL)
    {
	if(cJSON_GetObjectItem(jsonVal, "AssociatedClientsDiagnostics") != NULL)
	{
        	outerClientsArr = cJSON_GetObjectItem(jsonVal, "AssociatedClientsDiagnostics");
        	if(outerClientsArr != NULL)
        	{
    			outerArrCount = cJSON_GetArraySize(outerClientsArr);
                        CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: outerArrCount is %d\n", __FUNCTION__, outerArrCount));

    			if(outerArrCount == 0)
    			{
        			CcspHarvesterTrace(("RDK_LOG_INFO, Harvester %s: no associated devices clients are connected\n", __FUNCTION__));
        			if(assocDevVal != NULL)
        			{
                                        CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: Release assocDevVal if count is 0\n", __FUNCTION__));
           				rbusValue_Release(assocDevVal);
        			}
        			cJSON_Delete(jsonVal);
        			return 0;
    			}
    			for(i=0; i < outerArrCount; i++)
    			{
    				item = cJSON_GetArrayItem(outerClientsArr, i);
    				if(item != NULL){
    					innerClientArr = cJSON_GetObjectItem(item, "AssociatedClientDiagnostics"); }

    				if(innerClientArr != NULL) {
                                        *assocDevCount = cJSON_GetArraySize(innerClientArr);}

                                CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: assocDevCount is %d\n", __FUNCTION__, *assocDevCount));

    				if(*assocDevCount == 0)
    				{
        				CcspHarvesterTrace(("RDK_LOG_INFO, Harvester %s: no associated devices are connected\n", __FUNCTION__));
        				if(assocDevVal != NULL)
        				{
                                                CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: Release assocDevVal if count is 0\n", __FUNCTION__));
           					rbusValue_Release(assocDevVal);	
        				}
        				cJSON_Delete(jsonVal);
        				return 0;
    				}

    				dev = (wifi_associated_dev_t *)calloc(*assocDevCount, sizeof(wifi_associated_dev_t));
    				if (dev == NULL)
    				{
    					CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: calloc failed for associated_dev\n", __FUNCTION__));
    					if(assocDevVal != NULL)
    					{
    						rbusValue_Release(assocDevVal);
    					}
    					cJSON_Delete(jsonVal);
    					*associated_dev = NULL;
    					*mlo_associated_dev = NULL;
    					*assocDevCount = 0;
    					return 1;
    				}
    				*associated_dev = dev;
    				mlo_dev = (wifi_mlo_associated_dev_t *)calloc(*assocDevCount, sizeof(wifi_mlo_associated_dev_t));
    				if (mlo_dev == NULL)
      				{
    					CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: calloc failed for mlo_associated_dev\n", __FUNCTION__));
    					free(dev);
    					if(assocDevVal != NULL)
    					{
    						rbusValue_Release(assocDevVal);
    					}
    					cJSON_Delete(jsonVal);
    					*associated_dev = NULL;
    					*mlo_associated_dev = NULL;
    					*assocDevCount = 0;
    					return 1;
    				}
    				*mlo_associated_dev = mlo_dev;

        			for(count = 0; count < *assocDevCount; count++)
        			{
            				devData = cJSON_GetArrayItem(innerClientArr, count);

            				if(devData != NULL)
            				{
                                    cJSON *mldEnableItem = cJSON_GetObjectItem(devData, "MLDEnable");
                                    if(cJSON_IsString(mldEnableItem) && (mldEnableItem->valuestring != NULL))
                                    {
                                        mlo_dev[count].isMLDEnabled = (strcmp(mldEnableItem->valuestring, "1") == 0) ? true : false;
                                        CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing mlo_dev[%d] cli_MLDStatus %s i %d\n", count, mlo_dev[count].isMLDEnabled ? "true": "false", i));
                                    }

                                    cJSON *associationLinkItem = cJSON_GetObjectItem(devData, "AssociationLink");
                                    if(cJSON_IsString(associationLinkItem) && (associationLinkItem->valuestring != NULL))
                                    {
                                        mlo_dev[count].isAssociationLink = (strcmp(associationLinkItem->valuestring, "1") == 0) ? true : false;
                                        CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] AssociationLink %s i %d\n", count, mlo_dev[count].isAssociationLink ? "true" : "false",i));
                                    }

                            			if(cJSON_GetObjectItem(devData, "MAC") != NULL)
                            			{
                                			char * mac = cJSON_GetObjectItem(devData, "MAC")->valuestring;
                                                        CcspHarvesterTrace(("RDK_LOG_DEBUG, After rbusValue_GetString\n"));
                                			if(mac != NULL)
                                			{
                                    				sscanf(mac, "%2x%2x%2x%2x%2x%2x",
						            (unsigned int *)&dev[count].cli_MACAddress[0],
						            (unsigned int *)&dev[count].cli_MACAddress[1],
						            (unsigned int *)&dev[count].cli_MACAddress[2],
						            (unsigned int *)&dev[count].cli_MACAddress[3],
						            (unsigned int *)&dev[count].cli_MACAddress[4],
						            (unsigned int *)&dev[count].cli_MACAddress[5]);
                                                            CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] Inside Mac %s, i %d \n", count, mac, i));
                                			}
                             			}
                             if(cJSON_GetObjectItem(devData, "AuthenticationState") != NULL)
                             {
                                bool authState = (strcmp(cJSON_GetObjectItem(devData, "AuthenticationState")->valuestring,"1") == 0) ? true : false;
                                dev[count].cli_AuthenticationState = authState;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_AuthenticationState %s i %d\n", count, (dev[count].cli_AuthenticationState == 1)? "true": "false", i));
                             }
                             if(cJSON_GetObjectItem(devData, "DownlinkDataRate") != NULL)
                             {
                                uint32_t lstDataDwnRate = atoi(cJSON_GetObjectItem(devData, "DownlinkDataRate")->valuestring);
                                dev[count].cli_LastDataDownlinkRate = lstDataDwnRate;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_LastDataDownlinkRate %d i %d\n", count, dev[count].cli_LastDataDownlinkRate, i));
                             }
                             if(cJSON_GetObjectItem(devData, "UplinkDataRate") != NULL)
                             {
                                uint32_t lstDataUpRate = atoi(cJSON_GetObjectItem(devData, "UplinkDataRate")->valuestring);
                                dev[count].cli_LastDataUplinkRate = lstDataUpRate;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_LastDataUplinkRate %d i %d\n", count, dev[count].cli_LastDataUplinkRate, i));
                             }
                             if(cJSON_GetObjectItem(devData, "SignalStrength") != NULL)
                             {
                                int signalStrength  = atoi(cJSON_GetObjectItem(devData, "SignalStrength")->valuestring);
                                dev[count].cli_SignalStrength = signalStrength;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_SignalStrength %d i %d\n", count, dev[count].cli_SignalStrength, i));
                             }
                             if(cJSON_GetObjectItem(devData, "Retransmissions") != NULL)
                             {
                                uint32_t retransmission = atoi(cJSON_GetObjectItem(devData, "Retransmissions")->valuestring);
                                dev[count].cli_Retransmissions = retransmission;
                                CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_Retransmissions %d i %d\n", count, dev[count].cli_Retransmissions, i));
                             }
                             if(cJSON_GetObjectItem(devData, "Active") != NULL)
                             {
                                bool active  = (strcmp(cJSON_GetObjectItem(devData, "Active")->valuestring, "1") == 0) ? true: false;
                                dev[count].cli_Active = active;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_Active %s i %d\n", count, (dev[count].cli_Active == 1)? "true": "false", i));
                             }
                             if(cJSON_GetObjectItem(devData, "OperatingStandard") != NULL)
                             {
                                char * operatingStd=cJSON_GetObjectItem(devData, "OperatingStandard")->valuestring;
                                if(operatingStd != NULL)
                                {
                                    strcpy_s(dev[count].cli_OperatingStandard, 64, operatingStd);
                                  CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_OperatingStandard %s i %d\n", count, dev[count].cli_OperatingStandard, i));
                                }
                             }
                             if(cJSON_GetObjectItem(devData, "OperatingChannelBandwidth") != NULL)
                             {
                                char * opchanBtw=(char*)cJSON_GetObjectItem(devData, "OperatingChannelBandwidth")->valuestring;
                                if(opchanBtw != NULL)
                                {
                                    strcpy_s(dev[count].cli_OperatingChannelBandwidth, 64, opchanBtw);
                                  CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_OperatingChannelBandwidth %s i %d\n", count, dev[count].cli_OperatingChannelBandwidth, i));
                                }
                             }
                             if(cJSON_GetObjectItem(devData, "SNR") != NULL)
                             {
                                int snr  = atoi(cJSON_GetObjectItem(devData, "SNR")->valuestring);
                                dev[count].cli_SNR = snr;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_SNR %d i %d\n", count, dev[count].cli_SNR, i));
                             }
                             if(cJSON_GetObjectItem(devData, "InterferenceSources") != NULL)
                             {
                                char * interferenceSrc = (char*)cJSON_GetObjectItem(devData, "InterferenceSources")->valuestring;
                                if(interferenceSrc != NULL)
                                {
                                    strcpy_s(dev[count].cli_InterferenceSources, 64, interferenceSrc);
                                  CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_InterferenceSources %s i %d\n", count, dev[count].cli_InterferenceSources, i));
                                }
                             }
                             if(cJSON_GetObjectItem(devData, "Acknowledgements") != NULL)
                             {
                                uint32_t datFrameSentACK = atoi(cJSON_GetObjectItem(devData, "Acknowledgements")->valuestring);
                                dev[count].cli_DataFramesSentAck = datFrameSentACK;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_DataFramesSentAck %ld i %d\n", count, dev[count].cli_DataFramesSentAck, i));
                             }
                             if(cJSON_GetObjectItem(devData, "DataFramesSentNoAck") != NULL)
                             {
                                uint32_t datFrameSentNACK= atoi(cJSON_GetObjectItem(devData, "DataFramesSentNoAck")->valuestring);
                                dev[count].cli_DataFramesSentNoAck = datFrameSentNACK;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_DataFramesSentNoAck %ld i %d\n", count, dev[count].cli_DataFramesSentNoAck, i));
                             }
                             if(cJSON_GetObjectItem(devData, "BytesSent") != NULL)
                             {
                                uint32_t  bytesSent= atoi(cJSON_GetObjectItem(devData, "BytesSent")->valuestring);
                                dev[count].cli_BytesSent = bytesSent;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_BytesSent %ld i %d\n", count, dev[count].cli_BytesSent, i));
                             }
                             if(cJSON_GetObjectItem(devData, "BytesReceived") != NULL)
                             {
                                uint32_t bytesReceived = atoi(cJSON_GetObjectItem(devData, "BytesReceived")->valuestring);
                                dev[count].cli_BytesReceived = bytesReceived;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_BytesReceived %ld i %d\n", count, dev[count].cli_BytesReceived, i));
                             }
                             if(cJSON_GetObjectItem(devData, "RSSI") != NULL)
                             {
                                int rssi = atoi(cJSON_GetObjectItem(devData, "RSSI")->valuestring);
                                dev[count].cli_RSSI = rssi;
                                CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_RSSI %d i %d\n", count, dev[count].cli_RSSI, i));
                             }
                             if(cJSON_GetObjectItem(devData, "MinRSSI") != NULL)
                             {
                                int minRssi = atoi(cJSON_GetObjectItem(devData, "MinRSSI")->valuestring);
                                dev[count].cli_MinRSSI = minRssi;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_MinRSSI %d i %d\n", count, dev[count].cli_MinRSSI, i));
                             }
                             if(cJSON_GetObjectItem(devData, "MaxRSSI") != NULL)
                             {
                                int maxRssi = atoi(cJSON_GetObjectItem(devData, "MaxRSSI")->valuestring);
                                dev[count].cli_MaxRSSI = maxRssi;
                               CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_MaxRSSI %d i %d\n", count, dev[count].cli_MaxRSSI, i));
                             }
                             if(cJSON_GetObjectItem(devData, "Disassociations") != NULL)
                             {
                                uint32_t disassociate = atoi(cJSON_GetObjectItem(devData, "Disassociations")->valuestring);
                                dev[count].cli_Disassociations = disassociate;
                                CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_Disassociations %d i %d\n", count, dev[count].cli_Disassociations, i));
                             }
                             if(cJSON_GetObjectItem(devData, "AuthenticationFailures") != NULL)
                             {
                                uint32_t authFail = atoi(cJSON_GetObjectItem(devData, "AuthenticationFailures")->valuestring);
                                dev[count].cli_AuthenticationFailures = authFail;
                                CcspHarvesterTrace(("RDK_LOG_DEBUG, Printing dev[%d] cli_AuthenticationFailures %d i %d\n", count, dev[count].cli_AuthenticationFailures, i));
                             }
                        }
                    }
               }
        	}
	}
	if(assocDevVal != NULL)
        {
        	CcspHarvesterTrace(("RDK_LOG_DEBUG, Harvester %s: Release assocDevVal if count is not 0\n", __FUNCTION__));
        	rbusValue_Release(assocDevVal);	
        }
	cJSON_Delete(jsonVal);
    }
    return 0;
}

int rbus_wifi_getRadioTrafficStats2(int radioIndex, wifi_radioTrafficStats2_t *output_struct)
{
    char path[128] = {'\0'};
    int rc, resCount = 0;
    rbusProperty_t props = NULL;
    rbusValue_t paramValue_t = NULL;

    if(!rbusInitializedCheck())
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_open failed\n", __FUNCTION__));
        return 1;
    }

    snprintf(path, sizeof(path), "Device.WiFi.Radio.%d.Stats.", radioIndex);

    const char * wildCardPath[1];
    wildCardPath[0] = path;

    rc = rbus_getExt(rbus_handle, 1, wildCardPath, &resCount, &props);

    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_getExt failed for [%s] with error [%d]\n", __FUNCTION__, path, rc));
        rbusProperty_Release(props);
        return 1;
    }

    if(props != NULL)
    {
        rbusProperty_t next = props;
        while(next != NULL)
        {
            paramValue_t = rbusProperty_GetValue(next);
            if(paramValue_t != NULL)
            {
                char * propName =(char *)rbusProperty_GetName(next);
                if(propName != NULL)
                {
                    if(strstr(propName, ".BytesSent") != NULL)
                    {
                        uint32_t  bytesSent = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_BytesSent = bytesSent;
                    }
                    else if(strstr(propName, ".BytesReceived") != NULL)
                    {
                        uint32_t  bytesReceived = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_BytesReceived = bytesReceived;
                    }
                    else if(strstr(propName, ".PacketsSent") != NULL)
                    {
                        uint32_t  packetsSent = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_PacketsSent = packetsSent;
                    }
                    else if(strstr(propName, ".PacketsReceived") != NULL)
                    {
                        uint32_t  packetsReceived = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_PacketsReceived = packetsReceived;
                    }
                    else if(strstr(propName, ".ErrorsSent") != NULL)
                    {
                        uint32_t  errorsSent = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_ErrorsSent = errorsSent;
                    }
                    else if(strstr(propName, ".ErrorsReceived") != NULL)
                    {
                        uint32_t  errorsReceived = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_ErrorsReceived = errorsReceived;
                    }
                    else if(strstr(propName, ".DiscardPacketsSent") != NULL)
                    {
                        uint32_t  discardPacketsSent = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_DiscardPacketsSent = discardPacketsSent;
                    }
                    else if(strstr(propName, ".DiscardPacketsReceived") != NULL)
                    {
                        uint32_t  discardPacketsReceived = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_DiscardPacketsReceived = discardPacketsReceived;
                    }
                    else if(strstr(propName, ".PLCPErrorCount") != NULL)
                    {
                         uint32_t  plcpErrorCount = rbusValue_GetUInt32(paramValue_t);
                         output_struct->radio_PLCPErrorCount = plcpErrorCount;
                    }
                    else if(strstr(propName, ".FCSErrorCount") != NULL)
                    {
                        uint32_t  fcsErrorCount = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_FCSErrorCount = fcsErrorCount;
                    }
                    else if(strstr(propName, ".InvalidMACCount") != NULL)
                    {
                        uint32_t  invalidMACCount = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_InvalidMACCount = invalidMACCount;
                    }
                    else if(strstr(propName, ".PacketsOtherReceived") != NULL)
                    {
                        uint32_t  packetsOtherReceived = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_PacketsOtherReceived = packetsOtherReceived;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_NoiseFloor") != NULL)
                    {
                        int noiseFloor = rbusValue_GetInt32(paramValue_t);
                        output_struct->radio_NoiseFloor = noiseFloor;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_ChannelUtilization") != NULL)
                    {
                        uint32_t  channelUtilization = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_ChannelUtilization = channelUtilization;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_ActivityFactor") != NULL)
                    {
                        int activityFactor = rbusValue_GetInt32(paramValue_t);
                        output_struct->radio_ActivityFactor = activityFactor;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_CarrierSenseThreshold_Exceeded") != NULL)
                    {
                        int carrierSenseThreshold_Exceeded = rbusValue_GetInt32(paramValue_t);
                        output_struct->radio_CarrierSenseThreshold_Exceeded = carrierSenseThreshold_Exceeded;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_RetransmissionMetric") != NULL)
                    {
                        int retransmissionMetric = rbusValue_GetInt32(paramValue_t);
                        output_struct->radio_RetransmissionMetirc = retransmissionMetric;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_MaximumNoiseFloorOnChannel") != NULL)
                    {
                        int maximumNoiseFloorOnChannel = rbusValue_GetInt32(paramValue_t);
                        output_struct->radio_MaximumNoiseFloorOnChannel = maximumNoiseFloorOnChannel;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_MinimumNoiseFloorOnChannel") != NULL)
                    {
                        int minimumNoiseFloorOnChannel = rbusValue_GetInt32(paramValue_t);
                        output_struct->radio_MinimumNoiseFloorOnChannel = minimumNoiseFloorOnChannel;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_MedianNoiseFloorOnChannel") != NULL)
                    {
                        int medianNoiseFloorOnChannel = rbusValue_GetInt32(paramValue_t);
                        output_struct->radio_MedianNoiseFloorOnChannel = medianNoiseFloorOnChannel;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_StatisticsStartTime") != NULL)
                    {
                        uint32_t  statisticsStartTime = rbusValue_GetUInt32(paramValue_t);
                        output_struct->radio_StatisticsStartTime = statisticsStartTime;
                    }
                }
            }
            next = rbusProperty_GetNext(next);
        }
        rbusProperty_Release(props);
    }
    return 0;
}

int rbus_wifi_getNeighboringWiFiDiagnosticResult2(bool *executed, wifi_neighbor_ap2_t **neighbor_ap_array, UINT *array_size)
{
    int rc = 0;
    bool isCommit = true;
    int sessionId = 0;
    rbusError_t ret = RBUS_ERROR_BUS_ERROR;
    rbusProperty_t last = NULL, next = NULL;
    rbusValue_t setVal[2];

    if(!rbusInitializedCheck())
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_open failed\n", __FUNCTION__));
        return 1;
    }

    rbusSetOptions_t opts = {isCommit,sessionId};

    rbusValue_Init(&setVal[0]);
    rbusValue_SetFromString(setVal[0], RBUS_BOOLEAN, "true");

    rbusValue_Init(&setVal[1]);
    rbusValue_SetFromString(setVal[1], RBUS_STRING, "Requested");

    rbusProperty_Init(&next, "Device.WiFi.NeighboringWiFiDiagnostic.Enable", setVal[0]);
    rbusProperty_Init(&last, "Device.WiFi.NeighboringWiFiDiagnostic.DiagnosticsState", setVal[1]);
    rbusProperty_SetNext(next, last);

    ret = rbus_setMulti(rbus_handle, 2, next, &opts);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_set failed for [%s] with error [%d]\n", __FUNCTION__, "NeighboringWiFiDiagnostic", rc));
        if(setVal != NULL)
        {
            rbusValue_Release(setVal[0]);
            rbusValue_Release(setVal[1]);
        }
        return 1;
    }
    CcspHarvesterConsoleTrace(("RDK_LOG_INFO, Harvester %s: rbus_set to enable neighboring wifi diagnostics results is success %d\n", __FUNCTION__, ret));

    rbusValue_Release(setVal[0]);
    rbusValue_Release(setVal[1]);
    rbusProperty_Release(next);

    // Wait until the Device.WiFi.NeighboringWiFiDiagnostic.DiagnosticsState is "Completed"
    char state[128] = {0};
    char *stateParam = "Device.WiFi.NeighboringWiFiDiagnostic.DiagnosticsState";
    int cnt=0;
    while(ret == RBUS_ERROR_SUCCESS)
    {
        ret = rbus_getStringValue(state, stateParam);
        if(strncmp(state, "Completed", 9) == 0)
            break;
        else
            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s: rbus get of %s is %s, ret %d\n", __FUNCTION__, stateParam, state, ret));
        cnt++;
        if(cnt == 13) // Reached max wait of 12*5 = 60s, hence breakout of loop
            break;
        sleep(5);
    }

    rbusValue_t neighWiFiResultVal;
    char *path = "Device.WiFi.NeighboringWiFiDiagnostic.ResultNumberOfEntries";
    char wildcardPath[128] = {'\0'};
    char *params[1];
    rbusProperty_t props = NULL;
    int count = 0, resCount = 0;
    rbusValue_t paramValue_t = NULL;
    rc = rbus_get(rbus_handle, path, &neighWiFiResultVal);

    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_get failed for [%s] with error [%d]\n", __FUNCTION__, path, rc));
        if(neighWiFiResultVal != NULL)
        {
            rbusValue_Release(neighWiFiResultVal);
        }
        return 1;
    }

    *array_size = rbusValue_GetUInt32(neighWiFiResultVal);

    if(*array_size == 0)
    {
        CcspHarvesterTrace(("RDK_LOG_INFO, Harvester %s: neighboring wifi diagnostics results are not available\n", __FUNCTION__));
        return 1;
    }
    CcspHarvesterTrace(("RDK_LOG_INFO, Harvester %s: %d neighboring wifi diagnostics results available\n", __FUNCTION__, *array_size));

    wifi_neighbor_ap2_t *tmp_neighbor_ap_arr = NULL;

    tmp_neighbor_ap_arr = (wifi_neighbor_ap2_t *) calloc(*array_size, sizeof(wifi_neighbor_ap2_t));
    *neighbor_ap_array = tmp_neighbor_ap_arr;

    snprintf(wildcardPath, sizeof(wildcardPath), "Device.WiFi.NeighboringWiFiDiagnostic.Result.");
    params[0] = wildcardPath;

    CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s: calling rbus get for %s\n", __FUNCTION__, wildcardPath));

    rc = rbus_getExt(rbus_handle, 1, (const char **) params, &resCount, &props);

    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: rbus_getExt failed for [%s] with error [%d]\n", __FUNCTION__, wildcardPath, rc));
        rbusProperty_Release(props);
        return 1;
    }
    if(props != NULL)
    {
        rbusProperty_t next = props;

        while(next != NULL)
        {
            paramValue_t = rbusProperty_GetValue(next);

            if(paramValue_t != NULL)
            {
                char * propName = (char *) rbusProperty_GetName(next);

                sscanf(propName, "Device.WiFi.NeighboringWiFiDiagnostic.Result.%d", &count);
		count = count-1;
		if(count<0)
		{
			CcspHarvesterTrace(("RDK_LOG_ERROR, Harvester %s: the index %d, is less than 0\n", wildcardPath, count));
			return 1;
		}
                CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s: %d results available, iteration %d\n", wildcardPath, resCount, count));
                if(propName != NULL)
                {
                    if(strstr(propName, ".SSID") != NULL)
                    {
                        char * ssid = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(ssid != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_SSID,64,ssid);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".BSSID") != NULL)
                    {
                        char * bssid = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(bssid != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_BSSID,64,bssid);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".Mode") != NULL)
                    {
                        char * mode = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(mode != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_Mode,64,mode);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".Channel") != NULL)
                    {
                        uint32_t channel = rbusValue_GetUInt32(paramValue_t);
                        tmp_neighbor_ap_arr[count].ap_Channel = channel;
                    }
                    else if(strstr(propName, ".SignalStrength") != NULL)
                    {
                        int signalStrength  = rbusValue_GetInt32(paramValue_t);
                        tmp_neighbor_ap_arr[count].ap_SignalStrength = signalStrength;
                    }
                    else if(strstr(propName, ".SecurityModeEnabled") != NULL)
                    {
                        char * securityModeEnabled = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(securityModeEnabled != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_SecurityModeEnabled,64,securityModeEnabled);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".EncryptionMode") != NULL)
                    {
                        char * encryptionMode = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(encryptionMode != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_EncryptionMode,64,encryptionMode);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".OperatingFrequencyBand") != NULL)
                    {
                        char * operatingFrequencyBand = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(operatingFrequencyBand != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_OperatingFrequencyBand,16, operatingFrequencyBand);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".SupportedStandards") != NULL)
                    {
                        char * supportedStandards = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(supportedStandards != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_SupportedStandards,64,supportedStandards);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".OperatingStandards") != NULL)
                    {
                        char * operatingStandards = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(operatingStandards != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_OperatingStandards,16,operatingStandards);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".OperatingChannelBandwidth") != NULL)
                    {
                        char * operatingChannelBandwidth = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(operatingChannelBandwidth != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_OperatingChannelBandwidth,16, operatingChannelBandwidth);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".BeaconPeriod") != NULL)
                    {
                        uint32_t beaconPeriod = rbusValue_GetUInt32(paramValue_t);
                        tmp_neighbor_ap_arr[count].ap_BeaconPeriod = beaconPeriod;
                    }
                    else if(strstr(propName, ".Noise") != NULL)
                    {
                        int noise = rbusValue_GetInt32(paramValue_t);
                        tmp_neighbor_ap_arr[count].ap_Noise = noise;
                    }
                    else if(strstr(propName, ".BasicDataTransferRates") != NULL)
                    {
                        char * basicDataTransferRates = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(basicDataTransferRates != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_BasicDataTransferRates,256, basicDataTransferRates);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".SupportedDataTransferRates") != NULL)
                    {
                        char * supportedDataTransferRates = (char *)rbusValue_GetString(paramValue_t, NULL);
                        if(supportedDataTransferRates != NULL)
                        {
                            strcpy_s(tmp_neighbor_ap_arr[count].ap_SupportedDataTransferRates,256, supportedDataTransferRates);
                        }
                        else
                            CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester %s value is NULL\n", propName));
                    }
                    else if(strstr(propName, ".DTIMPeriod") != NULL)
                    {
                        uint32_t dtimPeriod = rbusValue_GetUInt32(paramValue_t);
                        tmp_neighbor_ap_arr[count].ap_DTIMPeriod = dtimPeriod;
                    }
                    else if(strstr(propName, ".X_COMCAST-COM_ChannelUtilization") != NULL)
                    {
                        uint32_t channelUtilization = rbusValue_GetUInt32(paramValue_t);
                        tmp_neighbor_ap_arr[count].ap_ChannelUtilization = channelUtilization;
                    }
                }
                else
                    CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester propName is NULL\n"));
            }
            else
                CcspHarvesterConsoleTrace(("RDK_LOG_DEBUG, Harvester paramValue_t is NULL\n" ));
            next = rbusProperty_GetNext(next);
        }
        rbusProperty_Release(props);
    }

    // setting the flag to indicate the diagnostic is already run
    *executed = true;
    return 0;
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
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: Unable to handle set request for property\n", __FUNCTION__));
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
    CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: Setting MLO RFC to %s\n", __FUNCTION__, paramVal ? "true" : "false"));

    if (set_HarvesterMLORfcEnable(paramVal) != 0) {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: set_HarvesterMLORfcEnable failed\n", __FUNCTION__));
        return RBUS_ERROR_BUS_ERROR;
    }

    CcspHarvesterTrace(("RDK_LOG_INFO, %s: MLO RFC set successfully to %s\n", 
                        __FUNCTION__, paramVal ? "true" : "false"));
    return RBUS_ERROR_SUCCESS;
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
    bool mloRfcEnabled = false;

    char *tmpchar = NULL;

    /* Get value from PSM DB */
    retPsmGet = rbus_GetValueFromPsmDB(HARVESTER_MLO_RFC_PARAM, &tmpchar);
    if (retPsmGet == RBUS_ERROR_SUCCESS)
    {
      if (tmpchar != NULL)
      {
          if ((strcmp(tmpchar, "true") == 0) || (strcmp(tmpchar, "TRUE") == 0))
          {
            pthread_mutex_lock(&mlorfc_mut);
            g_MLORfcEnabled = true;
            pthread_mutex_unlock(&mlorfc_mut);
          }
          else
          {
            pthread_mutex_lock(&mlorfc_mut);
            g_MLORfcEnabled = false;
            pthread_mutex_unlock(&mlorfc_mut);
          }
          CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: MLO RFC value from PSM = %s\n", __FUNCTION__, tmpchar));
          free(tmpchar);
      }
    }
    else
    {
        if (tmpchar)
            free(tmpchar);
        pthread_mutex_lock(&mlorfc_mut);
        CcspHarvesterTrace(("RDK_LOG_WARN, %s: PSM get failed ret %d, using cached value %d\n",__FUNCTION__, retPsmGet, g_MLORfcEnabled));
        pthread_mutex_unlock(&mlorfc_mut);
    }

    /* Use cached in-memory value; PSM is read at init and on set */
    pthread_mutex_lock(&mlorfc_mut);
    mloRfcEnabled = g_MLORfcEnabled;
    pthread_mutex_unlock(&mlorfc_mut);

    rbusValue_Init(&value);
    rbusValue_SetBoolean(value, mloRfcEnabled);
    rbusProperty_SetValue(property, value);
    rbusValue_Release(value);

    CcspHarvesterTrace(("RDK_LOG_INFO, %s: Mlo Rfc value fetched is %s\n", __FUNCTION__, mloRfcEnabled ? "true" : "false"));
    return RBUS_ERROR_SUCCESS;
}

/**
 * @brief Set MLO RFC enable status and persist to PSM
 */
int set_HarvesterMLORfcEnable(bool bValue)
{
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
    CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: PSM set success for parameter %s and value %s\n", __FUNCTION__, HARVESTER_MLO_RFC_PARAM, buf));

    /* Update global MLO RFC variable under mutex to avoid data races */
    pthread_mutex_lock(&mlorfc_mut);
    g_MLORfcEnabled = bValue;
    pthread_mutex_unlock(&mlorfc_mut);
    if(bValue == true)
    {
        CcspHarvesterTrace(("RDK_LOG_INFO, Harvester MLO RFC is enabled\n"));
    }
    else
    {
        CcspHarvesterTrace(("RDK_LOG_INFO, Harvester MLO RFC is disabled\n"));
    }
    free(buf);
    return 0;
}

/**
 * @brief Get MLO RFC enable status
 */
bool get_HarvesterMLORfcEnable(void)
{
    bool isRfc = false;
    pthread_mutex_lock(&mlorfc_mut);
    isRfc = g_MLORfcEnabled;
    pthread_mutex_unlock(&mlorfc_mut);
    return isRfc;
}


/**
 * To persist TR181 parameter values in PSM DB.
 */
int rbus_StoreValueIntoPsmDB(char *paramName, char *value)
{
    rbusHandle_t rbus_handle = get_rbus_handle();
    rbusObject_t inParams;
    rbusObject_t outParams;
    rbusValue_t setvalue;
    int rc = RBUS_ERROR_SUCCESS;

    if(!rbus_handle)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: failed as rbus_handle is empty\n", __FUNCTION__));
        return 1;
    }

    rbusObject_Init(&inParams, NULL);
    rbusValue_Init(&setvalue);
    rbusValue_SetString(setvalue, value);
    rbusObject_SetValue(inParams, paramName, setvalue);
    rbusValue_Release(setvalue);

    rc = rbusMethod_Invoke(rbus_handle, "SetPSMRecordValue()", inParams, &outParams);
    rbusObject_Release(inParams);
    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: SetPSMRecordValue failed with err %d: %s\n", __FUNCTION__, rc, rbusError_ToString(rc)));
    }
    else
    {
        CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: SetPSMRecordValue is success\n", __FUNCTION__));
        rbusObject_Release(outParams);
        return 0;
    }
    return 1;
}

/**
 * To fetch TR181 parameter values from PSM DB.
 */
int rbus_GetValueFromPsmDB( char* paramName, char** paramValue)
{
    rbusHandle_t rbus_handle = get_rbus_handle();
    rbusObject_t inParams;
    rbusObject_t outParams;
    rbusValue_t setvalue;
    int rc = RBUS_ERROR_SUCCESS;

    if(!rbus_handle)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: failed as rbus_handle is empty\n", __FUNCTION__));
        return 1;
    }

    rbusObject_Init(&inParams, NULL);
    rbusValue_Init(&setvalue);
    rbusValue_SetString(setvalue, "value");
    rbusObject_SetValue(inParams, paramName, setvalue);
    rbusValue_Release(setvalue);

    rc = rbusMethod_Invoke(rbus_handle, "GetPSMRecordValue()", inParams, &outParams);
    rbusObject_Release(inParams);
    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspHarvesterTrace(("RDK_LOG_ERROR, %s: GetPSMRecordValue failed with err %d: %s\n", __FUNCTION__, rc, rbusError_ToString(rc)));
    }
    else
    {
        CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: GetPSMRecordValue is success\n", __FUNCTION__));
        rbusProperty_t prop = NULL;
        rbusValue_t value = NULL;
        const char *str_value = NULL;
        prop = rbusObject_GetProperties(outParams);
        while(prop)
        {
            value = rbusProperty_GetValue(prop);
            if(value)
            {
                str_value = rbusValue_ToString(value,NULL,0);
                if(str_value)
                {
                    CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: Parameter Name : %s\n", __FUNCTION__, rbusProperty_GetName(prop)));
                    CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: Parameter Value fetched: %s\n", __FUNCTION__, str_value));
                }
            }
            prop = rbusProperty_GetNext(prop);
        }
        if(str_value != NULL)
        {
            *paramValue = strdup(str_value);
            if(*paramValue == NULL)
            {
                CcspHarvesterTrace(("RDK_LOG_ERROR, %s: strdup failed for parameter value\n", __FUNCTION__));
                rbusObject_Release(outParams);
                return 1;
            }
            CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: Requested param DB value [%s]\n", __FUNCTION__, *paramValue));
            rbusObject_Release(outParams);
            return 0;
        }
    }
    return 1;
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

    CcspHarvesterTrace(("RDK_LOG_DEBUG, %s: Registering MLO RFC parameter %s\n", __FUNCTION__, HARVESTER_MLO_RFC_PARAM));

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
// End of File

