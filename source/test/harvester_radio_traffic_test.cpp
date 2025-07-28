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
#include <avro.h>
#include "harvester_mock.h"
extern "C" 
{
#include "harvester_avro.h"
#include "harvester_radio_traffic.h"
#include "harvester.h"
BOOL isvalueinRISarray(ULONG val, ULONG *arr, int size);
void _rtsyscmd(FILE *f, char *retBuf, int retBufSize);
void print_rt_list();
void delete_rt_list();

}
extern BOOL RISHarvesterStatus;
extern ULONG RISReportingPeriod;
extern ULONG RISOverrideTTL;
extern ULONG currentRISReportingPeriod;
extern ULONG RadioTrafficPeriods[13];
extern ULONG RISPollingPeriod;
extern ULONG RISReportingPeriodDefault;
extern ULONG RISPollingPeriodDefault;
extern ULONG RISOverrideTTLDefault;
extern char RadioBSSID[MAX_NUM_RADIOS][19];
extern AnscDebugMock* g_anscDebugMock;
extern WifiHalMock *g_WifiHalMock;
extern AnscDebugMock* g_anscDebugMock;
extern SecureWrapperMock * g_securewrapperMock;
extern struct radiotrafficdata rIndex[MAX_NUM_RADIOS];
using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
using testing::StrEq;
//isvalueinRISarray
TEST_F(HarvesterTestFixture, ValuePresentInArray_radio) {
     ULONG sampleArray[] = {10, 20, 30, 40, 50};
    ULONG val = 30;
    int size = sizeof(sampleArray) / sizeof(sampleArray[0]);

    EXPECT_TRUE(isvalueinRISarray(val, sampleArray, size));
}

TEST_F(HarvesterTestFixture, ValueNotPresentInArray_radio) {
     ULONG sampleArray[] = {10, 20, 30, 40, 50};
    ULONG val = 60;
    int size = sizeof(sampleArray) / sizeof(sampleArray[0]);

    EXPECT_FALSE(isvalueinRISarray(val, sampleArray, size));
}

TEST_F(HarvesterTestFixture, EmptyArray_radio) {
    ULONG emptyArray[0];
    ULONG val = 10;

    EXPECT_FALSE(isvalueinRISarray(val, emptyArray, 0));
}

 TEST_F(HarvesterTestFixture, ReturnsTrueWhenStatusIsTrue_radio) {
    RISHarvesterStatus = true;  
    EXPECT_TRUE(GetRISHarvestingStatus());
}

TEST_F(HarvesterTestFixture, ReturnsFalseWhenStatusIsFalse_radio) {
    RISHarvesterStatus = false;  
    EXPECT_FALSE(GetRISHarvestingStatus());
}


TEST_F(HarvesterTestFixture, SetRISReportingPeriod_UpdatesCorrectly) {
    RISReportingPeriod = 0;
    ULONG newInterval = 30;


    int result = SetRISReportingPeriod(newInterval);


    EXPECT_EQ(result, 0);

    EXPECT_EQ(RISReportingPeriod, newInterval);

    EXPECT_EQ(RISReportingPeriod * 2, 60); 
}

//SetRISOverrideTTL
TEST_F(HarvesterTestFixture, TestSetRISOverrideTTL) {

   
    ULONG newCount = 500;

    
    int result = SetRISOverrideTTL(newCount);

   
    EXPECT_EQ(result, 0);              
    EXPECT_EQ(RISOverrideTTL, 500);   
}

//GetRISReportingPeriod
TEST_F(HarvesterTestFixture, TestGetRISReportingPeriod) {
    
    RISReportingPeriod = 120;


    
    ULONG result = GetRISReportingPeriod();

    
    EXPECT_EQ(result, 120); 
}

TEST_F(HarvesterTestFixture, ValidateRISPeriod_WithValidValue) {
    // Initialize RadioTrafficPeriods with predefined intervals
    RadioTrafficPeriods[0] = 1;
    RadioTrafficPeriods[1] = 5;
    RadioTrafficPeriods[2] = 15;
    RadioTrafficPeriods[3] = 30;
    RadioTrafficPeriods[4] = 60;
    RadioTrafficPeriods[5] = 300;
    RadioTrafficPeriods[6] = 900;
    RadioTrafficPeriods[7] = 1800;
    RadioTrafficPeriods[8] = 3600;
    RadioTrafficPeriods[9] = 10800;
    RadioTrafficPeriods[10] = 21600;
    RadioTrafficPeriods[11] = 43200;
    RadioTrafficPeriods[12] = 86400;

    ULONG validInterval = 30;  
    EXPECT_TRUE(ValidateRISPeriod(validInterval));  
}

TEST_F(HarvesterTestFixture, ValidateRISPeriod_WithInvalidValue) {
   
    RadioTrafficPeriods[0] = 1;
    RadioTrafficPeriods[1] = 5;
    RadioTrafficPeriods[2] = 15;
    RadioTrafficPeriods[3] = 30;
    RadioTrafficPeriods[4] = 60;
    RadioTrafficPeriods[5] = 300;
    RadioTrafficPeriods[6] = 900;
    RadioTrafficPeriods[7] = 1800;
    RadioTrafficPeriods[8] = 3600;
    RadioTrafficPeriods[9] = 10800;
    RadioTrafficPeriods[10] = 21600;
    RadioTrafficPeriods[11] = 43200;
    RadioTrafficPeriods[12] = 86400;

    ULONG invalidInterval = 12345;  
    EXPECT_FALSE(ValidateRISPeriod(invalidInterval));  
}


TEST_F(HarvesterTestFixture, GetRISPollingPeriod_ReturnsCorrectValue) {
    
    RISPollingPeriod = 120; 

    
    ULONG result = GetRISPollingPeriod();

    
    EXPECT_EQ(result, 120);  
}

//SetRISReportingPeriodDefault
TEST_F(HarvesterTestFixture, SetRISReportingPeriodDefault_Success) {
   
    RISReportingPeriodDefault = 60;  

    ULONG newInterval = 120;  
    int result = SetRISReportingPeriodDefault(newInterval);

   
    EXPECT_EQ(result, 0);  
    EXPECT_EQ(RISReportingPeriodDefault, newInterval); 
}

//GetRISReportingPeriodDefault
TEST_F(HarvesterTestFixture, GetRISReportingPeriodDefault_ReturnsCorrectValue) {
   
    RISReportingPeriodDefault = 300;  

    
    ULONG result = GetRISReportingPeriodDefault();

    
    EXPECT_EQ(result, RISReportingPeriodDefault);  
    EXPECT_EQ(result, 300);  
}

//SetRISPollingPeriodDefault
TEST_F(HarvesterTestFixture, SetRISPollingPeriodDefault_SetsNewValue) {
    
    RISPollingPeriodDefault = 600;  

    
    ULONG newInterval = 1200;

    
    int result = SetRISPollingPeriodDefault(newInterval);

    
    EXPECT_EQ(result, 0);

    
    EXPECT_EQ(RISPollingPeriodDefault, newInterval);
}

//GetRISPollingPeriodDefault
TEST_F(HarvesterTestFixture, GetRISPollingPeriodDefault_ReturnsCorrectValue) {
   
    RISPollingPeriodDefault = 600;  

    
    ULONG result = GetRISPollingPeriodDefault();

    
    EXPECT_EQ(result, 600);
}

//GetRISOverrideTTL
TEST_F(HarvesterTestFixture, GetRISOverrideTTL_ReturnsCorrectValue) {
   
    RISOverrideTTL = 500;  
    
    ULONG result = GetRISOverrideTTL();

    
    EXPECT_EQ(result, 500);
}

//GetRISOverrideTTLDefault
TEST_F(HarvesterTestFixture, GetRISOverrideTTLDefault_ReturnsCorrectValue) {
    
    RISOverrideTTLDefault = 500; 
    
    ULONG result = GetRISOverrideTTLDefault();

    
    EXPECT_EQ(result, 500);
}

//_rtsyscmd
TEST_F(HarvesterTestFixture, Test_rtsyscmd_SuccessfulExecution) {
    
    FILE* mockFile = tmpfile();  
    ASSERT_NE(mockFile, nullptr); 

    const char* testContent = "Test content from mock file.";
    fputs(testContent, mockFile);  
    rewind(mockFile);  

    char retBuf[256] = {0};  

    
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .WillRepeatedly(testing::Return());  

    
    _rtsyscmd(mockFile, retBuf, sizeof(retBuf));

    
    EXPECT_STREQ(retBuf, testContent); 

    fclose(mockFile);  
}

//_rtsyscmd
TEST_F(HarvesterTestFixture, Test_rtsyscmd_BufferOverflow) {
    
    FILE* mockFile = tmpfile();  
    ASSERT_NE(mockFile, nullptr);  

    const char* testContent = "This is a test content that exceeds the buffer size.";
    fputs(testContent, mockFile);  
    rewind(mockFile);  
    char retBuf[20] = {0};  
   
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .WillRepeatedly(testing::Return());  

   
    _rtsyscmd(mockFile, retBuf, sizeof(retBuf));

    
    EXPECT_LE(strlen(retBuf), sizeof(retBuf) - 1);  
    EXPECT_EQ(retBuf[sizeof(retBuf) - 1], '\0');    

    fclose(mockFile);  
}


TEST_F(HarvesterTestFixture, Test_rtsyscmd_EmptyFile) {
    
    FILE* mockFile = tmpfile();  
    ASSERT_NE(mockFile, nullptr);  

    char retBuf[256] = {0};  

   
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
        .WillRepeatedly(testing::Return());  

    
    _rtsyscmd(mockFile, retBuf, sizeof(retBuf));

    
    EXPECT_EQ(strlen(retBuf), 0); 
    fclose(mockFile);  
}

//print_rt_list
TEST_F(HarvesterTestFixture, PrintRtList_Success_radio) {
    

    
    ULONG mockRadioNum = 3; 
    EXPECT_CALL(*g_WifiHalMock, wifi_getRadioNumberOfEntries(::testing::_))
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(mockRadioNum), ::testing::Return(0)));

    
    for (int i = 0; i < mockRadioNum; ++i) {
        rIndex[i].radioBssid = strdup("00:11:22:33:44:55");  
        gettimeofday(&rIndex[i].timestamp, NULL);  
    }

    
    print_rt_list();

   
    for (int i = 0; i < mockRadioNum; ++i) {
        free(rIndex[i].radioBssid);
    }
}


//delete_rt_list
TEST_F(HarvesterTestFixture, DeleteRtList_Success) {
    ULONG mockRadioNum = MAX_NUM_RADIOS;  
    EXPECT_CALL(*g_WifiHalMock, wifi_getRadioNumberOfEntries(::testing::_))
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(mockRadioNum), ::testing::Return(0)));

    for (int i = 0; i < mockRadioNum; ++i) {
        rIndex[i].radioBssid = strdup("00:11:22:33:44:55");
        rIndex[i].radioOperatingFrequencyBand = strdup("2.4GHz");
        rIndex[i].radiOperatingChannelBandwidth = strdup("20MHz");
        rIndex[i].rtdata = static_cast<wifi_radioTrafficStats2_t*>(malloc(sizeof(wifi_radioTrafficStats2_t)));
        ASSERT_NE(rIndex[i].rtdata, nullptr);
        memset(rIndex[i].rtdata, 0, sizeof(wifi_radioTrafficStats2_t));
    }

    delete_rt_list();

    radiotrafficdata emptyData = {};
    for (int i = 0; i < mockRadioNum; ++i) {
        EXPECT_EQ(rIndex[i].radioBssid, nullptr);
        EXPECT_EQ(rIndex[i].radioOperatingFrequencyBand, nullptr);
        EXPECT_EQ(rIndex[i].radiOperatingChannelBandwidth, nullptr);
        EXPECT_EQ(rIndex[i].rtdata, nullptr);

        EXPECT_EQ(memcmp(&rIndex[i], &emptyData, sizeof(radiotrafficdata)), 0);
    }
}

