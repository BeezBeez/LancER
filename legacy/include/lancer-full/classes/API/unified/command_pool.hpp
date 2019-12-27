#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/command_pool.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: full command buffer
            // TODO: add implementation into `.cpp`
            class command_pool { public: 
                command_pool(const command_pool& command_pool) : device_(command_pool), cmdpool_(command_pool) {};
                command_pool(                              const stu::command_pool& cmdpool_ = {}) : cmdpool_(cmdpool_) {};
                command_pool(const stu::device_t& device_, const stu::command_pool& cmdpool_ = {}) : cmdpool_(cmdpool_), device_(device_) {};

                // TODO: add assigment by core types and shared_ptr types
                command_pool& operator=(const command_pool &command_pool) { 
                    this->cmdpool_ = command_pool;
                    this->device_ = (stu::device_t&)command_pool;
                    return *this;
                };

                // TODO: move into `.cpp` file
                operator stu::command_pool&() { return cmdpool_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::command_pool&() const { return cmdpool_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::command_pool_t* operator->() { return &(*this->cmdpool_); };
                const api::factory::command_pool_t* operator->() const { return &(*this->cmdpool_); };
                api::factory::command_pool_t& operator*() { return (*this->cmdpool_); };
                const api::factory::command_pool_t& operator*() const { return (*this->cmdpool_); };

                // TODO: Command Pool Support
                //svt::core::handle_ref<command_pool,core::api::result_t> create(const command_pool_create_info& info = {});

                // 
                operator uintptr_t&() { return (*cmdpool_); };
                operator const uintptr_t&() const { return (*cmdpool_); };
                operator core::api::command_pool_t&() { return (*cmdpool_); };
                operator const core::api::command_pool_t&() const { return (*cmdpool_); };
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };
                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            // 
            protected: friend command_pool;
                stu::command_pool cmdpool_ = {};
                stu::device_t device_ = {};
            };

        };
    };
};