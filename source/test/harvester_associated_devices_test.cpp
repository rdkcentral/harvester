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
#include <stdbool.h>
#include "harvester_mock.h"

using ::testing::_;
using ::testing::Return;

extern WifiHalMock *g_WifiHalMock;
extern BOOL IDWHarvesterStatus;
extern ULONG IDWReportingPeriod;
extern ULONG IDWReportingPeriodDefault;
extern ULONG AssociatedDevicePeriods[];
extern ULONG IDWPollingPeriod;
extern ULONG IDWPollingPeriodDefault;
extern ULONG IDWOverrideTTL;
extern ULONG IDWOverrideTTLDefault;
extern "C" {

#include "harvester_associated_devices.h"
#include "harvester_avro.h"
void print_list( struct associateddevicedata *headnode);
void delete_list(  struct associateddevicedata *headnode);
int GetWiFiApGetAssocDevicesData(int ServiceType, int wlanIndex, char* pSsid);
}

TEST_F(HarvesterTestFixture, GetCurrentTimeString) {
    
    char *time_str = GetCurrentTimeString();
    EXPECT_NE(time_str, nullptr);
    EXPECT_EQ(strlen(time_str), 25);

}

TEST_F(HarvesterTestFixture, GetCurrentTimeInSecond) {

    ulong time1 = GetCurrentTimeInSecond();
    EXPECT_GT(time1, 0);  // The current time should be greater than 0
    sleep(1);

    ulong time2 = GetCurrentTimeInSecond();
    EXPECT_GT(time2, time1);  // The second call should return a time greater than the first one
}

TEST_F(HarvesterTestFixture, isvalueinarray) {

    ULONG arr[] = {1, 2, 3, 4, 5};
    int size = sizeof(arr) / sizeof(arr[0]);


    EXPECT_TRUE(isvalueinarray(3, arr, size));
    EXPECT_TRUE(isvalueinarray(1, arr, size));
    EXPECT_FALSE(isvalueinarray(6, arr, size));
}

TEST_F(HarvesterTestFixture, GetIDWHarvestingStatus) {

    IDWHarvesterStatus = TRUE;

    BOOL result = GetIDWHarvestingStatus();
    EXPECT_TRUE(result);
}

TEST_F(HarvesterTestFixture, SetIDWReportingPeriod) {
   
    ULONG newInterval = 100;

    int result = SetIDWReportingPeriod(newInterval);
    EXPECT_EQ(IDWReportingPeriod, newInterval);
    EXPECT_EQ(result, 0);
}

TEST_F(HarvesterTestFixture, GetIDWReportingPeriod) {
    
    ULONG testPeriod = 12345;
    IDWReportingPeriod = testPeriod;  

    ULONG result = GetIDWReportingPeriod();
    EXPECT_EQ(result, testPeriod);  

}

TEST_F(HarvesterTestFixture, ValidateIDWPeriod) {

    ULONG validInterval = 300;  
    ULONG invalidInterval = 100;

    BOOL result = ValidateIDWPeriod(validInterval);
    EXPECT_EQ(result, TRUE);

    result = ValidateIDWPeriod(invalidInterval);
    EXPECT_EQ(result, FALSE);

}

TEST_F(HarvesterTestFixture, GetIDWPollingPeriod) {

    ULONG expectedPollingPeriod = 60;
    IDWPollingPeriod = expectedPollingPeriod;
    
    ULONG result = GetIDWPollingPeriod();
    EXPECT_EQ(result, expectedPollingPeriod); 
}

TEST_F(HarvesterTestFixture, SetIDWReportingPeriodDefault) {

    ULONG expectedPeriod = 60;  

    int result = SetIDWReportingPeriodDefault(expectedPeriod);
    EXPECT_EQ(IDWReportingPeriodDefault, expectedPeriod); 
    EXPECT_EQ(result, 0); 
}

TEST_F(HarvesterTestFixture, GetIDWPollingPeriodDefault) {
    
    ULONG expectedPeriod = 60;
    IDWPollingPeriodDefault = expectedPeriod;
  
    ULONG result = GetIDWPollingPeriodDefault();
    EXPECT_EQ(result, expectedPeriod);

}

TEST_F(HarvesterTestFixture, GetIDWOverrideTTL) {

    ULONG expectedTTL = 120; 
    IDWOverrideTTL = expectedTTL;
    
    ULONG result = GetIDWOverrideTTL();
    EXPECT_EQ(result, expectedTTL);

}

TEST_F(HarvesterTestFixture, SetIDWOverrideTTL) {

    ULONG initialTTL = 100; 
    ULONG newTTL = 200;
    IDWOverrideTTL = initialTTL;

    int result = SetIDWOverrideTTL(newTTL);
    EXPECT_EQ(IDWOverrideTTL, newTTL);
}

TEST_F(HarvesterTestFixture, GetIDWOverrideTTLDefault) {

    ULONG expectedTTLDefault = 1000;
    IDWOverrideTTLDefault = expectedTTLDefault;
    
    ULONG result = GetIDWOverrideTTLDefault();
    EXPECT_EQ(result, expectedTTLDefault);
}

TEST_F(HarvesterTestFixture, PrintList) {

    struct associateddevicedata* headnode = nullptr;
    print_list(headnode);
}

TEST_F(HarvesterTestFixture, DeleteList) {

    struct associateddevicedata* headnode = nullptr;
    delete_list(headnode);
}

TEST_F(HarvesterTestFixture, GetWiFiApGetAssocDevicesData) {

    EXPECT_CALL(*g_WifiHalMock, wifi_getApEnable(_,_)).Times(1)
        .WillOnce(Return(0));
   GetWiFiApGetAssocDevicesData(0, 1, "test_ssid");    
}

