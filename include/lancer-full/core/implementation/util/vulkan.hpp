#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>

// Currently Windows Only Supported
#if (defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER_) || defined(__MINGW64__)) 
#include <windows.h> // Fix HMODULE Type Error
#endif

// Default Backend
#if !defined(USE_D3D12) && !defined(USE_VULKAN)
#define USE_VULKAN
#endif

// 
#include <misc/args.hxx>
#include <misc/half.hpp>
#include <misc/pcg_random.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

// 
#include "./core/implementation/core/vulkan_hpp.inl"


namespace svt {
    namespace util {
        namespace api {
#ifdef USE_VULKAN
            template <typename T>
            static inline auto sgn(const T& val) { return (T(0) < val) - (val < T(0)); }

            template<class T = uint64_t>
            static inline T tiled(const T& sz, const T& gmaxtile) {
                // return (int32_t)ceil((double)sz / (double)gmaxtile);
                return sz <= 0 ? 0 : (sz / gmaxtile + sgn(sz % gmaxtile));
            }

            template <class T>
            static inline auto strided(const size_t& sizeo) { return sizeof(T) * sizeo; }

            // Read binary (for SPIR-V)
            // Updated 03.12.2019 (add No Data error)
            static inline auto readBinary(const std::string& filePath ) {
                std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
                std::vector<uint32_t> data = {};
                if (file.is_open()) {
                    std::streampos size = file.tellg();
                    data.resize(tiled(size_t(size), sizeof(uint32_t)));
                    file.seekg(0, std::ios::beg);
                    file.read((char *)data.data(), size);
                    file.close();
                }
                else {
                    std::cerr << "Failure to open " + filePath << std::endl;
                };
                if (data.size() < 1u) { std::cerr << "NO DATA " + filePath << std::endl; };
                return data;
            };

            // read source (unused)
            static inline auto readSource(const std::string& filePath, bool lineDirective = false ) {
                std::string content = "";
                std::ifstream fileStream(filePath, std::ios::in);
                if (!fileStream.is_open()) {
                    std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl; return content;
                }
                std::string line = "";
                while (!fileStream.eof()) {
                    std::getline(fileStream, line);
                    if (lineDirective || line.find("#line") == std::string::npos) content.append(line + "\n");
                }
                fileStream.close();
                return content;
            };

            static inline auto makeShaderModuleInfo(const std::vector<uint32_t>& code) {
                auto smi = vk::ShaderModuleCreateInfo{};
                smi.pCode = (uint32_t *)code.data();
                smi.codeSize = code.size()*4;
                smi.flags = {};
                return smi;
            };

            // create shader module
            static inline auto& createShaderModuleIntrusive(const vk::Device& device, const std::vector<uint32_t>& code, vk::ShaderModule& hndl) {
                return (hndl = device.createShaderModule(makeShaderModuleInfo(code)));
            };

            static inline auto createShaderModule(const vk::Device& device, const std::vector<uint32_t>& code) {
                auto sm = vk::ShaderModule{}; return std::move(createShaderModuleIntrusive(device, code, sm));
            };

            struct FixConstruction {
                vk::PipelineShaderStageCreateInfo spi = {};
                vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT sgmp = {};

                operator vk::PipelineShaderStageCreateInfo& () { return spi; };
                operator const vk::PipelineShaderStageCreateInfo& () const { return spi; };
                operator vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& () { return sgmp; };
                operator const vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& () const { return sgmp; };
            };

            // create shader module
            static inline auto makePipelineStageInfo(const vk::Device& device, const std::vector<uint32_t>& code, const char * entry = "main", vk::ShaderStageFlagBits stage = vk::ShaderStageFlagBits::eCompute) {
                vk::PipelineShaderStageCreateInfo spi = {};
                createShaderModuleIntrusive(device, code, spi.module);
                spi.pName = entry;
                spi.stage = stage;
                spi.pSpecializationInfo = {};
                return std::move(spi);
            };

            // create shader module
            static inline auto makeComputePipelineStageInfo(const vk::Device& device, const std::vector<uint32_t>& code, const char * entry = "main", const uint32_t& subgroupSize = 0u) {
                auto f = FixConstruction{};

                f.spi = vk::PipelineShaderStageCreateInfo{};;
                f.spi.flags = vk::PipelineShaderStageCreateFlagBits::eRequireFullSubgroupsEXT;
                createShaderModuleIntrusive(device, code, f.spi.module);
                f.spi.pName = entry;
                f.spi.stage = vk::ShaderStageFlagBits::eCompute;
                f.spi.pSpecializationInfo = {};

                f.sgmp = vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT{};
                f.sgmp.requiredSubgroupSize = subgroupSize;
                if (subgroupSize) f.spi.pNext = &f.sgmp;

                return std::move(f);
            };

            // create compute pipelines
            static inline auto createCompute(const vk::Device& device, const FixConstruction& spi, const vk::PipelineLayout& layout, const vk::PipelineCache& cache = {}, const uint32_t& subgroupSize = 0u) {
                auto cmpi = vk::ComputePipelineCreateInfo{};
                cmpi.flags = {};
                cmpi.layout = layout;
                cmpi.stage = spi;
                cmpi.basePipelineIndex = -1;
                return device.createComputePipeline(cache, cmpi);
            };

            // create compute pipelines
            static inline auto createCompute(const vk::Device& device, const std::vector<uint32_t>& code, const vk::PipelineLayout& layout, const vk::PipelineCache& cache = {}, const uint32_t& subgroupSize = 0u) {
                auto f = makeComputePipelineStageInfo(device, code, "main", subgroupSize);
                if (subgroupSize) f.spi.pNext = &f.sgmp; // fix link
                return createCompute(device, f, layout, cache, subgroupSize);
            };

            // create compute pipelines
            static inline auto createCompute(const vk::Device& device, const std::string& path, const vk::PipelineLayout& layout, const vk::PipelineCache& cache = {}, const uint32_t& subgroupSize = 0u) {
                return createCompute(device, readBinary(path), layout, cache, subgroupSize);
            };

            // general command buffer pipeline barrier
            static inline void commandBarrier(const vk::CommandBuffer& cmdBuffer) {
                vk::MemoryBarrier memoryBarrier = {};
                memoryBarrier.srcAccessMask = (vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eMemoryWrite);
                memoryBarrier.dstAccessMask = (vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eUniformRead);
                const auto srcStageMask = vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eAllGraphics | vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eRayTracingShaderNV | vk::PipelineStageFlagBits::eAccelerationStructureBuildNV;
                const auto dstStageMask = srcStageMask;
                cmdBuffer.pipelineBarrier(srcStageMask, dstStageMask, {}, { memoryBarrier }, {}, {});
            };

            // create secondary command buffers for batching compute invocations
            static inline auto createCommandBuffer(const vk::Device& device, const vk::CommandPool& cmdPool, bool secondary = true, bool once = true) {
                vk::CommandBuffer cmdBuffer = {};

                vk::CommandBufferAllocateInfo cmdi = vk::CommandBufferAllocateInfo{};
                cmdi.commandPool = cmdPool;
                cmdi.level = (secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary);
                cmdi.commandBufferCount = 1;
                cmdBuffer = (device.allocateCommandBuffers(cmdi))[0];

                vk::CommandBufferInheritanceInfo inhi = vk::CommandBufferInheritanceInfo{};
                inhi.pipelineStatistics = vk::QueryPipelineStatisticFlagBits::eComputeShaderInvocations;

                vk::CommandBufferBeginInfo bgi = vk::CommandBufferBeginInfo{};
                bgi.flags = {};
                bgi.flags = once ? vk::CommandBufferUsageFlagBits::eOneTimeSubmit : vk::CommandBufferUsageFlagBits::eSimultaneousUse;
                bgi.pInheritanceInfo = secondary ? &inhi : nullptr;
                cmdBuffer.begin(bgi);

                return cmdBuffer;
            };

            // add dispatch in command buffer (with default pipeline barrier)
            static inline vk::Result cmdDispatch(const vk::CommandBuffer& cmd, const vk::Pipeline& pipeline, uint32_t x = 1, uint32_t y = 1, uint32_t z = 1, bool barrier = true) {
                cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
                cmd.dispatch(x, y, z);
                if (barrier) {
                    commandBarrier(cmd); // put shader barrier
                }
                return vk::Result::eSuccess;
            };

            // low level copy command between (prefer for host and device)
            static inline vk::Result cmdCopyBufferL(const vk::CommandBuffer& cmd, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, const std::vector<vk::BufferCopy>& regions, std::function<void(vk::CommandBuffer)> barrierFn = commandBarrier) {
                if (srcBuffer && dstBuffer && regions.size() > 0) {
                    vk::CommandBuffer(cmd).copyBuffer(srcBuffer, dstBuffer, regions); barrierFn(cmd); // put copy barrier
                };
                return vk::Result::eSuccess;
            };


            // short data set with command buffer (alike push constant)
            template<class T>
            static inline vk::Result cmdUpdateBuffer(const vk::CommandBuffer& cmd, const vk::Buffer& dstBuffer, const vk::DeviceSize& offset, const std::vector<T>& data) {
                vk::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, data);
                //updateCommandBarrier(cmd);
                return vk::Result::eSuccess;
            };

            // short data set with command buffer (alike push constant)
            template<class T>
            static inline vk::Result cmdUpdateBuffer(const vk::CommandBuffer& cmd, const vk::Buffer& dstBuffer, const vk::DeviceSize& offset, const vk::DeviceSize& size, const T* data = nullptr) {
                vk::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, size, data);
                //updateCommandBarrier(cmd);
                return vk::Result::eSuccess;
            };


            // template function for fill buffer by constant value
            // use for create repeat variant
            template<uint32_t Rv>
            static inline vk::Result cmdFillBuffer(const vk::CommandBuffer& cmd, const vk::Buffer& dstBuffer, const vk::DeviceSize& size = 0xFFFFFFFF, const vk::DeviceSize& offset = 0) {
                vk::CommandBuffer(cmd).fillBuffer(vk::Buffer(dstBuffer), offset, size, Rv);
                //updateCommandBarrier(cmd);
                return vk::Result::eSuccess;
            };


            // submit command (with async wait)
            // TODO: return vk::Result 
            static inline void submitCmd(const vk::Device& device, const vk::Queue& queue, const std::vector<vk::CommandBuffer>& cmds, vk::SubmitInfo smbi = {}) {
                // no commands 
                if (cmds.size() <= 0) return;
                smbi.commandBufferCount = cmds.size();
                smbi.pCommandBuffers = (vk::CommandBuffer*)cmds.data();

                vk::Fence fence = {}; vk::FenceCreateInfo fin = vk::FenceCreateInfo{};
                device.createFence(&fin, nullptr, &fence);
                queue.submit(smbi, fence);
                device.waitForFences(1, &fence, true, INT64_MAX);
                device.destroyFence(fence, nullptr);
            };

            // once submit command buffer
            static inline void submitOnce(const vk::Device& device, const vk::Queue& queue, const vk::CommandPool& cmdPool, const std::function<void(vk::CommandBuffer&)>& cmdFn = {}, const vk::SubmitInfo& smbi = {}) {
                auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); cmdBuf.end();
                submitCmd(device, queue, { cmdBuf }); device.freeCommandBuffers(cmdPool, 1, &cmdBuf); // free that command buffer
            };

            // submit command (with async wait)
            static inline void submitCmdAsync(const vk::Device& device, const vk::Queue& queue, const std::vector<vk::CommandBuffer>& cmds, const std::function<void()>& asyncCallback = {}, vk::SubmitInfo smbi = {}) {
                // no commands 
                if (cmds.size() <= 0) return;
                smbi.commandBufferCount = cmds.size();
                smbi.pCommandBuffers = (vk::CommandBuffer*)cmds.data();

                vk::Fence fence = {}; vk::FenceCreateInfo fin = vk::FenceCreateInfo{};
                device.createFence(&fin, nullptr, &fence);
                queue.submit(1, (const vk::SubmitInfo*)& smbi, fence);
                device.waitForFences(1, &fence, true, INT64_MAX);
                device.destroyFence(fence, nullptr);
                if (asyncCallback) asyncCallback();
            };

            // once submit command buffer
            static inline void submitOnceAsync(const vk::Device& device, const vk::Queue& queue, const vk::CommandPool& cmdPool, const std::function<void(vk::CommandBuffer&)>& cmdFn = {}, const std::function<void(vk::CommandBuffer)>& asyncCallback = {}, const vk::SubmitInfo& smbi = {}) {
                auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); cmdBuf.end();
                submitCmdAsync(device, queue, { cmdBuf }, [&]() {
                    asyncCallback(cmdBuf); // call async callback
                    device.freeCommandBuffers(cmdPool, 1, &cmdBuf); // free that command buffer
                });
            };

            template <class T> static inline auto makeVector(const T * ptr, const size_t& size = 1) { std::vector<T>v(size); memcpy(v.data(), ptr, strided<T>(size)); return v; };

            // create fence function
            static inline vk::Fence createFence(const vk::Device& device, const bool& signaled = true) {
                vk::FenceCreateInfo info = {};
                if (signaled) info.setFlags(vk::FenceCreateFlagBits::eSignaled);
                return vk::Device(device).createFence(info);
            };

            // TODO: native image barrier in library 
            struct ImageBarrierInfo {
                vk::Image image = {};
                vk::ImageLayout targetLayout = vk::ImageLayout::eGeneral;
                vk::ImageLayout originLayout = vk::ImageLayout::eUndefined;
                vk::ImageSubresourceRange subresourceRange = { {}, 0u, 1u, 0u, 1u };
            };

            vk::Result imageBarrier(const vk::CommandBuffer& cmd = {}, const ImageBarrierInfo& info = {}) {
                vk::Result result = vk::Result::eSuccess; // planned to complete
                if (info.originLayout == info.targetLayout) return result; // no need transfering more

                vk::ImageMemoryBarrier imageMemoryBarriers = {};
                imageMemoryBarriers.srcQueueFamilyIndex = ~0U;
                imageMemoryBarriers.dstQueueFamilyIndex = ~0U;
                imageMemoryBarriers.oldLayout = info.originLayout;
                imageMemoryBarriers.newLayout = info.targetLayout;
                imageMemoryBarriers.subresourceRange = info.subresourceRange;
                imageMemoryBarriers.image = info.image;

                // Put barrier on top
                const auto  srcStageMask = vk::PipelineStageFlags{ vk::PipelineStageFlagBits::eBottomOfPipe };
                const auto  dstStageMask = vk::PipelineStageFlags{ vk::PipelineStageFlagBits::eTopOfPipe };
                const auto  dependencyFlags = vk::DependencyFlags{};
                      auto  srcMask = vk::AccessFlags{}, dstMask = vk::AccessFlags{};

                typedef vk::ImageLayout il;
                typedef vk::AccessFlagBits afb;

                // Is it me, or are these the same?
                switch (info.originLayout) {
                    case il::eUndefined: break;
                    case il::eGeneral: srcMask = afb::eTransferWrite; break;
                    case il::eColorAttachmentOptimal: srcMask = afb::eColorAttachmentWrite; break;
                    case il::eDepthStencilAttachmentOptimal: srcMask = afb::eDepthStencilAttachmentWrite; break;
                    case il::eDepthStencilReadOnlyOptimal: srcMask = afb::eDepthStencilAttachmentRead; break;
                    case il::eShaderReadOnlyOptimal: srcMask = afb::eShaderRead; break;
                    case il::eTransferSrcOptimal: srcMask = afb::eTransferRead; break;
                    case il::eTransferDstOptimal: srcMask = afb::eTransferWrite; break;
                    case il::ePreinitialized: srcMask = afb::eTransferWrite | afb::eHostWrite; break;
                    case il::ePresentSrcKHR: srcMask = afb::eMemoryRead; break;
                };

                switch (info.targetLayout) {
                    case il::eUndefined: break;
                    case il::eGeneral: dstMask = afb::eTransferWrite; break;
                    case il::eColorAttachmentOptimal: dstMask = afb::eColorAttachmentWrite; break;
                    case il::eDepthStencilAttachmentOptimal: dstMask = afb::eDepthStencilAttachmentWrite; break;
                    case il::eDepthStencilReadOnlyOptimal: dstMask = afb::eDepthStencilAttachmentRead; break;
                    case il::eShaderReadOnlyOptimal: dstMask = afb::eShaderRead; break;
                    case il::eTransferSrcOptimal: dstMask = afb::eTransferRead; break;
                    case il::eTransferDstOptimal: dstMask = afb::eTransferWrite; break;
                    case il::ePreinitialized: dstMask = afb::eTransferWrite; break;
                    case il::ePresentSrcKHR: dstMask = afb::eMemoryRead; break;
                };

                // assign access masks
                imageMemoryBarriers.srcAccessMask = srcMask;
                imageMemoryBarriers.dstAccessMask = dstMask;

                // barrier
                cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, std::array<vk::ImageMemoryBarrier, 1>{imageMemoryBarriers});
                return result;
            };

#endif
        };
    };
};