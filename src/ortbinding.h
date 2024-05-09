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

const int PZ_NUM_BOARD = 12 * 12 * (18 + 30 * 2);
const int PZ_NUM_POLICY = 12 * 12 * 30;

struct PZPrediction
{
    std::array<float, 1> value;
    std::array<float, PZ_NUM_POLICY> policy;
};

class PlayerZeroPredictor
{
private:
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Ort::Env env{ORT_LOGGING_LEVEL_INFO, "PlayerZero"};
    OrtCUDAProviderOptionsV2 *cuda_options = nullptr;
    Ort::SessionOptions session_options;
    Ort::Session *session = nullptr;

    PlayerZeroPredictor(const std::string &p_model_file);
    ~PlayerZeroPredictor();

public:
    static PlayerZeroPredictor *get(const std::string &p_model_file);
    static void unload_model(const std::string &p_model_file);

    void predict(std::array<float, PZ_NUM_BOARD> &board_input, std::array<float, PZ_NUM_POLICY> &mask_input, PZPrediction &prediciton);
};

#endif