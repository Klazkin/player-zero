#include "ortbinding.h"
#include "onnxruntime_c_api.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ORTBinding::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("dummy_method"), &ORTBinding::dummy_method);
}

void ORTBinding::dummy_method()
{
    const OrtApi *g_ort = NULL;
    g_ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
    if (!g_ort)
    {
        fprintf(stderr, "Failed to init ONNX Runtime engine.\n");
        return;
    }

    OrtEnv *env;
    g_ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "test", &env);
    if (env != NULL)
    {
        printf("Enviroment running.");
    }
}

ORTBinding::ORTBinding()
{
}

ORTBinding::~ORTBinding()
{
}
