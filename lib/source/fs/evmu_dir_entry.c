#include <evmu/fs/evmu_dir_entry.h>
#include <gimbal/strings/gimbal_string_buffer.h>

EVMU_EXPORT void EvmuTimestamp_setDateTime(EvmuTimestamp* pSelf, const GblDateTime* pDateTime) {
    const div_t years = div(pDateTime->date.year, 100);

    pSelf->century = GBL_BCD_BYTE_PACK(years.quot);
    pSelf->year    = GBL_BCD_BYTE_PACK(years.rem);
    pSelf->month   = GBL_BCD_BYTE_PACK(pDateTime->date.month);
    pSelf->day     = GBL_BCD_BYTE_PACK(pDateTime->date.day);
    pSelf->hour    = GBL_BCD_BYTE_PACK(pDateTime->time.hours);
    pSelf->minute  = GBL_BCD_BYTE_PACK(pDateTime->time.minutes);
    pSelf->second  = GBL_BCD_BYTE_PACK(pDateTime->time.seconds);
    pSelf->weekDay = GBL_BCD_BYTE_PACK(GblDate_weekDay(&pDateTime->date));
}

EVMU_EXPORT GblDateTime* EvmuTimestamp_dateTime(const EvmuTimestamp* pSelf, GblDateTime* pDateTime) {
    pDateTime->date.year     = GBL_BCD_BYTE_UNPACK(pSelf->century) * 100 +
                               GBL_BCD_BYTE_UNPACK(pSelf->year);
    pDateTime->date.month    = GBL_BCD_BYTE_UNPACK(pSelf->month);
    pDateTime->date.day      = GBL_BCD_BYTE_UNPACK(pSelf->day);
    pDateTime->time.hours    = GBL_BCD_BYTE_UNPACK(pSelf->hour);
    pDateTime->time.minutes  = GBL_BCD_BYTE_UNPACK(pSelf->minute);
    pDateTime->time.seconds  = GBL_BCD_BYTE_UNPACK(pSelf->second);
    pDateTime->time.nSeconds = 0;

    return pDateTime;
}

EVMU_EXPORT const char* EvmuDirEntry_name(const EvmuDirEntry* pSelf, GblStringBuffer* pBuffer) {
    GblStringBuffer_clear(pBuffer);
    GblStringBuffer_resize(pBuffer, EVMU_DIRECTORY_FILE_NAME_SIZE + 1);
    memcpy(GblStringBuffer_data(pBuffer), pSelf->fileName, EVMU_DIRECTORY_FILE_NAME_SIZE);

    return GblStringBuffer_cString(pBuffer);
}

EVMU_EXPORT size_t EvmuDirEntry_setName(EvmuDirEntry* pSelf, const char* pName) {
    const size_t len = strnlen(pName, EVMU_DIRECTORY_FILE_NAME_SIZE);

    memset(pSelf->fileName, '\0', EVMU_DIRECTORY_FILE_NAME_SIZE);
    memcpy(pSelf->fileName, pName, len);
    return len;
}

EVMU_EXPORT const char* EvmuDirEntry_fileTypeStr(const EvmuDirEntry* pSelf) {
    switch(pSelf->fileType) {
    case EVMU_FILE_TYPE_NONE: return "NONE";
    case EVMU_FILE_TYPE_DATA: return "DATA";
    case EVMU_FILE_TYPE_GAME: return "GAME";
    default:                  return "INVALID";
    }
}

EVMU_EXPORT const char* EvmuDirEntry_protectedStr(const EvmuDirEntry* pSelf) {
    switch(pSelf->copyProtection) {
    case 0:  return "NONE";
    case 1:  return "PROTECTED";
    default: return "INVALID";
    }
}
