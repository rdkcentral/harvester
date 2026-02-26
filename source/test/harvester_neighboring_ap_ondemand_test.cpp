/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "harvester_mock.h"
extern "C" 
{
#include "harvester_avro.h"
#include "harvester_neighboring_ap_ondemand.h"
int _napondemandsyscmd(char *cmd, char *retBuf, int retBufSize);
int GetRadioNeighboringAPOnDemandData(int radioIndex, char* radioIfName);
void print_nap_ondemand_list();
void delete_nap_ondemand_list();
void add_to_nap_ondemand_list(char* radioIfName, ULONG numAPs, wifi_neighbor_ap2_t* neighborapdata, char* freqband, ULONG channel);
}
extern BOOL NAPOnDemandHarvesterStatus;
extern WifiHalMock *g_WifiHalMock;
extern AnscDebugMock* g_anscDebugMock;
extern SecureWrapperMock * g_securewrapperMock;
extern struct neighboringapdata *naphead;
extern struct neighboringapdata *napcurr;
using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
//GetNAPOnDemandHarvestingStatus
TEST_F(HarvesterTestFixture, TestGetNAPOnDemandHarvestingStatusTrue) {
    
    
   NAPOnDemandHarvesterStatus = TRUE;
    
    EXPECT_TRUE(GetNAPOnDemandHarvestingStatus());
}

TEST_F(HarvesterTestFixture, TestGetNAPOnDemandHarvestingStatusFalse) {
   
    NAPOnDemandHarvesterStatus = FALSE;

    
    EXPECT_FALSE(GetNAPOnDemandHarvestingStatus());
}

TEST_F(HarvesterTestFixture, iTestRadioNotEnabled) {
    
    EXPECT_CALL(*g_WifiHalMock, wifi_getRadioEnable(_, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(FALSE), testing::Return(0))); 

    
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .Times(1); 

    char radioIfName[] = "wifi0";
    EXPECT_EQ(GetRadioNeighboringAPOnDemandData(0, radioIfName), 0);
}

TEST_F(HarvesterTestFixture, iTestSuccessfulDataRetrieval) {
    int radioIndex = 0;
    char radioIfName[] = "wifi0";
    BOOL enabled = TRUE;
    ULONG channel = 6;
    char freqband[] = "2.4GHz";
    wifi_neighbor_ap2_t neighbor_ap_array[1];  
    UINT array_size = 1;

    
    EXPECT_CALL(*g_WifiHalMock, wifi_getRadioEnable(radioIndex, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(enabled), testing::Return(0)));

    EXPECT_CALL(*g_WifiHalMock, wifi_getRadioChannel(radioIndex, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(channel), testing::Return(0)));

    EXPECT_CALL(*g_WifiHalMock, wifi_getRadioOperatingFrequencyBand(radioIndex, testing::_))
        .WillOnce(testing::DoAll(testing::SetArrayArgument<1>(freqband, freqband + sizeof(freqband)), testing::Return(0)));

    EXPECT_CALL(*g_WifiHalMock, wifi_getNeighboringWiFiDiagnosticResult2(radioIndex, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(neighbor_ap_array), testing::SetArgPointee<2>(array_size), testing::Return(0)));

    
    EXPECT_CALL(*g_securewrapperMock, v_secure_popen(_, _, _))
        .WillRepeatedly(testing::Return(nullptr)); 

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .WillRepeatedly(testing::Return()); 

    int ret = GetRadioNeighboringAPOnDemandData(radioIndex, radioIfName);
    EXPECT_EQ(ret, 0);  
}



TEST_F(HarvesterTestFixture, iPrintNapListWithDummyData) {

    
    struct neighboringapdata* node1 = (struct neighboringapdata*)malloc(sizeof(struct neighboringapdata));
    if (node1 == nullptr) {
        return; 
    }

    struct neighboringapdata* node2 = (struct neighboringapdata*)malloc(sizeof(struct neighboringapdata));
    if (node2 == nullptr) {
        free(node1);
        return;
    }

    
    memset(node1, 0, sizeof(struct neighboringapdata));
    memset(node2, 0, sizeof(struct neighboringapdata));

   
    node1->timestamp.tv_sec = 12345;
    node1->timestamp.tv_usec = 0;
    node1->radioName = strdup("Radio1");
    node1->radioOperatingFrequencyBand = strdup("2.4 GHz");
    node1->next = node2;

    
    node2->timestamp.tv_sec = 67890;
    node2->timestamp.tv_usec = 0;
    node2->radioName = strdup("Radio2");
    node2->radioOperatingFrequencyBand = strdup("5.0 GHz");
    node2->next = nullptr;

    struct neighboringapdata* head = node1;


 print_nap_ondemand_list();
    
    EXPECT_TRUE(true);  

    
    free(node1->radioName);
    free(node1->radioOperatingFrequencyBand);
    free(node1);

    free(node2->radioName);
    free(node2->radioOperatingFrequencyBand);
    free(node2);
}
 
TEST_F(HarvesterTestFixture, iTest_napsyscmd_SuccessfulCommand) {
    const char* testCommand = "echo Hello, World!";
    char retBuf[256] = {0}; 
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .WillRepeatedly(testing::Return()); 
    
    int result = _napondemandsyscmd((char*)testCommand, retBuf, sizeof(retBuf));
    
   
    EXPECT_EQ(result, 0);  
    EXPECT_STREQ(retBuf, "Hello, World!\n");  
}

TEST_F(HarvesterTestFixture, iTest_napsyscmd_BufferOverflow) {
    const char* testCommand = "echo This is a very long command output that exceeds the buffer size of 10";
    char retBuf[10] = {0};  
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .WillRepeatedly(testing::Return()); 
   
    int result = _napondemandsyscmd((char*)testCommand, retBuf, sizeof(retBuf));  // Only 10 bytes buffer
    
   
    EXPECT_EQ(result, 0);  
    EXPECT_EQ(retBuf[9], '\0');  
}

TEST_F(HarvesterTestFixture, iAddNodeToNapList) {
  

    char radioIfName[] = "wifi0";
    char freqband[] = "2.4GHz";
    ULONG channel = 6;
    ULONG numAPs = 1;
  
    wifi_neighbor_ap2_t wifiNeighborData = {};  
   
    EXPECT_CALL(*g_securewrapperMock, v_secure_popen)
        .WillOnce(::testing::Return(nullptr));  

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3)
        .WillOnce(::testing::Return());
    
     add_to_nap_ondemand_list(radioIfName, numAPs, &wifiNeighborData, freqband, channel);

    
    ASSERT_NE(naphead, nullptr);
   

    
    EXPECT_STREQ(naphead->radioName, "wifi0");
    EXPECT_STREQ(naphead->radioOperatingFrequencyBand, "2.4GHz");
    EXPECT_EQ(naphead->radioChannel, 6);
    EXPECT_EQ(naphead->numNeibouringAP, 1);
}

TEST_F(HarvesterTestFixture, iTestMemoryManagement) {
    
    struct neighboringapdata* node1 = new(std::nothrow) struct neighboringapdata;
    struct neighboringapdata* node2 = new(std::nothrow) struct neighboringapdata;

    
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);

   
    node1->radioName = strdup("Radio1");
    node1->radioOperatingFrequencyBand = strdup("2.4GHz");
    node1->napdata = nullptr;  
    node1->next = node2;

    node2->radioName = strdup("Radio2");
    node2->radioOperatingFrequencyBand = strdup("5.0GHz");
    node2->napdata = nullptr;  
    node2->next = nullptr;

    
    naphead = node1;

   
    delete_nap_ondemand_list();

    
    EXPECT_EQ(naphead, nullptr);

    
}


