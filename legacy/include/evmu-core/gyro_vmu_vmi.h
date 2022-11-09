#ifndef GYRO_VMU_VMI_H
#define GYRO_VMU_VMI_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VMU_VMI_FILE_SIZE                           108
#define VMU_VMI_FILE_INFO_FILE_MODE_GAME_POS        1
#define VMU_VMI_FILE_INFO_FILE_MODE_GAME_MASK       0x2
#define VMU_VMI_FILE_INFO_FILE_MODE_PROTECT_POS     0
#define VMU_VMI_FILE_INFO_FILE_MODE_PROTECT_MASK    0x1
#define VMU_VMI_FILE_INFO_VMS_RESOURCE_NAME_SIZE    8
#define VMU_VMI_VERSION                             0

#define VMI_FILE_MODE(type, permission) \
    ((type << VMU_VMI_FILE_INFO_FILE_MODE_GAME_POS) | (permission))

struct VMSFileInfo;

typedef enum VMI_FILE_MODE_GAME {
    VMI_FILE_MODE_GAME_DATA,
    VMI_FILE_MODE_GAME_GAME
} VMI_FILE_MODE_GAME;

typedef enum VMI_FILE_MODE_PROTECTED {
    VMI_FILE_MODE_PROTECTED_COPY_PROTECTED,
    VMI_FILE_MODE_PROTECTED_COPY_OK
} VMI_FILE_MODE_PROTECTED;

//NOTE THAT STRINGS ARE NOT NULL TERMINATED!!!
typedef struct VMIFileInfo {
    uint32_t    checksum;
    char        description[32];
    char        copyright[32];
    uint16_t    creationYear;
    uint8_t     creationMonth;
    uint8_t     creationDay;
    uint8_t     creationHour;
    uint8_t     creationMinute;
    uint8_t     creationSecond;
    uint8_t     creationWeekday;
    uint16_t    vmiVersion;
    uint16_t    fileNumber;
    char        vmsResourceName[VMU_VMI_FILE_INFO_VMS_RESOURCE_NAME_SIZE];
    char        fileNameOnVms[12];
    uint16_t    fileMode;
    uint16_t    unknown;
    uint32_t    fileSize;
} VMIFileInfo;

void        gyVmuVmiFileInfoResourceNameGet(const VMIFileInfo* info, char* string);
uint32_t    gyVmuVMIChecksumGenerate(const VMIFileInfo* info);
void        gyVmuFlashPrintVMIFileInfo(const VMIFileInfo* info);
void        gyVmuVmiGenerateFromVms(VMIFileInfo* vmi, const struct VMSFileInfo* vms, size_t vmsFileSize, VMI_FILE_MODE_GAME fileType);


#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_VMI_H

