#include "gyro_vmu_vmi.h"
#include "gyro_vmu_vms.h"
#include <gyro_system_api.h>
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

    _gyLog(GY_DEBUG_VERBOSE, "VMI File Attributes");
    _gyPush();

    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u [%s]", "Checksum",        vmi->checksum,
           (vmi->checksum == gyVmuVMIChecksumGenerate(vmi))? "VALID" : "INVALID");
    memcpy(string, vmi->description, sizeof(vmi->description));
    string[sizeof(vmi->description)] = 0;
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "Description",          string);
    memcpy(string, vmi->copyright, sizeof(vmi->copyright));
    string[sizeof(vmi->copyright)] = 0;
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "Copyright",            string);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Year Created",         vmi->creationYear);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Month Created",        vmi->creationMonth);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Day Created",          vmi->creationDay);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Hour Created",         vmi->creationHour);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Minute Created",       vmi->creationMinute);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Second Created",       vmi->creationSecond);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Weekday Created",       vmi->creationWeekday);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "VMI Version",          vmi->vmiVersion);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "File Number",          vmi->fileNumber);
    memcpy(string, vmi->vmsResourceName, sizeof(vmi->vmsResourceName));
    string[sizeof(vmi->vmsResourceName)] = 0;
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "VMS Resource Name",    string);
    memcpy(string, vmi->fileNameOnVms, sizeof(vmi->fileNameOnVms));
    string[sizeof(vmi->fileNameOnVms)] = 0;
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "File Name on VMS",     string);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u [%s|%s]", "File Mode",    vmi->fileMode,
           (vmi->fileMode&VMU_VMI_FILE_INFO_FILE_MODE_GAME_MASK)?       "GAME":"DATA",
           (vmi->fileMode&VMU_VMI_FILE_INFO_FILE_MODE_PROTECT_MASK)?    "COPY_PROTECTED":"COPY_OK");
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Unknown Field",        vmi->unknown);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "File Size",            vmi->fileSize);

    _gyPop(1);
}

void gyVmuVmiFileInfoResourceNameGet(const VMIFileInfo* info, char* string) {
    memcpy(string, info->vmsResourceName, sizeof(info->vmsResourceName));
    string[sizeof(info->vmsResourceName)] = 0;
}


void gyVmuVmiGenerateFromVms(VMIFileInfo* vmi, const VMSFileInfo* vms, size_t vmsFileSize, VMI_FILE_MODE_GAME fileType) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    _gyLog(GY_DEBUG_VERBOSE, "Generating VMI from VMS header.");
    _gyPush();

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

    _gyPop(1);
}

