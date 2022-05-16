#pragma once

#include <string>
#include <string_view>
#include <tuple>

struct Matrix {
    enum class ErrorStatus {
        kError,
        kNoError
    };

    static std::tuple<ErrorStatus, std::string> Process(std::string_view task, bool for_test = false);
};
