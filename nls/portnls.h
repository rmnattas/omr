/*******************************************************************************
 * Copyright IBM Corp. and others 1991
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#ifndef portnls_h
#define portnls_h

/* 0x504f5254 = PORT */

#define J9NLS_PORT_NLS_FAILURE__PREFIX "PORT000"
#define J9NLS_PORT_FILE_MEMORY_ALLOCATE_FAILURE__MODULE 0x504f5254
#define J9NLS_PORT_FILE_MEMORY_ALLOCATE_FAILURE__ID 1
#define J9NLS_PORT_FILE_MEMORY_ALLOCATE_FAILURE J9NLS_PORT_FILE_MEMORY_ALLOCATE_FAILURE__MODULE, J9NLS_PORT_FILE_MEMORY_ALLOCATE_FAILURE__ID
#define J9NLS_PORT_SL_UNKOWN_ERROR__MODULE 0x504f5254
#define J9NLS_PORT_SL_UNKOWN_ERROR__ID 2
#define J9NLS_PORT_SL_UNKOWN_ERROR J9NLS_PORT_SL_UNKOWN_ERROR__MODULE, J9NLS_PORT_SL_UNKOWN_ERROR__ID
#define J9NLS_PORT_SL_ERROR_LOADING_DEPENDANT_MODULE__MODULE 0x504f5254
#define J9NLS_PORT_SL_ERROR_LOADING_DEPENDANT_MODULE__ID 3
#define J9NLS_PORT_SL_ERROR_LOADING_DEPENDANT_MODULE J9NLS_PORT_SL_ERROR_LOADING_DEPENDANT_MODULE__MODULE, J9NLS_PORT_SL_ERROR_LOADING_DEPENDANT_MODULE__ID
#define J9NLS_PORT_SL_SYMBOL_RESOLUTION_FAILURE__MODULE 0x504f5254
#define J9NLS_PORT_SL_SYMBOL_RESOLUTION_FAILURE__ID 4
#define J9NLS_PORT_SL_SYMBOL_RESOLUTION_FAILURE J9NLS_PORT_SL_SYMBOL_RESOLUTION_FAILURE__MODULE, J9NLS_PORT_SL_SYMBOL_RESOLUTION_FAILURE__ID
#define J9NLS_PORT_SL_COULD_NOT_LOAD_MODULE__MODULE 0x504f5254
#define J9NLS_PORT_SL_COULD_NOT_LOAD_MODULE__ID 5
#define J9NLS_PORT_SL_COULD_NOT_LOAD_MODULE J9NLS_PORT_SL_COULD_NOT_LOAD_MODULE__MODULE, J9NLS_PORT_SL_COULD_NOT_LOAD_MODULE__ID
#define J9NLS_PORT_SL_MODULE_FORMAT_INCORRECT__MODULE 0x504f5254
#define J9NLS_PORT_SL_MODULE_FORMAT_INCORRECT__ID 6
#define J9NLS_PORT_SL_MODULE_FORMAT_INCORRECT J9NLS_PORT_SL_MODULE_FORMAT_INCORRECT__MODULE, J9NLS_PORT_SL_MODULE_FORMAT_INCORRECT__ID
#define J9NLS_PORT_SL_NOT_A_MODULE__MODULE 0x504f5254
#define J9NLS_PORT_SL_NOT_A_MODULE__ID 7
#define J9NLS_PORT_SL_NOT_A_MODULE J9NLS_PORT_SL_NOT_A_MODULE__MODULE, J9NLS_PORT_SL_NOT_A_MODULE__ID
#define J9NLS_PORT_SL_MODULE_ARCHITECTURE_INCORRECT__MODULE 0x504f5254
#define J9NLS_PORT_SL_MODULE_ARCHITECTURE_INCORRECT__ID 8
#define J9NLS_PORT_SL_MODULE_ARCHITECTURE_INCORRECT J9NLS_PORT_SL_MODULE_ARCHITECTURE_INCORRECT__MODULE, J9NLS_PORT_SL_MODULE_ARCHITECTURE_INCORRECT__ID
#define J9NLS_PORT_SL_FILE_NOT_FOUND__MODULE 0x504f5254
#define J9NLS_PORT_SL_FILE_NOT_FOUND__ID 9
#define J9NLS_PORT_SL_FILE_NOT_FOUND J9NLS_PORT_SL_FILE_NOT_FOUND__MODULE, J9NLS_PORT_SL_FILE_NOT_FOUND__ID
#define J9NLS_PORT_SL_PERMISSION_DENIED__MODULE 0x504f5254
#define J9NLS_PORT_SL_PERMISSION_DENIED__ID 10
#define J9NLS_PORT_SL_PERMISSION_DENIED J9NLS_PORT_SL_PERMISSION_DENIED__MODULE, J9NLS_PORT_SL_PERMISSION_DENIED__ID
#define J9NLS_PORT_SL_OUT_OF_MEMORY__MODULE 0x504f5254
#define J9NLS_PORT_SL_OUT_OF_MEMORY__ID 11
#define J9NLS_PORT_SL_OUT_OF_MEMORY J9NLS_PORT_SL_OUT_OF_MEMORY__MODULE, J9NLS_PORT_SL_OUT_OF_MEMORY__ID
#define J9NLS_PORT_SL_ERROR_PACE_BIT_SET__MODULE 0x504f5254
#define J9NLS_PORT_SL_ERROR_PACE_BIT_SET__ID 12
#define J9NLS_PORT_SL_ERROR_PACE_BIT_SET J9NLS_PORT_SL_ERROR_PACE_BIT_SET__MODULE, J9NLS_PORT_SL_ERROR_PACE_BIT_SET__ID
#define J9NLS_PORT_SL_DLL_TABLE_NOT_FOUND__MODULE 0x504f5254
#define J9NLS_PORT_SL_DLL_TABLE_NOT_FOUND__ID 13
#define J9NLS_PORT_SL_DLL_TABLE_NOT_FOUND J9NLS_PORT_SL_DLL_TABLE_NOT_FOUND__MODULE, J9NLS_PORT_SL_DLL_TABLE_NOT_FOUND__ID
#define J9NLS_PORT_SL_DLL_NOT_FOUND__MODULE 0x504f5254
#define J9NLS_PORT_SL_DLL_NOT_FOUND__ID 14
#define J9NLS_PORT_SL_DLL_NOT_FOUND J9NLS_PORT_SL_DLL_NOT_FOUND__MODULE, J9NLS_PORT_SL_DLL_NOT_FOUND__ID
#define J9NLS_PORT_SL_UNABLE_TO_RESOLVE_REFERENCES__MODULE 0x504f5254
#define J9NLS_PORT_SL_UNABLE_TO_RESOLVE_REFERENCES__ID 15
#define J9NLS_PORT_SL_UNABLE_TO_RESOLVE_REFERENCES J9NLS_PORT_SL_UNABLE_TO_RESOLVE_REFERENCES__MODULE, J9NLS_PORT_SL_UNABLE_TO_RESOLVE_REFERENCES__ID
#define J9NLS_PORT_SL_INTERNAL_ERROR__MODULE 0x504f5254
#define J9NLS_PORT_SL_INTERNAL_ERROR__ID 16
#define J9NLS_PORT_SL_INTERNAL_ERROR J9NLS_PORT_SL_INTERNAL_ERROR__MODULE, J9NLS_PORT_SL_INTERNAL_ERROR__ID
#define J9NLS_PORT_ERROR_OPERATION_FAILED__MODULE 0x504f5254
#define J9NLS_PORT_ERROR_OPERATION_FAILED__ID 17
#define J9NLS_PORT_ERROR_OPERATION_FAILED J9NLS_PORT_ERROR_OPERATION_FAILED__MODULE, J9NLS_PORT_ERROR_OPERATION_FAILED__ID
#define J9NLS_PORT_ERROR_MESSAGE_LOOKUP_FAILED__MODULE 0x504f5254
#define J9NLS_PORT_ERROR_MESSAGE_LOOKUP_FAILED__ID 18
#define J9NLS_PORT_ERROR_MESSAGE_LOOKUP_FAILED J9NLS_PORT_ERROR_MESSAGE_LOOKUP_FAILED__MODULE, J9NLS_PORT_ERROR_MESSAGE_LOOKUP_FAILED__ID
#define J9NLS_PORT_SHAREDHELPER_CANNOT_CREATE_TOP_LEVEL_DIR__MODULE 0x504f5254
#define J9NLS_PORT_SHAREDHELPER_CANNOT_CREATE_TOP_LEVEL_DIR__ID 19
#define J9NLS_PORT_SHAREDHELPER_CANNOT_CREATE_TOP_LEVEL_DIR J9NLS_PORT_SHAREDHELPER_CANNOT_CREATE_TOP_LEVEL_DIR__MODULE, J9NLS_PORT_SHAREDHELPER_CANNOT_CREATE_TOP_LEVEL_DIR__ID
#define J9NLS_PORT_SHARED_OPENED_STALE_MEMORY__MODULE 0x504f5254
#define J9NLS_PORT_SHARED_OPENED_STALE_MEMORY__ID 20
#define J9NLS_PORT_SHARED_OPENED_STALE_MEMORY J9NLS_PORT_SHARED_OPENED_STALE_MEMORY__MODULE, J9NLS_PORT_SHARED_OPENED_STALE_MEMORY__ID
#define J9NLS_PORT_SHARED_OPENED_STALE_SEM__MODULE 0x504f5254
#define J9NLS_PORT_SHARED_OPENED_STALE_SEM__ID 21
#define J9NLS_PORT_SHARED_OPENED_STALE_SEM J9NLS_PORT_SHARED_OPENED_STALE_SEM__MODULE, J9NLS_PORT_SHARED_OPENED_STALE_SEM__ID
#define J9NLS_PORT_ZOS_64_APPENDING_XDS__MODULE 0x504f5254
#define J9NLS_PORT_ZOS_64_APPENDING_XDS__ID 22
#define J9NLS_PORT_ZOS_64_APPENDING_XDS J9NLS_PORT_ZOS_64_APPENDING_XDS__MODULE, J9NLS_PORT_ZOS_64_APPENDING_XDS__ID
#define J9NLS_PORT_IEATDUMP_NAME_TOO_LONG__MODULE 0x504f5254
#define J9NLS_PORT_IEATDUMP_NAME_TOO_LONG__ID 23
#define J9NLS_PORT_IEATDUMP_NAME_TOO_LONG J9NLS_PORT_IEATDUMP_NAME_TOO_LONG__MODULE, J9NLS_PORT_IEATDUMP_NAME_TOO_LONG__ID
#define J9NLS_PORT_IEATDUMP_DISK_FULL__MODULE 0x504f5254
#define J9NLS_PORT_IEATDUMP_DISK_FULL__ID 24
#define J9NLS_PORT_IEATDUMP_DISK_FULL J9NLS_PORT_IEATDUMP_DISK_FULL__MODULE, J9NLS_PORT_IEATDUMP_DISK_FULL__ID
#define J9NLS_PORT_FILE_LOCK_INVALID_FLAG__MODULE 0x504f5254
#define J9NLS_PORT_FILE_LOCK_INVALID_FLAG__ID 25
#define J9NLS_PORT_FILE_LOCK_INVALID_FLAG J9NLS_PORT_FILE_LOCK_INVALID_FLAG__MODULE, J9NLS_PORT_FILE_LOCK_INVALID_FLAG__ID
#define J9NLS_PORT_FILE_OPEN_FILE_IS_DIR__MODULE 0x504f5254
#define J9NLS_PORT_FILE_OPEN_FILE_IS_DIR__ID 26
#define J9NLS_PORT_FILE_OPEN_FILE_IS_DIR J9NLS_PORT_FILE_OPEN_FILE_IS_DIR__MODULE, J9NLS_PORT_FILE_OPEN_FILE_IS_DIR__ID
#define J9NLS_PORT_MMAP_INVALID_MEMORY_PROTECTION__MODULE 0x504f5254
#define J9NLS_PORT_MMAP_INVALID_MEMORY_PROTECTION__ID 27
#define J9NLS_PORT_MMAP_INVALID_MEMORY_PROTECTION J9NLS_PORT_MMAP_INVALID_MEMORY_PROTECTION__MODULE, J9NLS_PORT_MMAP_INVALID_MEMORY_PROTECTION__ID
#define J9NLS_PORT_MMAP_INVALID_FLAG__MODULE 0x504f5254
#define J9NLS_PORT_MMAP_INVALID_FLAG__ID 28
#define J9NLS_PORT_MMAP_INVALID_FLAG J9NLS_PORT_MMAP_INVALID_FLAG__MODULE, J9NLS_PORT_MMAP_INVALID_FLAG__ID
#define J9NLS_PORT_MMAP_INVALID_FILE_HANDLE__MODULE 0x504f5254
#define J9NLS_PORT_MMAP_INVALID_FILE_HANDLE__ID 29
#define J9NLS_PORT_MMAP_INVALID_FILE_HANDLE J9NLS_PORT_MMAP_INVALID_FILE_HANDLE__MODULE, J9NLS_PORT_MMAP_INVALID_FILE_HANDLE__ID
#define J9NLS_PORT_LINUXDUMP_PIPE_CORE__MODULE 0x504f5254
#define J9NLS_PORT_LINUXDUMP_PIPE_CORE__ID 30
#define J9NLS_PORT_LINUXDUMP_PIPE_CORE J9NLS_PORT_LINUXDUMP_PIPE_CORE__MODULE, J9NLS_PORT_LINUXDUMP_PIPE_CORE__ID
#define J9NLS_PORT_ZOS_CONDITION_FOR_SOFTWARE_RAISED_SIGNAL_RECEIVED__MODULE 0x504f5254
#define J9NLS_PORT_ZOS_CONDITION_FOR_SOFTWARE_RAISED_SIGNAL_RECEIVED__ID 31
#define J9NLS_PORT_ZOS_CONDITION_FOR_SOFTWARE_RAISED_SIGNAL_RECEIVED J9NLS_PORT_ZOS_CONDITION_FOR_SOFTWARE_RAISED_SIGNAL_RECEIVED__MODULE, J9NLS_PORT_ZOS_CONDITION_FOR_SOFTWARE_RAISED_SIGNAL_RECEIVED__ID
#define J9NLS_PORT_HYPERVISOR_OPFAILED__MODULE 0x504f5254
#define J9NLS_PORT_HYPERVISOR_OPFAILED__ID 32
#define J9NLS_PORT_HYPERVISOR_OPFAILED J9NLS_PORT_HYPERVISOR_OPFAILED__MODULE, J9NLS_PORT_HYPERVISOR_OPFAILED__ID
#define J9NLS_PORT_UNSUPPORTED_HYPERVISOR__MODULE 0x504f5254
#define J9NLS_PORT_UNSUPPORTED_HYPERVISOR__ID 33
#define J9NLS_PORT_UNSUPPORTED_HYPERVISOR J9NLS_PORT_UNSUPPORTED_HYPERVISOR__MODULE, J9NLS_PORT_UNSUPPORTED_HYPERVISOR__ID
#define J9NLS_PORT_ZOS_SHMEM_STORAGE_KEY_MISMATCH__MODULE 0x504f5254
#define J9NLS_PORT_ZOS_SHMEM_STORAGE_KEY_MISMATCH__ID 34
#define J9NLS_PORT_ZOS_SHMEM_STORAGE_KEY_MISMATCH J9NLS_PORT_ZOS_SHMEM_STORAGE_KEY_MISMATCH__MODULE, J9NLS_PORT_ZOS_SHMEM_STORAGE_KEY_MISMATCH__ID
#define J9NLS_PORT_MALLOC31_FAILURE__MODULE 0x504f5254
#define J9NLS_PORT_MALLOC31_FAILURE__ID 35
#define J9NLS_PORT_MALLOC31_FAILURE J9NLS_PORT_MALLOC31_FAILURE__MODULE, J9NLS_PORT_MALLOC31_FAILURE__ID
#define J9NLS_PORT_CSRSIC_FAILURE__MODULE 0x504f5254
#define J9NLS_PORT_CSRSIC_FAILURE__ID 36
#define J9NLS_PORT_CSRSIC_FAILURE J9NLS_PORT_CSRSIC_FAILURE__MODULE, J9NLS_PORT_CSRSIC_FAILURE__ID
#define J9NLS_PORT_NO_HYPERVISOR__MODULE 0x504f5254
#define J9NLS_PORT_NO_HYPERVISOR__ID 37
#define J9NLS_PORT_NO_HYPERVISOR J9NLS_PORT_NO_HYPERVISOR__MODULE, J9NLS_PORT_NO_HYPERVISOR__ID
#define J9NLS_PORT_MALFORMED_HYPERVISOR_SETTINGS__MODULE 0x504f5254
#define J9NLS_PORT_MALFORMED_HYPERVISOR_SETTINGS__ID 38
#define J9NLS_PORT_MALFORMED_HYPERVISOR_SETTINGS J9NLS_PORT_MALFORMED_HYPERVISOR_SETTINGS__MODULE, J9NLS_PORT_MALFORMED_HYPERVISOR_SETTINGS__ID
#define J9NLS_PORT_HYPFS_UPDATE_FAILED__MODULE 0x504f5254
#define J9NLS_PORT_HYPFS_UPDATE_FAILED__ID 39
#define J9NLS_PORT_HYPFS_UPDATE_FAILED J9NLS_PORT_HYPFS_UPDATE_FAILED__MODULE, J9NLS_PORT_HYPFS_UPDATE_FAILED__ID
#define J9NLS_PORT_HYPFS_READ_FAILED__MODULE 0x504f5254
#define J9NLS_PORT_HYPFS_READ_FAILED__ID 40
#define J9NLS_PORT_HYPFS_READ_FAILED J9NLS_PORT_HYPFS_READ_FAILED__MODULE, J9NLS_PORT_HYPFS_READ_FAILED__ID
#define J9NLS_PORT_HYPFS_NOT_MOUNTED__MODULE 0x504f5254
#define J9NLS_PORT_HYPFS_NOT_MOUNTED__ID 41
#define J9NLS_PORT_HYPFS_NOT_MOUNTED J9NLS_PORT_HYPFS_NOT_MOUNTED__MODULE, J9NLS_PORT_HYPFS_NOT_MOUNTED__ID
#define J9NLS_PORT_SYSINFO_FILE_READ_FAILED__MODULE 0x504f5254
#define J9NLS_PORT_SYSINFO_FILE_READ_FAILED__ID 42
#define J9NLS_PORT_SYSINFO_FILE_READ_FAILED J9NLS_PORT_SYSINFO_FILE_READ_FAILED__MODULE, J9NLS_PORT_SYSINFO_FILE_READ_FAILED__ID
#define J9NLS_PORT_SYSINFO_USAGE_RETRIEVAL_ERROR_MSG__MODULE 0x504f5254
#define J9NLS_PORT_SYSINFO_USAGE_RETRIEVAL_ERROR_MSG__ID 43
#define J9NLS_PORT_SYSINFO_USAGE_RETRIEVAL_ERROR_MSG J9NLS_PORT_SYSINFO_USAGE_RETRIEVAL_ERROR_MSG__MODULE, J9NLS_PORT_SYSINFO_USAGE_RETRIEVAL_ERROR_MSG__ID
#define J9NLS_PORT_HYPERVISOR_OUT_OF_MEMORY_ERROR_MSG__MODULE 0x504f5254
#define J9NLS_PORT_HYPERVISOR_OUT_OF_MEMORY_ERROR_MSG__ID 44
#define J9NLS_PORT_HYPERVISOR_OUT_OF_MEMORY_ERROR_MSG J9NLS_PORT_HYPERVISOR_OUT_OF_MEMORY_ERROR_MSG__MODULE, J9NLS_PORT_HYPERVISOR_OUT_OF_MEMORY_ERROR_MSG__ID
#define J9NLS_PORT_SYSINFO_OUT_OF_MEMORY_ERROR_MSG__MODULE 0x504f5254
#define J9NLS_PORT_SYSINFO_OUT_OF_MEMORY_ERROR_MSG__ID 45
#define J9NLS_PORT_SYSINFO_OUT_OF_MEMORY_ERROR_MSG J9NLS_PORT_SYSINFO_OUT_OF_MEMORY_ERROR_MSG__MODULE, J9NLS_PORT_SYSINFO_OUT_OF_MEMORY_ERROR_MSG__ID
#define J9NLS_PORT_SL_EXECUTABLE_OPEN_ERROR__MODULE 0x504f5254
#define J9NLS_PORT_SL_EXECUTABLE_OPEN_ERROR__ID 46
#define J9NLS_PORT_SL_EXECUTABLE_OPEN_ERROR J9NLS_PORT_SL_EXECUTABLE_OPEN_ERROR__MODULE, J9NLS_PORT_SL_EXECUTABLE_OPEN_ERROR__ID
#define J9NLS_PORT_SL_BUFFER_EXCEEDED_ERROR__MODULE 0x504f5254
#define J9NLS_PORT_SL_BUFFER_EXCEEDED_ERROR__ID 47
#define J9NLS_PORT_SL_BUFFER_EXCEEDED_ERROR J9NLS_PORT_SL_BUFFER_EXCEEDED_ERROR__MODULE, J9NLS_PORT_SL_BUFFER_EXCEEDED_ERROR__ID
#define J9NLS_PORT_DUMP_PATH_EXISTS__MODULE 0x504f5254
#define J9NLS_PORT_DUMP_PATH_EXISTS__ID 48
#define J9NLS_PORT_DUMP_PATH_EXISTS J9NLS_PORT_DUMP_PATH_EXISTS__MODULE, J9NLS_PORT_DUMP_PATH_EXISTS__ID
#define J9NLS_PORT_LINUXDUMP_PIPE_CORE_NOT_FOUND__MODULE 0x504f5254
#define J9NLS_PORT_LINUXDUMP_PIPE_CORE_NOT_FOUND__ID 49
#define J9NLS_PORT_LINUXDUMP_PIPE_CORE_NOT_FOUND J9NLS_PORT_LINUXDUMP_PIPE_CORE_NOT_FOUND__MODULE, J9NLS_PORT_LINUXDUMP_PIPE_CORE_NOT_FOUND__ID
#define J9NLS_PORT_RUNNING_IN_CONTAINER_FAILURE__MODULE 0x504f5254
#define J9NLS_PORT_RUNNING_IN_CONTAINER_FAILURE__ID 50
#define J9NLS_PORT_RUNNING_IN_CONTAINER_FAILURE J9NLS_PORT_RUNNING_IN_CONTAINER_FAILURE__MODULE, J9NLS_PORT_RUNNING_IN_CONTAINER_FAILURE__ID

#endif
