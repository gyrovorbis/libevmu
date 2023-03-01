#include "gyro_vmu_vmi.h"
#include "gyro_vmu_vms.h"
#include <evmu/evmu_api.h>
#include <string.h>
#include <time.h>


uint32_t gyVmuVMIChecksumGenerate(const VMIFileInfo *info) {
    uint32_t checksum = 0;
    unsigned char* byte = (void*)&checksum;

    byte[0] = info->vmsResourceName[0] & 'S';
    byte[1] = info->vmsResourceName[1] & 'E';
    byte[2] = info->vmsResourceName[2] & 'G';
    byte[3] = info->vmsResourceName[3] & 'A';

    return checksum;
}

void gyVmuFlashPrintVMIFileInfo(const VMIFileInfo* vmi) {
    char string[33]; //temporary buffer to null-terminate string fields

    EVMU_LOG_VERBOSE("VMI File Attributes");
    EVMU_LOG_PUSH();

    EVMU_LOG_VERBOSE("%-20s: %40u [%s]", "Checksum",        vmi->checksum,
           (vmi->checksum == gyVmuVMIChecksumGenerate(vmi))? "VALID" : "INVALID");
    memcpy(string, vmi->description, sizeof(vmi->description));
    string[sizeof(vmi->description)] = 0;
    EVMU_LOG_VERBOSE("%-20s: %40s", "Description",          string);
    memcpy(string, vmi->copyright, sizeof(vmi->copyright));
    string[sizeof(vmi->copyright)] = 0;
    EVMU_LOG_VERBOSE("%-20s: %40s", "Copyright",            string);
    EVMU_LOG_VERBOSE("%-20s: %40u", "Year Created",         vmi->creationYear);
    EVMU_LOG_VERBOSE("%-20s: %40u", "Month Created",        vmi->creationMonth);
    EVMU_LOG_VERBOSE("%-20s: %40u", "Day Created",          vmi->creationDay);
    EVMU_LOG_VERBOSE("%-20s: %40u", "Hour Created",         vmi->creationHour);
    EVMU_LOG_VERBOSE("%-20s: %40u", "Minute Created",       vmi->creationMinute);
    EVMU_LOG_VERBOSE("%-20s: %40u", "Second Created",       vmi->creationSecond);
    EVMU_LOG_VERBOSE("%-20s: %40u", "Weekday Created",       vmi->creationWeekday);
    EVMU_LOG_VERBOSE("%-20s: %40u", "VMI Version",          vmi->vmiVersion);
    EVMU_LOG_VERBOSE("%-20s: %40u", "File Number",          vmi->fileNumber);
    memcpy(string, vmi->vmsResourceName, sizeof(vmi->vmsResourceName));
    string[sizeof(vmi->vmsResourceName)] = 0;
    EVMU_LOG_VERBOSE("%-20s: %40s", "VMS Resource Name",    string);
    memcpy(string, vmi->fileNameOnVms, sizeof(vmi->fileNameOnVms));
    string[sizeof(vmi->fileNameOnVms)] = 0;
    EVMU_LOG_VERBOSE("%-20s: %40s", "File Name on VMS",     string);
    EVMU_LOG_VERBOSE("%-20s: %40u [%s|%s]", "File Mode",    vmi->fileMode,
           (vmi->fileMode&VMU_VMI_FILE_INFO_FILE_MODE_GAME_MASK)?       "GAME":"DATA",
           (vmi->fileMode&VMU_VMI_FILE_INFO_FILE_MODE_PROTECT_MASK)?    "COPY_PROTECTED":"COPY_OK");
    EVMU_LOG_VERBOSE("%-20s: %40u", "Unknown Field",        vmi->unknown);
    EVMU_LOG_VERBOSE("%-20s: %40u", "File Size",            vmi->fileSize);

    EVMU_LOG_POP(1);
}

void gyVmuVmiFileInfoResourceNameGet(const VMIFileInfo* info, char* string) {
    memcpy(string, info->vmsResourceName, sizeof(info->vmsResourceName));
    string[sizeof(info->vmsResourceName)] = 0;
}


void gyVmuVmiGenerateFromVms(VMIFileInfo* vmi, const VMSFileInfo* vms, size_t vmsFileSize, VMI_FILE_MODE_GAME fileType) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    EVMU_LOG_VERBOSE("Generating VMI from VMS header.");
    EVMU_LOG_PUSH();

    memset(vmi, 0, sizeof(VMIFileInfo));
    memcpy(vmi->description, vms->dcDesc, VMU_VMS_FILE_INFO_DC_DESC_SIZE);
    strcpy(vmi->copyright, "Powered by ElysianVMU");
    vmi->vmiVersion = VMU_VMI_VERSION;
    memcpy(vmi->vmsResourceName, vms->dcDesc, VMU_VMI_FILE_INFO_VMS_RESOURCE_NAME_SIZE);
    memcpy(vmi->fileNameOnVms, vms->vmuDesc, VMU_VMI_FILE_INFO_VMS_RESOURCE_NAME_SIZE);
    vmi->fileMode = VMI_FILE_MODE(fileType, VMI_FILE_MODE_PROTECTED_COPY_OK);

    vmi->fileNumber         = 1;
    vmi->creationYear       = tm->tm_year;
    vmi->creationMonth      = tm->tm_mon+1;
    vmi->creationDay        = tm->tm_mday;
    vmi->creationHour       = tm->tm_hour;
    vmi->creationSecond     = tm->tm_sec;
    vmi->creationWeekday    = tm->tm_wday;
    vmi->fileSize           = /*gyVmuVmsFileInfoHeaderSize(vms) + */ vmsFileSize;
    vmi->checksum           = gyVmuVMSFileInfoCrcCalc((const unsigned char *)vms, vmi->fileSize, NULL);
    gyVmuFlashPrintVMIFileInfo(vmi);

    EVMU_LOG_POP(1);
}

