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

using ::testing::Return;
using ::testing::DoAll;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Invoke;
using ::testing::SetArgPointee;

extern AnscWrapperApiMock * g_anscWrapperApiMock;
extern BaseAPIMock * g_baseapiMock;
extern SafecLibMock* g_safecLibMock;
extern PtdHandlerMock * g_PtdHandlerMock;
extern parodusMock *g_parodusMock;
extern AnscDebugMock* g_anscDebugMock;
extern SyscfgMock * g_syscfgMock;
extern utopiaMock *g_utopiaMock;
extern SyseventMock *g_syseventMock;

extern "C" {
#include "webpa_interface.h"
void checkComponentHealthStatus(char * compName, char * dbusPath, char *status, int *retStatus, int status_size);
void waitForEthAgentComponentReady();
int check_ethernet_wan_status();
void *handle_parodus();
}
// Test for successful component discovery
TEST_F(HarvesterTestFixture, Cosa_FindDestComp_SuccessCase) {
    char* objName = "testObject";
    char* componentName = nullptr;
    char* dbusPath = nullptr;

    componentStruct_t mockComponent;
    mockComponent.componentName = "testComponent";
    mockComponent.dbusPath = "/test/path";
    componentStruct_t* mockComponents[] = { &mockComponent };
    int size = 1;

    EXPECT_CALL(*g_safecLibMock, _sprintf_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));

    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_discComponentSupportingNamespace(_, _, _, _, _, _))
        .WillOnce(Invoke([&](void* bus_handle, const char* path, const char* objName, const char* prefix,
                             componentStruct_t*** ppComponents, int* size) {
            *ppComponents = mockComponents;
            *size = 1;
            return CCSP_SUCCESS;
        }));

    EXPECT_CALL(*g_anscWrapperApiMock, AnscCloneString("testComponent"))
        .WillOnce(Return(strdup("testComponent")));

    EXPECT_CALL(*g_anscWrapperApiMock, AnscCloneString("/test/path"))
        .WillOnce(Return(strdup("/test/path")));

    EXPECT_CALL(*g_baseapiMock, free_componentStruct_t(_, _, _));

    BOOL result = Cosa_FindDestComp(objName, &componentName, &dbusPath);

    EXPECT_TRUE(result);
    EXPECT_STREQ(componentName, "testComponent");
    EXPECT_STREQ(dbusPath, "/test/path");

    free(componentName);
    free(dbusPath);
}

// Test for successful message sending
TEST_F(HarvesterTestFixture, sendWebpaMsgSuccess) {
    // Setup the inputs
    char serviceName[] = "testService";
    char dest[] = "testDest";
    char trans_id[] = "12345";
    char contentType[] = "application/json";
    char payload[] = "{\"key\":\"value\"}";
    unsigned int payload_len = strlen(payload);
    
    EXPECT_CALL(*g_safecLibMock, _sprintf_s_chk(_, _, _, _)).Times(1)
       .WillOnce(Return(EOK));
        
    EXPECT_CALL(*g_safecLibMock, _memset_s_chk(_, _, _, _, _)).Times(1) .WillOnce(Return(0)); 

    EXPECT_CALL(*g_parodusMock, libparodus_send(_, _))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _)).Times(1);

    sendWebpaMsg(serviceName, dest, trans_id, contentType, payload, payload_len);

}

TEST_F(HarvesterTestFixture, HandleParodus_Success) {

    EXPECT_CALL(*g_parodusMock, libparodus_init(_, _))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _)).Times(1);

    handle_parodus();

}

TEST_F(HarvesterTestFixture, ReturnsCorrectLoggerModule) {
    // Call the function under test
    const char* result = rdk_logger_module_fetch();

    // Assert that the returned string matches the expected string
    EXPECT_STREQ(result, "LOG.RDK.Harvester");
}

TEST_F(HarvesterTestFixture, CheckComponentHealthStatus_Success) {
    char compName[] = "TestComponent";
    char dbusPath[] = "/test/path";
    char status[100];
    int retStatus = 0;
    
    parameterValStruct_t parameterval[1];
    parameterval[0].parameterName = "HealthStatus";
    parameterval[0].parameterValue = "Healthy";
    
    int val_size = 1;
     parameterValStruct_t* parametervalPtr = &parameterval[0];

    // Mock behavior for sprintf_s and getParameterValues using Invoke
    EXPECT_CALL(*g_safecLibMock, _sprintf_s_chk(_, _, _, _)).Times(2)
        .WillRepeatedly(Return(EOK));


    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_getParameterValues(_, _, _, _, _, _, _))
        .WillOnce(Invoke([&val_size, &parametervalPtr](void* bus_handle, const char* parameterNames, const char* dbusPath,
                                                      char** parameterNamesArray, int paramCount, int* valSize, parameterValStruct_t*** parameterval) {
            *valSize = val_size;
            *parameterval = &parametervalPtr;
            return CCSP_SUCCESS; // Simulate successful fetch of parameter values
        }));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_baseapiMock, free_parameterValStruct_t(_, _, _));  // Free should be called
    
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _)).Times(4);

    // Call the function
    checkComponentHealthStatus(compName, dbusPath, status, &retStatus, sizeof(status));

    // Assertions
    EXPECT_EQ(retStatus, CCSP_SUCCESS);
    //EXPECT_STREQ(status, "Healthy");
}

TEST_F(HarvesterTestFixture, waitForEthAgentComponentReady) {
    char status[32];
    int retStatus = 0;
    
    parameterValStruct_t parameterval[1];
    parameterval[0].parameterName = "HealthStatus";
    parameterval[0].parameterValue = "Healthy";
    
    int val_size = 1;
     parameterValStruct_t* parametervalPtr = &parameterval[0];

    // Mock the checkComponentHealthStatus function to simulate "Green" health status after 1 check
    EXPECT_CALL(*g_safecLibMock, _sprintf_s_chk(_, _, _, _)).Times(2)
        .WillRepeatedly(Return(EOK));


    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_getParameterValues(_, _, _, _, _, _, _))
        .WillOnce(Invoke([&val_size, &parametervalPtr](void* bus_handle, const char* parameterNames, const char* dbusPath,
                                                      char** parameterNamesArray, int paramCount, int* valSize, parameterValStruct_t*** parameterval) {
            *valSize = val_size;
            *parameterval = &parametervalPtr;
            return CCSP_SUCCESS; // Simulate successful fetch of parameter values
        }));

EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_baseapiMock, free_parameterValStruct_t(_, _, _));  // Free should be called
        
        EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(_, _, _, _, _, _)).Times(1)
        .WillOnce(DoAll(testing::SetArgPointee<3>(0), testing::Return(0)));
        
        EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _)).Times(5);

    // Call the function being tested
    waitForEthAgentComponentReady();

    // Assertions
    // You can add additional assertions here if you want to check internal behaviors.
    // For instance, you could check if logging or health checking was done as expected.
}

TEST_F(HarvesterTestFixture, check_ethernet_wan_status_SyscfgGetSuccess) {

    EXPECT_CALL(*g_syscfgMock, syscfg_get(_, _, _, _))
     .Times(1)
    .WillOnce(Invoke([](const char*, const char*, char* out_value, int) {
          strcpy(out_value, "true");
          return 0;   
    }));
        
    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(_, _, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke([](const char* dest, rsize_t dmax, const char* src, 
                            int* resultp, const size_t destbos, const size_t srcbos) {
            *resultp = strcmp(dest, src);
            return (*resultp == 0) ? EOK : -1;
        }));
        
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _)).Times(1);
        
    int status = check_ethernet_wan_status();
    EXPECT_EQ(status,CCSP_SUCCESS);

}
TEST_F(HarvesterTestFixture, check_ethernet_wan_status_SyscfgGetFailure) {

    componentStruct_t mockComponent;
    mockComponent.componentName = "testComponent";
    mockComponent.dbusPath = "/test/path";
    componentStruct_t* mockComponents[] = { &mockComponent };
    
    int size = 1;
    parameterValStruct_t parameterval[1];
    parameterval[0].parameterName = "HealthStatus";
    parameterval[0].parameterValue = "Healthy";
    
    int val_size = 1;
     parameterValStruct_t* parametervalPtr = &parameterval[0];

       
   EXPECT_CALL(*g_syscfgMock, syscfg_get(_, _, _, _))
        .Times(1)
        .WillOnce(Return(-1)); 
     
    EXPECT_CALL(*g_safecLibMock, _sprintf_s_chk(_, _, _, _)).Times(3)
        .WillRepeatedly(Return(EOK));

    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_getParameterValues(_, _, _, _, _, _, _))
        .WillOnce(Invoke([&val_size, &parametervalPtr](void* bus_handle, const char* parameterNames, const char* dbusPath,
                                                      char** parameterNamesArray, int paramCount, int* valSize, parameterValStruct_t*** parameterval) {
            *valSize = val_size;
            *parameterval = &parametervalPtr;
            return CCSP_SUCCESS;
        }));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _)).Times(3).WillRepeatedly(Return(0));
   
    EXPECT_CALL(*g_baseapiMock, free_parameterValStruct_t(_, _, _));
        
    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(_, _, _, _, _, _)).Times(1)
        .WillOnce(DoAll(testing::SetArgPointee<3>(0), testing::Return(0)));
        
    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _)).Times(5);
        
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_discComponentSupportingNamespace(_, _, _, _, _, _))
    .WillOnce(Invoke([&](void* bus_handle, const char* path, const char* objName, const char* prefix,
                         componentStruct_t*** ppComponents, int* size) {
        *ppComponents = mockComponents;
        *size = 1;
        return CCSP_SUCCESS;
    }));

        
    EXPECT_CALL(*g_baseapiMock, free_componentStruct_t(_, _, _)).Times(1);
      
    int status = check_ethernet_wan_status();
    EXPECT_EQ(status,CCSP_SUCCESS);
}


TEST_F(HarvesterTestFixture, GetMacFromSysevent) {

    char deviceMACValue[32] = "00:11:22:33:44:55"; 
    char expectedMac[] = "00:11:22:33:44:55"; 
  
    EXPECT_CALL(*g_utopiaMock, s_sysevent_connect(testing::_)).WillOnce(testing::Return(0));
        
    EXPECT_CALL(*g_syscfgMock, syscfg_get(_, _, _, _))
     .Times(1)
    .WillOnce(Invoke([&](const char*, const char*, char* out_value, int) {
          strcpy(out_value, "true");
          return 0;   
    }));
        
    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(_, _, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke([](const char* dest, rsize_t dmax, const char* src, 
                            int* resultp, const size_t destbos, const size_t srcbos) {
            *resultp = strcmp(dest, src);
            return (*resultp == 0) ? EOK : -1;
        }));

    EXPECT_CALL(*g_anscDebugMock, Ccsplog3(_, _)).Times(1);
        
    EXPECT_CALL(*g_syseventMock, sysevent_get(_, _,_, _, _)).Times(1)
        .WillOnce(Invoke([&](const int fd, const token_t token, const char *inbuf, char *outbuf, int outbytes) {
            strncpy(outbuf, "00:11:22:33:44:55", outbytes - 1);
            outbuf[outbytes - 1] = '\0'; 
            return 0; 
        }));
        
    EXPECT_CALL(*g_anscWrapperApiMock, AnscMacToLower(testing::_, _, testing::_))
        .WillOnce(Invoke([&](char *dest, const char *src, size_t len) {
            strncpy(dest, src, len - 1);
            dest[len - 1] = '\0';
        }));

    char *mac = getDeviceMac();

    EXPECT_STREQ(mac, expectedMac);
}

