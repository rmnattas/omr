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

/**
 * @file
 * @ingroup Port
 * @brief Memory map
 *
 * This module provides memory mapping facilities that allow a user to map files
 * into the virtual address space of the process.  There are various options that
 * can be used when mapping a file into memory, such as copy on write.  Not all
 * of these options are available on all platforms, @ref omrmmap_capabilities
 * provides the list of supported options.  Note also that on some platforms
 * memory mapping facilites do not exist at all. On these platforms the API will
 * still be available, but will simply read the file into allocated memory.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "omrport.h"
#include "omrportasserts.h"
#include "portnls.h"
#include "ut_omrport.h"
#include "protect_helpers.h"
#include "omrportpriv.h"
#include "omrutil.h"

#if defined(J9ZOS390)
#include "omrvmem.h"
#if !defined(MAP_FAILED)
#define MAP_FAILED ((void *)(intptr_t)-1)
#endif /* !defined(MAP_FAILED) */
#endif /* defined(J9ZOS390) */

#if defined(J9ZOS39064) && !defined(__MAP_64)
#define __MAP_64 0x10
#endif /* defined(J9ZOS39064) && !defined(__MAP_64) */

#if defined(AIXPPC)
#include <sys/shm.h>
#include <sys/vminfo.h>
#endif/*AIXPPC*/

#if defined(J9ZOS390)
static J9MmapHandle *
omrmmap_read_map_file(struct OMRPortLibrary *portLibrary, intptr_t file, uint64_t offset, uintptr_t size, uint32_t category)
{
	/* default implementation will allocate memory and read the file into it */
	uintptr_t numBytesRead = 0;
	intptr_t rc = 0;
	void *mappedMemory = NULL;
	void *allocPointer = NULL;
	J9MmapHandle *returnVal = NULL;

	Trc_PRT_mmap_map_file_default_entered(file, size);

	if (-1 == file) {
		Trc_PRT_mmap_map_file_default_badfile();
		return NULL;
	}

	if (0 == size) {
		struct stat buf;

		memset(&buf, 0, sizeof(buf));
		if (-1 == fstat(file - FD_BIAS, &buf)) {
			portLibrary->error_set_last_error(portLibrary, errno, OMRPORT_ERROR_MMAP_MAP_FILE_STATFAILED);
			Trc_PRT_mmap_map_file_unix_filestatfailed_exit();
			return NULL;
		}
		size = buf.st_size;
	}

	if (0 != portLibrary->file_seek(portLibrary, file, offset, EsSeekSet)) {
		Trc_PRT_mmap_map_seek_failed(offset);
		return NULL;
	}

	allocPointer = portLibrary->mem_allocate_memory(portLibrary, size, OMR_GET_CALLSITE(), category);
	Trc_PRT_mmap_map_file_default_allocPointer(allocPointer, size);

	if (NULL == allocPointer) {
		Trc_PRT_mmap_map_file_default_badallocPointer();
		portLibrary->file_close(portLibrary, file);
		return NULL;
	}

	mappedMemory = allocPointer;
	Trc_PRT_mmap_map_file_default_mappedMemory(mappedMemory);

	numBytesRead = 0;
	rc = 0;
	while (size > numBytesRead) {
		rc = portLibrary->file_read(portLibrary, file, (void *)(((uintptr_t)mappedMemory) + numBytesRead), size - numBytesRead);
		if (rc <= 0) {
			/* failed to completely read the file */
			Trc_PRT_mmap_map_file_default_badread();
			portLibrary->mem_free_memory(portLibrary, allocPointer);
			return NULL;
		}
		numBytesRead += rc;
		Trc_PRT_mmap_map_file_default_readingFile(numBytesRead);
	}

	returnVal = (J9MmapHandle *)portLibrary->mem_allocate_memory(portLibrary, sizeof(J9MmapHandle), OMR_GET_CALLSITE(), category);
	if (NULL == returnVal) {
		portLibrary->mem_free_memory(portLibrary, allocPointer);
		Trc_PRT_mmap_map_file_cannotallocatehandle_exit();
		return NULL;
	}
	returnVal->pointer = mappedMemory;
	returnVal->allocPointer = allocPointer;
	returnVal->size = size;

	Trc_PRT_mmap_map_file_default_exit();
	return returnVal;
}
#endif /* defined(J9ZOS390) */
/**
 * Map a part of file into memory.
 *
 * @param [in]  portLibrary       The port library
 * @param [in]  file                       The file descriptor/handle of the already open file to be mapped
 * @param [in]  offset                  The file offset of the part to be mapped
 * @param [in]  size                     The number of bytes to be mapped, if zero, the whole file is mapped
 * @param [in]  mappingName      The name of the file mapping object to be created/opened.  This will be used as the basis of the name (invalid
 *                                                         characters being converted to '_') of the file mapping object on Windows
 *                                                         so that it can be shared between processes.  If a named object is not required, this parameter can be
 *                                                         specified as NULL
 * @param [in]   flags                  Flags relating to the mapping:
 * @args                                         OMRPORT_MMAP_FLAG_READ                     read only map
 * @args                                         OMRPORT_MMAP_FLAG_WRITE                   read/write map
 * @args                                         OMRPORT_MMAP_FLAG_COPYONWRITE copy on write map
 * @args                                         OMRPORT_MMAP_FLAG_SHARED              share memory mapping with other processes
 * @args                                         OMRPORT_MMAP_FLAG_PRIVATE              private memory mapping, do not share with other processes (implied by OMRPORT_MMAP_FLAG_COPYONWRITE)
 * @args                                         OMRPORT_MMAP_FLAG_ZOS_READ_MAPFILE     read the mapping file into allocated memory (which is the old behaviour of omrmmap_map_file()
 * 																						implementation on z/OS)
 * @param [in]  categoryCode     Memory allocation category code
 *
 * @return                       A J9MmapHandle struct or NULL is an error has occurred
 */
J9MmapHandle *
omrmmap_map_file(struct OMRPortLibrary *portLibrary, intptr_t file, uint64_t offset, uintptr_t size, const char *mappingName, uint32_t flags, uint32_t categoryCode)
{
	int mmapProt = 0;
	int mmapFlags = 0;
	void *pointer = NULL;
	int rwCount = 0;
	int spCount = 0;
	char const *errMsg;
	J9MmapHandle *returnVal;
	OMRMemCategory *category = omrmem_get_category(portLibrary, categoryCode);

#if defined(J9ZOS390)
	if (OMR_ARE_ANY_BITS_SET(flags, OMRPORT_MMAP_FLAG_ZOS_READ_MAPFILE)) {
		/* Use the old z/OS omrmmap_map_file() implementation that reads the file into allocated memory */
		return omrmmap_read_map_file(portLibrary, file, offset, size, categoryCode);
	}
#endif /* defined(J9ZOS390) */
	Trc_PRT_mmap_map_file_unix_entered(file, offset, size, mappingName, flags);

	/* Determine prot and flags values for mmap */
	/* One, and only one, read/write option can be specified and zero or one shared/private option */
	if (flags & OMRPORT_MMAP_FLAG_READ) {
		mmapProt = PROT_READ;
		mmapFlags = MAP_SHARED;
		rwCount++;
	}
	if (flags & OMRPORT_MMAP_FLAG_WRITE) {
		mmapProt = PROT_READ | PROT_WRITE;
		mmapFlags = MAP_SHARED;
		rwCount++;
	}
	if (flags & OMRPORT_MMAP_FLAG_COPYONWRITE) {
		mmapProt = PROT_READ | PROT_WRITE;
		mmapFlags = MAP_PRIVATE;
		rwCount++;
	}
	if (flags & OMRPORT_MMAP_FLAG_SHARED) {
		mmapFlags = MAP_SHARED;
		spCount++;
	}
	if (flags & OMRPORT_MMAP_FLAG_PRIVATE) {
		mmapFlags = MAP_PRIVATE;
		spCount++;
	}

#if defined(J9ZOS39064)
	if (OMR_ARE_ANY_BITS_SET(flags, OMRPORT_MMAP_FLAG_ZOS_64BIT)) {
		if (zos_version_at_least(ZOS_V2R4_RELEASE, ZOS_V2R4_VERSION)) {
			mmapFlags |= __MAP_64;
		} else {
			Trc_PRT_mmap_map_file_unix_invalidFlags();
			errMsg = portLibrary->nls_lookup_message(
						portLibrary,
						J9NLS_ERROR | J9NLS_DO_NOT_APPEND_NEWLINE,
						J9NLS_PORT_MMAP_INVALID_FLAG,
						NULL);
			portLibrary->error_set_last_error_with_message(
						portLibrary,
						OMRPORT_ERROR_MMAP_MAP_FILE_INVALIDFLAGS,
						errMsg);
			return NULL;
		}
	}
#endif /* defined(J9ZOS39064) */

	if (1 != rwCount) {
		Trc_PRT_mmap_map_file_unix_invalidFlags();
		errMsg = portLibrary->nls_lookup_message(portLibrary,
				 J9NLS_ERROR | J9NLS_DO_NOT_APPEND_NEWLINE,
				 J9NLS_PORT_MMAP_INVALID_MEMORY_PROTECTION,
				 NULL);
		portLibrary->error_set_last_error_with_message(portLibrary, OMRPORT_ERROR_MMAP_MAP_FILE_INVALIDFLAGS, errMsg);
		return NULL;
	}
	if (spCount > 1) {
		Trc_PRT_mmap_map_file_unix_invalidFlags();
		errMsg = portLibrary->nls_lookup_message(portLibrary,
				 J9NLS_ERROR | J9NLS_DO_NOT_APPEND_NEWLINE,
				 J9NLS_PORT_MMAP_INVALID_FLAG,
				 NULL);
		portLibrary->error_set_last_error_with_message(portLibrary, OMRPORT_ERROR_MMAP_MAP_FILE_INVALIDFLAGS, errMsg);
		return NULL;
	}
	Trc_PRT_mmap_map_file_unix_flagsSet(mmapProt, mmapFlags);

	if (0 == size) {
		struct stat buf;

		memset(&buf, 0, sizeof(struct stat));
		if (-1 == fstat(file - FD_BIAS, &buf)) {
			portLibrary->error_set_last_error(portLibrary, errno, OMRPORT_ERROR_MMAP_MAP_FILE_STATFAILED);
			Trc_PRT_mmap_map_file_unix_filestatfailed_exit();
			return NULL;
		}
		size = buf.st_size;
	}

	if (!(returnVal = (J9MmapHandle *)portLibrary->mem_allocate_memory(portLibrary, sizeof(J9MmapHandle), OMR_GET_CALLSITE(), categoryCode))) {
		Trc_PRT_mmap_map_file_cannotallocatehandle_exit();
		return NULL;
	}
#if defined(J9ZOS390)
	returnVal->allocPointer = NULL;
#endif /* defined(J9ZOS390) */
	/* Call mmap */
	pointer = mmap(0, size, mmapProt, mmapFlags, file - FD_BIAS, offset);
	if (pointer == MAP_FAILED) {
		portLibrary->mem_free_memory(portLibrary, returnVal);
		Trc_PRT_mmap_map_file_unix_badMmap(errno);
		portLibrary->error_set_last_error(portLibrary, errno, OMRPORT_ERROR_MMAP_MAP_FILE_MAPPINGFAILED);
		return NULL;
	}

	returnVal->category = category;
	omrmem_categories_increment_counters(category, size);

	returnVal->pointer = pointer;
	returnVal->size = size;

	/* Completed, return */
	Trc_PRT_mmap_map_file_unix_exiting(pointer, returnVal);
	return returnVal;
}
/**
 * UnMap previously mapped memory.
 *
 * @param[in] portLibrary The port library
 *
 * @param[in] handle - A pointer to a J9MmapHandle structure returned by omrmmap_map_file.
 */
void
omrmmap_unmap_file(struct OMRPortLibrary *portLibrary, J9MmapHandle *handle)
{
	intptr_t rc = 0;

	Trc_PRT_mmap_unmap_file_unix_entering(handle);

	if (handle != NULL) {
		Trc_PRT_mmap_unmap_file_unix_values(handle->pointer, handle->size);

#if defined(J9ZOS390)
		if (NULL != handle->allocPointer) {
			portLibrary->mem_free_memory(portLibrary, handle->allocPointer);
		} else
#endif /* defined(J9ZOS390) */
		{
			rc = munmap(handle->pointer, handle->size);
			omrmem_categories_decrement_counters(handle->category, handle->size);
		}
		portLibrary->mem_free_memory(portLibrary, handle);
	}

	Trc_PRT_mmap_unmap_file_unix_exiting(rc);
}
/**
 * Synchronise updates to memory mapped file region with file on disk.  The call may wait for the file write
 * to complete or this may be scheduled for a later time and the function return immediately, depending on
 * the flags setting
 *
 * @param [in]  portLibrary         The port library
 * @param [in]  start                      Pointer to the start of the memory mapped area to be synchronised
 * @param [in]  length                  Length of the memory mapped area to be synchronised
 * @param [in]  flags                    Flags controlling the behaviour of the function:
 * @args                                           OMRPORT_MMAP_SYNC_WAIT   Synchronous update required, function will not
 *                                                                     return until file updated.  Note that to achieve this on Windows requires the
 *                                                                     file to be opened with the FILE_FLAG_WRITE_THROUGH flag
 * @args                                           OMRPORT_MMAP_SYNC_ASYNC Asynchronous update required, function returns
 *                                                                    immediately, file will be updated later
 * @args                                           OMRPORT_MMAP_SYNC_INVALIDATE Requests that other mappings of the same
 *                                                                    file be invalidated, so that they can be updated with the values just written
 *
* @return                                          0 on success, -1 on failure.  Errors will be reported using the usual port library mechanism
 */
intptr_t
omrmmap_msync(struct OMRPortLibrary *portLibrary, void *start, uintptr_t length, uint32_t flags)
{
	int msyncFlags = 0;
	int rc = 0;

	Trc_PRT_mmap_msync_unix_entered(start, length, flags);

	/* Validate flags and translate to msyncFlags */
	if ((flags & OMRPORT_MMAP_SYNC_WAIT) && (flags & OMRPORT_MMAP_SYNC_ASYNC)) {
		portLibrary->error_set_last_error(portLibrary, -1, OMRPORT_ERROR_MMAP_MSYNC_INVALIDFLAGS);
		Trc_PRT_mmap_msync_unix_invalidFlags();
		return -1;
	}
	if (flags & OMRPORT_MMAP_SYNC_WAIT) {
		msyncFlags |= MS_SYNC;
	}
	if (flags & OMRPORT_MMAP_SYNC_ASYNC) {
		msyncFlags |= MS_ASYNC;
	}
	if (flags & OMRPORT_MMAP_SYNC_INVALIDATE) {
		msyncFlags |= MS_INVALIDATE;
	}
	Trc_PRT_mmap_msync_unix_flagsSet(msyncFlags);

	/* Call msync */
	rc = msync(start, length, msyncFlags);
	if (rc == -1) {
		Trc_PRT_mmap_msync_unix_badMsync(errno);
		portLibrary->error_set_last_error(portLibrary, errno, OMRPORT_ERROR_MMAP_MSYNC_FAILED);
		return -1;
	}

	Trc_PRT_mmap_msync_unix_exiting();
	return 0;
}
/**
 * PortLibrary shutdown.
 *
 * @param[in] portLibrary The port library
 *
 * This function is called during shutdown of the portLibrary.  Any resources that were created by @ref omrmmap_startup
 * should be destroyed here.
 *
 */
void
omrmmap_shutdown(struct OMRPortLibrary *portLibrary)
{
}
/**
 * PortLibrary startup.
 *
 * This function is called during startup of the portLibrary.  Any resources that are required for
 * the memory mapping operations may be created here.  All resources created here should be destroyed
 * in @ref omrmmap_shutdown.
 *
 * @param[in] portLibrary The port library
 *
 * @return 0 on success, negative error code on failure.  Error code values returned are
 * \arg OMRPORT_ERROR_STARTUP_MMAP
 *
 * @note Most implementations will simply return success.
 */
int32_t
omrmmap_startup(struct OMRPortLibrary *portLibrary)
{
#if defined(LINUX)
	uintptr_t handle;
	/* If omrsl_open_shared_library fails it already reports error, and we'll treat as if memfd_create is not avaiable */
	if (0 == omrsl_open_shared_library(portLibrary, NULL, &handle, 0)) {
		uintptr_t func = 0;
		omrsl_lookup_name(portLibrary, handle, "memfd_create", &func, NULL);
		/* If omrsl_lookup_name does not succeed, func value remains 0 */
		PPG_memfd_function = (memfd_function_t)func;
	}
#endif /* defined(LINUX) */

	return 0;
}
/**
 * Check the capabilities available for J9MMAP at runtime for the current platform.
 *
 *
 * @param[in] portLibrary The port library
 *
 * @return a bit map containing the capabilites supported by the omrmmap sub component of the port library.
 * Possible bit values:
 *   OMRPORT_MMAP_CAPABILITY_COPYONWRITE - if not present, platform is not capable of "copy on write" memory mapping.
 *
 */
int32_t
omrmmap_capabilities(struct OMRPortLibrary *portLibrary)
{
	return (OMRPORT_MMAP_CAPABILITY_COPYONWRITE
			| OMRPORT_MMAP_CAPABILITY_READ
#if !defined(J9ZOS390)
			| OMRPORT_MMAP_CAPABILITY_PROTECT
#endif /* defined(J9ZOS390) */
			/* If JSE platforms include WRITE and MSYNC */
#if ((defined(LINUX) && defined(J9X86)) \
  || (defined(LINUXPPC)) \
  || (defined(LINUX) && defined(S390)) \
  || (defined(LINUX) && defined(J9HAMMER)) \
  || (defined(LINUX) && defined(ARM)) \
  || (defined(LINUX) && defined(AARCH64)) \
  || (defined(LINUX) && defined(RISCV64)) \
  || (defined(AIXPPC)) \
  || (defined(J9ZOS390)) \
  || (defined(OSX)))
			| OMRPORT_MMAP_CAPABILITY_WRITE
			| OMRPORT_MMAP_CAPABILITY_MSYNC
#endif
		   );
}

intptr_t
omrmmap_protect(struct OMRPortLibrary *portLibrary, void *address, uintptr_t length, uintptr_t flags)
{
#if defined(J9ZOS390)
	return OMRPORT_PAGE_PROTECT_NOT_SUPPORTED;
#else /* defined(J9ZOS390) */
	return protect_memory(portLibrary, address, length, flags);
#endif /* defined(J9ZOS390) */
}

uintptr_t
omrmmap_get_region_granularity(struct OMRPortLibrary *portLibrary, void *address)
{
	return protect_region_granularity(portLibrary, address);
}

#if defined(J9ZOS390)
#pragma linkage (PGSERRM,OS)
#pragma map (Pgser_Release,"PGSERRM")
#if defined(OMR_ENV_DATA64)
#pragma linkage (omrdiscard_data,OS_NOSTACK)
int omrdiscard_data(void *address, int numFrames);
#endif /* defined(OMR_ENV_DATA64) */
#endif /* defined(J9ZOS390) */

/**
 * Disclaim entire pages only: don't release partial pages at the start and end.
 * This is to prevent reloading pages used by other callers and to satisfy madvise's constraints.
 */
void
omrmmap_dont_need(struct OMRPortLibrary *portLibrary, const void *startAddress, size_t length)
{
	size_t pageSize = portLibrary->mmap_get_region_granularity(portLibrary, (void *)startAddress);

	Trc_PRT_mmap_dont_need(pageSize, startAddress, length);

	if (pageSize > 0 && length >= pageSize) {
		uintptr_t endAddress = (uintptr_t) startAddress + length;
		uintptr_t roundedStart = ROUND_UP_TO_POWEROF2((uintptr_t) startAddress, pageSize);
		size_t roundedLength = ROUND_DOWN_TO_POWEROF2(endAddress - roundedStart, pageSize);
		if (roundedLength >= pageSize) {
			Trc_PRT_mmap_dont_need_oscall(roundedStart, roundedLength);
#if defined(LINUX) || defined(OSX)
			if (-1 == madvise((void *)roundedStart, roundedLength, MADV_DONTNEED)) {
				Trc_PRT_mmap_dont_need_madvise_failed((void *)roundedStart, roundedLength, errno);
			}
#elif defined(AIXPPC)
			/* madvise is not supported on AIX.  disclaim64() fails on virtual memory mapped to a file */
			if (-1 == disclaim64((void *)roundedStart, roundedLength, DISCLAIM_ZEROMEM)) {
				Trc_PRT_mmap_dont_need_disclaim64_failed((void *)roundedStart, roundedLength, errno);
			}
#elif defined(J9ZOS390) /* defined(AIXPPC) */
#if defined(OMR_ENV_DATA64)
			if (0 != omrdiscard_data((void *)roundedStart, roundedLength >> ZOS_REAL_FRAME_SIZE_SHIFT)) {
				Trc_PRT_mmap_dont_need_j9discard_data_failed((void *)roundedStart, roundedLength);
			}
#else /* defined(OMR_ENV_DATA64) */
			if (0 != Pgser_Release((void *)roundedStart, roundedLength)) {
				Trc_PRT_mmap_dont_need_Pgser_Release_failed((void *)roundedStart, roundedLength);
			}
#endif /* defined(OMR_ENV_DATA64) */
#endif /* defined(LINUX) || defined(OSX) */
		}
	}
}
