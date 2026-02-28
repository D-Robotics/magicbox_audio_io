#include "sherpa_tts.h"
#include <chrono>
#include <iostream>

SherpaTTS::SherpaTTS(): po_(""){
}

static std::string GetProgramPath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return std::string(result, count);
    }
    return "";
}

void SherpaTTS::Init(std::string &device_name, std::string &config_path){
    sherpa_onnx::ParseOptions po_("");
    int32_t sid = 0;
    po_.Register("device-name", &device_name, "ALSA playback device name");
    po_.Register("sid", &sid, "Speaker ID");
    sherpa_onnx::OfflineTtsConfig config;
    config.Register(&po_);
    std::string name = GetProgramPath();
    std::vector<std::string> args = {
        name,
        "--num-threads=4",
        "--matcha-acoustic-model=" + config_path + "/model-steps-3.onnx",
        "--matcha-vocoder=" + config_path + "/vocos-22khz-univ.onnx",
        "--matcha-lexicon=" + config_path + "/lexicon.txt",
        "--matcha-tokens=" + config_path + "/tokens.txt",
        "--matcha-dict-dir=" + config_path + "/dict",
        "--debug=0",
        "--tts-rule-fsts=" + config_path + "/phone.fst," +
                           config_path + "/date.fst," +
                           config_path + "/number.fst"
    };
    std::vector<char*> argv2;
    for (auto& s : args) {
        argv2.push_back(const_cast<char*>(s.c_str()));
    }
    int argc2 = static_cast<int>(argv2.size());

    po_.Read(argc2, argv2.data());

    if (!config.Validate()) {
        std::cerr << "Invalid config!\n";
    }
    tts_ptr_ = std::make_shared<sherpa_onnx::OfflineTts>(config);
}