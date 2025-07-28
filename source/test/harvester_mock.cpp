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

#include <gmock/gmock.h>
#include "harvester_mock.h"

SafecLibMock * g_safecLibMock = NULL;
UserTimeMock * g_usertimeMock = NULL;
SyscfgMock * g_syscfgMock = NULL;
AnscMemoryMock * g_anscMemoryMock = NULL;
cjsonMock *g_cjsonMock = NULL;
TraceMock * g_traceMock = NULL;
AnscFileIOMock* g_anscFileIOMock = NULL;
FileIOMock * g_fileIOMock = NULL;
SyseventMock *g_syseventMock = NULL;
MtaHalMock *g_mtaHALMock = NULL;
AnscWrapperApiMock * g_anscWrapperApiMock = NULL;
BaseAPIMock * g_baseapiMock = NULL;
AnscDebugMock* g_anscDebugMock = NULL;
SecureWrapperMock * g_securewrapperMock = NULL;
rbusMock *g_rbusMock = NULL;
base64Mock *g_base64Mock = NULL;
PsmMock * g_psmMock = NULL;
parodusMock *g_parodusMock = NULL;
utopiaMock *g_utopiaMock = NULL;
WifiHalMock *g_WifiHalMock = NULL;
PtdHandlerMock * g_PtdHandlerMock = NULL;
char  g_Subsystem[32] = {0};
ANSC_HANDLE bus_handle = NULL;
int consoleDebugEnable = 0;
FILE* debugLogFile = nullptr;

HarvesterTestFixture::HarvesterTestFixture()
{
    g_safecLibMock = new SafecLibMock;
    g_usertimeMock = new UserTimeMock;
    g_syscfgMock = new SyscfgMock;
    g_anscMemoryMock = new AnscMemoryMock;
    g_cjsonMock = new cjsonMock;
    g_traceMock = new TraceMock;
    g_anscFileIOMock = new AnscFileIOMock;
    g_fileIOMock = new FileIOMock;
    g_syseventMock = new SyseventMock;
    g_mtaHALMock = new MtaHalMock;
    g_anscWrapperApiMock = new AnscWrapperApiMock;
    g_baseapiMock  = new BaseAPIMock;
    g_anscDebugMock = new AnscDebugMock;
    g_securewrapperMock = new SecureWrapperMock;
    g_rbusMock    = new rbusMock;
    g_base64Mock = new base64Mock;
    g_psmMock   = new PsmMock;
    g_parodusMock = new parodusMock;
    g_utopiaMock = new  utopiaMock;
    g_WifiHalMock = new WifiHalMock;
    g_PtdHandlerMock = new PtdHandlerMock;
    
    
}

HarvesterTestFixture::~HarvesterTestFixture()
{
    delete g_safecLibMock;
    delete g_usertimeMock;
    delete g_syscfgMock;
    delete g_anscMemoryMock;
    delete g_cjsonMock;
    delete g_fileIOMock;
    delete g_syseventMock;
    delete g_mtaHALMock;
    delete g_anscWrapperApiMock;
    delete g_baseapiMock;
    delete g_traceMock;
    delete g_anscDebugMock;
    delete g_securewrapperMock;
    delete g_rbusMock;
    delete g_base64Mock;
    delete g_psmMock;
    delete g_parodusMock;
    delete g_utopiaMock;
    delete g_WifiHalMock;
    delete g_anscFileIOMock;
    delete g_PtdHandlerMock;

    g_safecLibMock = nullptr;
    g_usertimeMock = nullptr;
    g_syscfgMock = nullptr;
    g_anscMemoryMock = nullptr;
    g_cjsonMock = nullptr;
    g_fileIOMock = nullptr;
    g_mtaHALMock =nullptr;
    g_syseventMock =nullptr;
    g_anscWrapperApiMock = nullptr;
    g_baseapiMock = nullptr;
    g_securewrapperMock = nullptr;
    g_rbusMock = nullptr;
    g_base64Mock = nullptr;
    g_psmMock   = nullptr;
    g_parodusMock = nullptr;
    g_utopiaMock = nullptr;
    g_WifiHalMock = nullptr;
    g_traceMock = nullptr;
    g_anscDebugMock = nullptr;
    g_anscFileIOMock = nullptr;
    g_PtdHandlerMock = nullptr;
    
}

void HarvesterTestFixture::SetUp() {}
void HarvesterTestFixture::TearDown() {}
void HarvesterTestFixture::TestBody() {}

// end of file
