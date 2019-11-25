
#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>

namespace lancer {
    
    inline BufferMaker&& Device_T::createBufferMaker(const api::BufferCreateInfo& info, api::Buffer* lastbuf){
        return std::make_shared<Buffer_T>(shared_from_this(),info,lastbuf);
    };

    inline ImageMaker&& Device_T::createImageMaker(const api::ImageCreateInfo& info, api::Image* lastbuf){
        return std::make_shared<Image_T>(shared_from_this(),info,lastbuf);
    };
    
    inline GraphicsPipelineMaker&& Device_T::createGraphicsPipelineMaker(const api::GraphicsPipelineCreateInfo& info, api::Pipeline* pipeline, const uint32_t& width = 1u, const uint32_t& height = 1u){
        return std::make_shared<GraphicsPipeline_T>(shared_from_this(),info,pipeline,width,height);
    };

    inline DescriptorSetLayoutMaker&& Device_T::createDescriptorSetLayoutMaker(const api::DescriptorSetLayoutCreateInfo& info, api::DescriptorSetLayout* desclay){
        return std::make_shared<DescriptorSetLayout_T>(shared_from_this(),info,desclay);
    };

    inline DescriptorSetMaker&& Device_T::createDescriptorSet(const api::DescriptorSetAllocateInfo& info, api::DescriptorSet* descset){
        return std::make_shared<DescriptorSet_T>(shared_from_this(),info,descset);
    };

    inline RenderPassMaker&& Device_T::createRenderPassMaker(const api::RenderPassCreateInfo& info, api::RenderPass* renderpass){
        return std::make_shared<RenderPass_T>(shared_from_this(),info,renderpass);
    };

    inline SBTHelper&& Device_T::createSBTHelper(api::Pipeline* rtPipeline) {
        return std::make_shared<SBTHelper_T>(shared_from_this(),rtPipeline);
    };

    inline DeviceMaker&& PhysicalDeviceHelper_T::createDeviceMaker(const api::DeviceCreateInfo& info, api::Device* device) {
        return std::make_shared<Device_T>(shared_from_this(),info,device);
    };


    // SHOULD BE IN HEADER
    template<class T = MemoryAllocator_T> inline std::shared_ptr<T>&& Device_T::createAllocator(const uintptr_t& info) {
        auto allocator = std::make_shared<T>(shared_from_this(),info); 
        this->allocator = std::dynamic_pointer_cast<MemoryAllocator_T>(allocator);
        return std::move(allocator);
    };

#ifdef VMA_ALLOCATOR
    
#endif
    
};