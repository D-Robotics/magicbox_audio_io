// sherpa-onnx/csrc/sherpa-onnx-keyword-spotter.cc
//
// Copyright (c)  2023-2024  Xiaomi Corporation

#include <stdio.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "sherpa-onnx/csrc/keyword-spotter.h"
#include "sherpa-onnx/csrc/online-stream.h"
#include "sherpa-onnx/csrc/parse-options.h"
#include "sherpa-onnx/csrc/wave-reader.h"

typedef struct {
  std::unique_ptr<sherpa_onnx::OnlineStream> online_stream;
  std::string filename;
} Stream;


class SherpaKWS{
public:
  SherpaKWS(){}
  void Init(const std::string &config_path, bool use_int8);
  std::string GetKeyWord(std::vector<float> &samples);
  bool ExtractChineseFromFile(std::vector<std::string> &key_words_list);
  ~SherpaKWS(){}
private:
  std::unique_ptr<sherpa_onnx::KeywordSpotter> keyword_spotter_ptr_;
  std::string key_words_file_ = "";
};