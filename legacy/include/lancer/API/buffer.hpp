#pragma once

#include <cstdint>
#include "../lib/core.hpp"
#include "../API/memory.hpp"


namespace lancer {
    // Vookoo-Like 
    class Buffer_T : public std::enable_shared_from_this<Buffer_T> {
        protected: 
            DeviceMaker device = {};
            MemoryAllocation allocation = {}; // least allocation, may be vector 
            api::BufferCreateInfo bfc = {};
            api::Buffer* lastbuf = nullptr;
            api::BufferView* lbv = nullptr;
            bool smartFree = false;

        public: 
            inline  Buffer_T(const DeviceMaker& device = {}, const api::BufferCreateInfo& bfc = api::BufferCreateInfo().setSharingMode(api::SharingMode::eExclusive), api::Buffer* lastbuf = nullptr) : lastbuf(lastbuf), bfc(bfc), device(device) {};
            inline ~Buffer_T(){
                if (this->smartFree) {
                    this->allocation->freeBuffer(shared_from_this());
                    this->allocation = {};
                };
            }; // Here will notification about free memory

            // Get original Vulkan link 
            inline api::Buffer& least() { return *lastbuf; };
            inline const api::Buffer& least() const { return *lastbuf; };
            inline operator api::Buffer&() { return *lastbuf; };
            inline operator const api::Buffer&() const { return *lastbuf; };

            // Editable BufferCreateInfo
            inline api::BufferCreateInfo& getCreateInfo() { return bfc; };
            inline const api::BufferCreateInfo& getCreateInfo() const { return bfc; };

            // Memory now can binded 
            inline BufferMaker linkAllocation(const MemoryAllocation& allocation = {}, const vk::BindBufferMemoryInfo& bindinf = {}) {
                if (!this->allocation) {
                    this->allocation = allocation;
                    if (lastbuf && (*lastbuf)) {
                        const auto mem = allocation->getMemory();
                        if (!!mem) { device->least().bindBufferMemory2(vk::BindBufferMemoryInfo(bindinf).setBuffer(*lastbuf).setMemory(mem).setMemoryOffset(allocation->getMemoryOffset())); };
                    }
                    else { // Correct Buffer Size by Memory
                        bfc.size = std::max(this->allocation->getMemorySize(), bfc.size);
                    };
                };
                return shared_from_this(); };

            // Mapping from Allocator
            inline uint8_t* getMapped(){
                return allocation->getMapped();
            };

            // 
            inline MemoryAllocation& getAllocation() {
                return this->allocation;
            };

            // 
            inline const MemoryAllocation& getAllocation() const {
                return this->allocation;
            };

            //  
            inline BufferMaker queueFamilyIndices(const std::vector<uint32_t>& indices = {}) {
                bfc.queueFamilyIndexCount = indices.size();
                bfc.pQueueFamilyIndices = indices.data();
                return shared_from_this(); };

            // Link Editable Buffer 
            inline BufferMaker link(api::Buffer* buf = nullptr) { lastbuf = buf; return shared_from_this(); };

            // 
            inline BufferMaker allocate(const MemoryAllocator& mem, const uintptr_t& ptx = 0u) {
                if (!this->allocation) {
                    if (lastbuf && (*lastbuf)) { // But... Buffer Already Created...
                        auto req2 = device->least().getBufferMemoryRequirements2(vk::BufferMemoryRequirementsInfo2().setBuffer(*lastbuf));
                        mem->allocateForRequirements(allocation = mem->createAllocation(), req2, ptx);
                        device->least().bindBufferMemory2(vk::BindBufferMemoryInfo().setBuffer(*lastbuf).setMemory(allocation->getMemory()).setMemoryOffset(allocation->getMemoryOffset()));
                    }
                    else {
                        mem->allocateForBuffer(lastbuf, allocation = mem->createAllocation(), bfc, ptx);
                    };
                };
                return shared_from_this(); };

            // 
            inline BufferMaker allocate(const uintptr_t& ptx = 0u) { return this->allocate(device->getAllocatorPtr(),ptx); };
            inline BufferMaker free() { 
                this->smartFree = true;
                this->allocation->smartFree();
                //this->allocation = {};
                return shared_from_this(); 
            };

            // 
            inline BufferMaker create(const vk::BindBufferMemoryInfo& bindinf = {}) { // 
                if (lastbuf && !(*lastbuf)) {
                    *lastbuf = device->least().createBuffer(bfc.setSize(!!allocation ? std::max(this->allocation->getMemorySize(), bfc.size) : bfc.size)); // Allocator Will Create Buffer Anyways
                };
                if (!!allocation) {
                    const auto mem = allocation->getMemory();
                    if (!!mem) { device->least().bindBufferMemory2(vk::BindBufferMemoryInfo(bindinf).setBuffer(*lastbuf).setMemory(mem).setMemoryOffset(allocation->getMemoryOffset())); };
                };
                return shared_from_this(); };

            // Create With Buffer View 
            inline BufferMaker createView(api::BufferView* bfv, const api::Format& format, const uintptr_t& offset = 0u, const size_t& size = 16u) {
                (*bfv = allocation->getDevice()->least().createBufferView(api::BufferViewCreateInfo{{}, *lastbuf, format, offset, size})); lbv = bfv;
                return shared_from_this(); };

            // Create With Region
            // TODO: another format 
            //inline BufferRegionU8Maker createRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset = 0u, const size_t& size = VK_WHOLE_SIZE);

            template<class T = uint8_t>
            inline std::shared_ptr<BufferRegion_T<T>> createRegion(api::DescriptorBufferInfo* reg = nullptr, const uintptr_t& offset = 0u, const size_t& size = VK_WHOLE_SIZE);
    };



    // TODO: unify functionality 

    template<class T>
    class BufferRegion_T : public std::enable_shared_from_this<BufferRegion_T<T>> {
        public:
            inline ~BufferRegion_T<T>(){};
            inline  BufferRegion_T<T>(const BufferMaker& buffer = {}, api::DescriptorBufferInfo* bufInfo = nullptr) : buffer(buffer), bufInfo(bufInfo) { if (!!buffer) { this->bufInfo->buffer = this->buffer->least(); }; };
            inline  BufferRegion_T<T>(const BufferMaker& buffer, api::DescriptorBufferInfo* bufInfo, const api::DeviceSize& offset, const api::DeviceSize& size = VK_WHOLE_SIZE) : buffer(buffer), bufInfo(bufInfo) {
                this->bufInfo->buffer = buffer->least();
                this->bufInfo->offset = offset;
                this->bufInfo->range = size * sizeof(T);
            };

            // UPDATE 04.12.2019 - Remake Buffer Region 
            // WARNING - MAY REQUIRED RE-LINK `vk::Buffer*`
            inline BufferRegion_T<T>(const std::shared_ptr<BufferRegion_T<T>>& region, api::DescriptorBufferInfo* bufInfo = nullptr) : buffer(region->least()), bufInfo(bufInfo), mapped(mapped) {

            };

            inline T* const& map() { mapped = (T*)((uint8_t*)buffer->getMapped() + bufInfo->offset); return mapped; };
            inline T* const& data() { this->map(); return mapped; };
            inline const T*& data() const { return mapped; };

            // 
            inline size_t size() const { return size_t(bufInfo->range / sizeof(T)); };
            inline const api::DeviceSize& range() const { return bufInfo->range; };

            // at function 
            inline const T& at(const uintptr_t& i) const { return mapped[i]; };
            inline T& at(const uintptr_t& i) { return mapped[i]; };

            // array operator 
            inline const T& operator [] (const uintptr_t& i) const { return at(i); };
            inline T& operator [] (const uintptr_t& i) { return at(i); };

            // begin ptr
            inline const T*& begin() const { return data(); };
            inline T* const& begin() { return map(); };

            // end ptr
            inline const T*& end() const { return &at(size() - 1ul); };
            inline T* end() { return &at(size() - 1ul); };

            // return only pointer
            inline api::DescriptorBufferInfo* getRegionInfo() const { return bufInfo; };

            // SHARED_PTR DOESN'T WORKING ON TEMPLATES
            //std::shared_ptr<BufferRegion_T<T>> setRegionInfo(api::DescriptorBufferInfo* bufInfo = nullptr) {
            inline BufferRegion_T<T>* setRegionInfo(api::DescriptorBufferInfo* bufInfo = nullptr) {
                this->bufInfo = bufInfo; return this;
                //return shared_from_this();
            };

            // 
            //std::shared_ptr<BufferRegion_T<T>> 
            inline BufferRegion_T<T>* setBufferMaker(const BufferMaker& bufferMaker = {}) {
                this->buffer = bufferMaker; return this;
                //return shared_from_this();
            };

            // 
            inline BufferMaker& least() { return buffer; };
            inline const BufferMaker& least() const { return buffer; };

            // 
            const api::Buffer& handle() const { return (this->bufInfo->buffer = (this->bufInfo->buffer ? this->bufInfo->buffer : this->buffer->least())); };
            api::Buffer& handle() { return (this->bufInfo->buffer = (this->bufInfo->buffer ? this->bufInfo->buffer : this->buffer->least())); };

            // 
            inline operator const api::DescriptorBufferInfo&() const { return *bufInfo; };
            inline operator const api::Buffer&() const { return this->handle(); };
            inline const api::DeviceSize& offset() const { return bufInfo->offset; };

        protected:
            T* mapped = {};
            BufferMaker buffer = {};
            api::DescriptorBufferInfo* bufInfo = {};
    };

    // Geometry RTX Required
    template<class T>
    inline std::shared_ptr<BufferRegion_T<T>> Buffer_T::createRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset, const size_t& size) {
        (*reg = api::DescriptorBufferInfo{ this->least(), offset, size * sizeof(T) }); //return shared_from_this(); 
        return std::make_shared<BufferRegion_T<T>>(shared_from_this(), reg, offset, size);
    };

    // defer implement 
    // TODO: another format of BufferRegion
    //inline BufferRegionU8Maker Buffer_T::createRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset, const size_t& size) {
    //    (*reg = api::DescriptorBufferInfo{ *lastbuf, offset, size }); //return shared_from_this(); 
    //    return std::make_shared<BufferRegionU8_T>(shared_from_this(), reg, offset, size);
    //};

    // Wrap as Vector (like STD)
    template<class T = uint8_t>
    class Vector {
        public:
            friend Vector<T>;
            inline ~Vector<T>() {};
            //inline  Vector() {}; 
            inline Vector<T>(const BufferMaker& buffer = {}, api::DescriptorBufferInfo* bufInfo = nullptr) { region = std::make_shared<BufferRegion_T<T>>(buffer, bufInfo); };
            inline Vector<T>(const BufferMaker& buffer, api::DescriptorBufferInfo* bufInfo, const api::DeviceSize& offset, const api::DeviceSize& size = VK_WHOLE_SIZE) { region = std::make_shared<BufferRegion_T<T>>(buffer, bufInfo, size, offset); };
            inline Vector<T>(const Vector<T>& vector) : region(vector.region) {};
            inline Vector<T>(const std::shared_ptr<BufferRegion_T<T>>& region) : region(region) {};

            // can be assigned
            inline Vector<T>& operator=(const Vector<T>& another) { this->region = another.region; return *this; };
            inline Vector<T>& operator=(const std::shared_ptr<BufferRegion_T<T>>& region) { this->region = region; return *this; };

            // map through
            inline T* const& map() { return region->map(); };
            inline void unmap() { return region->unmap(); };

            inline T* const& data() { return region->data(); };
            inline const T*& data() const { return region->data(); };

            // sizing 
            inline size_t size() const { return region->size(); };
            inline const api::DeviceSize& range() const { return region->range(); };

            // at function 
            inline const T& at(const uintptr_t& i) const { return region->at(i); };
            inline T& at(const uintptr_t& i) { return region->at(i); };

            // array operator 
            inline const T& operator [] (const uintptr_t& i) const { at(i); };
            inline T& operator [] (const uintptr_t& i) { return at(i); };

            // begin ptr
            inline const T*& begin() const { region->begin(); };
            inline T* const& begin() { return region->begin(); };

            // end ptr
            inline const T* end() const { return region->end(); };
            inline T* end() { return region->end(); };

            // 
            inline operator const api::DescriptorBufferInfo&() const { return *region; };
            inline operator const api::Buffer&() const { return *region; };
            inline const api::DeviceSize& offset() const { return region->offset(); };


            // Update from 03.12.2019 (make able to use methods as pointer)
            inline operator BufferMaker& () { return region->least(); };
            inline operator const BufferMaker& () const { return region->least(); };

            // 
            inline BufferMaker& least() { return region->least(); };
            inline const BufferMaker& least() const { return region->least(); };

            // 
            inline BufferRegion_T<T>* operator->() { return &(*region); };
            inline const BufferRegion_T<T>* operator->() const { return &(*region); };

            // 
            inline operator BufferRegion_T<T>& () { return *region; };
            inline operator const BufferRegion_T<T>& () const { return *region; };

            // 
            inline operator std::shared_ptr<BufferRegion_T<T>>& () { return region; };
            inline operator const std::shared_ptr<BufferRegion_T<T>>& () const { return region; };

        protected:
            std::shared_ptr<BufferRegion_T<T>> region = {};
    };

};