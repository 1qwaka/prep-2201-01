#pragma once

#include <chrono>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>
#include <tuple>
#include <vector>
#include <unistd.h>
#include <iostream>

// #define DEBUG

#ifdef DEBUG
#include <algorithm>
// #include <iostream>
#include <iterator>
#define LOGGER std::cerr
#define LOG_MESSAGE LOGGER << "[DEBUG] " << __FILE__ << ":" << __LINE__ << ": "
#define LOG_ERROR LOGGER << "[ERROR] " << __FILE__ << ":" << __LINE__ << ": "
#endif

enum class ErrorStatus {
    kNoError,
    kError
};

template <typename MessageTypeEnum>
struct Message {
    using Type = MessageTypeEnum;
    Message(Type type = Type::kUnknown, std::string_view content = "") : type(type), content(content) {}
    Type type;
    std::string content;
};

enum class RequestType {
    kUnknown,
    kPost,
    kGet
};

enum class ResponseType {
    kUnknown,
    kNew,
    kWork,
    kError,
    kDone
};

using Request = Message<RequestType>;
using Response = Message<ResponseType>;

template <typename MessageType>
struct NetworkMessage {
    static std::tuple<ErrorStatus, MessageType> Deserialize(const std::vector<std::byte> &network_bytes);
    static std::vector<std::byte> Serialize(const MessageType &message);
};

template <typename MessageType>
std::vector<std::byte> NetworkMessage<MessageType>::Serialize(const MessageType &message) {
    std::vector<std::byte> network_data;
    // TODO(mu_username):- Implement me - нужно точно рассчитать количество байт и аккуратно скопировать их в массив
    ////
    std::byte* p = (std::byte*)&message.type;
    for (size_t i = 0; i < sizeof(message.type); ++i) {
        network_data.push_back(*(p + i));
    }
    for (size_t i = 0; i < message.content.size(); ++i) {
        network_data.push_back((std::byte)message.content[i]);
    }
    network_data.push_back((std::byte)'\0');
    ////
    return network_data;
}

template <typename MessageType>
std::tuple<ErrorStatus, MessageType> NetworkMessage<MessageType>::Deserialize(const std::vector<std::byte> &network_bytes) {
    // TODO(mu_username):- Implement me
    ////
    MessageType message{
        MessageType::Type::kUnknown,
        std::string{}
    };
    ErrorStatus error = ErrorStatus::kNoError;

    std::byte* p = (std::byte*)&message.type;
    size_t i = 0;
    for (; i < sizeof(message.type); ++i) {
        if (network_bytes.size() <= i) {
            error = ErrorStatus::kError;
            break;
        }
        *(p + i) = network_bytes[i];
    }
    for (; network_bytes[i] != (std::byte)'\0'; ++i) {
        // cout << "i: " << i << " = " << (char)network_bytes[i] << endl;
        if (network_bytes.size() <= i) {
            error = ErrorStatus::kError;
            break;
        }
        message.content.push_back((char)network_bytes[i]);
    }
    // cout << "i: " << i << " = " << (char)network_bytes[i] << endl;
    ////
    return std::make_tuple(error, message);
}

using NetworkRequest = NetworkMessage<Request>;
using NetworkResponse = NetworkMessage<Response>;

#ifdef DEBUG
inline std::ostream &operator<<(std::ostream &os, const Request::Type &type) {
    switch (type) {
        case Request::Type::kGet: { os << "Get"; break; }
        case Request::Type::kPost: { os << "Post"; break; }
        case Request::Type::kUnknown:
        default: { os << "Unknown"; break; }
    }
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const Request &request) {
    os << "Request {type=" << request.type << ", content=" << request.content << "}";
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const Response::Type &type) {
    switch (type) {
        case Response::Type::kNew: { os << "New"; break; }
        case Response::Type::kWork: { os << "Work"; break; }
        case Response::Type::kDone: { os << "Done"; break; }
        case Response::Type::kError: { os << "Error"; break; }
        case Response::Type::kUnknown:
        default: { os << "Unknown"; break; }
    }
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const Response &response) {
    os << "Response {type=" << response.type << ", content=" << response.content << "}";
    return os;
}
#endif

static constexpr int kBadFileDescriptor = -1;

struct AutoClosingFileDescriptor {
    AutoClosingFileDescriptor(int fd = kBadFileDescriptor) : fd_(fd) {
#ifdef DEBUG
            LOG_MESSAGE << "Opening fd " << fd_ << "\n";
#endif
    }

    ~AutoClosingFileDescriptor() {
        if (fd_ != kBadFileDescriptor) {
#ifdef DEBUG
            LOG_MESSAGE << "Closing fd " << fd_ << "\n";
#endif
            close(fd_);
        }
    }

    operator int() const { return fd_; }

  private:
    int fd_;
};

enum class ReadStatus {
    kNoData,
    kFailed,
    kSucceed
};

inline ReadStatus ReadFromNetwork(int fd, std::vector<std::byte> &bytes_to_read,
                           bool retry_if_no_data = true,
                           const std::chrono::seconds &timeout = std::chrono::seconds(0),
                           const std::chrono::milliseconds &retry_time_interval = std::chrono::milliseconds(100)) {

    // TODO(mu_username): нужно считать данные из сокета в network_bytes
    // если данных нет - вернуть ReadStatus::kNoData (если не стоит retry_if_no_data и не вышел таймаут)
    // если произошла ошибка - вернуть ReadStatus::kFailed
    ////
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    std::byte tmp_byte;
    while (true) {
        // cout << "start read" << endl;
        ssize_t r = read(fd, &tmp_byte, 1);
        if (r == -1 && errno == EWOULDBLOCK) {
            if (bytes_to_read.size() == 0) {
                // cout << "error: " << errno << endl;
                // cout << "EWUOLDBLOCK: " << EWOULDBLOCK << endl;
                // perror("lol");
                 if (retry_if_no_data && start - std::chrono::steady_clock::now() < timeout) {
                    std::cout << "time: " << (int)(start - std::chrono::steady_clock::now() < timeout) << std::endl;
                    std::this_thread::sleep_for(retry_time_interval);
                    continue;
                } else {
                    return ReadStatus::kNoData;
                }
                std::this_thread::sleep_for(retry_time_interval);
                continue;
            } else {
                return ReadStatus::kSucceed;
            }
        } else if (r == -1) {
            // perror("====ERROR====");
            return ReadStatus::kFailed;
        }
        // cout << "read " << r << " bytes: " << (int)tmp_byte << endl;

        if (r == 0 && retry_if_no_data) {
            // using std::chrono_literals::operator""ms;
            std::this_thread::sleep_for(retry_time_interval);
            continue;
        } else if (r == 0) {
            if (bytes_to_read.size() == 0) {
                return ReadStatus::kNoData;
            } else {
                return ReadStatus::kSucceed;
            }
        }

        bytes_to_read.push_back(tmp_byte);
        // cout << "end read" << endl;
    }
    
}

inline ErrorStatus SendToNetwork(int socket_fd, const std::vector<std::byte> &message_data) {
    // TODO(mu_username):- нужно отправить массив байт через сокет
    // если произошла ошибка - вернуть
    ////
    ssize_t msg_size = message_data.size();
    ssize_t wrote = write(socket_fd, message_data.data(), msg_size);
    if (wrote != msg_size) {
#ifdef DEBUG
        LOG_ERROR << "Failed to wrote " << msg_size << " bytes (only " << wrote <<" wrote) fd:" << socket_fd << "\n";
        perror("write error: ");
#endif
        return ErrorStatus::kError;
    }
    ////
    return ErrorStatus::kNoError;
}

enum FileDescriptorOptions : int {
    kFileDescriptor_NoOptions = 0,
    kFileDescriptor_NonBlocking = O_NONBLOCK
};

inline ErrorStatus ConfigureFileDescriptor(int fd, FileDescriptorOptions options = kFileDescriptor_NoOptions) {
    // TODO(mu_username):- Сконфигурируйте дескриптор через fcntl - например, чтобы сделать чтение неблокирующим
    ////
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | options);
    ////
    return ErrorStatus::kNoError;
}

static constexpr std::string_view kFromNetPipeTag = "from_net";
static constexpr std::string_view kToNetPipeTag = "to_net";

struct NamedPipe {
    static constexpr std::string_view kNamedPipePrefix = "named-pipe-";

    NamedPipe(std::filesystem::path pipe_path = kUninitializedPath) : pipe_path_(pipe_path) {}

    static std::string GetPerProcessPipeName(
        std::string_view working_directory,
        std::string_view tag,
        pid_t process_id = getpid());
    static std::tuple<ErrorStatus, NamedPipe> Create(const std::filesystem::path &pipe_path);
    static std::tuple<ErrorStatus, int> GetReader(
        const std::filesystem::path &pipe_path,
        FileDescriptorOptions options = kFileDescriptor_NoOptions);
    static std::tuple<ErrorStatus, int> GetWriter(
        const std::filesystem::path &pipe_path);

    void Remove();

    std::tuple<ErrorStatus, int> GetReader(FileDescriptorOptions options = kFileDescriptor_NoOptions) const;
    std::tuple<ErrorStatus, int> GetWriter() const;

    template <typename ObjectType>
    ErrorStatus Write(const ObjectType &object) const;

    template <typename StructType>
    ReadStatus Read(
        StructType &read_object,
        FileDescriptorOptions options = kFileDescriptor_NoOptions,
        bool retry_if_no_data = true,
        const std::chrono::milliseconds &retry_time_interval = std::chrono::milliseconds(100)) const;

  private:
    static constexpr std::string_view kUninitializedPath = "";

    std::filesystem::path pipe_path_;
};

inline std::tuple<ErrorStatus, int> NamedPipe::GetReader(
        const std::filesystem::path &pipe_path,
        FileDescriptorOptions options) {
    // TODO(mu_username):- нужно вернуть дескриптор для чтения (сконфигурированного с опциями) из именованного канала
    ////
    int fd = open(pipe_path.c_str(), O_RDONLY);
    ConfigureFileDescriptor(fd, options);
    ////
    return std::make_tuple(ErrorStatus::kNoError, fd);
}

inline std::tuple<ErrorStatus, int> NamedPipe::GetWriter(
        const std::filesystem::path &pipe_path) {
    // TODO(mu_username):- нужно вернуть дескриптор для записи из именованного канала
    ////
    int fd = open(pipe_path.c_str(), O_WRONLY, 0644);
    ////
    return std::make_tuple(ErrorStatus::kNoError, fd);
}

inline std::string NamedPipe::GetPerProcessPipeName(std::string_view working_directory, std::string_view tag, pid_t process_id) {
    return (std::filesystem::path{working_directory} / std::filesystem::path{
        std::string{kNamedPipePrefix} + std::string{tag} + std::to_string(process_id)
    }).string();
}

inline std::tuple<ErrorStatus, int> NamedPipe::GetReader(FileDescriptorOptions options) const {
    return GetReader(std::filesystem::path{pipe_path_}, options);
}

inline std::tuple<ErrorStatus, int> NamedPipe::GetWriter() const {
    return GetWriter(std::filesystem::path{pipe_path_});
}

inline std::tuple<ErrorStatus, NamedPipe> NamedPipe::Create(const std::filesystem::path &pipe_path) {
    // TODO(mu_username):- нужно создать именованный pipe
    ////
    NamedPipe np(pipe_path);
    ErrorStatus error = ErrorStatus::kNoError;
    int rc = mkfifo(pipe_path.c_str(), 0644);
    if (rc == -1) {
        // unlink(pipe_path.c_str());
        error = ErrorStatus::kError;
    }
    ////
    return std::make_tuple(error, np);
}

template <typename ObjectType>
ErrorStatus WriteToFileDescriptor(int fd, const ObjectType &object) {
    // TODO(mu_username):- запись цельного объекта (полезного набора байт) произвольного типа в дескриптор
    ////
    ssize_t size = sizeof(object);
    ssize_t wrote = write(fd, &object, size);
    if (wrote != size) {
        return ErrorStatus::kError;
    }
    ////
    return ErrorStatus::kNoError;
}

inline ErrorStatus WriteToFileDescriptor(int fd, const std::vector<std::byte> &bytes) {
    // TODO(mu_username):- запись массива байт в дескриптор
    ////
    ssize_t size = bytes.size();
    ssize_t wrote = write(fd, bytes.data(), size);
    if (wrote != size) {
        return ErrorStatus::kError;
    }
    ////
    return ErrorStatus::kNoError;
}

template <typename ObjectType>
ErrorStatus NamedPipe::Write(const ObjectType &object) const {
    // TODO(mu_username):- запись в инициализированный Named Pipe
    ////
    auto [error, fd] = GetWriter();
    if (error == ErrorStatus::kError) {
        close(fd);
        return ErrorStatus::kError;
    }
    error = WriteToFileDescriptor(fd, object);
    close(fd);
    return error;
    ////
}

inline void NamedPipe::Remove() {
    // TODO(mu_username):- удаление Named Pipe
    ////
    unlink(pipe_path_.c_str());
    ////
}

struct AutoClosableNamedPipe : NamedPipe {
    explicit AutoClosableNamedPipe(const NamedPipe &pipe_obj) : NamedPipe(pipe_obj), pipe_obj_(pipe_obj) {}
    ~AutoClosableNamedPipe() {
        pipe_obj_.Remove();
    }

    static std::tuple<ErrorStatus, AutoClosableNamedPipe> Create(const std::filesystem::path &pipe_path) {
        auto [error, named_pipe] = NamedPipe::Create(pipe_path);
        return std::make_tuple(error, AutoClosableNamedPipe(named_pipe));
    }

  private:
    NamedPipe pipe_obj_;
};

struct Pipe {
    Pipe(int read_fd = kBadFileDescriptor, int write_fd = kBadFileDescriptor,
         FileDescriptorOptions read_options = kFileDescriptor_NoOptions) :
            read_fd_(read_fd), write_fd_(write_fd), read_options_(read_options) {}

    template <typename ObjectType>
    ErrorStatus Write(const ObjectType &object) const;

    template <typename StructType>
    ReadStatus Read(
        StructType &read_object,
        bool retry_if_no_data = true,
        const std::chrono::milliseconds &retry_time_interval = std::chrono::milliseconds(100)) const;

    inline ErrorStatus Close();

    inline static std::tuple<ErrorStatus, Pipe> Create(FileDescriptorOptions read_options = kFileDescriptor_NoOptions);

    int ReaderFd() const { return read_fd_; }
    int WriterFd() const { return write_fd_; }

#ifdef DEBUG
    friend std::ostream &operator<<(std::ostream &os, const Pipe &pipe) {
        os << "Pipe<read_fd=" << pipe.read_fd_ << ", write_fd=" << pipe.write_fd_ << ">";
        return os;
    }
#endif

  private:
    int read_fd_ = kBadFileDescriptor;
    int write_fd_ = kBadFileDescriptor;
    FileDescriptorOptions read_options_ = kFileDescriptor_NoOptions;
};

template <typename ObjectType>
ErrorStatus Pipe::Write(const ObjectType &object) const {
    return WriteToFileDescriptor(write_fd_, object);
}

template <typename StructType>
ReadStatus ReadFromFileDescriptor(
        int fd,
        StructType &read_object,
        bool retry_if_no_data = true,
        const std::chrono::milliseconds &retry_time_interval = std::chrono::milliseconds(100)) {
    // TODO(mu_username): похоже на ReadFromNetwork, только для файлового дескриптора
    // read_object - цельный объект, который можно создать без параметров и заполнить единой операцией чтения
    ////
    // bytes_to_read.clear();
    std::byte tmp_byte;
    std::byte* start = (std::byte*)&read_object;
    int byte_pos = 0;
    while (true) {
        // cout << "start read" << endl;
        ssize_t r = read(fd, &tmp_byte, 1);
        if (r == -1 && errno == EWOULDBLOCK) {
            if (byte_pos < (int)sizeof(StructType)) {
                // cout << "error: " << errno << endl;
                // cout << "EWUOLDBLOCK: " << EWOULDBLOCK << endl;
                // perror("lol");
                if (retry_if_no_data) {
                    std::this_thread::sleep_for(retry_time_interval);
                    continue;
                } else {
                    return ReadStatus::kNoData;
                }
            } else {
                return ReadStatus::kSucceed;
            }
        } else if (r == -1) {
            // perror("====ERROR====");
            return ReadStatus::kFailed;
        }
        // cout << "read " << r << " bytes: " << (int)tmp_byte << endl;

        if (r == 0 && retry_if_no_data) {
            // using std::chrono_literals::operator""ms;
            std::this_thread::sleep_for(retry_time_interval);
            continue;
        } else if (r == 0) {
            if (byte_pos == 0) {
                return ReadStatus::kNoData;
            } else {
                return ReadStatus::kSucceed;
            }
        }

        if (byte_pos >= (int)sizeof(StructType)) {
#ifdef DEBUG
            LOG_ERROR << "There is extra data in file descriptor after read" << "\n";
#endif
            return ReadStatus::kSucceed;
        }
        
        *(start + byte_pos) = tmp_byte;
        byte_pos++;
        // cout << "end read" << endl;
    }
    ////
}

template <>
inline ReadStatus ReadFromFileDescriptor<std::vector<std::byte>>(
        int fd,
        std::vector<std::byte> &bytes_to_read,
        bool retry_if_no_data,
        const std::chrono::milliseconds &retry_time_interval) {
    // TODO(mu_username):- частный случай ReadFromFileDescriptor, когда объект не простой, а представляет из себя vector<std::byte>
    ////
    // bytes_to_read.clear();
    std::byte tmp_byte;
    while (true) {
        // cout << "start read" << endl;
        ssize_t r = read(fd, &tmp_byte, 1);
        if (r == -1 && errno == EWOULDBLOCK) {
            if (bytes_to_read.size() == 0 && retry_if_no_data) {
                // cout << "error: " << errno << endl;
                // cout << "EWUOLDBLOCK: " << EWOULDBLOCK << endl;
                // perror("lol");
                std::this_thread::sleep_for(retry_time_interval);
                continue;
            } else {
                return ReadStatus::kSucceed;
            }
        } else if (r == -1) {
            // perror("====ERROR====");
            return ReadStatus::kFailed;
        }
        // cout << "read " << r << " bytes: " << (int)tmp_byte << endl;

        if (r == 0 && retry_if_no_data) {
            // using std::chrono_literals::operator""ms;
            std::this_thread::sleep_for(retry_time_interval);
            continue;
        } else if (r == 0) {
            if (bytes_to_read.size() == 0) {
                return ReadStatus::kNoData;
            } else {
                return ReadStatus::kSucceed;
            }
        }

        bytes_to_read.push_back(tmp_byte);
        // cout << "end read" << endl;
    }
    
    ////
}

template <typename StructType>
ReadStatus NamedPipe::Read(
        // а где параметр timeout?
        StructType &read_object,
        FileDescriptorOptions options,
        bool retry_if_no_data,
        const std::chrono::milliseconds &retry_time_interval) const {
    // TODO(mu_username):- чтение из именованного канала
    ////
    auto [error, fd] = GetReader(options);
    if (error == ErrorStatus::kError) {
        close(fd);
        return ReadStatus::kFailed;
    }
    ReadStatus read_error = ReadFromFileDescriptor(fd, read_object, retry_if_no_data, retry_time_interval);
    close(fd);
    return read_error;
    ////
}

template <typename StructType>
ReadStatus Pipe::Read(
        StructType &read_object,
        bool retry_if_no_data,
        const std::chrono::milliseconds &retry_time_interval) const {
    // TODO(mu_username):- чтение из неименованного канала (внимательно убедитесь на тестовых примерах, в чём разница неблокирующего чтения
    // из именованного и неименованного канала в зависимости от того, начато ли оно раньше, чем была запись, или если чтение
    // осуществляется меньшими "кусками", чем было записано, или если было несколько раз что-то записано, а потом за одну-несколько
    // итераций - считано
    // Эксперименты и понимание корректного чтения лишат вас огромного количества глупых ошибок
    ////
    ReadStatus error = ReadFromFileDescriptor(read_fd_, read_object, retry_if_no_data, retry_time_interval);
    return error;
    ////
}

ErrorStatus Pipe::Close() {
    // TODO(mu_username):- все ресурсы нужно освобождать после работы с ними. Pipe (обёртка над int fd[2]) - не исключение
    ////
    bool close_read = close(read_fd_) == 0;
    if (close_read) {
        read_fd_ = kBadFileDescriptor;
    }

    bool close_write = close(write_fd_) == 0;
    if (close_write) {
        write_fd_ = kBadFileDescriptor;
    }

    bool success = close_read && close_write;
    return success ? ErrorStatus::kNoError : ErrorStatus::kError;
    ////
}

std::tuple<ErrorStatus, Pipe> Pipe::Create(FileDescriptorOptions read_options) {
    // TODO(mu_username):- создайте удобную обёртку над int fd[2] и необходимостью явно вызывать pipe во внешнем коде
    ////
    ErrorStatus error = ErrorStatus::kError;
    Pipe pip;
    int fds[2];
    bool success = pipe(fds) == 0;

    if (success) {
        ConfigureFileDescriptor(fds[0], read_options);
        pip.read_fd_ = fds[0];
        pip.write_fd_ = fds[1];
        pip.read_options_ = read_options;
        error = ErrorStatus::kNoError;
    }

    return std::make_tuple(error, pip);
    ////
}

struct AutoClosingPipe : Pipe {
    explicit AutoClosingPipe(const Pipe &pipe_obj) : Pipe(pipe_obj), pipe_obj_(pipe_obj) {}
    ~AutoClosingPipe() {
        pipe_obj_.Close();
    }

  private:
    Pipe pipe_obj_;
};

