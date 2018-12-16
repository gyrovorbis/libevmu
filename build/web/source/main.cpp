#include <stdio.h>
#include <emscripten/bind.h>

#include "vmuwrapper.h"

void gyInit() {
  GYDebugCallbacks dbgCallbacks = {
    NULL, NULL, NULL
  };
  gySysDebugEnable(GY_DEBUG_VERBOSE, &dbgCallbacks);
  gyPrintf("libGyro init\n");
  gyDebugInit();
  gyFileInit(NULL);
  gySysInit(NULL);
  gyVidInit(NULL);
  gyTimerInit();
  gyAudInit();
  gyInputInit(NULL);
  gyNetInit(NULL);
  gyPrintf("libGyro init complete\n");
}

void gyUninit() {
  gyPrintf("libGyro uninit\n");
  gyNetUninit();
  gyInputUninit();
  gyAudUninit();
  gyTimerUninit();
  gySysUninit();
  gyFileQuit();
  gyDebugUninit();
  gyVidUninit();
  gyPrintf("libGyro uninit complete\n");
}


EMSCRIPTEN_BINDINGS(evmu) {
  emscripten::class_<VMUWrapper>("VMUWrapper")
    .constructor<>()
    .function("flashFormatDefault", &VMUWrapper::flashFormatDefault)
    .function("flashRootBlockPrint", &VMUWrapper::flashRootBlockPrint)
    .function("deviceCreate", &VMUWrapper::deviceCreate)
    .function("deviceUpdate", &VMUWrapper::deviceUpdate)
    .function("displayPixelGet", &VMUWrapper::displayPixelGet)
    .function("loadBios", &VMUWrapper::loadBios)
    .function("resetCPU", &VMUWrapper::resetCPU)
    .function("flashLoadImage", &VMUWrapper::flashLoadImage, emscripten::allow_raw_pointers())
    // .property("displayWidth", &VMUWrapper::getDisplayWidth, &VMUWrapper::noSetPropInt)
    // .property("displayHeight", &VMUWrapper::getDisplayHeight, &VMUWrapper::noSetPropInt)
  ;
  emscripten::function("gyInit", &gyInit);
  emscripten::function("gyUninit", &gyUninit);
}
