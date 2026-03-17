// Copyright (c) 2024，D-Robotics.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDE_HBAUDIOIO_H_
#define INCLUDE_HBAUDIOIO_H_

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <alsa/asoundlib.h>

#include "sherpa_tts.h"
#include "utils/alsa_device.h"

#include "audio_msg/msg/smart_audio_data.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"
#include "lighting_control/ws2812b.h"

namespace hobot {
namespace audio {

using rclcpp::NodeOptions;
class HBAudioIo : public rclcpp::Node {
 public:
  // node_name为创建的节点名，options为选项，用法和ROS Node相同
  HBAudioIo(const std::string& node_name,
                 const NodeOptions& options = NodeOptions());

  virtual ~HBAudioIo();
  int Run();

 public:
  int Init();
  int DeInit();
  int Start();
  int Stop();
  void LoadTtsModelAsync();
 private:
  int MicphoneGetThread();
  int SpeakerThread();
  int TTSThread();
  void asr_send_th();
  void TTSMsgCallback(const std_msgs::msg::String::SharedPtr msg);
  void CheckLLMNodeExistence();
  void PubASRDataFunc(std::string cmd_word, std::string key_word);
  void AudioASRFunc(std::string asr);
  void StatusServiceHandle(const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
      std::shared_ptr<std_srvs::srv::Trigger::Response> response);

 private:
  int micphone_enable_ = 1;
  std::shared_ptr<std::thread> micphone_thread_;
  alsa_device_t* micphone_device_ = nullptr;
  alsa_device_t* speaker_device_ = nullptr;
  int audio_num_ = 0;
  std::string micphone_name_ = "plughw:0,0";
  int micphone_rate_ = 16000;
  int micphone_chn_ = 2;
  int micphone_buffer_time_ = 0;
  int micphone_nperiods_ = 4;
  int micphone_period_size_ = 512;
  int voip_mode_ = 0;
  int mic_type_ = 0;
  int asr_output_mode_ = 0;
  int asr_output_channel_ = 3;

  // audio_sdk_path_ will be updated at runtime with env "TROS_DISTRO"
  std::string pkg_name_ = "audio_io";
  std::string asr_model_path_ = "/userdata/magicbox/config/sense-voice-small-fp16.gguf";
  std::string kws_config_path_ = "/userdata/magicbox/dep/sherpa-onnx/sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01";
  std::string audio_pub_topic_name_ = "/audio_smart";
  std::string asr_pub_topic_name_ = "/prompt_text";
  std::string tts_sub_topic_name_ = "/tts_text";
  std::string tts_config_path_ = "/userdata/magicbox/dep/matcha-icefall-zh-baker";
  std::ofstream audio_infile_;
  std::ofstream audio_sdk_;
  bool continuous_wake_mode_ = false; 
  bool save_audio_ = false;
  void* tts_ = nullptr;
  char* pcm_data_ = nullptr;
  std::queue<PlaybackItem> playback_queue_;

  std::mutex tts_queue_mtx_;
  std::mutex micphone_mtx_;
  std::mutex playback_queue_mtx_;
  std::mutex llm_node_init_mtx_;

  std::condition_variable tts_queue_cv_;
  std::condition_variable micphone_cv_;
  std::condition_variable playback_queue_cv_;
  std::condition_variable llm_node_init_cv_;

  bool llm_node_init_ = false;
  bool wait_for_llm_ = true;
  bool micphone_stop_ = true;
  std::atomic<bool> publish_{true};

  bool exit_ = true;
  bool is_init_ = false;
  bool exiting_ = false;


  std::string tts_msg_ = "";
  std::queue<std::string> tts_data_queue_;
  rclcpp::Publisher<audio_msg::msg::SmartAudioData>::SharedPtr msg_publisher_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr asr_msg_publisher_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr tts_msg_subscriber_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr status_service_;
  WS2812B lamp;
  SherpaTTS sherpa_tts_;
  std::shared_ptr<sherpa_onnx::AlsaPlay> alsa_ = nullptr;
  bool start_run_ = false;
  std::string language_type_ = "zh";
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace audio
}  // namespace hobot

#endif
