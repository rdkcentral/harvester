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
#include "harvester.h" 
enum channel_bandwidth_e {
    MHZ20,
    MHZ40,
    MHZ80,
    MHZ80_80,
    MHZ160,
};
extern "C" 
{
#include "harvester_avro.h"
#include "harvester_rbus_api.h"
char* GetRISSchemaBuffer();
int GetRISSchemaBufferSize();
char* GetRISSchemaIDBuffer();
int GetRISSchemaIDBufferSize();
int get_radiOperatingChannelBandwidth_from_name(char *name, enum channel_bandwidth_e *type_ptr);
void ap_avro_cleanup();
}
extern char* rt_schema_buffer;
extern char* ris_schemaidbuffer;
extern SafecLibMock * g_safecLibMock;
TEST_F(HarvesterTestFixture, TestGetRISSchemaBuffer) {
    
    char* expected_buffer = "testdata"; 
    rt_schema_buffer = expected_buffer; 

 
    char* actual_buffer = GetRISSchemaBuffer();

    
    EXPECT_EQ(actual_buffer, expected_buffer); 
}


TEST_F(HarvesterTestFixture, TestGetRISSchemaBufferSize_ValidBuffer) {
    
    char* expected_buffer = "TestBuffer";
    rt_schema_buffer = expected_buffer; 

   
    int buffer_size = GetRISSchemaBufferSize();

   
    EXPECT_EQ(buffer_size, strlen(expected_buffer));
}

TEST_F(HarvesterTestFixture, TestGetRISSchemaBufferSize_NullBuffer) {
    
    rt_schema_buffer = NULL;

   
    int buffer_size = GetRISSchemaBufferSize();

    
    EXPECT_EQ(buffer_size, 0); 
}


TEST_F(HarvesterTestFixture, TestGetRISSchemaIDBuffer) {
    
    const char* expected_schemaid = "4d1f3d40-ab59-4672-89e6-c8cfdca739a0/44b72f483a79e851ad61073dd5373535";
    ris_schemaidbuffer = const_cast<char*>(expected_schemaid); 

    
    char* actual_schemaid = GetRISSchemaIDBuffer();

   
    EXPECT_STREQ(actual_schemaid, expected_schemaid); 
}

TEST_F(HarvesterTestFixture, TestGetRISSchemaIDBufferSize_ValidBuffer) {
    
    const char* expected_schemaid = "4d1f3d40-ab59-4672-89e6-c8cfdca739a0/44b72f483a79e851ad61073dd5373535";
    ris_schemaidbuffer = const_cast<char*>(expected_schemaid); 

    
    int buffer_size = GetRISSchemaIDBufferSize();

    
    EXPECT_EQ(buffer_size, strlen(expected_schemaid)); 
}

TEST_F(HarvesterTestFixture, TestGetRISSchemaIDBufferSize_NullBuffer) {
    
    ris_schemaidbuffer = NULL; 

    
    int buffer_size = GetRISSchemaIDBufferSize();

    
    EXPECT_EQ(buffer_size, 0); 
}
TEST_F(HarvesterTestFixture, TestGetOperatingChannelBandwidth_Success) {
   
    const char* name = "20MHz";
    enum channel_bandwidth_e expected_bandwidth = MHZ20;
    enum channel_bandwidth_e actual_bandwidth;
    
    
    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(name, strlen(name), "20MHz", ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const char*, rsize_t, const char*, int* resultp, const size_t, const size_t) {
            *resultp = 0;  
            return 0;      
        });
    
  
    
    int result = get_radiOperatingChannelBandwidth_from_name(const_cast<char*>(name), &actual_bandwidth);
    
    
    EXPECT_EQ(result, 1);  
    EXPECT_EQ(actual_bandwidth, expected_bandwidth);  
}

TEST_F(HarvesterTestFixture, TestGetOperatingChannelBandwidth_Failure1) {
 
    const char* name = NULL;
    enum channel_bandwidth_e expected_bandwidth = static_cast<enum channel_bandwidth_e>(-1);
    enum channel_bandwidth_e actual_bandwidth;
    
    
   
    int result = get_radiOperatingChannelBandwidth_from_name(const_cast<char*>(name), &actual_bandwidth);
    
    
    EXPECT_EQ(result, 0);  
}

TEST_F(HarvesterTestFixture, TestGetOperatingChannelBandwidth_Failure_StrcmpFails) {
    
    const char* name = "10MHz";
    enum channel_bandwidth_e expected_bandwidth = static_cast<enum channel_bandwidth_e>(-1);  
    enum channel_bandwidth_e actual_bandwidth = expected_bandwidth;  

   
    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillRepeatedly([](const char*, rsize_t, const char*, int* resultp, const size_t, const size_t) {
            *resultp = -1;  
            return -1;      
        });
    
  
    int result = get_radiOperatingChannelBandwidth_from_name(const_cast<char*>(name), &actual_bandwidth);
    
    
    EXPECT_EQ(result, 0);  
    EXPECT_EQ(actual_bandwidth, expected_bandwidth); 
}

TEST_F(HarvesterTestFixture, ApAvroCleanup_BufferInitialized) {
    
    rt_schema_buffer = (char*)malloc(100); 
    strcpy(rt_schema_buffer, "{\"type\":\"record\",\"name\":\"Person\"}");
    
    ap_avro_cleanup();
}

