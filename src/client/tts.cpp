#include "tts.h"


TTS::TTS() {
    spdlog::set_default_logger(spdlog::stderr_color_st("piper"));

    spdlog::debug("Loading voice from {} (config={})",
                  MODEL_PATH,
                  MODEL_CONFIG_PATH);

    auto startTime = std::chrono::steady_clock::now();
    loadVoice(piperConfig, std::string(MODEL_PATH),
              std::string(MODEL_CONFIG_PATH), voice, speakerId,
              false);
    auto endTime = std::chrono::steady_clock::now();
    spdlog::info("Loaded voice in {} second(s)",
                 std::chrono::duration<double>(endTime - startTime).count());
    //piperConfig.useESpeak = false;
    auto exePath = std::filesystem::canonical("/proc/self/exe");
    spdlog::debug("Voice uses eSpeak phonemes ({})",
                  voice.phonemizeConfig.eSpeak.voice);
    piperConfig.eSpeakDataPath = std::filesystem::absolute(
              exePath.parent_path().append("../res/espeak-ng-data"))
              .string();
    piper::initialize(piperConfig);
}

TTS::~TTS() {
    piper::terminate(piperConfig);
}

void rawOutputProc(std::vector<int16_t> &sharedAudioBuffer, std::mutex &mutAudio,
                   std::condition_variable &cvAudio, bool &audioReady,
                   bool &audioFinished) {
  std::vector<int16_t> internalAudioBuffer;
  while (true) {
    {
      std::unique_lock lockAudio{mutAudio};
      cvAudio.wait(lockAudio, [&audioReady] { return audioReady; });

      if (sharedAudioBuffer.empty() && audioFinished) {
        break;
      }

      copy(sharedAudioBuffer.begin(), sharedAudioBuffer.end(),
           std::back_inserter(internalAudioBuffer));

      sharedAudioBuffer.clear();

      if (!audioFinished) {
        audioReady = false;
      }
    }

    std::cout.write((const char *)internalAudioBuffer.data(),
               sizeof(int16_t) * internalAudioBuffer.size());
    std::cout.flush();
    internalAudioBuffer.clear();
  }

}

void TTS::run(std::string line) {
    piper::SynthesisResult result;
    //auto speakerId = voice.synthesisConfig.speakerId;

    // Timestamp is used for path to output WAV file
    const auto now = std::chrono::system_clock::now();
    std::stringstream outputName;
    outputName << "output.wav";
    std::filesystem::path outputPath = std::filesystem::path(OUTPUT_PATH);
    outputPath.append(outputName.str());
    // Output audio to automatically-named WAV file in a directory
    std::ofstream audioFile(outputPath.string(), std::ios::binary);
    piper::textToWavFile(piperConfig, voice, line, audioFile, result);
    std::cout << outputPath.string() << std::endl;
    /* RAW FILE
    std::mutex mutAudio;
    std::condition_variable cvAudio;
    bool audioReady = false;
    bool audioFinished = false;
    std::vector<int16_t> audioBuffer;
    std::vector<int16_t> sharedAudioBuffer;
    std::thread rawOutputThread(rawOutputProc, std::ref(sharedAudioBuffer),
                             std::ref(mutAudio), std::ref(cvAudio), std::ref(audioReady),
                             std::ref(audioFinished));
    auto audioCallback = [&audioBuffer, &sharedAudioBuffer, &mutAudio,
                          &cvAudio, &audioReady]() {
      // Signal thread that audio is ready
      {
        std::unique_lock lockAudio(mutAudio);
        copy(audioBuffer.begin(), audioBuffer.end(),
             back_inserter(sharedAudioBuffer));
        audioReady = true;
        cvAudio.notify_one();
      }
    };
    piper::textToAudio(piperConfig, voice, line, audioBuffer, result,
                       audioCallback);
    // Signal thread that there is no more audio
    {
      std::unique_lock lockAudio(mutAudio);
      audioReady = true;
      audioFinished = true;
      cvAudio.notify_one();
    }
    // Wait for audio output to finish
    spdlog::info("Waiting for audio to finish playing...");
    rawOutputThread.join();
    spdlog::info("Real-time factor: {} (infer={} sec, audio={} sec)",
                 result.realTimeFactor, result.inferSeconds,
                 result.audioSeconds);

    // Restore config (--json-input)
    voice.synthesisConfig.speakerId = speakerId;
    */
}
