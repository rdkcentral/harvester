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
#include <string.h>
#include "harvester_mock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::DoAll;
using ::testing::SaveArg;

extern "C" {
#include "harvester_mlo.h"
#include <rbus/rbus.h>
#include "harvester_rbus_api.h"
}

extern rbusMock *g_rbusMock;
extern SafecLibMock *g_safecLibMock;
extern rbusMock *g_rbusMock;
extern SafecLibMock *g_safecLibMock;
extern cjsonMock *g_cjsonMock;

class HarvesterMLOTest : public HarvesterTestFixture {
protected:
    rbusHandle_t mockHandle = (rbusHandle_t)0x12345;

    void SetUp() override {
        HarvesterTestFixture::SetUp();
        EXPECT_CALL(*g_rbusMock, rbus_open(_, _)).WillOnce(DoAll(SetArgPointee<0>(mockHandle), Return(RBUS_ERROR_SUCCESS)));
        harvesterRbusInit("Harvester");
    }

    void TearDown() override {
        HarvesterTestFixture::TearDown();
    }
};

TEST_F(HarvesterMLOTest, MLO_RfcEnable_GetSet) {
    EXPECT_EQ(get_HarvesterMLORfcEnable(), false);

    // Mock setup calls for rbus_StoreValueIntoPsmDB
    EXPECT_CALL(*g_rbusMock, rbusObject_Init(_, _)).WillRepeatedly(Return((rbusObject_t)0x10));
    EXPECT_CALL(*g_rbusMock, rbusValue_Init(_)).WillRepeatedly(Return((rbusValue_t)0x20));
    EXPECT_CALL(*g_rbusMock, rbusValue_SetString(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusObject_SetValue(_, _, _)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusObject_Release(_)).WillRepeatedly(Return());

    // Expect SetPSMRecordValue for true
    EXPECT_CALL(*g_rbusMock, rbusMethod_Invoke(mockHandle, testing::StrEq("SetPSMRecordValue()"), _, _))
        .WillOnce(Return(RBUS_ERROR_SUCCESS));

    EXPECT_EQ(set_HarvesterMLORfcEnable(true), 0);
    EXPECT_EQ(get_HarvesterMLORfcEnable(), true);

    // Expect SetPSMRecordValue for false
    EXPECT_CALL(*g_rbusMock, rbusMethod_Invoke(mockHandle, testing::StrEq("SetPSMRecordValue()"), _, _))
        .WillOnce(Return(RBUS_ERROR_SUCCESS));

    EXPECT_EQ(set_HarvesterMLORfcEnable(false), 0);
    EXPECT_EQ(get_HarvesterMLORfcEnable(), false);
}

TEST_F(HarvesterMLOTest, MLO_RfcEnable_SetFail) {
    EXPECT_CALL(*g_rbusMock, rbusObject_Init(_, _)).WillRepeatedly(Return((rbusObject_t)0x10));
    EXPECT_CALL(*g_rbusMock, rbusValue_Init(_)).WillRepeatedly(Return((rbusValue_t)0x20));
    EXPECT_CALL(*g_rbusMock, rbusValue_SetString(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusObject_SetValue(_, _, _)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusObject_Release(_)).WillRepeatedly(Return());

    EXPECT_CALL(*g_rbusMock, rbusMethod_Invoke(mockHandle, testing::StrEq("SetPSMRecordValue()"), _, _))
        .WillOnce(Return(RBUS_ERROR_BUS_ERROR));

    EXPECT_EQ(set_HarvesterMLORfcEnable(true), 1);
    EXPECT_EQ(get_HarvesterMLORfcEnable(), true);
}

TEST_F(HarvesterMLOTest, AddToMloList_Success) {
    struct mlo_associated_device_data *head = NULL;
    mlo_assoc_dev_t *devData = (mlo_assoc_dev_t *)calloc(1, sizeof(mlo_assoc_dev_t));
    
    add_to_mlo_list(&head, (char*)"1", 1, devData);

    ASSERT_NE(head, nullptr);
    EXPECT_STREQ(head->vapIndex, "1");
    EXPECT_EQ(head->numAssocDevices, 1);
    EXPECT_EQ(head->devicedata, devData);
    EXPECT_EQ(head->next, nullptr);

    delete_mlo_list(head);
}

TEST_F(HarvesterMLOTest, AddToMloList_Append) {
    struct mlo_associated_device_data *head = NULL;
    mlo_assoc_dev_t *devData1 = (mlo_assoc_dev_t *)calloc(1, sizeof(mlo_assoc_dev_t));
    mlo_assoc_dev_t *devData2 = (mlo_assoc_dev_t *)calloc(1, sizeof(mlo_assoc_dev_t));

    add_to_mlo_list(&head, (char*)"1", 1, devData1);
    add_to_mlo_list(&head, (char*)"2", 2, devData2);

    ASSERT_NE(head, nullptr);
    ASSERT_NE(head->next, nullptr);
    EXPECT_STREQ(head->next->vapIndex, "2");
    EXPECT_EQ(head->next->numAssocDevices, 2);

    delete_mlo_list(head);
}

TEST_F(HarvesterMLOTest, DeleteMloList_Null) {
    delete_mlo_list(NULL);
}

TEST_F(HarvesterMLOTest, PrintMloList_Valid) {
    struct mlo_associated_device_data *head = NULL;
    mlo_assoc_dev_t *devData = (mlo_assoc_dev_t *)calloc(1, sizeof(mlo_assoc_dev_t));
    add_to_mlo_list(&head, (char*)"1", 1, devData);

    print_mlo_list(head);

    delete_mlo_list(head);
}

TEST_F(HarvesterMLOTest, MLO_Rfc_Handler_Test) {
    rbusDataElement_t* capturedElements = NULL;

    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(mockHandle, 1, _))
        .WillOnce(DoAll(SaveArg<2>(&capturedElements), Return(RBUS_ERROR_SUCCESS)));

    ASSERT_EQ(regHarvesterDataModel(), 0);
    ASSERT_NE(capturedElements, nullptr);

    rbusGetHandler_t getHandler = capturedElements[0].cbTable.getHandler;
    rbusSetHandler_t setHandler = capturedElements[0].cbTable.setHandler;

    ASSERT_NE(getHandler, nullptr);
    ASSERT_NE(setHandler, nullptr);

    // Common mock setup
    EXPECT_CALL(*g_rbusMock, rbusObject_Init(_, _)).WillRepeatedly(Return((rbusObject_t)0x10));
    EXPECT_CALL(*g_rbusMock, rbusValue_Init(_)).WillRepeatedly(Return((rbusValue_t)0x20));
    EXPECT_CALL(*g_rbusMock, rbusValue_SetString(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusObject_SetValue(_, _, _)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusObject_Release(_)).WillRepeatedly(Return());
    EXPECT_CALL(*g_rbusMock, rbusValue_SetBoolean(_, _)).WillRepeatedly(Return());

    // Test Get Handler (GetPSMRecordValue -> "true")
    rbusProperty_t prop;
    rbusProperty_Init(&prop, HARVESTER_MLO_RFC_PARAM, NULL);

    // Expect rbusProperty_GetName to allow the trace to be printed
    EXPECT_CALL(*g_rbusMock, rbusProperty_GetName(prop))
        .WillRepeatedly(Return(HARVESTER_MLO_RFC_PARAM));
    
    // Simulate GetPSMRecordValue success and data return
    // We must set the outParams (4th arg) to a valid handle for subsequent calls
    EXPECT_CALL(*g_rbusMock, rbusMethod_Invoke(mockHandle, testing::StrEq("GetPSMRecordValue()"), _, _))
        .WillOnce(DoAll(SetArgPointee<3>((rbusObject_t)0x99), Return(RBUS_ERROR_SUCCESS)));
        
    EXPECT_CALL(*g_rbusMock, rbusObject_GetProperties((rbusObject_t)0x99)).WillOnce(Return((rbusProperty_t)0x30));
    EXPECT_CALL(*g_rbusMock, rbusProperty_GetValue((rbusProperty_t)0x30)).WillOnce(Return((rbusValue_t)0x40));
    EXPECT_CALL(*g_rbusMock, rbusValue_ToString((rbusValue_t)0x40, NULL, 0)).WillOnce(Return((char*)"true"));
    EXPECT_CALL(*g_rbusMock, rbusProperty_GetNext((rbusProperty_t)0x30)).WillOnce(Return((rbusProperty_t)NULL));
    EXPECT_CALL(*g_rbusMock, rbusProperty_GetName((rbusProperty_t)0x30)).WillRepeatedly(Return("Value"));
    EXPECT_CALL(*g_rbusMock, rbusObject_Release((rbusObject_t)0x99)).WillOnce(Return());
    
    EXPECT_CALL(*g_rbusMock, rbusProperty_SetValue(prop, _)).WillRepeatedly(Return());

    EXPECT_EQ(getHandler(mockHandle, prop, NULL), RBUS_ERROR_SUCCESS);

    // Test Set Handler (SetPSMRecordValue -> success)
    rbusValue_t val;
    rbusValue_Init(&val);
    rbusValue_SetBoolean(val, false);
    rbusProperty_SetValue(prop, val);

    EXPECT_CALL(*g_rbusMock, rbusProperty_GetValue(prop)).WillOnce(Return(val));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(val)).WillOnce(Return(RBUS_BOOLEAN));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetBoolean(val)).WillOnce(Return(false));
    
    EXPECT_CALL(*g_rbusMock, rbusMethod_Invoke(mockHandle, testing::StrEq("SetPSMRecordValue()"), _, _))
        .WillOnce(Return(RBUS_ERROR_SUCCESS));

    EXPECT_EQ(setHandler(mockHandle, prop, NULL), RBUS_ERROR_SUCCESS);

    rbusProperty_Release(prop);
    rbusValue_Release(val);
}

TEST_F(HarvesterMLOTest, MLO_Rfc_SetHandler_ErrorPaths) {
    rbusDataElement_t* capturedElements = NULL;

    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(mockHandle, 1, _))
        .WillOnce(DoAll(SaveArg<2>(&capturedElements), Return(RBUS_ERROR_SUCCESS)));

    regHarvesterDataModel();
    rbusSetHandler_t setHandler = capturedElements[0].cbTable.setHandler;

    rbusProperty_t prop;
    rbusProperty_Init(&prop, HARVESTER_MLO_RFC_PARAM, NULL);
    rbusValue_t val;
    rbusValue_Init(&val);

    // Case 1: Wrong type
    EXPECT_CALL(*g_rbusMock, rbusProperty_GetValue(prop)).WillOnce(Return(val));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(val)).WillOnce(Return(RBUS_STRING));
    EXPECT_EQ(setHandler(mockHandle, prop, NULL), RBUS_ERROR_INVALID_INPUT);

    // Case 2: Wrong parameter name
    rbusProperty_t wrongProp;
    rbusProperty_Init(&wrongProp, "Device.Wrong.Param", NULL);
    EXPECT_EQ(setHandler(mockHandle, wrongProp, NULL), RBUS_ERROR_ELEMENT_DOES_NOT_EXIST);

    rbusProperty_Release(prop);
    rbusProperty_Release(wrongProp);
    rbusValue_Release(val);
}

TEST_F(HarvesterMLOTest, MLO_Rfc_Uninit_Success) {
    // Expect unregDataElements
    EXPECT_CALL(*g_rbusMock, rbus_unregDataElements(mockHandle, 1, _))
        .WillOnce(Return(RBUS_ERROR_SUCCESS));

    harvesterMLO_RfcUninit();
}

TEST_F(HarvesterMLOTest, MLO_Rfc_Uninit_NullHandle) {
    // Force rbus_handle to NULL by recalling init with a mock that sets it to NULL
    EXPECT_CALL(*g_rbusMock, rbus_open(_, _))
        .WillOnce(DoAll(SetArgPointee<0>((rbusHandle_t)NULL), Return(RBUS_ERROR_BUS_ERROR)));
    harvesterRbusInit("ForceNull");

    // Expect NO unregDataElements call
    EXPECT_CALL(*g_rbusMock, rbus_unregDataElements(_, _, _))
        .Times(0);

    harvesterMLO_RfcUninit();
}

TEST_F(HarvesterMLOTest, Parse_NullInputs) {
    uint32_t count = 0;
    char *vap = NULL;
    mlo_assoc_dev_t *dev = NULL;
    
    EXPECT_EQ(mlo_parseAssociatedDeviceDiagnostics(NULL, &dev, &count, &vap), 1);
}

TEST_F(HarvesterMLOTest, Parse_NoAssociatedClientsDiagnostics) {
    cJSON *json = cJSON_CreateObject();
    uint32_t count = 0;
    char *vap = NULL;
    mlo_assoc_dev_t *dev = NULL;

    EXPECT_CALL(*g_cjsonMock, cJSON_GetObjectItem(_, testing::StrEq("AssociatedClientsDiagnostics")))
        .WillOnce(Return((cJSON*)NULL));

    EXPECT_EQ(mlo_parseAssociatedDeviceDiagnostics(json, &dev, &count, &vap), 1);
    
    cJSON_Delete(json);
}
