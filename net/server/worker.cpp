// Copyright (c) 2016 Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License file in the project root for license information.
#include <fstream>
#include <csignal>
#include <filesystem>

#include <calc/matrix.hpp>
#include <net/server/worker.hpp>

struct Worker::WorkerImpl {
    WorkerImpl(const Pipe &to_worker_pipe,
        const Pipe &from_worker_pipe,
        std::string_view working_directory) :
        to_worker_pipe_(to_worker_pipe),
        from_worker_pipe_(from_worker_pipe),
        working_directory_(working_directory) {}

    ErrorStatus Run();
    static ErrorStatus CleanWorkingDirectory(
                                           std::string_view working_directory);

 private:
    static void HandlerWorkerSignal(int sig_number) {
        // TODO(mu_username): а что же должен делать таск-воркер по
        // SIGINT и иным сигналам?
        ////
        if (sig_number != SIGINT && sig_number != SIGTERM) {
            return;
        }
        worker_running_ = false;
        ////
    }

    static constexpr std::string_view kProcessTaskPrefix = "task-";

    ErrorStatus RunReceiveTask(int client_pid);
    ErrorStatus RunGetQueueUpdate(int client_pid, int task_id);
    ErrorStatus RunPostQueueUpdate(int client_pid,
                                   std::string_view task_content);

    ErrorStatus RunProcessTask(int task_id) const;

    ErrorStatus SendResponse(int client_pid, const Response &response);

    std::tuple<ErrorStatus, std::string> LoadTask(int task_id) const;
    ErrorStatus SaveTask(int task_id, std::string_view task_content) const;
    void RemoveTask(int task_id) const;

    std::filesystem::path TaskPath(int task_id) const;

    inline static volatile bool worker_running_ = false;

    AutoClosingPipe to_worker_pipe_;
    AutoClosingPipe from_worker_pipe_;
    std::string_view working_directory_;
};

ErrorStatus Worker::WorkerImpl::CleanWorkingDirectory(
                                          std::string_view working_directory) {
    for (const auto &entry : std::filesystem::directory_iterator(working_directory)) {
        // TODO(mu_username): Удаляем все созданные файлы с задачами при окончании работы
        ////
        std::filesystem::path p = entry.path();
        if (p.string().find(kProcessTaskPrefix) == 0) {
#ifdef DEBUG
            LOG_MESSAGE << "NOT Deleting file " << p << "\n";
#endif
            // unlink(p.string().c_str());
        }
        ////
    }

    return ErrorStatus::kNoError;
}

Worker::Worker(const Pipe &to_worker_pipe,
               const Pipe &from_worker_pipe,
               std::string_view working_directory) :
    impl_(std::make_unique<WorkerImpl>(to_worker_pipe, from_worker_pipe, working_directory)) {}

ErrorStatus Worker::Run() { return impl_->Run(); }

Worker::~Worker() = default;

ErrorStatus Worker::CleanWorkingDirectory(std::string_view working_directory) {
    return WorkerImpl::CleanWorkingDirectory(working_directory);
}

std::filesystem::path Worker::WorkerImpl::TaskPath(int task_id) const {
    auto result = std::filesystem::path {working_directory_} /
        (std::filesystem::path{kProcessTaskPrefix}.concat(std::to_string(task_id)));
#ifdef DEBUG
    LOG_MESSAGE << "[WORKER pid=" << getpid() << "]" << "TaskPath(" << task_id << ") = " << result << "\n";
#endif
    return result;
}

ErrorStatus Worker::WorkerImpl::SendResponse(int client_pid,
                                             const Response &response) {
    // TODO(mu_username): Сериализуем результат (response) в
    // массив байт и отправляем в NamedPipe to_net
    ////
    std::vector<std::byte> network_bytes =
                                          NetworkResponse::Serialize(response);

    auto [error_status, to_net] = AutoClosableNamedPipe::Create(
        NamedPipe::GetPerProcessPipeName(
            working_directory_, kToNetPipeTag, client_pid));

    if (error_status == ErrorStatus::kError) {
        return ErrorStatus::kError;
    }

    return to_net.Write(network_bytes);
    ////
}

void Worker::WorkerImpl::RemoveTask(int task_id) const {
#ifdef DEBUG
    LOG_MESSAGE << "[WORKER pid=" << getpid() << "]"
    << "RemoveTask(" << task_id << ")\n";
#endif
    // TODO(mu_username): удаляем файл с задачей
    ////
    auto path = TaskPath(task_id);
    close(open(path.c_str(), 0644));
    // unlink(path.string().c_str());
#ifdef DEBUG
    LOG_MESSAGE << "[WORKER pid=" << getpid() << "]"
    << "task removed("<< path << ")\n";
#endif
    ////
}

std::tuple<ErrorStatus, std::string> Worker::WorkerImpl::LoadTask(int task_id) const {
#ifdef DEBUG
    LOG_MESSAGE << "[WORKER pid=" << getpid() << "]" << "LoadTask(" << task_id << ")\n";
#endif
    // TODO(mu_username): Читаем содержимое файла с задачей и бросаем ошибку, если случилась
    ////
    auto path = TaskPath(task_id);
    int fd = open(path.string().c_str(), 0644);
    if (fd < 0) {
        return std::make_tuple(ErrorStatus::kError, std::string{});
    }
    AutoClosingFileDescriptor task_fd(fd);
    std::vector<std::byte> str;
    ReadStatus read_status = ReadFromFileDescriptor(task_fd, str);

    return std::make_tuple(read_status == ReadStatus::kSucceed ? ErrorStatus::kNoError : ErrorStatus::kError,
                           std::string{reinterpret_cast<char *>(str.data())});
    ////
}

ErrorStatus Worker::WorkerImpl::SaveTask(int task_id,
                                        std::string_view task_content) const {
    // TODO(mu_username): Сохраняем задачу в файл TaskPath()
    ////
    auto path = TaskPath(task_id);
    int fd = open(path.string().c_str(), 0644);
    if (fd < 0) {
        return ErrorStatus::kError;
    }
    AutoClosingFileDescriptor task_fd(fd);
    return write(task_fd, task_content.data(), task_content.size())
            >= 0 ? ErrorStatus::kNoError : ErrorStatus::kError;
    ////
}

// обработка POST-запроса от клиента
ErrorStatus Worker::WorkerImpl::RunPostQueueUpdate(int client_pid,
                                                std::string_view task_content) {
    // Запрашиваем уникальный id
    if (from_worker_pipe_.Write(
            UpdateQueueRequest{UpdateQueueRequest::Type::kCreate}) == ErrorStatus::kError) {
        // TODO(mu_username): что делать при подобных ошибках?
        ////
#ifdef DEBUG
        LOG_MESSAGE << "[WORKER pid=" << getpid() << "]"
        << "unable to write in from_worker_pipe" << "\n";
#endif
        return ErrorStatus::kError;
        ////
    }

    // TODO(mu_username): получаем ответ от планировщика с уникальным id
    UpdateQueueResponse response;
    auto read_status = to_worker_pipe_.Read(response);
    ////
    if (read_status != ReadStatus::kSucceed) {
#ifdef DEBUG
        LOG_MESSAGE << "[WORKER pid=" << getpid() << "]"
        << "unable to get new task id (read_status=" << static_cast<int>(read_status)
        << ")\n";
#endif
        return ErrorStatus::kError;
    }
    ////

    // TODO(mu_username): Сохраняем задачу в файл TaskPath()
    // (не забываем обрабатывать всевозможные ошибки)
    ////
    ErrorStatus error_status = SaveTask(response.id.for_task, task_content);
#ifdef DEBUG
    if (error_status == ErrorStatus::kError) {
        LOG_MESSAGE << "[WORKER pid=" << getpid() << "]"
        << "failed to save task id=" << response.id.for_task << "\n";
    }
#endif

    ////

    // TODO(mu_username): Обновляем статус задачи на New, если
    // сохранение в файл прошло успешно
    // Удаляем задачу - если ошибка
    // Отправляем net-воркеру результат
    ////
    if (error_status == ErrorStatus::kNoError) {
        error_status = from_worker_pipe_.Write(UpdateQueueRequest{
            UpdateQueueRequest::Type::kNew, response.id.for_task
        });
        if (error_status == ErrorStatus::kError) {
#ifdef DEBUG
        LOG_MESSAGE << "[WORKER pid=" << getpid() << "]"
        << "error while sending update task id=" << response.id.for_task
        << " kNew status request to worker " << client_pid << "\n";
#endif
            SendResponse(client_pid, Response{
                ResponseType::kError, std::to_string(response.id.for_task)
            });
            return ErrorStatus::kError;
        }
        error_status = SendResponse(client_pid, Response{
            ResponseType::kNew, std::to_string(response.id.for_task)
        });
    } else {
        error_status = from_worker_pipe_.Write(UpdateQueueRequest{
            UpdateQueueRequest::Type::kDelete, response.id.for_task
        });
        if (error_status == ErrorStatus::kError) {
#ifdef DEBUG
        LOG_MESSAGE << "[WORKER pid=" << getpid() << "]"
        << "error while sending update task id=" << response.id.for_task
        << " kDelete status request to worker " << client_pid << "\n";
#endif
            SendResponse(client_pid, Response{
                ResponseType::kError, std::to_string(response.id.for_task)
            });
            return ErrorStatus::kError;
        }
        error_status = SendResponse(client_pid, Response{
            ResponseType::kError, std::to_string(response.id.for_task)
        });
    }
    ////

    // TODO(mu_username): что возвращаем?
    ////
    return error_status;
    ////
}

// TODO(mu_username): обработка GET-запроса пользователя
ErrorStatus Worker::WorkerImpl::RunGetQueueUpdate(int client_pid, int task_id) {
    // TODO(mu_username): Запрашиваем статус задачи по id у планировщика
    // Отправляем ответ в зависимости от полученного статуса

    ////
    ErrorStatus error_status = from_worker_pipe_.Write(
        UpdateQueueRequest{UpdateQueueRequest::Type::kCheck, task_id});
    // if (error_status == ErrorStatus::kError) {
    //     SendResponse(client_pid, Response{
    //         Response::Type::kError, std::to_string(task_id)
    //     });
    //     return ErrorStatus::kError;
    // }

    UpdateQueueResponse response;
    /*ReadStatus read_status = */to_worker_pipe_.Read(response);
    ////


    switch (response.task_status) {
      ////
      case WorkerTask::Status::kNew: {
          return SendResponse(client_pid, Response{
            Response::Type::kNew, std::to_string(task_id)
          });
      }
      case WorkerTask::Status::kWork: {
          return SendResponse(client_pid, Response{
            Response::Type::kWork, std::to_string(task_id)
          });
      }
      ////
      case WorkerTask::Status::kDone: {
        // TODO(mu_username): в Done и Error не забываем удалять
        // файлы с диска и очереди!
        ////
        RemoveTask(task_id);
        if (error_status == ErrorStatus::kError) {
            SendResponse(client_pid, Response{
              Response::Type::kError, std::to_string(task_id)
            });
            return ErrorStatus::kError;
        }

        auto [error_status, content] = LoadTask(task_id);
        if (error_status == ErrorStatus::kNoError) {
            return SendResponse(client_pid, Response{
                Response::Type::kDone, content
            });
        } else {
            return SendResponse(client_pid, Response{
                Response::Type::kError, std::to_string(task_id)
            });
        }
        ////
      }
      case WorkerTask::Status::kError: {
        RemoveTask(task_id);
        return SendResponse(client_pid, Response{
            ////
            Response::Type::kError, std::to_string(task_id)
            ////
        });
      }
      case WorkerTask::Status::kUnknown:
      default: {
        return SendResponse(client_pid, Response{
            ////
            Response::Type::kError, std::to_string(task_id)
            ////
        });
      }
    }

    return ErrorStatus::kNoError;
}

// TODO(mu_username): Обработка Receive-задачи
ErrorStatus Worker::WorkerImpl::RunReceiveTask(int client_pid) {
    // TODO(mu_username): Считываем из named-pipe from-net данные
    ////
    AutoClosableNamedPipe named_pipe(NamedPipe(
        NamedPipe::GetPerProcessPipeName(
            working_directory_, kFromNetPipeTag, client_pid)));
    ////

    std::vector<std::byte> network_bytes;
    auto read_status = named_pipe.Read(
        network_bytes, kFileDescriptor_NonBlocking);

#ifdef DEBUG
    LOG_MESSAGE << "[WORKER pid=" << getpid() << "]"
    << "Read from named pipe from pid = " << client_pid << " result...\n";
    std::transform(
        std::begin(network_bytes),
         std::end(network_bytes),
          std::ostream_iterator<int>(LOGGER, " "),
                    [](const auto &byte) {
                        return static_cast<int>(byte);
                    });
    LOGGER << "\n";
#endif

    if (read_status == ReadStatus::kFailed) {
#ifdef DEBUG
        LOG_ERROR << "[WORKER pid=" << getpid() << "]" << "Failed read...\n";
#endif
        return ErrorStatus::kError;
    }

    // TODO(mu_username): Десереализуем ответ (с обработкой ошибок)
    auto [error_status, request] = NetworkRequest::Deserialize(network_bytes);


#ifdef DEBUG
    LOG_MESSAGE << "[WORKER pid=" << getpid() << "]" <<
    "Worker recevied request from named pipe - " << request << "\n";
#endif

    // TODO(mu_username): if error_status -> plan Send failure
    // (write message to file and send)
    //
    // if no error_status ->
    // if get -> ask planner for status and plan send
    // if post -> ask planner for create, move data from "from-net" to
    // task- and write to client- "<task_id> OK"
    ////
    if (error_status == ErrorStatus::kError) {
        // write(AutoClosingFileDescriptor(open(TaskPath(request.), 0644)));
        SendResponse(client_pid, Response{Response::Type::kError});
        return ErrorStatus::kError;
    }
    ////

    if (read_status == ReadStatus::kSucceed) {
        switch (request.type) {
          case Request::Type::kGet: {
            // TODO(mu_username): Implement me
            ////
            int task_id = std::stoi(
                request.content.substr(request.content.find(" ")));
            // error_status = from_worker_pipe_.Write(
            // UpdateQueueRequest{UpdateQueueRequest::Type::kGet, task_id});
            // if (error_status == ErrorStatus::kError) {
            //     SendResponse(client_pid, Response{Response::Type::kError});
            //     return ErrorStatus::kError;
            // }

            // UpdateQueueResponse response;
            // ReadStatus read_status = to_worker_pipe_.Read(response);

            // if (read_status != ReadStatus::kSucceed) {
            //     SendResponse(client_pid, Response{Response::Type::kError});
            //     return ErrorStatus::kError;
            // }
            error_status = RunGetQueueUpdate(client_pid, task_id);

            break;
            ////
          }
          case Request::Type::kPost: {
            // TODO(mu_username): Implement me
            ////
            // int task_id = std::stoi(request.content.substr(
            // request.content.find(" ")));
            error_status = RunPostQueueUpdate(
                client_pid, request.content.substr(request.content.find(" ")));
            break;
            ////
          }
          case Request::Type::kUnknown:
          default: {
            // TODO(mu_username): Implement me
            ////
            SendResponse(client_pid, Response{Response::Type::kError});
            break;
            ////
          }
        }
    }

    // TODO(mu_username): что возвращаем?
    ////
    return error_status;
    ////
}

// TODO(mu_username): Обработка задачи типа Process
ErrorStatus Worker::WorkerImpl::RunProcessTask(int task_id) const {
    // TODO(mu_username): Загружаем данные из файла TaskPath
    ////
    auto [error_status, task] = LoadTask(task_id);
    ////
    // if (error_status == ErrorStatus::kError) {
    //     return ErrorStatus::kError;
    // }

    // TODO(mu_username): Запускаем обработку
    ////
    auto [matrix_error_status, task_result] = Matrix::Process(task);
#ifdef DEBUG
    LOG_MESSAGE << "[WORKER pid=" << getpid()
    << "] error in matrix process"<< "\n";
#endif
    ////

    // TODO(mu_username): Сохраняем результат
    ////
    error_status = SaveTask(task_id, std::string_view(task_result))
                   == ErrorStatus::kNoError &&
                   matrix_error_status == Matrix::ErrorStatus::kNoError
                   ? ErrorStatus::kNoError : ErrorStatus::kError;

    // TODO(mu_username): Обновляем статус планировщика
    ////
    error_status = from_worker_pipe_.Write(UpdateQueueRequest{
        error_status == ErrorStatus::kNoError ?
        UpdateQueueRequest::Type::kDone : UpdateQueueRequest::Type::kError,
        task_id
    }) == ErrorStatus::kNoError &&
    error_status == ErrorStatus::kNoError ?
    ErrorStatus::kNoError : ErrorStatus::kError;
    ////

    // TODO(mu_username): Что возвращаем?
    ////
    return error_status;
    ////
}

// TODO(mu_username): Основная функция task-воркера
ErrorStatus Worker::WorkerImpl::Run() {
    // TODO(mu_username): ask planner for task
    // receive task
    // execute it
    // send status to planner

#ifdef DEBUG
    LOG_MESSAGE << "[WORKER] My pid is " << getpid() << "...\n";
#endif

    auto worker_status = ErrorStatus::kNoError;

    // TODO(mu_username): регистрируем сигналы
    ////
    std::signal(SIGINT, &HandlerWorkerSignal);
    std::signal(SIGTERM, &HandlerWorkerSignal);
    ////

    worker_running_ = true;

    while (worker_running_) {
        // TODO(mu_username): Просим очередную задачу у планировщика
        from_worker_pipe_.Write(UpdateQueueRequest{
            UpdateQueueRequest::Type::kGet
        });

        // TODO(mu_username): Если нет задач - спим
        // Если пришла - обрабатываем
        ////
        UpdateQueueResponse received_task;
        auto read_status = to_worker_pipe_.Read(received_task);

        if (read_status == ReadStatus::kNoData) {
            using std::chrono_literals::operator""ms;
            std::this_thread::sleep_for(100ms);
            continue;
        } else if (read_status == ReadStatus::kFailed) {
            worker_running_ = false;
            continue;
        }
        ////

        ErrorStatus task_result = ErrorStatus::kNoError;
        switch (received_task.task_type) {
            case WorkerTask::Type::kReceive: {
                // TODO(mu_username): Implement me
                ////
                task_result = RunReceiveTask(received_task.id.for_task);
                if (task_result == ErrorStatus::kError) {
#ifdef DEBUG
                    LOG_ERROR << "[WORKER pid=" << getpid() << "]"
                    << "Failed RunReceiveTask(" << received_task.id.for_task
                    << ")...\n";
#endif
                }
                ////
                break;
            }
            case WorkerTask::Type::kProcess: {
                // TODO(mu_username): Implement me
                ////
                task_result = RunProcessTask(received_task.id.for_task);
                ////
                if (task_result == ErrorStatus::kError) {
#ifdef DEBUG
                    LOG_ERROR << "[WORKER pid=" << getpid() << "]"
                    << "Failed RunProcessTask(" << received_task.id.for_task
                    << ")...\n";
#endif
                }
                break;
            }
            default: {
#ifdef DEBUG
              LOG_ERROR << "[WORKER pid=" << getpid() << "]"
              << "Received unknown worker task type...\n";
#endif
              task_result = ErrorStatus::kError;
            break;
            }
        }

        if (task_result == ErrorStatus::kError) {
#ifdef DEBUG
            LOG_ERROR << "[WORKER pid=" << getpid() << "]"
            << "Failed task...\n";
#endif
            worker_status = ErrorStatus::kError;
            worker_running_ = false;
            ////
            from_worker_pipe_.Write(UpdateQueueRequest{
                UpdateQueueRequest::Type::kError, received_task.id.for_task
            });
            continue;
            ////
        }
        ////
        from_worker_pipe_.Write(UpdateQueueRequest{
            UpdateQueueRequest::Type::kDone, received_task.id.for_task
        });
        ////
    }

#ifdef DEBUG
    LOG_MESSAGE << "[WORKER " << getpid() << "] Finishing...\n";
#endif

    return worker_status;
}
