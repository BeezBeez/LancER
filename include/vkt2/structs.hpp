#pragma once

#include "./utils.hpp"

namespace vkt {
    using namespace lancer;

    // application surface format information structure
    //struct SurfaceFormat : public std::enable_shared_from_this<SurfaceFormat> {
    struct SurfaceFormat {
        api::Format colorFormat = {};
        api::Format depthFormat = {};
        api::Format stencilFormat = {};
        api::ColorSpaceKHR colorSpace = {};
        api::FormatProperties colorFormatProperties = {};
    };

    // framebuffer with command buffer and fence
    struct Framebuffer : public std::enable_shared_from_this<Framebuffer> {
        api::Framebuffer frameBuffer = {};
        api::CommandBuffer commandBuffer = {}; // terminal command (barrier)
        api::Fence waitFence = {};
        api::Semaphore semaphore = {};
    };

#pragma pack(push, 1)
    struct GeometryInstance {
        //float transform[12];
        glm::mat3x4 transform;
        uint32_t instanceId : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };
#pragma pack(pop)

    


};
