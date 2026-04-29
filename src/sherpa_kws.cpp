#include "sherpa_kws.h"
#include <chrono>
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <cstring>
#include <fstream>

static std::string GetProgramPath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return std::string(result, count);
    }
    return "";
}

void SherpaKWS::Init(const std::string &config_path, bool use_int8){
  const char *kUsageMessage = R"usage(
    Usage:

    (1) Streaming transducer

    ./bin/sherpa-onnx-keyword-spotter \
        --tokens=/path/to/tokens.txt \
        --encoder=/path/to/encoder.onnx \
        --decoder=/path/to/decoder.onnx \
        --joiner=/path/to/joiner.onnx \
        --provider=cpu \
        --num-threads=2 \
        --keywords-file=keywords.txt \
        /path/to/foo.wav [bar.wav foobar.wav ...]

    Note: It supports decoding multiple files in batches

    Default value for num_threads is 2.
    Valid values for provider: cpu (default), cuda, coreml.
    foo.wav should be of single channel, 16-bit PCM encoded wave file; its
    sampling rate can be arbitrary and does not need to be 16kHz.

    Please refer to
    https://k2-fsa.github.io/sherpa/onnx/pretrained_models/index.html
    for a list of pre-trained models to download.
    )usage";

  SherpaOnnxKeywordSpotterConfig config;
  memset(&config, 0, sizeof(config));
  std::string i8_suffix = "";
  if (use_int8 == true){
    std::string i8_suffix = ".int8";
  }
  std::string encoder =
      config_path + "/encoder-epoch-12-avg-2-chunk-16-left-64" +
      i8_suffix + ".onnx";
  std::string decoder =
      config_path + "/decoder-epoch-12-avg-2-chunk-16-left-64" +
      i8_suffix + ".onnx";
  std::string joiner =
      config_path + "/joiner-epoch-12-avg-2-chunk-16-left-64" +
      i8_suffix + ".onnx";
  std::string tokens =
      config_path + "/tokens.txt";
  std::string keywords_file =
      config_path + "/keywords.txt";

  config.model_config.transducer.encoder = encoder.c_str();
  config.model_config.transducer.decoder = decoder.c_str();
  config.model_config.transducer.joiner  = joiner.c_str();

  config.model_config.tokens = tokens.c_str();
  config.keywords_file       = keywords_file.c_str();

  config.model_config.provider = "cpu";
  // config.model_config.num_threads = 1;
  // config.model_config.debug = 1;

  key_words_file_ = keywords_file;

  keyword_spotter_ptr_ = SherpaOnnxCreateKeywordSpotter(&config);
}


//关键词检测
std::string SherpaKWS::GetKeyWord(std::vector<float> &samples){
  int32_t sampling_rate = 16000;

  const SherpaOnnxOnlineStream *stream = SherpaOnnxCreateKeywordStream(keyword_spotter_ptr_);
  SherpaOnnxOnlineStreamAcceptWaveform(stream, sampling_rate, samples.data(), samples.size());

  std::vector<float> tail_paddings(static_cast<int>(0.8 * sampling_rate));
  SherpaOnnxOnlineStreamAcceptWaveform(stream, sampling_rate, tail_paddings.data(),
                                       tail_paddings.size());
  SherpaOnnxOnlineStreamInputFinished(stream);
  std::string result = "";
  while (SherpaOnnxIsKeywordStreamReady(keyword_spotter_ptr_, stream)) {
    SherpaOnnxDecodeKeywordStream(keyword_spotter_ptr_, stream);

    auto *r = SherpaOnnxGetKeywordResult(keyword_spotter_ptr_, stream);
    if (strlen(r->keyword) > 0) {
      SherpaOnnxResetKeywordStream(keyword_spotter_ptr_, stream);
      result = r->keyword;
      SherpaOnnxDestroyKeywordResult(r);
      return result;
    }
    SherpaOnnxDestroyKeywordResult(r);
  }
  return result;
}

//获取关键词列表
bool SherpaKWS::ExtractChineseFromFile(std::vector<std::string> &key_words_list) {
    std::ifstream fin(key_words_file_);
    if (!fin.is_open()) {
        std::cerr << "[ERROR] Unable to open the file: " << key_words_file_ << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(fin, line)) {
        size_t pos = line.find('@');
        if (pos != std::string::npos && pos + 1 < line.size()) {
            std::string chinese = line.substr(pos + 1);
            key_words_list.push_back(chinese);
        }
    }
    fin.close();
    return true;
}