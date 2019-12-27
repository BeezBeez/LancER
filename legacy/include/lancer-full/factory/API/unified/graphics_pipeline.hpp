#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class graphics_pipeline_t : public std::enable_shared_from_this<graphics_pipeline_t> { public: 
                core::api::graphics_pipeline_t pipeline_ = API_NULL_HANDLE;

                graphics_pipeline_t() {};
                graphics_pipeline_t(const graphics_pipeline_t& graphics_pipeline_t) : pipeline_(graphics_pipeline_t) {};
                graphics_pipeline_t(core::api::graphics_pipeline_t pipeline_ = API_NULL_HANDLE) : pipeline_(pipeline_) {};
                graphics_pipeline_t& operator=(const graphics_pipeline_t& graphics_pipeline_t) { this->pipeline_ = graphics_pipeline_t; return *this; };
                graphics_pipeline_t& operator=(const std::shared_ptr<graphics_pipeline_t>& pipeline_) { this->pipeline_ = *pipeline_; return *this; };
                graphics_pipeline_t& operator=(const core::api::graphics_pipeline_t& pipeline_) { this->pipeline_ = pipeline_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(pipeline_); };
                operator const uintptr_t&() const { return (uintptr_t&)(pipeline_); };
                operator core::api::graphics_pipeline_t&() { return pipeline_; };
                operator const core::api::graphics_pipeline_t&() const { return pipeline_; };

                core::api::graphics_pipeline_t* operator->() { return &(this->pipeline_); };
                const core::api::graphics_pipeline_t* operator->() const { return &(this->pipeline_); };
                core::api::graphics_pipeline_t& operator*() { return (this->pipeline_); };
                const core::api::graphics_pipeline_t& operator*() const { return (this->pipeline_); };
            };
        };
    };
};