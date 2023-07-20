/*! \file
 *  \ingroup file_formats
 *  \brief   .DCI and .DCM Nexus file formats
 *
 *  This file contains the public API for
 *  managing save file formats associated with
 *  the Nexus Memory card (.DCI and .DCM).
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_NEXUS_H
#define EVMU_NEXUS_H

#include "evmu_fs_utils.h"

#define EVMU_NEXUS_FLASH_EXT ".dcm"
#define EVMU_NEXUS_FILE_EXT  ".dci"

//! Applies Nexus byte reordering to the given data pointer, whose size must be a multiple of 4
EVMU_EXPORT void* EvmuNexus_applyByteOrdering(void* pData, size_t bytes);

#endif // EVMU_NEXUS_H
