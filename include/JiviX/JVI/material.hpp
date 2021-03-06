#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace jvi {

    // 
#pragma pack(push, 1)
    struct MaterialUnit {
        glm::vec4 diffuse = { 1.f,1.f,1.f,1.f };
        glm::vec4 specular = { 0.f,0.f,0.f,1.f };
        glm::vec4 normals = { 0.5f,0.5f,1.f,1.f };
        glm::vec4 emission = { 0.0f,0.0f,0.f,1.f };

        int diffuseTexture = -1;
        int specularTexture = -1;
        int normalsTexture = -1;
        int emissionTexture = -1;

        glm::uvec4 udata = glm::uvec4(0u);
    };
#pragma pack(pop)

    // WIP Materials
    class Material : public std::enable_shared_from_this<Material> { public: // 
        Material() {};
        Material(const vkt::uni_ptr<Context>& context) : context(context) { this->construct(); };
        //Material(Context* context) { this->context = vkt::uni_ptr<Context>(context); this->construct(); };
        ~Material() {};

        // 
        virtual vkt::uni_ptr<Material> sharedPtr() { return shared_from_this(); };
        virtual vkt::uni_ptr<const Material> sharedPtr() const { return shared_from_this(); };

        // 
        virtual uPTR(Material) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // 
            this->rawMaterials = vkt::Vector<vkh::VsGeometryInstance>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(MaterialUnit) * 64u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuMaterials = vkt::Vector<vkh::VsGeometryInstance>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(MaterialUnit) * 64u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);
            return uTHIS;
        };

        // 
        virtual uPTR(Material) setContext(const std::shared_ptr<Context>& context) {
            this->context = context;
            this->descriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->materialDescriptorSetLayout,this->thread->getDescriptorPool());
            return uTHIS;
        };

        // 
        virtual uPTR(Material) setRawMaterials(const vkt::Vector<MaterialUnit>& rawMaterials = {}, const vk::DeviceSize& materialCounter = 0u) {
            this->rawMaterials = rawMaterials; this->materialCounter = materialCounter;
            return uTHIS;
        };

        // 
        virtual uPTR(Material) setGpuMaterials(const vkt::Vector<MaterialUnit>& gpuMaterials = {}) {
            this->gpuMaterials = gpuMaterials;
            return uTHIS;
        };

        // 
        virtual vk::DeviceSize pushMaterial(const MaterialUnit& material = {}) {
            const auto materialID = materialCounter++;
            this->rawMaterials[materialID] = material;
            return materialID;
        };

        // 
        virtual uPTR(Material) resetMaterials() {
            materialCounter = 0u;
            return uTHIS;
        };

        // 
        virtual uPTR(Material) resetSampledImages() {
            this->sampledImages.resize(0u);
            return uTHIS;
        };

        // 
        virtual uPTR(Material) pushSampledImage(const vkh::VkDescriptorImageInfo& info = {}) {
            this->sampledImages.push_back(info);
            return uTHIS;
        };

        // 
        virtual uPTR(Material) copyBuffers(const vk::CommandBuffer& buildCommand = {}) {
            buildCommand.copyBuffer(this->rawMaterials, this->gpuMaterials, { vk::BufferCopy{ this->rawMaterials.offset(), this->gpuMaterials.offset(), this->gpuMaterials.range() } });
            return uTHIS;
        };

        // 
        virtual uPTR(Material) createDescriptorSet() {
            this->descriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(context->materialDescriptorSetLayout, driver->descriptorPool);

            if (sampledImages.size() > 0u) { // Setup Textures
                vkh::VsDescriptorHandle<VkDescriptorImageInfo> imagesHandle = this->descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .descriptorCount = uint32_t(sampledImages.size()),
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                });
                memcpy(&imagesHandle.offset<VkDescriptorImageInfo>(), sampledImages.data(), sampledImages.size() * sizeof(VkDescriptorImageInfo));
                //for (uint32_t i = 0u; i < sampledImages.size(); i++) { imagesHandle.offset<VkDescriptorImageInfo>(i) = sampledImages[i]; };
            };

            // 
            this->descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 1u,
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            }).offset<vkh::VkDescriptorBufferInfo>() = (vkh::VkDescriptorBufferInfo&)this->gpuMaterials;

            // Reprojection WILL NOT write own depth... 
            this->context->descriptorSets[4] = this->descriptorSet = driver->getDevice().allocateDescriptorSets(this->descriptorSetInfo)[0];
            this->driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet, vkh::VkWriteDescriptorSet>(this->descriptorSetInfo.setDescriptorSet(this->descriptorSet)), {});
            return uTHIS;
        };

    protected: // 
        std::vector<vkh::VkDescriptorImageInfo> sampledImages = {};

        // 
        vkt::Vector<MaterialUnit> rawMaterials = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<MaterialUnit> gpuMaterials = {};

        // 
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetInfo = {};
        vk::DescriptorSet descriptorSet = {};

        // 
        vk::DeviceSize materialCounter = 0u;
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
    };
};
