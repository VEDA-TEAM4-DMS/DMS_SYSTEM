#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "../json.hpp"
#include "../../thirdparty/piper/src/cpp/piper.hpp"

using json = nlohmann::json;

constexpr std::string_view MODEL_PATH = "../res/en_US-amy-low.onnx";
constexpr std::string_view MODEL_CONFIG_PATH = "../res/en_US-amy-low.onnx.json";
constexpr std::string_view OUTPUT_PATH = "../res/";

class TTS {
public:
    TTS();
    ~TTS();
    void run(std::string line);

private:
    //void rawOutputProc(std::vector<int16_t> &sharedAudioBuffer, std::mutex &mutAudio, std::condition_variable &cvAudio, bool &audioReady, bool &audioFinished);

    piper::PiperConfig piperConfig;
    piper::Voice voice;
    std::optional<piper::SpeakerId> speakerId;

};