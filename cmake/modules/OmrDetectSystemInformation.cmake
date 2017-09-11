###############################################################################
# Copyright (c) 2017, 2017 IBM Corp. and others
#
# This program and the accompanying materials are made available under
# the terms of the Eclipse Public License 2.0 which accompanies this
# distribution and is available at http://eclipse.org/legal/epl-2.0
# or the Apache License, Version 2.0 which accompanies this distribution
# and is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the
# Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
# version 2 with the GNU Classpath Exception [1] and GNU General Public
# License, version 2 with the OpenJDK Assembly Exception [2].
#
# [1] https://www.gnu.org/software/classpath/license.html
# [2] http://openjdk.java.net/legal/assembly-exception.html
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
#############################################################################

# Translate from CMake's view of the system to the OMR view of the system. 
# Exports a number of variables indicicating platform, os, endianness, etc. 
# - OMR_ARCH_{X86,ARM,S390} # TODO: Add POWER
# - OMR_ENV_DATA{32,64}  
# - OMR_ENV_TARGET_DATASIZE (either 32 or 64) 
# - OMR_ENV_LITTLE_ENDIAN
# - OMR_HOST_OS
macro(omr_detect_system_information) 

   set(OMR_TEMP_DATA_SIZE "unknown")
   
   if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
      set(OMR_HOST_ARCH "x86")
      set(OMR_ARCH_X86 ON)
      set(OMR_ENV_LITTLE_ENDIAN ON)
      set(OMR_TEMP_DATA_SIZE "64")
   elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|i[3-6]86")
      set(OMR_HOST_ARCH "x86")
      set(OMR_ARCH_X86 ON)
      set(OMR_ENV_LITTLE_ENDIAN ON)
      set(OMR_TEMP_DATA_SIZE "32")
   elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
      set(OMR_HOST_ARCH "arm")
      set(OMR_ARCH_ARM ON)
      set(OMR_ENV_LITTLE_ENDIAN ON)
      set(OMR_TEMP_DATA_SIZE "32")
   elseif(CMAKE_SYSTEM_NAME MATCHES "OS390")
      set(OMR_HOST_ARCH "s390")
      set(OMR_ARCH_S390 ON)
      set(OMR_TEMP_DATA_SIZE "64")
   elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ppc64le")
      set(OMR_HOST_ARCH "ppc")
      set(OMR_ARCH_POWER ON)
      set(OMR_ENV_LITTLE_ENDIAN ON)
      set(OMR_TEMP_DATA_SIZE "64")
   elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "s390x")
      set(OMR_HOST_ARCH "s390")
      set(OMR_ARCH_S390 ON)
      set(OMR_TEMP_DATA_SIZE "64")
   elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
      set(OMR_HOST_ARCH "ppc")
      set(OMR_ARCH_POWER ON)
      set(OMR_TEMP_DATA_SIZE "64")
   else()
      message(FATAL_ERROR "Unknown processor: ${CMAKE_SYSTEM_PROCESSOR}")
   endif()
   
   if(NOT (OMR_ENV_DATA64 OR OMR_ENV_DATA32))
      #Env data size was not set in the cache or via the command line
      if(OMR_TEMP_DATA_SIZE STREQUAL 64)
         set(OMR_ENV_DATA64 ON)
         set(OMR_ENV_TARGET_DATASIZE 64)
      elseif(OMR_TEMP_DATA_SIZE STREQUAL 32)
         set(OMR_ENV_DATA32 ON)
         set(OMR_ENV_TARGET_DATASIZE 32)
      else()
         message(FATAL_ERROR "Error setting default env data size")
      endif()
   else()
      #Env data size was set in the cache or on the command line
      if(OMR_ENV_DATA64)
         #TODO assert OMR_ENV_DATA32 is not also set
         set(OMR_ENV_TARGET_DATASIZE 64)
      elseif(OMR_ENV_DATA32)
         #TODO assert OMR_ENV_DATA64 is not also set
         set(OMR_ENV_TARGET_DATASIZE 32)
      else()
         message(FATAL_ERROR "Error configuring the env data size")
      endif()
   endif()

   if(CMAKE_SYSTEM_NAME MATCHES "Linux")
      set(OMR_HOST_OS "linux")
   elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
      set(OMR_HOST_OS "osx")
   elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
      set(OMR_HOST_OS "win")
   elseif(CMAKE_SYSTEM_NAME MATCHES "AIX")
      set(OMR_HOST_OS "aix")
   elseif(CMAKE_SYSTEM_NAME MATCHES "OS390")
      set(OMR_HOST_OS "zos")
   else()
      message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
   endif()

   if(OMR_HOST_OS STREQUAL "linux")
      # Linux specifics
      set(OMR_TOOLCONFIG "gnu")
   endif()

   if(OMR_HOST_OS STREQUAL "win")
      #TODO: should probably check for MSVC
      set(OMR_WINVER "0x501")
      set(OMR_TOOLCONFIG "msvc") 
   endif()

   if(OMR_HOST_OS STREQUAL "zos")
      # ZOS specifics
      set(OMR_TOOLCONFIG "gnu")
   endif()

   if(OMR_HOST_OS STREQUAL "osx")
      # OSX specifics
      set(OMR_TOOLCONFIG "gnu") 
   endif()
   
   if(OMR_HOST_OS STREQUAL "aix")
      # AIX specifics
      set(OMR_TOOLCONFIG "xlc") 
   endif()

   message(STATUS "OMR: The CPU architecture is ${OMR_HOST_ARCH}")
   message(STATUS "OMR: The OS is ${OMR_HOST_OS}")
   message(STATUS "OMR: The tool configuration is ${OMR_TOOLCONFIG}")

endmacro(omr_detect_system_information)
