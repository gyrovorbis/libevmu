#include "vmuwrapper.h"

VMUWrapper::VMUWrapper() {
  gyPrintf("VMUWrapper\n");
}

void VMUWrapper::deviceCreate() {
  gyPrintf("Creating device...\n");
  this->device = gyVmuDeviceCreate();
  gyPrintf("Device created\n");
}

int VMUWrapper::flashLoadImage(std::string file) {
  gyPrintf("Attempting rom load...\n");
  VMU_LOAD_IMAGE_STATUS status;
  auto* dirEntry = gyVmuFlashLoadImage(this->device, file.c_str(), &status);

  // TODO: return proper status
  return 1;
}

void VMUWrapper::loadBios(std::string file) {
  gyPrintf("Loading bios ", file.c_str(), "...");
  gyloadbios(this->device, file.c_str());
}

void VMUWrapper::deviceUpdate(float delta) {
  gyVmuDeviceUpdate(this->device, delta);
}

int VMUWrapper::displayPixelGet(int x, int y) {
  return gyVmuDisplayPixelGet(this->device, x, y);
}

void VMUWrapper::flashFormatDefault() {
  gyVmuFlashFormatDefault(device);
}

void VMUWrapper::flashRootBlockPrint() {
  gyVmuFlashRootBlockPrint(device);
}

void VMUWrapper::resetCPU() {
  gyVmuDeviceReset(this->device);
  gyVmuBuzzerEnabledSet(this->device, true);
  gyVmuDisplayGhostingEnabledSet(this->device, false);
}

int VMUWrapper::getDisplayWidth() {
  return VMU_DISP_PIXEL_WIDTH;
}

int VMUWrapper::getDisplayHeight() {
  return VMU_DISP_PIXEL_HEIGHT;
}

void VMUWrapper::noSetPropInt(int v) {
  // does fuck all, emscripten requires it tho
}
