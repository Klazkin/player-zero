#include "ortbinding.h"
#include "onnxruntime_cxx_api.h"
#include <assert.h>
#include <godot_cpp/core/class_db.hpp>
#include <array>
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace godot;

void ORTBinding::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("dummy_method"), &ORTBinding::dummy_method);
}

void ORTBinding::dummy_method()
{
    _procedure();
}

void _procedure()
{
    std::array<float, 12> input{};
    std::array<float, 1> output{};

    std::fill(input.begin(), input.end(), 0.f);
    std::fill(output.begin(), output.end(), 0.f);

    Ort::Env env{ORT_LOGGING_LEVEL_VERBOSE};
    Ort::Session session_{env, L"model.onnx", Ort::SessionOptions{nullptr}};

    Ort::Value input_tensor_{nullptr};
    std::array<int64_t, 2> input_shape_{1, 12};

    Ort::Value output_tensor_{nullptr};
    std::array<int64_t, 2> output_shape_{1, 1};

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input.data(), input.size(), input_shape_.data(), input_shape_.size());
    output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, output.data(), output.size(), output_shape_.data(), output_shape_.size());

    const char *input_names[] = {"input_12"};
    const char *output_names[] = {"dense_14"};

    Ort::RunOptions run_options;
    session_.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);

    std::cout << output[0] << std::endl;
}

ORTBinding::ORTBinding()
{
}

ORTBinding::~ORTBinding()
{
}
