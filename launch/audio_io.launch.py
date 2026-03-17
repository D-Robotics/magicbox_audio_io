# Copyright (c) 2024，D-Robotics.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node


def generate_launch_description():
    tts_config_path = PythonExpression([
        "'",
        LaunchConfiguration('tts_config_path'),
        "' if '",
        LaunchConfiguration('tts_config_path'),
        "' != '' else (",
        "'/userdata/magicbox/dep/matcha-icefall-zh-baker' if '",
        LaunchConfiguration('language_type'),
        "' == 'zh' else '/userdata/magicbox/dep/matcha-icefall-en_US-ljspeech'"
        ")"
    ])

    kws_config_path = PythonExpression([
        "'",
        LaunchConfiguration('kws_config_path'),
        "' if '",
        LaunchConfiguration('kws_config_path'),
        "' != '' else (",
        "'/userdata/magicbox/dep/sherpa-onnx/sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01' if '",
        LaunchConfiguration('language_type'),
        "' == 'zh' else '/userdata/magicbox/dep/sherpa-onnx/sherpa-onnx-kws-zipformer-gigaspeech-3.3M-2024-01-01'"
        ")"
    ])
    return LaunchDescription([
        SetEnvironmentVariable(
            'RMW_IMPLEMENTATION', 'rmw_cyclonedds_cpp'
        ),
        DeclareLaunchArgument(
            'micphone_name',
            default_value='plughw:0,0',
            description='audio capture id'),
        DeclareLaunchArgument(
            'asr_pub_topic_name',
            default_value='/prompt_text',
            description='hobot audio publish topic name'),
        DeclareLaunchArgument(
            'asr_model_path',
            default_value='/userdata/magicbox/config/sense-voice-small-fp16.gguf',
            description='ASR model path'),
        DeclareLaunchArgument(
            "continuous_wake_mode", 
            default_value="False",
            description='Is it set to continuous wake-up mode'),
         DeclareLaunchArgument(
            "wait_for_llm", 
            default_value="True",
            description='should we wait for the large model to start'),
         DeclareLaunchArgument(
            "language_type", 
            default_value="zh",
            description='Language type, only supports zh or en.'),
        DeclareLaunchArgument(
            'tts_config_path',
            default_value='',
            description='TTS config path'),
        DeclareLaunchArgument(
            'kws_config_path',
            default_value='',
            description='KWS config path'),
        # 启动音频采集pkg
        Node(
            package='audio_io',
            executable='audio_io',
            output='screen',
            parameters=[
                {"micphone_name": LaunchConfiguration('micphone_name')},
                {"asr_pub_topic_name": LaunchConfiguration('asr_pub_topic_name'),
                "tts_config_path": tts_config_path,
                "kws_config_path": kws_config_path,
                "asr_model_path": LaunchConfiguration('asr_model_path'),
                "continuous_wake_mode": LaunchConfiguration('continuous_wake_mode'),
                "wait_for_llm": LaunchConfiguration('wait_for_llm'),
                "language_type": LaunchConfiguration('language_type'),
                }
            ],
            arguments=['--ros-args', '--log-level', 'warn']
        )
    ])
