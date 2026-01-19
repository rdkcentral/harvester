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
#include <rbus/rbus.h>
#include "harvester_rbus_api.h"
}

extern rbusHandle_t rbus_handle;
extern rbusMock *g_rbusMock;
extern AnscDebugMock* g_anscDebugMock;

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::StrEq;
using ::testing::AnyNumber;


//Test case for get_rbus_handle
TEST_F(HarvesterTestFixture, TestGetRbusHandle) {

    rbusHandle_t expected_handle = reinterpret_cast<rbusHandle_t>(0x1234);
    rbus_handle = expected_handle; 

    rbusHandle_t actual_handle = get_rbus_handle();

    EXPECT_EQ(actual_handle, expected_handle);
}


//Test cases for rbusInitializedCheck
TEST_F(HarvesterTestFixture, TestRbusInitializedCheck_Initialized) {
   
    rbus_handle = reinterpret_cast<rbusHandle_t>(0x1234); 

    bool is_initialized = rbusInitializedCheck();

    EXPECT_TRUE(is_initialized); 
}

TEST_F(HarvesterTestFixture, TestRbusInitializedCheck_Uninitialized) {
   
    rbus_handle = NULL;


    bool is_initialized = rbusInitializedCheck();

    
    EXPECT_FALSE(is_initialized); 
}


//Test cases for harvesterRbusInit
TEST_F(HarvesterTestFixture, HarvesterRbusInit_Success) {
 
    const char *componentName = "HarvesterComponent";
    EXPECT_CALL(*g_rbusMock, rbus_open(_, componentName))
        .WillOnce(Return(RBUS_ERROR_SUCCESS)); 

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());
 

    int result = harvesterRbusInit(componentName);

   
    EXPECT_EQ(result, 0); 
}


TEST_F(HarvesterTestFixture, HarvesterRbusInit_Failure) {
    
    const char *componentName = "HarvesterComponent";
    EXPECT_CALL(*g_rbusMock, rbus_open(_, componentName))
        .WillOnce(Return(RBUS_ERROR_BUS_ERROR)); 

   EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());
  

    int result = harvesterRbusInit(componentName);

 
    EXPECT_EQ(result, 1); 
}

// Test case for harvesterRbus_Uninit
TEST_F(HarvesterTestFixture, HarvesterRbusUninit) {
   
    rbus_handle = reinterpret_cast<rbusHandle_t>(0x1234); 

    EXPECT_CALL(*g_rbusMock, rbus_close(rbus_handle))
    .Times(1)
    .WillOnce(Return(RBUS_ERROR_SUCCESS));
    
    harvesterRbus_Uninit();
}

//Test cases for rbus_getBoolValue
TEST_F(HarvesterTestFixture, TestRbusGetBoolValue_NotInitialized) {
 
    BOOL value;
    char path[] = "test.path";
    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _)).Times(0); 
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());


    rbus_handle = NULL;


    int result = rbus_getBoolValue(&value, path);


    EXPECT_EQ(result, 1); 
}

TEST_F(HarvesterTestFixture, TestRbusGetBoolValue_RbusGetFails) {
    // Arrange
    BOOL value;
    char path[] = "test.path";
    rbus_handle = reinterpret_cast<rbusHandle_t>(0x1234); 

    rbusValue_t boolVal = reinterpret_cast<rbusValue_t>(0x5678); 
    EXPECT_CALL(*g_rbusMock, rbus_get(rbus_handle, StrEq(path), _))
        .WillOnce(DoAll(SetArgPointee<2>(boolVal), Return(RBUS_ERROR_BUS_ERROR))); 
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(boolVal))
        .Times(1); 
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());


    int result = rbus_getBoolValue(&value, path);


    EXPECT_EQ(result, 1); 
}

TEST_F(HarvesterTestFixture, TestRbusGetBoolValue_Success) {

    BOOL value;
    char path[] = "test.path";
    rbus_handle = reinterpret_cast<rbusHandle_t>(0x1234); 

    rbusValue_t boolVal = reinterpret_cast<rbusValue_t>(0x5678); 
    EXPECT_CALL(*g_rbusMock, rbus_get(rbus_handle, StrEq(path), _))
        .WillOnce(DoAll(SetArgPointee<2>(boolVal), Return(RBUS_ERROR_SUCCESS))); 
    EXPECT_CALL(*g_rbusMock, rbusValue_GetBoolean(boolVal))
        .WillOnce(Return(true)); 
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(boolVal))
        .Times(1); 
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());
    

 
    int result = rbus_getBoolValue(&value, path);

   
    EXPECT_EQ(result, 0); 
    EXPECT_TRUE(value);   
}

//Test cases for rbus_getStringValue
TEST_F(HarvesterTestFixture, RbusGetStringValue_NotInitialized) {
    
    char value[128] = {0};
    char path[] = "test.path";
    rbus_handle = NULL; 
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());


    int result = rbus_getStringValue(value, path);

    EXPECT_EQ(result, 1);
}

TEST_F(HarvesterTestFixture, RbusGetStringValue_RbusGetFails) {

    char value[128] = {0};
    char path[] = "test.path";
    rbus_handle = reinterpret_cast<rbusHandle_t>(0x1234);

    rbusValue_t strVal = nullptr;

    EXPECT_CALL(*g_rbusMock, rbus_get(rbus_handle, path, _))
        .WillOnce(DoAll(SetArgPointee<2>(strVal), Return(RBUS_ERROR_INVALID_INPUT)));

    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
        .Times(0); 

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());    
    int result = rbus_getStringValue(value, path);
    EXPECT_EQ(result, 1); 
}

TEST_F(HarvesterTestFixture, RbusGetStringValue_Success) {
    
    char value[128] = {0};
    char path[] = "test.path";
    rbus_handle = reinterpret_cast<rbusHandle_t>(0x1234);

    const char* expectedString = "TestValue";
    rbusValue_t strVal = reinterpret_cast<rbusValue_t>(0x5678);

    EXPECT_CALL(*g_rbusMock, rbus_get(rbus_handle, path, _))
        .WillOnce(DoAll(SetArgPointee<2>(strVal), Return(RBUS_ERROR_SUCCESS)));

    EXPECT_CALL(*g_rbusMock, rbusValue_GetString(strVal, nullptr))
        .WillOnce(Return(expectedString));

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());    

    EXPECT_CALL(*g_rbusMock, rbusValue_Release(strVal))
        .Times(1);

    int result = rbus_getStringValue(value, path);
    EXPECT_EQ(result, 0); 
    EXPECT_STREQ(value, expectedString); 
}



// Test cases for rbus_getUInt32Value
TEST_F(HarvesterTestFixture, RbusGetUInt32Value_NotInitialized) {

    ULONG value = 0;
    char path[] = "test.path";
    rbus_handle = NULL; 

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());   

    
    int result = rbus_getUInt32Value(&value, path);

    EXPECT_EQ(result, 1); 
}

TEST_F(HarvesterTestFixture, RbusGetUInt32Value_RbusGetFails) {

    ULONG value = 0;
    char path[] = "test.path";
    rbus_handle = reinterpret_cast<rbusHandle_t>(0x1234); 

    rbusValue_t uintVal = nullptr;

    EXPECT_CALL(*g_rbusMock, rbus_get(rbus_handle, path, _))
        .WillOnce(DoAll(SetArgPointee<2>(uintVal), Return(RBUS_ERROR_INVALID_INPUT)));

    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
        .Times(0); 

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());       

    int result = rbus_getUInt32Value(&value, path);

    EXPECT_EQ(result, 1); 
}

TEST_F(HarvesterTestFixture, RbusGetUInt32Value_Success) {
    ULONG value = 0;
    char path[] = "test.path";
    rbus_handle = reinterpret_cast<rbusHandle_t>(0x1234); 

    ULONG expectedValue = 1234;
    rbusValue_t uintVal = reinterpret_cast<rbusValue_t>(0x5678);

    EXPECT_CALL(*g_rbusMock, rbus_get(rbus_handle, path, _))
        .WillOnce(DoAll(SetArgPointee<2>(uintVal), Return(RBUS_ERROR_SUCCESS)));

    EXPECT_CALL(*g_rbusMock, rbusValue_GetUInt32(uintVal))
        .WillOnce(Return(expectedValue));

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _))
    .Times(AnyNumber());       

    EXPECT_CALL(*g_rbusMock, rbusValue_Release(uintVal))
        .Times(1);


    int result = rbus_getUInt32Value(&value, path);

    
    EXPECT_EQ(result, 0); 
    EXPECT_EQ(value, expectedValue); 
}