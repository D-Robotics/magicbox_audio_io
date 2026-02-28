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

#include <memory>
#include <string>

#include "hb_audio_io.h"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  RCLCPP_WARN(rclcpp::get_logger("audio_io"),
              "This is audio capture example!");

  std::string node_name = "audio_io";
  hobot::audio::HBAudioIo audio_io(node_name);
  if (audio_io.Init() == 0) {
    if (audio_io.Run() != 0) {
      RCLCPP_ERROR(rclcpp::get_logger("audio_io"),
                   "Run HBAudioIo failed!");
    } else {
      RCLCPP_INFO(rclcpp::get_logger("audio_io"),
                  "Run HBAudioIo done!");
    }
  } else {
    RCLCPP_ERROR(rclcpp::get_logger("audio_io"),
                 "Init HBAudioIo failed!");
  }

  rclcpp::shutdown();
  return 0;
}
