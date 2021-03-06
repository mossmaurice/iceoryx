// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef IOX_POSH_ROUDI_ROUDI_MULTI_PROCESS_HPP
#define IOX_POSH_ROUDI_ROUDI_MULTI_PROCESS_HPP

#include <cstdio>
#include <thread>

#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/roudi/introspection/mempool_introspection.hpp"
#include "iceoryx_posh/internal/roudi/roudi_process.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_manager.hpp"
#include "iceoryx_posh/roudi/roudi_app.hpp"
#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"
#include "iceoryx_utils/platform/file.hpp"
#include "iceoryx_utils/posix_wrapper/posix_access_rights.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
using namespace iox::units::duration_literals;

class RouDi
{
  public:
    // indicate whether the message queue thread will start directly or deferred
    // this is important for derived classes which may need to initialize their members before the thread starts
    enum class MQThreadStart
    {
        IMMEDIATE,
        DEFER_START
    };

    RouDi& operator=(const RouDi& other) = delete;
    RouDi(const RouDi& other) = delete;

    RouDi(RouDiMemoryInterface& roudiMemoryInteface,
          PortManager& portManager,
          const config::MonitoringMode f_monitoringMode = config::MonitoringMode::ON,
          const bool f_killProcessesInDestructor = true,
          const MQThreadStart mqThreadStart = MQThreadStart::IMMEDIATE,
          const version::CompatibilityCheckLevel compatibilityCheckLevel = version::CompatibilityCheckLevel::PATCH);

    virtual ~RouDi();

  protected:
    /// @brief Starts the roudi message queue thread
    /// Once this is done, applications can register and Roudi is fully operational.
    void startMQThread();

    /// @brief Stops threads and kills all process known to RouDi
    /// Called in d'tor
    ///
    /// @note Intentionally not virtual to be able to call it in derived class
    void shutdown();
    virtual void processMessage(const runtime::MqMessage& f_data,
                                const iox::runtime::MqMessageType& f_cmd,
                                const std::string& f_name);
    virtual void cyclicUpdateHook();
    void mqMessageErrorHandler();

    version::VersionInfo parseRegisterMessage(const runtime::MqMessage& f_message,
                                              int& f_pid,
                                              uid_t& f_userId,
                                              int64_t& f_transmissionTimestamp);

    /// @brief Handles the registration request from process
    /// @param [in] name of the process which wants to register at roudi; this is equal to the mqueue name
    /// @param [in] pid is the host system process id
    /// @param [in] user is the posix user id to which the process belongs
    /// @param [in] transmissionTimestamp is an ID for the application to check for the expected response
    /// @param [in] sessionId is an ID generated by RouDi to prevent sending outdated mqueue transmission
    /// @param [in] versionInfo Version of iceoryx used
    /// @return Returns if the process could be added successfully.
    bool registerProcess(const std::string& name,
                         int pid,
                         posix::PosixUser user,
                         int64_t transmissionTimestamp,
                         const uint64_t sessionId,
                         const version::VersionInfo& versionInfo);

    /// @brief Creates a unique ID which can be used to check outdated mqueue transmissions
    /// @return a unique, monotonic and consecutive increasing number
    static uint64_t getUniqueSessionIdForProcess();

  private:
    void mqThread();

    void processThread();

    cxx::GenericRAII m_unregisterRelativePtr{[] {}, [] { RelativePointer::unregisterAll(); }};
    bool m_killProcessesInDestructor;
    std::atomic_bool m_runThreads;

    const units::Duration m_messageQueueTimeout{100_ms};

  protected:
    RouDiMemoryInterface* m_roudiMemoryInterface{nullptr};
    /// @note destroy the memory right at the end of the dTor, since the memory is not needed anymore and we know that
    /// the lifetime of the MemoryBlocks must be at least as long as RouDi; this saves us from issues if the
    /// RouDiMemoryManager outlives some MemoryBlocks
    cxx::GenericRAII m_roudiMemoryManagerCleaner{[]() {}, [this]() { this->m_roudiMemoryInterface->destroyMemory(); }};
    PortManager* m_portManager;
    ProcessManager m_prcMgr;

  private:
    std::thread m_processManagementThread;
    std::thread m_processMQThread;

  protected:
    ProcessIntrospectionType m_processIntrospection;
    MemPoolIntrospectionType m_mempoolIntrospection;

  private:
    config::MonitoringMode m_monitoringMode{config::MonitoringMode::ON};
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ROUDI_MULTI_PROCESS_HPP
