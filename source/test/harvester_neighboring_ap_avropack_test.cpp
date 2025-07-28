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
char* GetNAPSchemaBuffer();
int GetNAPSchemaBufferSize();
char* GetNAPSchemaIDBuffer();
int GetNAPSchemaIDBufferSize();
char* GetNeighborAPAvroBuf();
int NumberofNAPElementsinLinkedList(struct neighboringapdata* head);
ULONG NumberofNAPDevicesinLinkedList(struct neighboringapdata* head);
int GetNeighborAPAvroBufSize();
void ap_avro_cleanup();
}
extern char* nap_schema_buffer;
extern char* nap_schemaidbuffer;
#define WRITER_BUF_SIZE  (1024 * 150) // 150K
extern char AvroNAPSerializedBuf[WRITER_BUF_SIZE];
extern size_t AvroNAPSerializedSize;


//GetNAPSchemaBuffer
TEST_F(HarvesterTestFixture, ReturnsNullWhenBufferNotSet) {
    EXPECT_EQ(GetNAPSchemaBuffer(), nullptr);
}

TEST_F(HarvesterTestFixture, ReturnsCorrectBufferWhenSet) {
    const char* testString = "Test NAP Schema Buffer";
    nap_schema_buffer = strdup(testString);

    EXPECT_STREQ(GetNAPSchemaBuffer(), testString);

    free(nap_schema_buffer);
    nap_schema_buffer = nullptr;
}

//GetNAPSchemaBufferSize
TEST_F(HarvesterTestFixture, ReturnsZeroWhenBufferNotSet) {
    EXPECT_EQ(GetNAPSchemaBufferSize(), 0);
}

TEST_F(HarvesterTestFixture, ReturnsCorrectSizeWhenBufferSet) {
    const char* testString = "Test NAP Schema Buffer";
    nap_schema_buffer = strdup(testString);

    EXPECT_EQ(GetNAPSchemaBufferSize(), strlen(testString));
}

TEST_F(HarvesterTestFixture, ReturnsZeroForEmptyBuffer) {
    nap_schema_buffer = strdup("");

    EXPECT_EQ(GetNAPSchemaBufferSize(), 0);
}

//GetNAPSchemaIDBuffer
TEST_F(HarvesterTestFixture, ReturnsCorrectSchemaIDBuffer) {
   const char* expectedSchemaID = "e375b355-988b-45f8-9ec9-feb4b53ed843/4ae36536e6cbf4e4a0d5d873d0668bfb";
    nap_schemaidbuffer = strdup(expectedSchemaID);
    EXPECT_STREQ(GetNAPSchemaIDBuffer(), expectedSchemaID);
    free(nap_schemaidbuffer);
    nap_schemaidbuffer = nullptr;
}


//GetNAPSchemaIDBufferSize
TEST_F(HarvesterTestFixture, ReturnsCorrectSizeWhenIDBufferSet) {
    const char* testIDString = "e375b355-988b-45f8-9ec9-feb4b53ed843/4ae36536e6cbf4e4a0d5d873d0668bfb";
    nap_schemaidbuffer = strdup(testIDString);

    EXPECT_EQ(GetNAPSchemaIDBufferSize(), strlen(testIDString));

   
    free(nap_schemaidbuffer);
    nap_schemaidbuffer = nullptr;
}

TEST_F(HarvesterTestFixture, ReturnsZeroWhenIDBufferNull) {
    nap_schemaidbuffer = nullptr;
    EXPECT_EQ(GetNAPSchemaIDBufferSize(), 0);
}

//GetNeighborAPAvroBuf
TEST_F(HarvesterTestFixture, ReturnsCorrectBufferAddress) {
    
    EXPECT_EQ(GetNeighborAPAvroBuf(), AvroNAPSerializedBuf);
}

TEST_F(HarvesterTestFixture, BufferHoldsExpectedContent) {
    const char* testContent = "Test Neighbor AP Data";
    strncpy(AvroNAPSerializedBuf, testContent, WRITER_BUF_SIZE);
    EXPECT_STREQ(GetNeighborAPAvroBuf(), testContent);
}

//GetNeighborAPAvroBufSize
TEST_F(HarvesterTestFixture, ReturnsCorrectSizeWhenSet) {
    AvroNAPSerializedSize = 1024; 
    EXPECT_EQ(GetNeighborAPAvroBufSize(), 1024);
}

TEST_F(HarvesterTestFixture, ReturnsZeroWhenNotSet) {
    AvroNAPSerializedSize = 0; 
    EXPECT_EQ(GetNeighborAPAvroBufSize(), 0);
}

//NumberofNAPElementsinLinkedList
TEST_F(HarvesterTestFixture, iEmptyList) {
    neighboringapdata* headnode = nullptr; 
    EXPECT_EQ(NumberofNAPElementsinLinkedList(headnode), 0);
}

TEST_F(HarvesterTestFixture, iSingleElement) {
    neighboringapdata* headnode = new neighboringapdata();
    headnode->next = nullptr; 
    EXPECT_EQ(NumberofNAPElementsinLinkedList(headnode), 1);
    delete headnode; 
}

TEST_F(HarvesterTestFixture, iMultipleElements) {
    neighboringapdata* headnode = new neighboringapdata(); 
    headnode->next = new neighboringapdata(); 
    headnode->next->next = new neighboringapdata(); 
    headnode->next->next->next = nullptr; 
    EXPECT_EQ(NumberofNAPElementsinLinkedList(headnode), 3);

   
    delete headnode->next->next;
    delete headnode->next;
    delete headnode;
}

TEST_F(HarvesterTestFixture, iTwoElements) {
    neighboringapdata* headnode = new neighboringapdata(); 
    headnode->next = new neighboringapdata(); 
    headnode->next->next = nullptr; 
    EXPECT_EQ(NumberofNAPElementsinLinkedList(headnode), 2);

    
    delete headnode->next;
    delete headnode;
}

// NumberofNAPDevicesinLinkedList tests
TEST_F(HarvesterTestFixture, EmptyList) {
    neighboringapdata* headnode = nullptr;
    EXPECT_EQ(NumberofNAPDevicesinLinkedList(headnode), 0);
}

TEST_F(HarvesterTestFixture, SingleElement) {
    neighboringapdata* headnode = new neighboringapdata();
    headnode->numNeibouringAP = 5; 
    headnode->next = nullptr;
    EXPECT_EQ(NumberofNAPDevicesinLinkedList(headnode), 5);
    delete headnode; 
}

TEST_F(HarvesterTestFixture, MultipleElements) {
    neighboringapdata* headnode = new neighboringapdata();
    headnode->numNeibouringAP = 2; 
    headnode->next = new neighboringapdata();
    headnode->next->numNeibouringAP = 3; 
    headnode->next->next = new neighboringapdata();
    headnode->next->next->numNeibouringAP = 4; 
    headnode->next->next->next = nullptr;

    EXPECT_EQ(NumberofNAPDevicesinLinkedList(headnode), 9); 

    
    delete headnode->next->next;
    delete headnode->next;
    delete headnode;
}

TEST_F(HarvesterTestFixture, TwoElements) {
    neighboringapdata* headnode = new neighboringapdata();
    headnode->numNeibouringAP = 7; 
    headnode->next = new neighboringapdata();
    headnode->next->numNeibouringAP = 1; 
    headnode->next->next = nullptr;

    EXPECT_EQ(NumberofNAPDevicesinLinkedList(headnode), 8); 

    
    delete headnode->next;
    delete headnode;
}

TEST_F(HarvesterTestFixture, avroclean) {
    nap_schema_buffer = (char*)malloc(100); 
strcpy(nap_schema_buffer, "{\"type\":\"record\",\"name\":\"TestSchema\"}"); 

    ap_avro_cleanup();
}


