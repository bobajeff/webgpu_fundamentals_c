#include "wgpu.h"

WGPUShaderModuleDescriptor load_wgsl(const char *name);

void request_adapter_callback(WGPURequestAdapterStatus status,
                              WGPUAdapter received, const char *message,
                              void *userdata);

void request_device_callback(WGPURequestDeviceStatus status,
                             WGPUDevice received, const char *message,
                             void *userdata);

void readBufferMap(WGPUBufferMapAsyncStatus status, void *userdata);

void initializeLog(void);

void printGlobalReport(WGPUGlobalReport report);
void printAdapterFeatures(WGPUAdapter adapter);
void printSurfaceCapabilities(WGPUSurface surface, WGPUAdapter adapter);

void handle_device_lost(WGPUDeviceLostReason reason, char const *message,
                               void *userdata);
void handle_uncaptured_error(WGPUErrorType type, char const *message,
                                    void *userdata);