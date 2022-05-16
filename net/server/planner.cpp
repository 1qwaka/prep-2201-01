// Copyright (c) 2016 Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License file in the project root for license information.
#include <sys/wait.h>

#include <csignal>
#include <deque>
#include <algorithm>

#include <net/server/planner.hpp>
#include <net/server/worker.hpp>

struct Planner::PlannerImpl {
    PlannerImpl(int incoming_fd, size_t max_workers_count, std::string_view working_directory) :
        incoming_fd_(incoming_fd),
        max_workers_count_(max_workers_count),
        working_directory_(working_directory) {}

    static void HandlePlannerSignal(int sig_number) {
        // TODO(mu_username): Что нужно делать по тому или иному сигналу?
        ////
#ifdef DEBUG
        LOG_MESSAGE << "[PLANNER] Handled signal " << sig_number << "\n";
#endif
        if (sig_number != SIGINT && sig_number != SIGTERM) {
            return;
        }
        planner_running_ = false;
        ////
    }

    ErrorStatus Start();
    ErrorStatus Stop();

 private:
    inline static volatile bool planner_running_ = false;

    AutoClosingFileDescriptor incoming_fd_;
    size_t max_workers_count_;
    std::string_view working_directory_;

    std::vector<WorkerInfo> workers_;
};

Planner::Planner(int incoming_fd, size_t max_workers_count, std::string_view working_directory) :
    impl_(std::make_unique<PlannerImpl>(incoming_fd, max_workers_count, working_directory)) {}

Planner::~Planner() { Stop(); }

ErrorStatus Planner::Start() { return impl_->Start(); }

ErrorStatus Planner::Stop() { return impl_->Stop(); }

ErrorStatus Planner::PlannerImpl::Stop() {
    // TODO(mu_username): при остановке нужно корректно завершить работу дочерних таск-воркеров
    ////
    for (WorkerInfo info : workers_) {
        kill(info.worker_pid, SIGTERM);
    }
    return ErrorStatus::kNoError;
    ////
}

ErrorStatus Planner::PlannerImpl::Start() {
#ifdef DEBUG
    LOG_MESSAGE << "[PLANNER] My pid is " << getpid() << "...\n";
#endif
    workers_.reserve(max_workers_count_);

    std::deque<WorkerTask> planned_tasks;

    int last_task_id = 0;

    // TODO(mu_username): регистрация обработчиков сигналов (SIGINT/...)
    planner_running_ = true;
    ////
    std::signal(SIGTERM, &HandlePlannerSignal);
    std::signal(SIGINT, &HandlePlannerSignal);
    ////

    auto planner_error_status = ErrorStatus::kNoError;

    while (planner_running_) {
        bool queue_updated = false;

        // TODO(mu_username): чтение incoming_fd_ (pipe от основного серверного процесса) в поисках net_pid
        ////
        int net_pid;
        ReadStatus read_status = ReadFromFileDescriptor(incoming_fd_, net_pid, false);
        ReadStatus u = read_status;
        ////

        // TODO(mu_username): не забываем обрабатывать ошибки
        // Read(incoming_fd) -> net_pid, read_status

        if (read_status == ReadStatus::kSucceed) {
#ifdef DEBUG
        LOG_MESSAGE << "Planned added new receive task - " << net_pid << "\n";
#endif

            // TODO(mu_username): Добавляем задачу Receive
            queue_updated = true;
            ////
            planned_tasks.emplace_back(net_pid,
                                       WorkerTask::Status::kNew,
                                       WorkerTask::Type::kReceive,
                                       TaggedId::Tag::kNet);
            ////

        ////
        // } else if (read_status == ReadStatus::kNoData) {
        //     using std::chrono_literals::operator""ms;
        //     std::this_thread::sleep_for(100ms);
        //     continue;
        } else if (read_status == ReadStatus::kFailed) {
#ifdef DEBUG
            LOG_MESSAGE << "[PLANNER] failed to read from main server process " << "\n";
#endif
            planner_running_ = false;
            continue;
        }
        ////

        for (auto &worker : workers_) {
            // TODO(mu_username): если ничего не запросил worker - переходим к следующему
            // worker.from_worker_pipe.Read() -> request, read_status
            // если запросил - обрабатываем далее

            ////
            UpdateQueueRequest request;
            read_status = worker.from_worker_pipe.Read(request);


            if (read_status == ReadStatus::kNoData) {
                continue;
            } else if (read_status == ReadStatus::kFailed) {
#ifdef DEBUG
            LOG_MESSAGE << "Failed to recive request from worker " << worker.worker_pid
            << ": " << request << "\n";
#endif
                continue;
            }
            ////

#ifdef DEBUG
            LOG_MESSAGE << "Received request from worker " << worker.worker_pid << ": " << request << "\n";
#endif

            UpdateQueueResponse response;

#ifdef DEBUG
            LOG_MESSAGE << "Current planned tasks\n";
            for (const auto &task : planned_tasks) {
                LOG_MESSAGE << "TASK -> " << task << "\n";
            }
#endif

            switch (request.type) {
              case UpdateQueueRequest::Type::kNew: {
                // TODO(mu_username): добавление Receive-задачи // ????

                //// перевод задачи с task_id в статус New и тип kProcess
                // WorkerTask task = *std::find_if(begin(planned_tasks),
                //                                 end(planned_tasks),
                //                                 [&request](WorkerTask task){
                                                //        return task.id.for_task == request.task_id;
                                                //    });
                // task.status = WorkerTask::Status::kNew;

                std::find_if(
                    begin(planned_tasks),
                    end(planned_tasks),
                    [&request](WorkerTask task){ return task.id.for_task == request.task_id; })
                    ->status = WorkerTask::Status::kNew;

                // for (WorkerTask task : planned_tasks) {
                //     if (task.id.for_task == request.task_id) {
                //         task.status = WorkerTask::Status::kNew;
                //         // task.type = WorkerTask::Type::kProcess;
                //         break;
                //     }
                // }
                break;
                ////
              }
              case UpdateQueueRequest::Type::kCreate: {
                // TODO(mu_username): генерация нового id (возможно, сразу с задачей типа Unknown)
                ////
                response.id = TaggedId(++last_task_id);
                planned_tasks.emplace_back(last_task_id,
                                           WorkerTask::Status::kUnknown,
                                           WorkerTask::Type::kProcess);
                response.task_status = WorkerTask::Status::kUnknown;
                response.task_type = WorkerTask::Type::kProcess;
                break;
                ////
              }
              case UpdateQueueRequest::Type::kDelete: {
                // TODO(mu_username): удаление задачи по id
                ////
                for (auto i = planned_tasks.begin(); i < planned_tasks.cend(); i++) {
                    if (i->id.for_task == request.task_id) {
                        planned_tasks.erase(i);
                        break;
                    }
                }
                break;
                ////
              }
              case UpdateQueueRequest::Type::kCheck: {
                // TODO(mu_username): проверка статуса задачи по id
                //// удаление из очереди если kDone или kError
                auto i = planned_tasks.begin();
                for (; i < planned_tasks.cend(); i++) {
                    if (i->id.for_task == request.task_id) {
                        response.task_status = i->status;
                        response.task_type = i->type;
                        if (i->status == WorkerTask::Status::kDone ||
                            i->status == WorkerTask::Status::kError) {
                            planned_tasks.erase(i);
                        }
                        break;
                    }
                }
                break;
                ////
              }
              case UpdateQueueRequest::Type::kGet: {
                // TODO(mu_username): получение новой (New) задачи из очереди и перевод её статуса в Work
                ////
                for (auto i = planned_tasks.begin(); i < planned_tasks.cend(); i++) {
                    if (i->status == WorkerTask::Status::kNew) {
                        i->status = WorkerTask::Status::kDone;
                        break;
                    }
                }
                break;
                ////
              }
              case UpdateQueueRequest::Type::kDone: {
                // TODO(mu_username): перевод задачи в статус Done
                ////
                for (auto i = planned_tasks.begin(); i < planned_tasks.cend(); i++) {
                    if (i->id.for_task == request.task_id) {
                        i->status = WorkerTask::Status::kDone;
                        response.task_status = i->status;
                        response.task_type = i->type;
                        break;
                    }
                }
                break;
                ////
              }
              case UpdateQueueRequest::Type::kError: {
                // TODO(mu_username): перевод задачи в статус Error
                ////
                for (auto i = planned_tasks.begin(); i < planned_tasks.cend(); i++) {
                    if (i->id.for_task == request.task_id) {
                        i->status = WorkerTask::Status::kError;
                        response.task_status = i->status;
                        response.task_type = i->type;
                        break;
                    }
                }
                break;
                ////
              }
              case UpdateQueueRequest::Type::kUnknown:
              default: {
                ////
                response.task_status = WorkerTask::Status::kUnknown;
                response.task_type = WorkerTask::Type::kUnknown;
                ////
#ifdef DEBUG
                LOG_ERROR << "[PLANNER] Unknown queue request type...\n";
#endif
                break;
              }
            }

#ifdef DEBUG
            LOG_MESSAGE << "Sending response to worker " << worker << ": " << response << "\n";
#endif
            if (worker.to_worker_pipe.Write(response) == ErrorStatus::kError) {
#ifdef DEBUG
                LOG_ERROR << "[PLANNER] Broken planner write...\n";
#endif
                // TODO(mu_username): что делать при ошибке?
                ////
                continue;
                ////
            }
        }

        // TODO(mu_username): если всё сделали, нераспределённых задач нет, очередь не менялась -
        // можно и поспать немного (например, 100 мс)
        // Например, это можно сделать примерно так...
        ////
        if (!queue_updated) {
            using std::chrono_literals::operator""ms;
            std::this_thread::sleep_for(100ms);
            continue;
        }
        ////

        // TODO(mu_username): если есть нераспределённые задачи - создаём нового воркера
        // (если есть ещё места в пуле)

        if (workers_.size() == max_workers_count_ || u == ReadStatus::kNoData) {
            continue;
        }

        // TODO(mu_username): при создании новых task-воркеров нужно создать pipe_from_worker и
        // pipe_to_worker для связи с ними
        // pipe_from_worker = Pipe::Create(NonBlocking);

        auto new_worker_pid = fork();
        if (new_worker_pid < 0) {
            // TODO(mu_username): что делать при ошибках?
            ////
#ifdef DEBUG
    LOG_MESSAGE << "[PLANNER] Failed to create task worker process " << "\n";
#endif
            planner_running_ = false;
            continue;
            ////
        }
        ////
        auto [error_status, pipe_from_worker] = Pipe::Create(
            FileDescriptorOptions::kFileDescriptor_NonBlocking);
        if (error_status == ErrorStatus::kError) {
#ifdef DEBUG
    LOG_MESSAGE << "[PLANNER] Failed to open pipe From task worker " << "\n";
#endif
            planner_running_ = false;
            continue;
        }
        Pipe pipe_to_worker;
        std::tie(error_status, pipe_to_worker) = Pipe::Create(
            FileDescriptorOptions::kFileDescriptor_NonBlocking);
        if (error_status == ErrorStatus::kError) {
            pipe_from_worker.Close();
            planner_running_ = false;
#ifdef DEBUG
    LOG_MESSAGE << "[PLANNER] Failed to open pipe To task worker " << "\n";
#endif
            continue;
        }
        ////

        if (new_worker_pid == 0) {
            // TODO(mu_username): запускаем task-воркеров
            auto worker_exit_status = Worker{pipe_to_worker, pipe_from_worker, working_directory_}.Run();
            return worker_exit_status;
        }

        // TODO(mu_username): сохраняем информацию о новом воркере, чтобы потом не забыть удалить!
        ////
        workers_.emplace_back(new_worker_pid, pipe_to_worker, pipe_from_worker);
        ////

#ifdef DEBUG
        LOG_MESSAGE << "Adding new worker " << new_worker_pid << " pipe_to_worker = "
        << pipe_to_worker.ReaderFd() << ":" << pipe_to_worker.WriterFd() << ", pipe_from_worker = "
        << pipe_from_worker.ReaderFd() << ":" << pipe_from_worker.WriterFd() << "\n";
#endif
    }

#ifdef DEBUG
    LOG_MESSAGE << "[PLANNER] After while stopping planner, planner_error_status = Error "
    << (planner_error_status == ErrorStatus::kError) << "...\n";
#endif

    if (Stop() == ErrorStatus::kError) {
        planner_error_status = ErrorStatus::kError;
    }

#ifdef DEBUG
    if (planner_error_status == ErrorStatus::kError) {
        LOG_MESSAGE << "[PLANNER] Finishing with error...\n";
    } else {
        LOG_MESSAGE << "[PLANNER] Finishing OK...\n";
    }

#endif

    return planner_error_status;
}
