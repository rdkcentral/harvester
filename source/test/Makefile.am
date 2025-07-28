##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2019 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER

AM_CPPFLAGS = -Wall -g -Werror
AM_CXXFLAGS = -std=c++11

ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS = harvester_gtest.bin

harvester_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest \
                               -I${top_srcdir}/gtest/include \
                               -I${top_srcdir}/include \
                               -I${top_srcdir}/source \
                               -I${top_srcdir}/source/include \
                               -I${top_srcdir}/source/HarvesterCore \
                               -I${top_srcdir}/source/HarvesterSsp \
                               -DUNIT_TEST_DOCKER_SUPPORT
                               

harvester_gtest_bin_SOURCES =  gtest_main.cpp \
                               ${top_srcdir}/source/HarvesterSsp/cosa_harvester_dml.c \
                               ${top_srcdir}/source/HarvesterSsp/cosa_harvester_internal.c \
                               ${top_srcdir}/source/HarvesterSsp/harvester_associated_devices.c \
                               ${top_srcdir}/source/HarvesterSsp/harvester_associated_devices_avropack.c \
                               ${top_srcdir}/source/HarvesterSsp/harvester_neighboring_ap.c \
                               ${top_srcdir}/source/HarvesterSsp/harvester_neighboring_ap_avropack.c \
                               ${top_srcdir}/source/HarvesterSsp/harvester_neighboring_ap_ondemand.c \
                               ${top_srcdir}/source/HarvesterSsp/harvester_radio_traffic.c \
                               ${top_srcdir}/source/HarvesterSsp/harvester_radio_traffic_avropack.c \
                               ${top_srcdir}/source/HarvesterSsp/harvester_rbus_api.c \
                               ${top_srcdir}/source/HarvesterSsp/plugin_main.c \
                               ${top_srcdir}/source/HarvesterSsp/webpa_interface.c \
                               ${top_srcdir}/source/HarvesterSsp/webpa_interface_with_seshat.c \
                               ${top_srcdir}/source/HarvesterSsp/webpa_interface_without_seshat.c \
                               harvester_mock.cpp \
                               harvester_neighboring_ap_test.cpp \
                               harvester_neighboring_ap_avropack_test.cpp \
                               harvester_neighboring_ap_ondemand_test.cpp \
                               harvester_radio_traffic_avropack_test.cpp  \
                               harvester_radio_traffic_test.cpp \
                               webpa_interface_test.cpp \
                               harvester_associated_devices_avropack_test.cpp \
                               harvester_associated_devices_test.cpp \
                               cosa_harvester_internal_test.cpp \
                               cosa_harvester_dml_test.cpp \
                               harvester_rbus_api_test.cpp


harvester_gtest_bin_LDFLAGS = -lgtest -lgmock -lgcov -lpthread -lavro -luuid

harvester_gtest_bin_LDADD =$(HOME)/usr/local/lib/libmock_ccsp_common.la \
                           $(HOME)/usr/local/lib/libmock_syscfg.la \
                           $(HOME)/usr/local/lib/libmock_rbus.la \
                           $(HOME)/usr/local/lib/libmock_secure_wrapper.la \
                           $(HOME)/usr/local/lib/libmock_safec_lib.la \
                           $(HOME)/usr/local/lib/libmock_cjson.la \
                           $(HOME)/usr/local/lib/libmock_base64.la \
                           $(HOME)/usr/local/lib/libmock_psm.la \
                           $(HOME)/usr/local/lib/libmock_utopia.la \
                           $(HOME)/usr/local/lib/libmock_sysevent.la \
                           $(HOME)/usr/local/lib/libmock_wifi_hal.la \
                           $(HOME)/usr/local/lib/libmock_parodus.la


# Adding Code Coverage Flags
AM_CFLAGS += -fprofile-arcs -ftest-coverage
AM_CXXFLAGS += -fprofile-arcs -ftest-coverage
harvester_gtest_bin_LDFLAGS += --coverage


