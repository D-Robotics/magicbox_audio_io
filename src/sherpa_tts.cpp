#include "sherpa_tts.h"
#include <chrono>
#include <iostream>
#include <fstream>

SherpaTTS::SherpaTTS() {
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
    int32_t sid = 0;
    SherpaOnnxOfflineTtsConfig config;
    memset(&config, 0, sizeof(config));
    std::string acoustic_model = config_path + "/model-steps-3.onnx";
    std::string vocoder        = config_path + "/vocos-22khz-univ.onnx";
    std::string lexicon        = config_path + "/lexicon.txt";
    std::string tokens         = config_path + "/tokens.txt";
    std::string dict_dir       = config_path + "/dict";
    std::string rule_fsts =
        config_path + "/phone.fst," +
        config_path + "/date.fst," +
        config_path + "/number.fst";

    config.model.matcha.acoustic_model = acoustic_model.c_str();
    config.model.matcha.vocoder        = vocoder.c_str();
    config.model.matcha.lexicon       = lexicon.c_str();
    config.model.matcha.tokens        = tokens.c_str();
    config.model.matcha.dict_dir      = dict_dir.c_str();
    config.rule_fsts = rule_fsts.c_str();
    config.model.provider = "cpu";
    config.model.num_threads = 8;
    // config.model.debug = 1;
    tts_ptr_ = SherpaOnnxCreateOfflineTts(&config);
    tts_cfg_.sid = 0;
    tts_cfg_.speed = 1.0f;  // larger -> faster in speech speed
    tts_cfg_.silence_scale = config.silence_scale;
}