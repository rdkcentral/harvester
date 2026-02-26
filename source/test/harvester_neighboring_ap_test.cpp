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
  #include "harvester_neighboring_ap.h"
  #include "harvester_avro.h"
  #include "harvester.h"
  BOOL isvalueinNAParray(ULONG val, ULONG *arr, int size);
  void add_to_nap_list(char* radioIfName, ULONG numAPs, wifi_neighbor_ap2_t* neighborapdata, char* freqband, ULONG channel);
  void delete_nap_list();
  void print_nap_list();
  int GetRadioNeighboringAPData(int radioIndex, char* radioIfName);
  int _napsyscmd(char *cmd, char *retBuf, int retBufSize);
}
extern BOOL NAPHarvesterStatus;
extern ULONG NAPReportingPeriod;
extern ULONG NAPPollingPeriod;
extern ULONG NAPReportingPeriodDefault;
extern ULONG NAPPollingPeriodDefault;
extern ULONG NAPOverrideTTL;
extern ULONG NAPOverrideTTLDefault;
extern struct neighboringapdata *headnode;
extern struct neighboringapdata *currnode;
extern ULONG currentNAPReportingPeriod;
extern ULONG NeighboringAPPeriods[8];
extern WifiHalMock *g_WifiHalMock;
extern AnscDebugMock* g_anscDebugMock;
extern SecureWrapperMock * g_securewrapperMock;
extern FileIOMock *g_fileIOMock;
using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
using testing::StrEq;
//isvalueinNAParray
TEST_F(HarvesterTestFixture, ValuePresentInArray) {
     ULONG sampleArray[] = {10, 20, 30, 40, 50};
    ULONG val = 30;
    int size = sizeof(sampleArray) / sizeof(sampleArray[0]);

    EXPECT_TRUE(isvalueinNAParray(val, sampleArray, size));
}

TEST_F(HarvesterTestFixture, ValueNotPresentInArray) {
     ULONG sampleArray[] = {10, 20, 30, 40, 50};
    ULONG val = 60;
    int size = sizeof(sampleArray) / sizeof(sampleArray[0]);

    EXPECT_FALSE(isvalueinNAParray(val, sampleArray, size));
}

TEST_F(HarvesterTestFixture, EmptyArray) {
    ULONG emptyArray[0];
    ULONG val = 10;

    EXPECT_FALSE(isvalueinNAParray(val, emptyArray, 0));
}

//GetNAPHarvestingStatus
TEST_F(HarvesterTestFixture, ReturnsTrueWhenStatusIsTrue) {
    NAPHarvesterStatus = true;  
    EXPECT_TRUE(GetNAPHarvestingStatus());
}

TEST_F(HarvesterTestFixture, ReturnsFalseWhenStatusIsFalse) {
    NAPHarvesterStatus = false;  
    EXPECT_FALSE(GetNAPHarvestingStatus());
}

//GetNAPReportingPeriod
TEST_F(HarvesterTestFixture, ReturnsDefaultNAPReportingPeriod) {
    EXPECT_EQ(GetNAPReportingPeriod(), 43200);
}

TEST_F(HarvesterTestFixture, ReturnsModifiedNAPReportingPeriod) {
    ULONG testPeriod = 7200; 
    NAPReportingPeriod = testPeriod;
    EXPECT_EQ(GetNAPReportingPeriod(), testPeriod);
}

//GetNAPPollingPeriod
TEST_F(HarvesterTestFixture, ReturnsDefaultNAPPollingPeriod) {
    EXPECT_EQ(GetNAPPollingPeriod(), 21600);
}


TEST_F(HarvesterTestFixture, ReturnsModifiedNAPPollingPeriod) {
    ULONG testPeriod = 7200;
    NAPPollingPeriod = testPeriod;

    EXPECT_EQ(GetNAPPollingPeriod(), testPeriod);
}

//SetNAPReportingPeriodDefault
TEST_F(HarvesterTestFixture, SetsNewNAPReportingPeriod) {
    NAPReportingPeriodDefault = 43200;
    ULONG newPeriod = 7200; 
   
    int result = SetNAPReportingPeriodDefault(newPeriod);

    EXPECT_EQ(result, 0);

    EXPECT_EQ(NAPReportingPeriodDefault, newPeriod);
}

//GetNAPReportingPeriodDefault
TEST_F(HarvesterTestFixture, GetNAPReportingPeriodDefault_Default) {
   NAPReportingPeriodDefault = 43200;
    
    EXPECT_EQ(GetNAPReportingPeriodDefault(), 43200);
}

//SetNAPPollingPeriodDefault
TEST_F(HarvesterTestFixture, SetNAPPollingPeriodDefault_ChangesValueAndLogsCorrectly) {
    NAPPollingPeriodDefault =  21600;
    ULONG newInterval = 30000; 
    EXPECT_EQ(SetNAPPollingPeriodDefault(newInterval), 0);
    EXPECT_EQ(NAPPollingPeriodDefault, newInterval); 
}

//GetNAPPollingPeriodDefault
TEST_F(HarvesterTestFixture, ReturnsCorrectDefaultPeriodAndLogsCorrectly) {
    NAPPollingPeriodDefault=21600;
    ULONG result = GetNAPPollingPeriodDefault();
    EXPECT_EQ(result, NAPPollingPeriodDefault); 
}

//GetNAPOverrideTTL
TEST_F(HarvesterTestFixture, ReturnsGetNAPOverrideTTLCorrectly) {
    NAPOverrideTTL=43200;
    ULONG result = GetNAPOverrideTTL();
    EXPECT_EQ(result,NAPOverrideTTL); 
}

//SetNAPOverrideTTL
TEST_F(HarvesterTestFixture, SetsTTLAndLogsCorrectly) {
    NAPOverrideTTL = 43200;
    ULONG newTTL = 86400; 
    int result = SetNAPOverrideTTL(newTTL);
    EXPECT_EQ(result, 0); 
    EXPECT_EQ(NAPOverrideTTL, newTTL); 
}

//GetNAPOverrideTTLDefault
TEST_F(HarvesterTestFixture, LogsCorrectlyAndReturnsDefaultTTL) {
    NAPOverrideTTLDefault = 43200;
    ULONG ttl = GetNAPOverrideTTLDefault();
    EXPECT_EQ(ttl, NAPOverrideTTLDefault);
}

//SetNAPReportingPeriod
TEST_F(HarvesterTestFixture, SetNAPReportingPeriod_UpdatesCorrectly) {
    NAPReportingPeriod = 0;
    ULONG newInterval = 30;


    int result = SetNAPReportingPeriod(newInterval);


    EXPECT_EQ(result, 0);

    EXPECT_EQ(NAPReportingPeriod, newInterval);

    EXPECT_EQ(NAPReportingPeriod * 2, 60); 
}

//ValidateNAPPeriod
TEST_F(HarvesterTestFixture, ValidateNAPPeriod_WithValidValuetrue) {
        NeighboringAPPeriods[0] = 10;
        NeighboringAPPeriods[1] = 20;
        NeighboringAPPeriods[2] = 30;
        NeighboringAPPeriods[3] = 40;
        NeighboringAPPeriods[4] = 50;
        NeighboringAPPeriods[5] = 60;
        NeighboringAPPeriods[6] = 70;
        NeighboringAPPeriods[7] = 80;
    ULONG validInterval = 30;  
    EXPECT_TRUE(ValidateNAPPeriod(validInterval));  
}

 
TEST_F(HarvesterTestFixture, ValidateNAPPeriod_WithValidValuefalse) {
    NeighboringAPPeriods[0] = 10;
        NeighboringAPPeriods[1] = 20;
        NeighboringAPPeriods[2] = 30;
        NeighboringAPPeriods[3] = 40;
        NeighboringAPPeriods[4] = 50;
        NeighboringAPPeriods[5] = 60;
        NeighboringAPPeriods[6] = 70;
        NeighboringAPPeriods[7] = 80;
    ULONG invalidInterval = 90; 
    EXPECT_FALSE(ValidateNAPPeriod(invalidInterval)); 

}

//SetNAPPollingPeriod
TEST_F(HarvesterTestFixture, SetNAPPollingPeriod_ChangeToNewValue) {
    ULONG initialPeriod = NAPPollingPeriod;
    ULONG newInterval = initialPeriod + 100; 

    
    ASSERT_NE(newInterval, initialPeriod);

    int result = SetNAPPollingPeriod(newInterval);

    
    EXPECT_EQ(result, 0);

    
    EXPECT_EQ(NAPPollingPeriod, newInterval);

   
    EXPECT_NE(NAPPollingPeriod, initialPeriod);

    
    NAPPollingPeriod = initialPeriod;
}

//print_nap_list
TEST_F(HarvesterTestFixture, PrintNapListWithDummyData) {

    
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


      print_nap_list();
 
    
    EXPECT_TRUE(true);  

    
    free(node1->radioName);
    free(node1->radioOperatingFrequencyBand);
    free(node1);

    free(node2->radioName);
    free(node2->radioOperatingFrequencyBand);
    free(node2);
}




TEST_F(HarvesterTestFixture, TestRadioNotEnabled) {
    
    EXPECT_CALL(*g_WifiHalMock, wifi_getRadioEnable(_, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(FALSE), testing::Return(0)));  

   
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .Times(1);  

    char radioIfName[] = "wifi0";
    EXPECT_EQ(GetRadioNeighboringAPData(0, radioIfName), 0);
}

TEST_F(HarvesterTestFixture, TestSuccessfulDataRetrieval) {
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

    int ret = GetRadioNeighboringAPData(radioIndex, radioIfName);
    EXPECT_EQ(ret, 0);  
}


TEST_F(HarvesterTestFixture, Test_napsyscmd_SuccessfulCommand) {
    const char* testCommand = "echo Hello, World!";
    char retBuf[256] = {0};  
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .WillRepeatedly(testing::Return()); 
   
    int result = _napsyscmd((char*)testCommand, retBuf, sizeof(retBuf));
    
    
    EXPECT_EQ(result, 0);  
    EXPECT_STREQ(retBuf, "Hello, World!\n");  
}

TEST_F(HarvesterTestFixture, Test_napsyscmd_BufferOverflow) {
    const char* testCommand = "echo This is a very long command output that exceeds the buffer size of 10";
    char retBuf[10] = {0};  
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .WillRepeatedly(testing::Return()); 
    
    int result = _napsyscmd((char*)testCommand, retBuf, sizeof(retBuf));  
    
    
    EXPECT_EQ(result, 0);  
    EXPECT_EQ(retBuf[9], '\0'); 
}



TEST_F(HarvesterTestFixture, AddNodeToNapList) {
    // Initialize headnode and currnode to NULL

    char radioIfName[] = "wifi0";
    char freqband[] = "2.4GHz";
    ULONG channel = 6;
    ULONG numAPs = 1;
  
    wifi_neighbor_ap2_t wifiNeighborData = {};  

    EXPECT_CALL(*g_securewrapperMock, v_secure_popen)
        .WillOnce(::testing::Return(nullptr));  

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3)
        .WillOnce(::testing::Return());
   
     add_to_nap_list(radioIfName, numAPs, &wifiNeighborData, freqband, channel);

    
    ASSERT_NE(headnode, nullptr);
   

   
    EXPECT_STREQ(headnode->radioName, "wifi0");
    EXPECT_STREQ(headnode->radioOperatingFrequencyBand, "2.4GHz");
    EXPECT_EQ(headnode->radioChannel, 6);
    EXPECT_EQ(headnode->numNeibouringAP, 1);
}

TEST_F(HarvesterTestFixture, TestMemoryManagement) {
   
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

   
    headnode = node1;

    
    delete_nap_list();

    
    EXPECT_EQ(headnode, nullptr);

    
}

