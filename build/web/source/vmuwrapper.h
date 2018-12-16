#ifndef EVMU_WEB_WRAPPER
#define EVMU_WEB_WRAPPER

#include <string>
#include <gyro_vmu_device.h>
#include <gyro_vmu_display.h>
#include <gyro_system_api.h>
#include <gyro_file_api.h>
#include <gyro_timer_api.h>
#include <gyro_audio_api.h>
#include <gyro_input_api.h>
#include <gyro_video_api.h>
#include <gyro_net_api.h>

class VMUWrapper {
  private:
    VMUDevice* device = 0;

  public:
    VMUWrapper();

    void deviceCreate();
    void deviceUpdate(float delta);

    int flashLoadImage(std::string file);
    void loadBios(std::string file);
    void resetCPU();

    int displayPixelGet(int x, int y);

    int getDisplayWidth();
    int getDisplayHeight();

    void flashFormatDefault();
    void flashRootBlockPrint();

    void noSetPropInt(int v);
};

#endif
