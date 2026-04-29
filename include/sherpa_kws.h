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

#include "sherpa-onnx/c-api/c-api.h"

typedef struct {
  std::unique_ptr<SherpaOnnxOnlineStream> online_stream;
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
  const SherpaOnnxKeywordSpotter* keyword_spotter_ptr_ = nullptr;
  std::string key_words_file_ = "";
};