set(LOCALREPO_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
function(local_repo name)
    set(local_repo_prefix LOCALREPO)
    set(local_repo_oneval PATCH_COMMAND CONFIGURE_COMMAND BUILD_COMMAND INSTALL_COMMAND)
    cmake_parse_arguments(${local_repo_prefix} "" "${local_repo_oneval}" "" ${ARGN})

    #using configure_file to setup build target bypass
    configure_file(
        ${LOCALREPO_MODULE_PATH}/localrepo-build.cmake.in
        ${CMAKE_BINARY_DIR}/localrepo-${name}-build.cmake
    )
    include(${CMAKE_BINARY_DIR}/localrepo-${name}-build.cmake)

endfunction()

function(local_repo_addstep name step)
    set(addstep_prefix ADDSTEP)
    set(addstep_oneval COMMAND)
    set(addstep_multival BEFORE AFTER)
    cmake_parse_arguments(${addstep_prefix} "" "${addstep_oneval}" "${addstep_multival}" ${ARGN})

    add_custom_target(${name}-${step} ${ADDSTEP_COMMAND})
    if(ADDSTEP_BEFORE)
        foreach(before_step ${ADDSTEP_BEFORE})
            add_dependencies(${name}-${step} ${name}-${before_step})
        endforeach()
    endif()
    if(ADDSTEP_AFTER)
        foreach(after_step ${ADDSTEP_AFTER})
            add_dependencies(${name}-${after_step} ${name}-${step})
        endforeach()
    endif()
    add_dependencies(${name} ${name}-${step})
endfunction()

function(local_repo_cmake name)
    set(local_repo_cmake_prefix LOCALREPOCMAKE)
    set(local_repo_cmake_opt SHARED RELEASE)
    set(local_repo_cmake_oneval SOURCE OPTIONS DESTINATION BUILD PATCH_COMMAND CONFIGURE_COMMAND BUILD_COMMAND INSTALL_COMMAND)
    cmake_parse_arguments(${local_repo_cmake_prefix} "${local_repo_cmake_opt}" "${local_repo_cmake_oneval}" "" ${ARGN})

    # compute relavant paths
    if(NOT LOCALREPOCMAKE_SOURCE)
        message(FATAL_ERROR "no sources given to project ${name}")
    endif()
    if(NOT LOCALREPOCMAKE_BUILD)
        set(LOCALREPOCMAKE_BUILD "${LOCALREPOCMAKE_SOURCE}-build")
    endif()
    if(NOT LOCALREPOCMAKE_DESTINATION)
        set(LOCALREPOCMAKE_DESTINATION "${LOCALREPOCMAKE_SOURCE}-target")
    endif()

    #prepare the directory above
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LOCALREPOCMAKE_BUILD} ${LOCALREPOCMAKE_DESTINATION}
    )

    #compute options
    if(LOCALREPOCMAKE_RELEASE)
        set(LOCALREPOCMAKE_OPTIONS "${LOCALREPOCMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Release")
    endif()
    if(LOCALREPOCMAKE_SHARED)
        set(LOCALREPOCMAKE_OPTIONS "${LOCALREPOCMAKE_OPTIONS} -DBUILD_SHARED_LIBS=ON")
    else()
        set(LOCALREPOCMAKE_OPTIONS "${LOCALREPOCMAKE_OPTIONS} -DBUILD_SHARED_LIBS=OFF")
    endif()
    set(LOCALREPOCMAKE_OPTIONS "${LOCALREPOCMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=${LOCALREPOCMAKE_DESTINATION}")

    #compute commands
    set(patch_command "")
    set(configure_command "${CMAKE_COMMAND} -E chdir ${LOCALREPOCMAKE_BUILD} ${CMAKE_COMMAND} ${LOCALREPOCMAKE_SOURCE} ${LOCALREPOCMAKE_OPTIONS}")
    set(build_command "${CMAKE_COMMAND} -E chdir ${LOCALREPOCMAKE_BUILD} ${CMAKE_MAKE_PROGRAM}")
    set(install_command "${CMAKE_COMMAND} -E chdir ${LOCALREPOCMAKE_BUILD} ${CMAKE_MAKE_PROGRAM} install")
    if(LOCALREPOCMAKE_PATCH_COMMAND)
        set(patch_command "${CMAKE_COMMAND} -E chdir ${LOCALREPOCMAKE_BUILD} ${LOCALREPOCMAKE_PATCH_COMMAND}")
    endif()
    if(LOCALREPOCMAKE_CONFIGURE_COMMAND)
        set(configure_command "${CMAKE_COMMAND} -E chdir ${LOCALREPOCMAKE_BUILD} ${LOCALREPOCMAKE_CONFIGURE_COMMAND}")
    endif()
    if(LOCALREPOCMAKE_BUILD_COMMAND)
        set(build_command "${CMAKE_COMMAND} -E chdir ${LOCALREPOCMAKE_BUILD} ${LOCALREPOCMAKE_BUILD_COMMAND}")
    endif()
    if(LOCALREPOCMAKE_INSTALL_COMMAND)
        set(install_command "${CMAKE_COMMAND} -E chdir ${LOCALREPOCMAKE_BUILD} ${LOCALREPOCMAKE_INSTALL_COMMAND}")
    endif()

    #call generic project
    local_repo(
        ${name}
        PATCH_COMMAND ${patch_command}
        CONFIGURE_COMMAND ${configure_command}
        BUILD_COMMAND ${build_command}
        INSTALL_COMMAND ${install_command}
    )
endfunction()

function(local_repo_autotool name)
    set(autotool_prefix AUTOTOOL)
    set(local_repo_autotool_opt SHARED)
    set(local_repo_autotool_oneval SOURCE BUILD OPTIONS DESTINATION PATCH_COMMAND CONFIGURE_COMMAND TARGET INSTALL)
    cmake_parse_arguments(${autotool_prefix} "${local_repo_autotool_opt}" "${local_repo_autotool_oneval}" "" ${ARGN})

    #compute directories
    if(NOT AUTOTOOL_SOURCE)
        message(FATAL_ERROR "no sources given to project ${name}")
    endif()
    if(NOT AUTOTOOL_BUILD)
        set(AUTOTOOL_BUILD "${AUTOTOOL_BUILD}-build")
    endif()
    if(NOT AUTOTOOL_DESTINATION)
        set(AUTOTOOL_DESTINATION "${AUTOTOOL_SOURCE}-target")
    endif()

    #prepare directory
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${AUTOTOOL_DESTINATION}
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${AUTOTOOL_SOURCE} ${AUTOTOOL_BUILD}
    )

    #compute options
    set(AUTOTOOL_OPTIONS "${AUTOTOOL_OPTIONS} --enable-static")
    if(AUTOTOOL_SHARED)
        set(AUTOTOOL_OPTIONS "${AUTOTOOL_OPTIONS} --enable-shared")
    endif()
    set(AUTOTOOL_OPTIONS "${AUTOTOOL_OPTIONS} --prefix=${AUTOTOOL_DESTINATION}")

    #compute commands
    set(target "")
    if(AUTOTOOL_TARGET)
        set(target ${AUTOTOOL_TARGET})
    endif()
    set(install "install")
    if(AUTOTOOL_INSTALL)
        set(install ${AUTOTOOL_INSTALL})
    endif()
    set(patch_command "")
    set(configure_command "${CMAKE_COMMAND} -E chdir ${AUTOTOOL_BUILD} ./configure ${AUTOTOOL_OPTIONS}")
    set(build_command "${CMAKE_COMMAND} -E chdir ${AUTOTOOL_BUILD} ${CMAKE_MAKE_PROGRAM} ${target}")
    set(install_command "${CMAKE_COMMAND} -E chdir ${AUTOTOOL_BUILD} ${CMAKE_MAKE_PROGRAM} ${install}")
    if(AUTOTOOL_PATCH_COMMAND)
        set(patch_command "${CMAKE_COMMAND} -E chdir ${AUTOTOOL_BUILD} ${AUTOTOOL_PATCH_COMMAND}")
    endif()
    if(AUTOTOOL_CONFIGURE_COMMAND)
        set(configure_command "${CMAKE_COMMAND} -E chdir ${AUTOTOOL_BUILD} ${AUTOTOOL_CONFIGURE_COMMAND}")
    endif()

    #call generic project
    local_repo(
        ${name}
        PATCH_COMMAND ${patch_command}
        CONFIGURE_COMMAND ${configure_command}
        BUILD_COMMAND ${build_command}
        INSTALL_COMMAND ${install_command}
    )
endfunction()
