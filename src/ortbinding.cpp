#include "ortbinding.h"

#include <assert.h>
#include <godot_cpp/core/class_db.hpp>
#include <array>
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace godot;

void ORTBinding::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("predict", "p_array"), &ORTBinding::predict);
}

float _predict(std::array<float, 12> input)
{
    std::array<float, 1> output{};

    // std::fill(input.begin(), input.end(), 0.f);
    // std::fill(output.begin(), output.end(), 0.f);

    Ort::Env env{ORT_LOGGING_LEVEL_VERBOSE};
    Ort::Session session_{env, L"model.onnx", Ort::SessionOptions{nullptr}};

    Ort::Value input_tensor_{nullptr};
    std::array<int64_t, 2> input_shape_{1, 12};

    Ort::Value output_tensor_{nullptr};
    std::array<int64_t, 2> output_shape_{1, 1};

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input.data(), input.size(), input_shape_.data(), input_shape_.size());
    output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, output.data(), output.size(), output_shape_.data(), output_shape_.size());

    const char *input_names[] = {"input_12"}; // TODO define proper names for input/output tensors
    const char *output_names[] = {"dense_14"};

    Ort::RunOptions run_options;
    session_.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);

    return output[0];
}

float ORTBinding::dueler_predict(std::array<float, 114> input)
{
    std::array<float, 1> output{};

    Ort::Env env{ORT_LOGGING_LEVEL_VERBOSE};
    Ort::Session session_{env, L"dueler_model.onnx", Ort::SessionOptions{nullptr}};

    Ort::Value input_tensor_{nullptr};
    std::array<int64_t, 2> input_shape_{1, 114};

    Ort::Value output_tensor_{nullptr};
    std::array<int64_t, 2> output_shape_{1, 1};

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input.data(), input.size(), input_shape_.data(), input_shape_.size());
    output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, output.data(), output.size(), output_shape_.data(), output_shape_.size());

    const char *input_names[] = {"inputs"};
    const char *output_names[] = {"activation"};

    Ort::RunOptions run_options;
    session_.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);

    return output[0];
}

ORTBinding::ORTBinding()
{
}

std::array<float, 2> ORTBinding::winner_predict(std::array<float, 96> input)
{
    std::array<float, 2> output{};

    Ort::Env env{ORT_LOGGING_LEVEL_VERBOSE};
    Ort::Session session_{env, L"wpred.onnx", Ort::SessionOptions{nullptr}};

    Ort::Value input_tensor_{nullptr};
    std::array<int64_t, 2> input_shape_{1, 96};

    Ort::Value output_tensor_{nullptr};
    std::array<int64_t, 2> output_shape_{1, 2};

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input.data(), input.size(), input_shape_.data(), input_shape_.size());
    output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, output.data(), output.size(), output_shape_.data(), output_shape_.size());

    const char *input_names[] = {"input_19"};
    const char *output_names[] = {"re_lu_28"};

    Ort::RunOptions run_options;
    session_.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);
    return output;
}

float ORTBinding::predict(const TypedArray<float> &p_array)
{
    if (p_array.size() != 12)
    {
        return -1.f;
    }

    std::array<float, 12> input{};

    for (int i = 0; i < 12; i++)
    {
        input[i] = (float)p_array[i];
    }

    return _predict(input);
}

ORTBinding::~ORTBinding()
{
}

WinPredictor::WinPredictor()
{
    std::cout << "creating cuda prov options";
    Ort::GetApi().CreateCUDAProviderOptions(&cuda_options);

    std::vector<const char *> keys{"device_id", "gpu_mem_limit", "arena_extend_strategy", "cudnn_conv_algo_search", "do_copy_in_default_stream", "cudnn_conv_use_max_workspace", "cudnn_conv1d_pad_to_nc1d"};
    std::vector<const char *> values{"0", "2147483648", "kSameAsRequested", "DEFAULT", "1", "1", "1"};

    Ort::GetApi().UpdateCUDAProviderOptions(cuda_options, keys.data(), values.data(), keys.size());
    Ort::GetApi().SessionOptionsAppendExecutionProvider_CUDA_V2(session_options, cuda_options);

    // Ort::Session session{env, L"wpred.onnx", session_options};
    session = new Ort::Session(env, L"wpred.onnx", session_options);
    std::cout << "finished settings session options\n";
}

WinPredictor::~WinPredictor()
{
    Ort::GetApi().ReleaseCUDAProviderOptions(cuda_options);
}

std::array<float, 2> WinPredictor::predict(std::array<float, 96> input)
{
    // std::cout << "starting inference\n";
    std::array<float, 2> output{};

    Ort::Value input_tensor_{nullptr};
    std::array<int64_t, 2> input_shape_{1, 96};

    Ort::Value output_tensor_{nullptr};
    std::array<int64_t, 2> output_shape_{1, 2};

    input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input.data(), input.size(), input_shape_.data(), input_shape_.size());
    output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, output.data(), output.size(), output_shape_.data(), output_shape_.size());

    const char *input_names[] = {"input_19"};
    const char *output_names[] = {"re_lu_28"};

    session->Run(Ort::RunOptions{}, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);
    // std::cout << "finished inference\n";
    return output;
}

PlayerZeroPredictor *PlayerZeroPredictor::instance = nullptr;

PlayerZeroPredictor *PlayerZeroPredictor::get()
{
    if (instance == nullptr)
    {
        instance = new PlayerZeroPredictor();
    }
    return instance;
}

void PlayerZeroPredictor::reload_model()
{
    delete instance;
    instance = nullptr;
}

PlayerZeroPredictor::PlayerZeroPredictor()
{
    std::cout << "Creating cuda prov options.\n";
    Ort::GetApi().CreateCUDAProviderOptions(&cuda_options);

    std::vector<const char *> keys{"device_id", "gpu_mem_limit", "arena_extend_strategy", "cudnn_conv_algo_search", "do_copy_in_default_stream", "cudnn_conv_use_max_workspace", "cudnn_conv1d_pad_to_nc1d"};
    std::vector<const char *> values{"0", "2147483648", "kSameAsRequested", "DEFAULT", "1", "1", "1"};

    Ort::GetApi().UpdateCUDAProviderOptions(cuda_options, keys.data(), values.data(), keys.size());
    Ort::GetApi().SessionOptionsAppendExecutionProvider_CUDA_V2(session_options, cuda_options);

    session = new Ort::Session(env, L"player_zero.onnx", session_options); // todo change model file name
    std::cout << "Finished settings session options.\n";
}

PlayerZeroPredictor::~PlayerZeroPredictor()
{
    Ort::GetApi().ReleaseCUDAProviderOptions(cuda_options);
}

void PlayerZeroPredictor::predict(std::array<float, PZ_NUM_BOARD> &board_input, std::array<float, PZ_NUM_POLICY> &mask_input, PZPrediction &prediciton)
{
    std::array<int64_t, 2> input_shape_1{1, PZ_NUM_BOARD};
    std::array<int64_t, 2> input_shape_2{1, PZ_NUM_POLICY};

    std::array<int64_t, 2> output_shape_1{1, 1};
    std::array<int64_t, 2> output_shape_2{1, PZ_NUM_POLICY};

    const char *input_names[] = {"board_input", "mask_input"};
    const char *output_names[] = {"value", "policy"};

    std::vector<Ort::Value> input_tensors;
    std::vector<Ort::Value> output_tensors;

    input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, board_input.data(), board_input.size(), input_shape_1.data(), input_shape_1.size()));
    input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, mask_input.data(), mask_input.size(), input_shape_2.data(), input_shape_2.size()));
    output_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, prediciton.value.data(), prediciton.value.size(), output_shape_1.data(), output_shape_1.size()));
    output_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, prediciton.policy.data(), prediciton.policy.size(), output_shape_2.data(), output_shape_2.size()));

    session->Run(Ort::RunOptions{nullptr}, input_names, input_tensors.data(), 2, output_names, output_tensors.data(), 2);
}
