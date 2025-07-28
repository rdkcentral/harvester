/*
 * If not stated otherwise in this file or this component's LICENSE file the
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
#include "cosa_harvester_internal.h"

extern PsmMock * g_psmMock;
extern SafecLibMock* g_safecLibMock;
extern COSA_DATAMODEL_HARVESTER* g_pHarvester;

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::StrEq;
using ::testing::AnyNumber;

extern "C"
{
    #include "cosa_harvester_dml.h"
    #include "harvester_associated_devices.h"
    #include "cosa_harvester_internal.h"
    #include "ccsp_base_api.h"
    #include "ansc_platform.h"
    #include "ansc_load_library.h"
    #include "cosa_plugin_api.h"
}


//Test cases for GetNVRamULONGConfiguration 
TEST_F(HarvesterTestFixture, SuccessfulRetrieval) {
    char* setting = (char*)"testsetting";
    ULONG value = 0;

    CCSP_MESSAGE_BUS_INFO mockBusInfo = {};
    mockBusInfo.freefunc = (CCSP_MESSAGE_BUS_FREE)[](void* str) { free(str); }; 
    bus_handle = &mockBusInfo; 

   
    char* mockValue = strdup("123");
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2(_, _, setting, _, _))
        .WillOnce(DoAll(SetArgPointee<4>(mockValue), Return(CCSP_SUCCESS)));

    ASSERT_EQ(GetNVRamULONGConfiguration(setting, &value), CCSP_SUCCESS);
    EXPECT_EQ(value, 123UL);

    
}

TEST_F(HarvesterTestFixture, RetrievalFailure) 
{ 
     char* setting = (char*)"testsetting";
     ULONG value = 0;

    CCSP_MESSAGE_BUS_INFO* busInfo = (CCSP_MESSAGE_BUS_INFO*)malloc(sizeof(CCSP_MESSAGE_BUS_INFO));
    ASSERT_NE(busInfo, nullptr);  
    memset(busInfo, 0, sizeof(CCSP_MESSAGE_BUS_INFO));

    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2(_, _, setting, _, _))
        .WillOnce(Return(CCSP_FAILURE));

    ASSERT_EQ(GetNVRamULONGConfiguration(setting, &value), CCSP_FAILURE);
    EXPECT_EQ(value, 0UL); 

    free(busInfo);
}


//Test cases for  SetNVRamULONGConfiguration
TEST_F(HarvesterTestFixture, ValueAlreadySet_ReturnsSuccess) {
    char* setting = (char*)"testsetting";
    ULONG value = 123;
    ULONG existingValue = 123;

    CCSP_MESSAGE_BUS_INFO mockBusInfo = {};
    mockBusInfo.freefunc = (CCSP_MESSAGE_BUS_FREE)[](void* str) { free(str); }; 
    bus_handle = &mockBusInfo; 

    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2(_, _, StrEq(setting), _, _))
        .WillOnce(DoAll(SetArgPointee<4>(strdup("123")), Return(CCSP_SUCCESS)));

    EXPECT_EQ(SetNVRamULONGConfiguration(setting, value), CCSP_SUCCESS);
}

TEST_F(HarvesterTestFixture, SetValueSuccess) {
    char* setting = (char*)"testsetting";
    ULONG value = 123;
    ULONG existingValue = 456;

    CCSP_MESSAGE_BUS_INFO mockBusInfo = {};
    mockBusInfo.freefunc = (CCSP_MESSAGE_BUS_FREE)[](void* str) { free(str); }; 
    bus_handle = &mockBusInfo;

    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2(_, _, StrEq(setting), _, _))
        .WillOnce(DoAll(SetArgPointee<4>(strdup("456")), Return(CCSP_SUCCESS))); 

     EXPECT_CALL(*g_safecLibMock, _sprintf_s_chk(_, _, _, _))
         .WillOnce(Return(EOK)) ;   

      EXPECT_CALL(*g_psmMock, PSM_Set_Record_Value2(_, _,_,_,_))
        .WillOnce(Return(CCSP_SUCCESS)); 

    EXPECT_EQ(SetNVRamULONGConfiguration(setting, value), CCSP_SUCCESS);
}

TEST_F(HarvesterTestFixture, GetNVRamULONGConfigurationFailure_ReturnsFailure) {
    char* setting = (char*)"testsetting";
    ULONG value = 123;

    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2(_, _, StrEq(setting), _, _))
        .WillOnce(Return(CCSP_FAILURE));   

    EXPECT_CALL(*g_safecLibMock, _sprintf_s_chk(_, _, _, _))
         .WillOnce(Return(EOK)) ;  

    EXPECT_CALL(*g_psmMock, PSM_Set_Record_Value2(_, _,_,_,_))
        .WillOnce(Return(CCSP_SUCCESS));        

    EXPECT_EQ(SetNVRamULONGConfiguration(setting, value), CCSP_SUCCESS);
}

TEST_F(HarvesterTestFixture, SetValueFailure) {
    char* setting = (char*)"testsetting";
    ULONG value = 123;
    ULONG existingValue = 456;

    CCSP_MESSAGE_BUS_INFO mockBusInfo = {};
    mockBusInfo.freefunc = (CCSP_MESSAGE_BUS_FREE)[](void* str) { free(str); }; 
    bus_handle = &mockBusInfo;

    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2(_, _, StrEq(setting), _, _))
        .WillOnce(DoAll(SetArgPointee<4>(strdup("456")), Return(CCSP_SUCCESS))); 

     EXPECT_CALL(*g_safecLibMock, _sprintf_s_chk(_, _, _, _))
         .WillOnce(Return(EOK)) ;      

    EXPECT_CALL(*g_psmMock, PSM_Set_Record_Value2(_, _,_,_,_))
        .WillOnce(Return(CCSP_FAILURE)); 

    EXPECT_EQ(SetNVRamULONGConfiguration(setting, value), CCSP_FAILURE);
}

TEST_F(HarvesterTestFixture, SprintfFailure_ReturnsFailure) {
    char* setting = (char*)"testsetting";
    ULONG value = ULONG_MAX;

     CCSP_MESSAGE_BUS_INFO mockBusInfo = {};
    mockBusInfo.freefunc = (CCSP_MESSAGE_BUS_FREE)[](void* str) { free(str); }; 
    bus_handle = &mockBusInfo;

    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2(_, _, StrEq(setting), _, _))
        .WillOnce(DoAll(SetArgPointee<4>(strdup("456")), Return(CCSP_SUCCESS))); 

   
    EXPECT_CALL(*g_safecLibMock, _sprintf_s_chk(_, _, _, _))
        .WillOnce(Return(-1));

    EXPECT_EQ(SetNVRamULONGConfiguration(setting, value), CCSP_FAILURE);
}

//Test cases for CosaDmlHarvesterInit
TEST_F(HarvesterTestFixture, ParamNameMatchesEnabled) {

    BOOL pBool = FALSE;
    char paramName[] = "Enabled";

    
    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Enabled"), strlen("Enabled"), StrEq("Enabled"), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK))); 
   
    BOOL result = InterfaceDevicesWifi_GetParamBoolValue(nullptr, paramName, &pBool);
    EXPECT_TRUE(result); 
    EXPECT_EQ(pBool, GetIDWHarvestingStatus()); 
}

TEST_F(HarvesterTestFixture, ParamNameDoesNotMatchEnabled) {
 
    BOOL pBool = FALSE;
    char paramName[] = "InvalidParam";

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Enabled"), strlen("Enabled"), StrEq("InvalidParam"), _, _, _))
        .WillOnce(Return(EOK)); 

    
    BOOL result = InterfaceDevicesWifi_GetParamBoolValue(nullptr, paramName, &pBool);

    EXPECT_FALSE(result); 
    EXPECT_EQ(pBool, FALSE); 
}

TEST_F(HarvesterTestFixture, ParamNameIsNull) {
 
    BOOL pBool = FALSE;
    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(_, _, _, _, _,_)).Times(0);

    BOOL result = InterfaceDevicesWifi_GetParamBoolValue(nullptr, nullptr, &pBool);
    EXPECT_FALSE(result);
}

TEST_F(HarvesterTestFixture, StrcmpFails) {

    BOOL pBool = FALSE;
    char paramName[] = "Enabled"; 

    
    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Enabled"), strlen("Enabled"), StrEq("Enabled"), _, _, _))
        .WillOnce(Return(ESNULLP)); 

    BOOL result = InterfaceDevicesWifi_GetParamBoolValue(nullptr, paramName, &pBool);

    EXPECT_FALSE(result); 
    EXPECT_EQ(pBool, FALSE);
}


//Test cases for InterfaceDevicesWifi_SetParamBoolValue
TEST_F(HarvesterTestFixture, ParamNameIs_Null) {
    char* paramName = nullptr;
    BOOL newValue = TRUE;

    PCOSA_DATAMODEL_HARVESTER harvester = (PCOSA_DATAMODEL_HARVESTER)malloc(sizeof(COSA_DATAMODEL_HARVESTER));
    ASSERT_NE(harvester, nullptr);
    memset(harvester, 0, sizeof(COSA_DATAMODEL_HARVESTER));
    BOOL result = InterfaceDevicesWifi_SetParamBoolValue(harvester, paramName, newValue);
    EXPECT_FALSE(result);

    free(harvester);
}


TEST_F(HarvesterTestFixture, ParamNameDoesNotMatch_Enabled) {

    char paramName[] = "InvalidParam";
    BOOL newValue = TRUE;

    PCOSA_DATAMODEL_HARVESTER harvester = (PCOSA_DATAMODEL_HARVESTER)malloc(sizeof(COSA_DATAMODEL_HARVESTER));
    ASSERT_NE(harvester, nullptr);
    memset(harvester, 0, sizeof(COSA_DATAMODEL_HARVESTER));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Enabled"), strlen("Enabled"), StrEq(paramName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    BOOL result = InterfaceDevicesWifi_SetParamBoolValue(harvester, paramName, newValue);

    EXPECT_FALSE(result);

    free(harvester);
}

TEST_F(HarvesterTestFixture, Strcmp_Fails) {

    char paramName[] = "Enabled";
    BOOL newValue = TRUE;


    PCOSA_DATAMODEL_HARVESTER harvester = (PCOSA_DATAMODEL_HARVESTER)malloc(sizeof(COSA_DATAMODEL_HARVESTER));
    ASSERT_NE(harvester, nullptr);
    memset(harvester, 0, sizeof(COSA_DATAMODEL_HARVESTER));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Enabled"), strlen("Enabled"), StrEq(paramName), _, _, _))
        .WillOnce(Return(ESNULLP));

    BOOL result = InterfaceDevicesWifi_SetParamBoolValue(harvester, paramName, newValue);
    EXPECT_FALSE(result);
    free(harvester);
}
