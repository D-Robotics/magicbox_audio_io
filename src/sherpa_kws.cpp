#include "sherpa_kws.h"
#include <chrono>
#include <iostream>
#include <unistd.h>
#include <limits.h>

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

  sherpa_onnx::ParseOptions po(kUsageMessage);
  sherpa_onnx::KeywordSpotterConfig config;

  
  std::string i8_suffix = "";
  if (use_int8 == true){
    std::string i8_suffix = ".int8";
  }
  std::string name = GetProgramPath();
  std::vector<std::string> args = {
      name,
      "--encoder=" + config_path + "/encoder-epoch-12-avg-2-chunk-16-left-64" + i8_suffix + ".onnx",
      "--decoder=" + config_path + "/decoder-epoch-12-avg-2-chunk-16-left-64" + i8_suffix + ".onnx",
      "--joiner=" + config_path + "/joiner-epoch-12-avg-2-chunk-16-left-64" + i8_suffix + ".onnx",
      "--tokens=" + config_path + "/tokens.txt",
      "--keywords-file=" + config_path + "/keywords.txt",
      ""
  };
  size_t pos = args[5].find('=');
  if (pos != std::string::npos && pos + 1 < args[5].size()) {
      key_words_file_ = args[5].substr(pos + 1);
  } else {
      std::cerr << "Path not found" << std::endl;
      return;
  }
  
  std::vector<char*> argv2;
  for (auto& s : args) {
      argv2.push_back(const_cast<char*>(s.c_str()));
  }
  int argc2 = static_cast<int>(argv2.size());

  config.Register(&po);
  po.Read(argc2, argv2.data());
  if (po.NumArgs() < 1) {
    po.PrintUsage();
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "%s\n", config.ToString().c_str());

  if (!config.Validate()) {
    fprintf(stderr, "Errors in config!\n");
  }

  keyword_spotter_ptr_ = std::make_unique<sherpa_onnx::KeywordSpotter>(config);
}


//关键词检测
std::string SherpaKWS::GetKeyWord(std::vector<float> &samples){
  int32_t sampling_rate = 16000;

  auto s = keyword_spotter_ptr_->CreateStream();
  s->AcceptWaveform(sampling_rate, samples.data(), samples.size());

  std::vector<float> tail_paddings(static_cast<int>(0.8 * sampling_rate));
  s->AcceptWaveform(sampling_rate, tail_paddings.data(),
                    tail_paddings.size());

  s->InputFinished();
  std::string result = "";
  while (keyword_spotter_ptr_->IsReady(s.get())) {
    keyword_spotter_ptr_->DecodeStream(s.get());

    auto r = keyword_spotter_ptr_->GetResult(s.get());
    if (!r.keyword.empty()) {
      keyword_spotter_ptr_->Reset(s.get());
      result = r.keyword;
      return result;
    }
  }
  return result;
}

//获取关键词列表
bool SherpaKWS::ExtractChineseFromFile(std::vector<std::string> &key_words_list) {
    std::ifstream fin(key_words_file_);
    if (!fin.is_open()) {
        std::cerr << "无法打开文件: " << key_words_file_ << std::endl;
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