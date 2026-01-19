/*
  ==============================================================================
    VT-2W White - EMU AUDIO
    Clean / Hi-Fi / Transparent Saturation Plugin
  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================
/**
 * VT-2W White Processor
 *
 * 超低歪ソリッドステート回路をベースにした精密サチュレーションモデル。
 * 「音を壊さず、質感だけを足す」Clean / Hi-Fi / Transparent 設計。
 */
class VT2WWhiteProcessor : public juce::AudioProcessor {
public:
  //==============================================================================
  VT2WWhiteProcessor();
  ~VT2WWhiteProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  //==============================================================================
  // パラメータアクセス
  juce::AudioProcessorValueTreeState &getParameters() { return parameters; }

private:
  //==============================================================================
  // パラメータ
  juce::AudioProcessorValueTreeState parameters;

  std::atomic<float> *driveParameter = nullptr;
  std::atomic<float> *mixParameter = nullptr;

  //==============================================================================
  // DSP状態
  double currentSampleRate = 44100.0;

  // エンベロープフォロワー（トランジェント追従用）
  float envelopeL = 0.0f;
  float envelopeR = 0.0f;

  // スムージング
  juce::SmoothedValue<float> smoothedDrive;
  juce::SmoothedValue<float> smoothedMix;

  //==============================================================================
  // DSP処理関数

  /**
   * クリーンサチュレーション
   * ソリッドステート的な応答で、非常に歪み感の少ない飽和
   */
  float processSaturation(float input, float drive);

  /**
   * 微小倍音付加
   * デジタル的な冷たさを除去する程度の極小量
   */
  float processHarmonics(float input, float drive);

  /**
   * トランジェント保護
   * ほぼそのまま保持し、輪郭だけを整える
   */
  float processTransient(float input, float &envelope, float drive);

  /**
   * ゲイン補償
   */
  float calculateMakeupGain(float drive);

  //==============================================================================
  // パラメータレイアウト作成
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VT2WWhiteProcessor)
};
