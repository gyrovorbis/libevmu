#include "gyro_vmu_vmi.h"
#include <gyro_system_api.h>
#include <string.h>


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

