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

extern char *buffer;
extern char *idw_schemaidbuffer;
extern void* iface;
extern bool schema_file_parsed;

extern SyscfgMock * g_syscfgMock;
extern SafecLibMock* g_safecLibMock;
extern utopiaMock *g_utopiaMock;
extern SyseventMock *g_syseventMock;

extern "C" {

#include "harvester_associated_devices.h"
#include "harvester_avro.h"
}

TEST_F(HarvesterTestFixture, GetIDWSchemaBuffer) {

    buffer = (char*)malloc(64 * sizeof(char));
    strcpy(buffer, "Test Buffer Content");

    char* returnedBuffer = GetIDWSchemaBuffer();
    
    ASSERT_NE(returnedBuffer, nullptr);
    EXPECT_STREQ(returnedBuffer, "Test Buffer Content");

    free(buffer);
    buffer = nullptr;  
}

TEST_F(HarvesterTestFixture, GetIDWSchemaBufferSize) {

    buffer = (char*)malloc(64 * sizeof(char));
    ASSERT_NE(buffer, nullptr);
    strcpy(buffer, "Test Buffer Content");

    int size = GetIDWSchemaBufferSize();
    EXPECT_EQ(size, 19);

    free(buffer);
    buffer = nullptr;  
}

TEST_F(HarvesterTestFixture, GetIDWSchemaIDBuffer) {

    idw_schemaidbuffer = (char*)malloc(64 * sizeof(char));
    strcpy(idw_schemaidbuffer, "Test Buffer Content");

    char* returnedBuffer = GetIDWSchemaIDBuffer();
    
    ASSERT_NE(returnedBuffer, nullptr);
    EXPECT_STREQ(returnedBuffer, "Test Buffer Content");

    free(idw_schemaidbuffer);
    idw_schemaidbuffer = nullptr;  
}

TEST_F(HarvesterTestFixture, GetIDWSchemaIDBufferSize) {

    idw_schemaidbuffer = (char*)malloc(64 * sizeof(char));
    ASSERT_NE(idw_schemaidbuffer, nullptr);
    strcpy(idw_schemaidbuffer, "Test Buffer Content");

    int size = GetIDWSchemaIDBufferSize();
    EXPECT_EQ(size, 19);

    free(idw_schemaidbuffer);
    idw_schemaidbuffer = nullptr;  
}

TEST_F(HarvesterTestFixture, NumberofElementsinLinkedList) {

    struct associateddevicedata* node = new struct associateddevicedata;
    node->timestamp.tv_sec = 1625248400;
    node->timestamp.tv_usec = 0;
    node->sSidName = new char[8];
    strcpy(node->sSidName, "SSID");
    node->bssid = new char[20];
    strcpy(node->bssid, "00:1A:2B:3C:4D:5E");
    node->radioOperatingFrequencyBand = new char[7];
    strcpy(node->radioOperatingFrequencyBand, "2.4G");
    node->radioChannel = 6;
    node->numAssocDevices = 1;
    node->next = nullptr;

    EXPECT_EQ(NumberofElementsinLinkedList(node), 1); 
    delete[] node->sSidName;
    delete[] node->bssid;
    delete[] node->radioOperatingFrequencyBand;
    delete node;
}

TEST_F(HarvesterTestFixture, NumberofDevicesinLinkedList) {

    struct associateddevicedata* node = new struct associateddevicedata;
    node->timestamp.tv_sec = 1625248400;
    node->timestamp.tv_usec = 0;
    node->sSidName = new char[8];
    strcpy(node->sSidName, "SSID");
    node->bssid = new char[20];
    strcpy(node->bssid, "00:1A:2B:3C:4D:5E");
    node->radioOperatingFrequencyBand = new char[7];
    strcpy(node->radioOperatingFrequencyBand, "2.4G");
    node->radioChannel = 6;
    node->numAssocDevices = 1;
    node->next = nullptr;

    EXPECT_EQ(NumberofDevicesinLinkedList(node), 1); 
    delete[] node->sSidName;
    delete[] node->bssid;
    delete[] node->radioOperatingFrequencyBand;
    delete node;
}

TEST_F(HarvesterTestFixture, harvester_avro_cleanup) {

    buffer = (char*)malloc(16);
    ASSERT_NE(buffer, nullptr);
    
    iface = (char*)malloc(16);
    ASSERT_NE(iface, nullptr);
    schema_file_parsed = true;

    harvester_avro_cleanup();

    EXPECT_EQ(buffer, nullptr);
    EXPECT_EQ(iface, nullptr);
    EXPECT_EQ(schema_file_parsed, false);
}

