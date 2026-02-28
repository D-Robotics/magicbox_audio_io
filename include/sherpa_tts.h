#ifndef INCLUDE_SHERPA_TTS_H_
#define INCLUDE_SHERPA_TTS_H_
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <csignal>
#include <memory>
#include <future>
#include <sstream>

#include "sherpa-onnx/csrc/offline-tts.h"
#include "sherpa-onnx/csrc/alsa-play.h"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;

struct PlaybackItem {
    std::vector<float> samples;
    int32_t sample_rate;
    bool playback;
    std::shared_ptr<std::promise<std::string>> promise; // 播放完后 set_value(...)
};

class SherpaTTS{
  public:
    SherpaTTS();
    void Init(std::string &device_name, std::string &config_path);
    ~SherpaTTS(){}
    std::shared_ptr<sherpa_onnx::OfflineTts> tts_ptr_ = nullptr;
  private:
    sherpa_onnx::ParseOptions po_;
    int32_t sid_ = 0;
};


#endif //INCLUDE_SHERPA_TTS_H_