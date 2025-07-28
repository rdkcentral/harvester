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


extern "C"
{
    #include "cosa_harvester_internal.h"
    #include "cosa_harvester_dml.h"
}

extern PsmMock * g_psmMock;
extern AnscMemoryMock * g_anscMemoryMock;

//Test case for CosaHarvesterCreate
TEST_F(HarvesterTestFixture, ReturnsNonNullOnSuccess) {

    ANSC_HANDLE pHarvesterObject = CosaHarvesterCreate();
    ASSERT_NE(pHarvesterObject, nullptr) << "Expected non-null handle on successful allocation";

    free(pHarvesterObject);
}

//Test case for CosaHarvesterInitialize
TEST_F(HarvesterTestFixture, InitializesSuccessfully) {

    PCOSA_DATAMODEL_HARVESTER pHarvesterObject = (PCOSA_DATAMODEL_HARVESTER)malloc(sizeof(COSA_DATAMODEL_HARVESTER));
    ASSERT_NE(pHarvesterObject, nullptr) << "Memory allocation for pHarvesterObject failed.";
    memset(pHarvesterObject, 0, sizeof(COSA_DATAMODEL_HARVESTER));

   EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_ )).Times(::testing::AnyNumber()).WillRepeatedly(::testing::Return(0));

    EXPECT_EQ(CosaHarvesterInitialize(pHarvesterObject), ANSC_STATUS_SUCCESS)
        << "Expected CosaHarvesterInitialize to return ANSC_STATUS_SUCCESS on successful initialization.";

    free(pHarvesterObject);
}

//Test case for CosaHarvesterRemove
TEST_F(HarvesterTestFixture, CallsAnscFreeMemorySuccessfully) {
    
    PCOSA_DATAMODEL_HARVESTER pHarvesterObject = (PCOSA_DATAMODEL_HARVESTER)malloc(sizeof(COSA_DATAMODEL_HARVESTER));
    ASSERT_NE(pHarvesterObject, nullptr) << "Memory allocation for pHarvesterObject failed.";
    memset(pHarvesterObject, 0, sizeof(COSA_DATAMODEL_HARVESTER));

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemory(::testing::Eq((ANSC_HANDLE)pHarvesterObject)))
        .Times(1);

    EXPECT_EQ(CosaHarvesterRemove(pHarvesterObject), ANSC_STATUS_SUCCESS)
        << "Expected CosaHarvesterRemove to return ANSC_STATUS_SUCCESS.";

    free(pHarvesterObject);
}
