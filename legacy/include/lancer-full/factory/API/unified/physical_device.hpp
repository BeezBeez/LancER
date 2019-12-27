#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class physical_device_t : public std::enable_shared_from_this<physical_device_t> { public: 
                core::api::physical_device_t physical_device_ = API_NULL_HANDLE;
                core::api::physical_device_properties_t properties_ = {};
                core::api::physical_device_features_t features_ = {};

                physical_device_t() {};
                physical_device_t(const physical_device_t& physical_device) : physical_device_(physical_device) {};
                physical_device_t(const core::api::physical_device_t& physical_device_) : physical_device_(physical_device_) {};
                physical_device_t& operator=(const physical_device_t& physical_device) { this->physical_device_ = physical_device; return *this; };
                physical_device_t& operator=(const std::shared_ptr<physical_device_t>& physical_device) { this->physical_device_ = *physical_device; return *this; };
                physical_device_t& operator=(const core::api::physical_device_t& physical_device_) { this->physical_device_ = physical_device_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(physical_device_); };
                operator const uintptr_t&() const { return (uintptr_t&)(physical_device_); };
                operator core::api::physical_device_t&() { return physical_device_; };
                operator const core::api::physical_device_t&() const { return physical_device_; };

                core::api::physical_device_t* operator->() { return &(this->physical_device_); };
                const core::api::physical_device_t* operator->() const { return &(this->physical_device_); };
                core::api::physical_device_t& operator*() { return (this->physical_device_); };
                const core::api::physical_device_t& operator*() const { return (this->physical_device_); };

                // TODO: MOVE TO `.CPP` DUE VULKAN API
                surface_capabilities get_surface_capabilities(const core::api::surface_t& surface) const;
                std::vector<present_mode> get_present_modes(const core::api::surface_t& surface) const;
                format_properties get_format_properties(const format& format) const;
                surface_format get_surface_formats(const core::api::surface_t& surface) const;
            };

        };
    };
};