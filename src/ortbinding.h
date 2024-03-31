#ifndef ORTBinding_H
#define ORTBinding_H

#include <godot_cpp/classes/ref_counted.hpp>
#include "onnxruntime_cxx_api.h"

float _predict(std::array<float, 12> input);

namespace godot
{
    class ORTBinding : public RefCounted
    {
        GDCLASS(ORTBinding, RefCounted);

    protected:
        static void _bind_methods();

    public:
        ORTBinding();
        ~ORTBinding();

        float predict(const TypedArray<float> &p_array);
        static float ORTBinding::dueler_predict(std::array<float, 114> input);
        std::array<float, 2> ORTBinding::winner_predict(std::array<float, 96> input);
    };
}

class WinPredictor
{
private:
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Ort::Env env{ORT_LOGGING_LEVEL_INFO, "WinPredictor"};
    // Ort::Session session_{env, L"wpred.onnx", Ort::SessionOptions{nullptr}};
    OrtCUDAProviderOptionsV2 *cuda_options = nullptr;
    // OrtDirect
    Ort::SessionOptions session_options;
    Ort::Session *session = nullptr;

public:
    WinPredictor();
    ~WinPredictor();
    std::array<float, 2> predict(std::array<float, 96> input);
};

struct PZPrediction
{
    std::array<float, 1> &value;
    std::array<float, 2880> &policy;
};

class PlayerZeroPredictor
{
private:
    static PlayerZeroPredictor *instance;

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Ort::Env env{ORT_LOGGING_LEVEL_INFO, "PlayerZero"};
    OrtCUDAProviderOptionsV2 *cuda_options = nullptr;
    Ort::SessionOptions session_options;
    Ort::Session *session = nullptr;

    PlayerZeroPredictor();
    ~PlayerZeroPredictor();

public:
    static PlayerZeroPredictor *get();

    // input space is 12 * 12 * 18, output space is 1 + 12 * 12 * 20,
    PZPrediction predict(std::array<float, 2592> input);
};

#endif