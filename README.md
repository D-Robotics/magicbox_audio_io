# 功能介绍

audio_io功能包用于音频处理，ASR使用[SenseVoice](https://www.modelscope.cn/models/lovemefan/SenseVoiceGGUF)，[TTS](https://forum.d-robotics.cc/t/topic/33355)以及[KWS](https://forum.d-robotics.cc/t/topic/33616)使用[sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx)，实现以下两条链路：
- 麦克风获取   --> ASR+KWS --> 发布语音识别内容
- 接收文字内容 -->   TTS   --> 扬声器输出


# 板端编译

1、编译环境确认

- 板端已安装X5 Ubuntu系统。
- 当前编译终端已设置TogetherROS环境变量：`source PATH/setup.bash`。其中PATH为TogetherROS的安装路径。
- 已安装ROS2编译工具colcon。安装的ROS不包含编译工具colcon, 需要手动安装colcon。colcon安装命令：`pip install -U colcon-common-extensions`

2、依赖安装
```shell
apt install libfst-dev ros-humble-std-srvs
```

3、编译依赖

链接第三方仓库 [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx.git):
 
```shell
git clone https://github.com/k2-fsa/sherpa-onnx.git
cd sherpa-onnx  
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j6
# 链接sherpa-onnx到工程目录下，路径自行配置
cd audio_io && ln -s sherpa-onnx_path .

# 拉取TTS模型，路径可通过tts_config_path配置
wget https://github.com/k2-fsa/sherpa-onnx/releases/download/tts-models/matcha-icefall-zh-baker.tar.bz2
tar xvf matcha-icefall-zh-baker.tar.bz2
rm matcha-icefall-zh-baker.tar.bz2

# 拉取KWS模型，路径可通过kws_config_path配置
wget https://github.com/k2-fsa/sherpa-onnx/releases/download/kws-models/sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01.tar.bz2
tar xf sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01.tar.bz2
rm sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01.tar.bz2
```

4、编译
```shell
colcon build --packages-select audio_io

#不支持SPI灯光
colcon build --packages-select audio_io --cmake-args -DUSE_LIGHT_CONTROL=OFF
```


# 使用方法

## 准备工作

在体验之前，需要具备以下基本条件：

- RDK X5已烧录好Ubuntu 22.04系统镜像
- 确保MagicBox的麦克风扬声器能够正常使用
- 或者USB音响正确连接到RDK X5的usb接口、音频板正确连接到RDK X5的3.5mm的耳麦接口


## 运行智能语音程序

启动audio_io package：

`ros2 launch audio_io audio_io.launch.py`

## 功能说明
- 该功能于qwen_llm功能包一起使用，所有会堵塞等待大模型功能启动，若想单独使用请设置wait_for_llm。
- 扬声器和麦克风不会同时开启，在ASR完成且确认为中文后会停止麦克风获取(灯光进入思考模式)，等待TTS的输出完成且接受到"end"的消息之后重新开启(灯光常亮)，主要为了适应大模型的持续输出。
- 默认为持续对话模式，若想使用“一唤醒一对话”方式，请设置continuous_wake_mode，“你好地瓜”唤醒

# 接口说明
## 话题

| 名称    | 消息类型                                                   | 说明                               |
| ------------ | -----------------------------------------------------| ---------------------------------- |
| /prompt_text | std_msgs/msg/String                                  | 发布ASR识别结果，该名称与大模型输入话题一致 |
| /tts_text    | std_msgs/msg/String                                  | 接收要TTS的内容，与大模型结果发布话题一致   |

## 参数

| 参数名               | 类型        | 解释               | 是否必须 | 支持的配置       | 默认值       |
| -------------------- | ----------- | ------------------ | -------- | ---------------- | ------------ |
| micphone_name          | std::string | 语音采集设备       | 否       | 根据实际情况配置 | plughw:0,0     |
| continuous_wake_mode | bool | 是否启动持续唤醒模型 | 否       | true/false | false |
| wait_for_llm         | bool | 是否等待大模型启动   | 否   | true/false | true |
| asr_pub_topic_name   | std::string | ASR结果发布话题    | 否       | 根据实际情况配置 | /prompt_text   |
| tts_sub_topic_name   | std::string | TTS内容接收话题    | 否       | 根据实际情况配置 | /tts_text   |
| tts_config_path      | std::string | 配置TTS路径       | 否        | 根据实际情况配置 | /userdata/magicbox/dep/matcha-icefall-zh-baker |
| kws_config_path      | std::string | 配置KWS路径       | 否        | 根据实际情况配置 | /userdata/magicbox/dep/sherpa-onnx/sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01 |
```

# 常见问题

1. 无法打开音频设备？

- 确认音频设备连接是否正常
- 确认是否正确配置音频设备
