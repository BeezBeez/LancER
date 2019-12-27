#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/image.hpp"

namespace svt {
    namespace api {
        namespace classes {
            // TODO: image layout support and image layout barrier
            class image { public: 
                image(                              const stu::image& image_ = {}) : image_(image_) {};
                image(const stu::device_t& device_, const stu::image& image_ = {}) : image_(image_), device_(device_) {};
                image(const image& image = {}) : allocator_(image.allocator_), device_(image), image_(image) {};

                // TODO: merge into `.cpp`
                operator stu::image&() { return image_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator stu::allocator&() { return allocator_; };
                operator const stu::image&() const { return image_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };
                operator const stu::allocator&() const { return allocator_; };

                // Currently Aggregator
                // TODO: create image view
                svt::core::handle_ref<image, core::api::result_t> create(const stu::allocator& allocator_ = {}, const image_create_info& info = {}, const uintptr_t& info_ptr = 0u, const image_layout& initial_layout = image_layout::t_undefined);
                svt::core::handle_ref<image_view, core::api::result_t> create_image_view(const image_view_create_info& info = {});

                // TODO: move into `.cpp` file
                // TODO: add assigment by core types and shared_ptr types
                image& operator=(const image &image) { 
                    this->image_ = image;
                    this->allocator_ = image.allocator_;
                    this->device_ = (stu::device_t&)image;
                    return *this;
                };

                // TODO: move into `.cpp` file
                api::factory::image_t* operator->() { return &(*this->image_); };
                const api::factory::image_t* operator->() const { return &(*this->image_); };
                api::factory::image_t& operator*() { return (*this->image_); };
                const api::factory::image_t& operator*() const { return (*this->image_); };

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::image_t _get_image_t();

                // 
                operator uintptr_t&() { return (*image_); };
                operator const uintptr_t&() const { return (*image_); };
                operator core::api::image_t&() { return (*image_); };
                operator const core::api::image_t&() const { return (*image_); };
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };
                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };


            protected: friend image; friend allocator;
                stu::image image_ = {};
                //stu::allocation allocation_ = {};
                stu::allocator allocator_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};