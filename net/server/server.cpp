// Copyright (c) 2016 Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License file in the project root for license information.
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>  // NOLINT(build/c++11)
#include <csignal>
#include <cstddef>
#include <cstring>
#include <deque>
#include <memory>
#include <thread>  // NOLINT(build/c++11)
#include <vector>
////
#include <cassert>
////

#include <net/common/utils.hpp>
#include <net/server/planner.hpp>
#include <net/server/server.hpp>
#include <net/server/worker.hpp>

struct Server::ServerImpl {
    ServerImpl(unsigned int port = kDefaultServerPort,
               std::string_view working_directory = kDefaultWorkingDirectory,
               size_t max_workers_count = kDefaultMaxWorkersCount) :
            port_(port),
            working_directory_(working_directory),
            max_workers_count_(max_workers_count) {}

    ErrorStatus Start();

 private:
    static void HandleServerInterrupt(int sig_number) {
        // TODO(mu_username): Как сервер должен реагировать на сигналы?
        ////
        if (sig_number != SIGINT && sig_number != SIGTERM) {
            return;
        }
        server_running_ = false;
        ////
    }

    static void HandleNetClientInterrupt(int sig_number) {
        // TODO(mu_username): Как net-воркер должен реагировать на сигналы?
        ////
        if (sig_number != SIGINT && sig_number != SIGTERM) {
            return;
        }
        net_client_running_ = false;
        ////
    }

    static constexpr int kListenQueueSize = 10;

    ErrorStatus RunNetProcess(AutoClosingFileDescriptor client_fd, const AutoClosingPipe &synchro_pipe);

    inline static volatile bool server_running_ = false;

    std::vector<pid_t> net_workers_;

    inline static volatile bool net_client_running_ = false;

    unsigned int port_;
    std::string_view working_directory_;
    size_t max_workers_count_;
};

ErrorStatus Server::ServerImpl::RunNetProcess(AutoClosingFileDescriptor client_fd,
                                              const AutoClosingPipe &synchro_pipe) {
    // TODO(mu_username): Основная функция net-воркера

    // TODO(mu_username): регистрация сигналов
    ////
    std::signal(SIGINT, &HandleNetClientInterrupt);
    std::signal(SIGTERM, &HandleNetClientInterrupt);
    ////

    // TODO(mu_username): считывание данных из сети (возможно, неблокирующее,
    // с проверкой отсутствия изменения состояния процесса из-за какого-нибудь SIGINT)
    // Read -> network_bytes, error_status
    ////
    std::vector<std::byte> network_bytes;
    ReadStatus error_status = ReadFromNetwork(client_fd, network_bytes, false);  // СДЕЛАТЬ НЕБЛОКИРУЮЩИМ
    if (error_status == ReadStatus::kFailed) {
        // туду
        return ErrorStatus::kError;
    }
    ////

    // TODO(mu_username): считывание "флажка" из synchro-pipe,
    // что говорит о создании основным процессом named-pipe (возможно, в неблокирующем режиме)
    bool named_pipe_created = false;
    error_status = synchro_pipe.Read(named_pipe_created, false);
    ////
    if (error_status == ReadStatus::kFailed) {
        // туду
        return ErrorStatus::kError;
    }
    ////
    // AutoClosable? Поэкспериментируйте в разных местах с ними, чтобы не терять Close...
    auto from_net_pipe = NamedPipe{
        NamedPipe::GetPerProcessPipeName(working_directory_, kFromNetPipeTag)
    };

    // TODO(mu_username): пишем полученные из сети данные в named_pipe from_net
    ////
    if (from_net_pipe.Write(network_bytes) == ErrorStatus::kError) {
    ////
#ifdef DEBUG
    LOG_ERROR << "[NET WORKER pid=" << getpid() << "]" << "Sending client request to named pipe failed...\n";
#endif
        return ErrorStatus::kError;
    }

    ////
    auto to_net_pipe = NamedPipe{
        NamedPipe::GetPerProcessPipeName(working_directory_, kToNetPipeTag)
    };
    ////

    // TODO(mu_username): читаем ответ из named_pipe to_net
    // в неблокирующем режиме, проверяя состояние процесса, например, как-то так:
    ////
    std::vector<std::byte> serialized_response;

    net_client_running_ = true;
    // error_status = ReadStatus::kNoData;
    do {
        error_status = to_net_pipe.Read(serialized_response, kFileDescriptor_NonBlocking, false);

        if (error_status == ReadStatus::kNoData) {
            // TODO(mu_username): ждём...
            ////
            using std::chrono_literals::operator""ms;            std::this_thread::sleep_for(100ms);
            ////
        }
    } while (net_client_running_ && error_status == ReadStatus::kNoData);
    ////

    // TODO(mu_username): после таких циклов имеет смысл проверить причину выхода

#ifdef DEBUG
    LOG_MESSAGE << "Received client response from named pipe - ";
    std::transform(std::begin(serialized_response),
                   std::end(serialized_response),
                   std::ostream_iterator<int>(LOGGER, " "),
                    [](const auto &byte) {
                        return static_cast<int>(byte);
                    });
    LOGGER << "\n";
#endif

    if (error_status == ReadStatus::kFailed) {
#ifdef DEBUG
        LOG_ERROR << "[WORKER pid=" << getpid() << "]" << "Failed read...\n";
#endif
        return ErrorStatus::kError;
    }

    return SendToNetwork(client_fd, serialized_response);
}

Server::Server(unsigned int port, std::string_view working_directory, size_t max_workers_count) :
    impl_(std::make_unique<ServerImpl>(port, working_directory, max_workers_count)) {}

Server::~Server() = default;

ErrorStatus Server::Start() { return impl_->Start(); }

ErrorStatus Server::ServerImpl::Start() {
    // TODO(mu_username): Функция основного процесса сервера

    // TODO(mu_username): (добавить проверки)
    auto [error_status, pipe_to_planner] = Pipe::Create(kFileDescriptor_NonBlocking);
    ////
    if (error_status == ErrorStatus::kError) {
        return ErrorStatus::kError;
    }
    ////

    auto planner_pid = fork();
    ////
    if (planner_pid < 0) {
        pipe_to_planner.Close();
        return ErrorStatus::kError;
    }
    ////

    if (planner_pid == 0) {
        // создаём Planner
        auto planner_status = Planner{
            pipe_to_planner.ReaderFd(), max_workers_count_, working_directory_}.Start();
        // TODO(mu_username): что делаем дальше с Planner?
        ////
        return planner_status;
        ////
    }


    // TODO(mu_username): создаём сокет на порту и настраиваем его
    ////
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(socket_fd > 0);
    ConfigureFileDescriptor(socket_fd, FileDescriptorOptions::kFileDescriptor_NonBlocking);

    sockaddr_in addr = { 0, 0, 0, 0 };
    addr.sin_family = AF_INET;
    int rc = inet_aton("127.0.0.1", &addr.sin_addr);
    assert(rc > 0);
    addr.sin_port = htons(port_);

    // int opt_value = 1;
    // rc = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));
    // assert(!rc && "failed to set SO_REUSEADDR option for listening socket");

    rc = bind(socket_fd, (struct sockaddr*)&addr, (socklen_t)sizeof(addr));
    if (rc != 0) {
        perror("bind error:");
    }
    assert(rc == 0);

    rc = listen(socket_fd, 1024);
    assert(rc == 0);
#ifdef DEBUG
    LOG_MESSAGE << "Socket " << socket_fd << " configured and listening\n";
#endif
    ////

    // TODO(mu_username): не забываем про регистрацию обработчиков сигналов
    //// тут что то еще надо добавить?
    std::signal(SIGINT, &HandleServerInterrupt);
    std::signal(SIGTERM, &HandleServerInterrupt);

    server_running_ = true;

    auto return_status = ErrorStatus::kNoError;

#ifdef DEBUG
    LOG_MESSAGE << "Starting server main loop...\n";
#endif
    while (server_running_) {
        sockaddr_storage client_addr_info_storage = {};
        socklen_t client_addr_info_size = sizeof(client_addr_info_storage);

        auto client_fd = accept(socket_fd,
                                reinterpret_cast<sockaddr *>(&client_addr_info_storage),
                                &client_addr_info_size);
        if (client_fd < 0) {
            if (errno == EWOULDBLOCK) {  // если делать сокет блокирующим, то SIGINT его "не убьёт"...
                using std::chrono_literals::operator""ms;                std::this_thread::sleep_for(100ms);
                continue;
            }
    #ifdef DEBUG
        LOG_ERROR << "accept failed " << std::strerror(errno) << "\n";
    #endif
            server_running_ = false;
            return_status = ErrorStatus::kError;
            ////?????
            continue;
            ////?????
        }


        // TODO(mu_username): создаём synchro-pipe
        ////
        auto [error_status, synchro_net_pipe] = Pipe::Create(
                                                FileDescriptorOptions::kFileDescriptor_NonBlocking);
        if (error_status == ErrorStatus::kError) {
#ifdef DEBUG
            LOG_ERROR << "Failed to create synchro pipe for client " << client_fd << "\n";
#endif

            close(client_fd);

            server_running_ = false;  // ?????
            return_status = ErrorStatus::kError;
            continue;
        }
        ////

        auto net_worker_pid = fork();
        if (net_worker_pid < 0) {
#ifdef DEBUG
            LOG_ERROR << "Failed to fork net process for client " << client_fd << "\n";
#endif
            close(client_fd);

            server_running_ = false;
            return_status = ErrorStatus::kError;
            continue;
        }

        if (net_worker_pid == 0) {
            // TODO(mu_username): запускаем net-воркера
            //// что тут добавить надо?
            auto net_process_exit_status = RunNetProcess(client_fd, AutoClosingPipe(synchro_net_pipe));
#ifdef DEBUG
            LOG_MESSAGE << "Stopping net worker " << getpid() << "...\n";
#endif
            return net_process_exit_status;
        }

#ifdef DEBUG
        LOG_MESSAGE << "[PLANNER] Created net process " << net_worker_pid << "...\n";
#endif


        // TODO(mu_username): Создаём named-pipe from-net и to-net (не забываем обработать ошибки!)

        auto [from_net_error, from_net_pipe] = NamedPipe::Create(
            NamedPipe::GetPerProcessPipeName(working_directory_, kFromNetPipeTag, net_worker_pid));

        ////
        if (from_net_error == ErrorStatus::kError) {
            //// ЗАВЕРШИТЬ ПРОЦЕСС НЕТ ВОРКЕРА
#ifdef DEBUG
            LOG_ERROR << "Failed to create from_net_pipe for client " << client_fd
            << "; pid: " << net_worker_pid << "\n";
#endif

            close(client_fd);

            server_running_ = false;
            return_status = ErrorStatus::kError;
            continue;
        }
        ////

        auto [to_net_error, to_net_pipe] = NamedPipe::Create(
            NamedPipe::GetPerProcessPipeName(working_directory_, kToNetPipeTag, net_worker_pid));

        ////
        if (to_net_error == ErrorStatus::kError) {
            //// ЗАВЕРШИТЬ ПРОЦЕСС НЕТ ВОРКЕРА

#ifdef DEBUG
            LOG_ERROR << "Failed to create to_net_pipe for client "
            << client_fd << "; pid: " << net_worker_pid << "\n";
#endif

            close(client_fd);

            server_running_ = false;
            return_status = ErrorStatus::kError;
            continue;
        }
        ////

        // TODO(mu_username): передаём планировщику pid net-воркера через созданный ранее pipe
        ////
        ErrorStatus write_planner_error = pipe_to_planner.Write(net_worker_pid);
        if (write_planner_error == ErrorStatus::kError) {
            //// ЗАВЕРШИТЬ ПРОЦЕСС НЕТ ВОРКЕРА

#ifdef DEBUG
            LOG_ERROR << "Failed sending net worker pid " << net_worker_pid
            << " to planner for client " << client_fd << "\n";
#endif

            close(client_fd);

            server_running_ = false;
            return_status = ErrorStatus::kError;
            continue;
        }
        
#ifdef DEBUG
        else {
            LOG_MESSAGE << "Success sending net worker pid " << net_worker_pid
            << " to planner for client " << client_fd << "\n";
        }
#endif
        ////

        // TODO(mu_username): уведомляем через synchro-pipe net-воркера, что named-pipe-создан
        ////
        ErrorStatus notify_net_worker_error = synchro_net_pipe.Write(true);
        if (notify_net_worker_error == ErrorStatus::kError) {
            //// ЗАВЕРШИТЬ ПРОЦЕСС НЕТ ВОРКЕРА

#ifdef DEBUG
            LOG_ERROR << "Failed to notify net worker pid " << net_worker_pid
            << " for client " << client_fd << "\n";
#endif

            close(client_fd);

            server_running_ = false;
            return_status = ErrorStatus::kError;
            continue;
        }
        ////

        // TODO(mu_username): сохраняем информацию о воркере, чтобы в конце не забыть его убить
        net_workers_.emplace_back(net_worker_pid);
        //// что то еще сделать надо?
        /// сохранить pid воркера
    }

    // TODO(mu_username): после SIGINT должны оказаться тут и корректно
    // завершить работу планировщика и net-воркеров
    // например, можно всех "убить" и "подождать" в неблокирующем режиме
    // если какой-то из воркеров вернул ошибку - этот тоже должен вернуть ошибку
    close(socket_fd);

    return return_status;
}

#ifndef TESTS_ENABLED
#ifdef DEBUG
// #include <cassert>

int main(int argc, char *argv[]) {
    const unsigned int port = argc > 1 ? std::stoi(argv[1]) : kDefaultServerPort;
    const std::string_view working_directory = argc > 2 ? argv[2] : kDefaultWorkingDirectory;
    const size_t max_workers_count = argc > 3 ? std::stoi(argv[3]) : kDefaultMaxWorkersCount;
    auto server = Server{port, working_directory, max_workers_count};
    assert(server.Start() == ErrorStatus::kNoError);
}
#endif
#endif
