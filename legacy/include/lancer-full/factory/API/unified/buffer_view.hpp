#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class buffer_view_t : public std::enable_shared_from_this<buffer_view_t> { public: 
                core::api::buffer_view_t buffer_view_ = API_NULL_HANDLE;
                api::format format_ = api::format::t_undefined;

                buffer_view_t() {};
                buffer_view_t(const buffer_view_t& buffer_view) : buffer_view_(buffer_view) {};
                buffer_view_t(const core::api::buffer_view_t& buffer_view_) : buffer_view_(buffer_view_) {};
                buffer_view_t& operator=(const buffer_view_t& buffer_view) { this->buffer_view_ = buffer_view; return *this; };
                buffer_view_t& operator=(const std::shared_ptr<buffer_view_t>& buffer_view) { this->buffer_view_ = *buffer_view; return *this; };
                buffer_view_t& operator=(const core::api::buffer_view_t& buffer_view_){ this->buffer_view_ = buffer_view_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(buffer_view_); };
                operator const uintptr_t&() const { return (uintptr_t&)(buffer_view_); };
                operator core::api::buffer_view_t&() { return buffer_view_; };
                operator const core::api::buffer_view_t&() const { return buffer_view_; };

                core::api::buffer_view_t* operator->() { return &(this->buffer_view_); };
                const core::api::buffer_view_t* operator->() const { return &(this->buffer_view_); };
                core::api::buffer_view_t& operator*() { return (this->buffer_view_); };
                const core::api::buffer_view_t& operator*() const { return (this->buffer_view_); };
            };
        };
    };
};