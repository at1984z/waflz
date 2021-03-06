# ----------------------------------------------------------------------------
# Copyright (C) 2016 Verizon.  All Rights Reserved.
# All Rights Reserved
#
#   Author: Reed P Morrison
#   Date:   09/13/2016
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
# project setup
# ------------------------------------------------------------------------------
project(waflz C CXX)
cmake_minimum_required(VERSION 3.5)
if (POLICY CMP0050)
    cmake_policy(SET CMP0050 OLD)
endif()
# ------------------------------------------------------------------------------
# Can't add for now...
# ------------------------------------------------------------------------------
add_definitions(-DHAVE_CONFIG_H)
# ------------------------------------------------------------------------------
# header files to install
# ------------------------------------------------------------------------------
LIST(APPEND INST_HDRS ${HDRS})
# ------------------------------------------------------------------------------
# include paths
# ------------------------------------------------------------------------------
include_directories(
    #${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    # ------------------------------------------------------
    # proto
    # ------------------------------------------------------
    ${CMAKE_BINARY_DIR}/proto
    # ------------------------------------------------------
    # yajl
    # ------------------------------------------------------
    ${CMAKE_BINARY_DIR}/ext_yajl-prefix/src/yajl-2.1.1/include
    # ------------------------------------------------------
    # waflz include
    # ------------------------------------------------------
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    # ------------------------------------------------------
    # cityhash libs
    # ------------------------------------------------------
    ${CMAKE_SOURCE_DIR}/ext/cityhash
    # ------------------------------------------------------
    # libinjection
    # ------------------------------------------------------
    ${CMAKE_SOURCE_DIR}/ext/libinjection/src
    # ------------------------------------------------------
    # libmaxminddb
    # ------------------------------------------------------
    ${CMAKE_SOURCE_DIR}/ext/libmaxminddb/src
    # ------------------------------------------------------
    # external libs
    # ------------------------------------------------------
    ${CMAKE_SOURCE_DIR}/ext
    ${CMAKE_SOURCE_DIR}/ext/yajl
    /usr/include/libxml2
)
# ------------------------------------------------------------------------------
# source files
# ------------------------------------------------------------------------------
SET(SRCS
    waflz_ts.cc
)
set(CMAKE_CXX_FLAGS "-Wall -Weffc++ -Werror -std=c++11")
# ------------------------------------------------------------------------------
# headers
# ------------------------------------------------------------------------------
SET(HDRS
    ${CMAKE_SOURCE_DIR}/include/waflz/acl.h
    ${CMAKE_SOURCE_DIR}/include/waflz/arg.h
    ${CMAKE_SOURCE_DIR}/include/waflz/atomic.h
    ${CMAKE_SOURCE_DIR}/include/waflz/challenge.h
    ${CMAKE_SOURCE_DIR}/include/waflz/city.h
    ${CMAKE_SOURCE_DIR}/include/waflz/config_parser.h
    ${CMAKE_SOURCE_DIR}/include/waflz/def.h
    ${CMAKE_SOURCE_DIR}/include/waflz/engine.h
    ${CMAKE_SOURCE_DIR}/include/waflz/geoip2_mmdb.h
    ${CMAKE_SOURCE_DIR}/include/waflz/instance.h
    ${CMAKE_SOURCE_DIR}/include/waflz/instances.h
    ${CMAKE_SOURCE_DIR}/include/waflz/parser.h
    ${CMAKE_SOURCE_DIR}/include/waflz/profile.h
    ${CMAKE_SOURCE_DIR}/include/waflz/render.h
    ${CMAKE_SOURCE_DIR}/include/waflz/rqst_ctx.h
    ${CMAKE_SOURCE_DIR}/include/waflz/rules.h
    ${CMAKE_SOURCE_DIR}/include/waflz/scopes_configs.h
    ${CMAKE_SOURCE_DIR}/include/waflz/scopes.h
    ${CMAKE_SOURCE_DIR}/include/waflz/trace.h
    ${CMAKE_SOURCE_DIR}/include/waflz/waf.h
    ${CMAKE_SOURCE_DIR}/include/waflz/waflz.h
    # ------------------------------------------------------
    # db
    # ------------------------------------------------------
    ${CMAKE_SOURCE_DIR}/include/waflz/kv_db.h
    ${CMAKE_SOURCE_DIR}/include/waflz/kycb_db.h
    ${CMAKE_SOURCE_DIR}/include/waflz/redis_db.h
    # ------------------------------------------------------
    # limit
    # ------------------------------------------------------
    ${CMAKE_SOURCE_DIR}/include/waflz/config.h
    ${CMAKE_SOURCE_DIR}/include/waflz/configs.h
    ${CMAKE_SOURCE_DIR}/include/waflz/enforcer.h
    ${CMAKE_SOURCE_DIR}/include/waflz/limit.h
    ${CMAKE_SOURCE_DIR}/include/waflz/rl_obj.h
    # ------------------------------------------------------
    # protocol buffers
    # ------------------------------------------------------
    ${CMAKE_BINARY_DIR}/proto/acl.pb.h
    ${CMAKE_BINARY_DIR}/proto/action.pb.h
    ${CMAKE_BINARY_DIR}/proto/event.pb.h
    ${CMAKE_BINARY_DIR}/proto/limit.pb.h
    ${CMAKE_BINARY_DIR}/proto/profile.pb.h
    ${CMAKE_BINARY_DIR}/proto/request_info.pb.h
    ${CMAKE_BINARY_DIR}/proto/rule.pb.h
    ${CMAKE_BINARY_DIR}/proto/scope.pb.h
)
# ------------------------------------------------------------------------------
# library
# ------------------------------------------------------------------------------
add_library(waflzts SHARED ${SRCS} ${PB_SRCS})
add_dependencies(waflzts waflzcore)
target_link_libraries(waflzts ${CMAKE_BINARY_DIR}/src/libwaflzcore.a ${LIBRARIES})

target_include_directories(waflzts PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/proto/ ${CMAKE_BINARY_DIR}/)


SET_TARGET_PROPERTIES(waflzts
  PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

# Add install target for the library
install(TARGETS waflzts
  DESTINATION lib
  COMPONENT Libraries
)

# Add install target for header files
install(FILES ${HDRS}
  DESTINATION include/waflz
  COMPONENT Headers
)

