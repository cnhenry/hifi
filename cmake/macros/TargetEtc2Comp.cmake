#
#  Copyright 2018 High Fidelity, Inc.
#  Created by Sam Gondelman on 5/2/2018
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_ETC2COMP)
    if (ANDROID)
        set(INSTALL_DIR ${HIFI_ANDROID_PRECOMPILED}/etc2comp)
        set(ETC2COMP_INCLUDE_DIRS "${INSTALL_DIR}/include/Etc" "${INSTALL_DIR}/include/EtcCodec")
        set(ETC2COMP_LIBRARY_DEBUG ${INSTALL_DIR}/lib/libEtcLib.a)
        set(ETC2COMP_LIBRARY_RELEASE ${INSTALL_DIR}/lib/libEtcLib.a)
        select_library_configurations(ETC2COMP)
    else()
        add_dependency_external_projects(etc2comp)
        find_package(Etc2Comp REQUIRED)
    endif()

    target_include_directories(${TARGET_NAME} PRIVATE ${ETC2COMP_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} ${ETC2COMP_LIBRARIES})
endmacro()
