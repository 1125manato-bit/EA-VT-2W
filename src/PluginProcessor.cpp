/*
  ==============================================================================
    VT-2W White - EMU AUDIO
    Clean / Hi-Fi / Transparent Saturation Plugin

    設計思想:
    - 超低歪ソリッドステート回路モデル
    - 音の輪郭と解像度向上
    - デジタル的な冷たさの除去
    - マスタリングやアコースティック用途OK
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
// VT-2W White 定数定義
namespace VT2WConstants {
// サチュレーション - クリーンでHi-Fi
constexpr float kSaturationCoeffMax = 0.8f; // 少し上げる (0.5 -> 0.8)
constexpr float kSaturationKnee =
    0.90f; // 少しだけ早く効くように (0.95 -> 0.90)

// 倍音生成 - 聴感上の「太さ」を強化
constexpr float kHarmonic2ndAmount = 0.15f; // アナログ感を強める (0.08 -> 0.15)
constexpr float kHarmonic3rdAmount =
    0.04f; // 極小だが少し存在感を出す (0.02 -> 0.04)

// トランジェント - 輪郭を明瞭にする
constexpr float kTransientAmountMax =
    0.20f; // 歪みに負けない輪郭 (0.15 -> 0.20)
constexpr float kEnvelopeAttack = 0.002f;
constexpr float kEnvelopeRelease = 0.100f;

// パラメータ範囲
constexpr float kDriveMin = 0.0f;
constexpr float kDriveMax = 10.0f;
constexpr float kDriveDefault = 0.0f;
constexpr float kMixMin = 0.0f;
constexpr float kMixMax = 100.0f;
constexpr float kMixDefault = 100.0f;
} // namespace VT2WConstants

//==============================================================================
VT2WWhiteProcessor::VT2WWhiteProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("VT2WWhite"),
                 createParameterLayout()) {
  driveParameter = parameters.getRawParameterValue("drive");
  mixParameter = parameters.getRawParameterValue("mix");
}

VT2WWhiteProcessor::~VT2WWhiteProcessor() {}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
VT2WWhiteProcessor::createParameterLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  // Drive パラメータ
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"drive", 1}, "Drive",
      juce::NormalisableRange<float>(VT2WConstants::kDriveMin,
                                     VT2WConstants::kDriveMax, 0.1f),
      VT2WConstants::kDriveDefault,
      juce::AudioParameterFloatAttributes().withLabel("Drive")));

  // Mix パラメータ
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"mix", 1}, "Mix",
      juce::NormalisableRange<float>(VT2WConstants::kMixMin,
                                     VT2WConstants::kMixMax, 1.0f),
      VT2WConstants::kMixDefault,
      juce::AudioParameterFloatAttributes().withLabel("%")));

  return {params.begin(), params.end()};
}

//==============================================================================
const juce::String VT2WWhiteProcessor::getName() const {
  return JucePlugin_Name;
}

bool VT2WWhiteProcessor::acceptsMidi() const { return false; }
bool VT2WWhiteProcessor::producesMidi() const { return false; }
bool VT2WWhiteProcessor::isMidiEffect() const { return false; }
double VT2WWhiteProcessor::getTailLengthSeconds() const { return 0.0; }

int VT2WWhiteProcessor::getNumPrograms() { return 1; }
int VT2WWhiteProcessor::getCurrentProgram() { return 0; }
void VT2WWhiteProcessor::setCurrentProgram(int) {}
const juce::String VT2WWhiteProcessor::getProgramName(int) { return {}; }
void VT2WWhiteProcessor::changeProgramName(int, const juce::String &) {}

//==============================================================================
void VT2WWhiteProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  currentSampleRate = sampleRate;

  // スムージング設定
  smoothedDrive.reset(sampleRate, 0.05); // 少しゆっくり追従
  smoothedMix.reset(sampleRate, 0.05);

  envelopeL = 0.0f;
  envelopeR = 0.0f;
}

void VT2WWhiteProcessor::releaseResources() {}

bool VT2WWhiteProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;

  return true;
}

//==============================================================================
void VT2WWhiteProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                      juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  juce::ignoreUnused(midiMessages);

  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  float drive = *driveParameter;
  float mix = *mixParameter / 100.0f;

  smoothedDrive.setTargetValue(drive);
  smoothedMix.setTargetValue(mix);

  auto *channelDataL = buffer.getWritePointer(0);
  auto *channelDataR =
      totalNumInputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

  for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
    float currentDrive = smoothedDrive.getNextValue();
    float currentMix = smoothedMix.getNextValue();

    float dryL = channelDataL[sample];
    float dryR = channelDataR ? channelDataR[sample] : dryL;

    // クリーンブースト
    // Driveマックスでも+6dB程度に抑える（歪みより質感重視）
    float preDriveGain =
        1.0f + (currentDrive / VT2WConstants::kDriveMax) * 1.0f;

    // === L ch ===
    float wetL = dryL * preDriveGain;
    wetL = processSaturation(wetL, currentDrive);
    wetL += processHarmonics(dryL * preDriveGain, currentDrive);
    wetL = processTransient(wetL, envelopeL, currentDrive);
    wetL *= calculateMakeupGain(currentDrive);

    // === R ch ===
    float wetR = dryR;
    if (channelDataR != nullptr) {
      wetR = dryR * preDriveGain;
      wetR = processSaturation(wetR, currentDrive);
      wetR += processHarmonics(dryR * preDriveGain, currentDrive);
      wetR = processTransient(wetR, envelopeR, currentDrive);
      wetR *= calculateMakeupGain(currentDrive);
    } else {
      wetR = wetL;
    }

    // Mix (Dry/Wet)
    channelDataL[sample] = dryL * (1.0f - currentMix) + wetL * currentMix;
    if (channelDataR != nullptr)
      channelDataR[sample] = dryR * (1.0f - currentMix) + wetR * currentMix;
  }
}

//==============================================================================
// DSP Implementations

float VT2WWhiteProcessor::processSaturation(float input, float drive) {
  if (std::abs(input) < 0.0001f)
    return input;

  float normalizedDrive = drive / VT2WConstants::kDriveMax;

  // 質感（太さ）を出すためのS字カーブ
  // k をもう少し積極的にし、高域の明瞭度を保つために cubic だけではなく tanh
  // 的な 挙動を少し混ぜる
  float k = 0.12f * normalizedDrive;
  float out = input - k * (input * input * input);

  // 安全のためのリミッティング（Hi-Fiさを損なわない程度）
  return std::tanh(out * (1.0f + 0.1f * normalizedDrive)) /
         (1.0f + 0.1f * normalizedDrive);
}

float VT2WWhiteProcessor::processHarmonics(float input, float drive) {
  float normalizedDrive = drive / VT2WConstants::kDriveMax;

  // 非対称な歪みによる2次倍音付加
  // DCオフセットは極小量なので、ここでは簡略化しつつ効果を高める
  float h2 = (input * std::abs(input)) * VT2WConstants::kHarmonic2ndAmount *
             normalizedDrive;
  float h3 = (input * input * input) * VT2WConstants::kHarmonic3rdAmount *
             normalizedDrive;

  return h2 - h3; // 2次（太さ）と3次（エッジ）の組み合わせ
}

float VT2WWhiteProcessor::processTransient(float input, float &envelope,
                                           float drive) {
  // トランジェント保護
  // アタック部分の歪みを避けるために、アタック時に少しゲインを下げるのではなく
  // 逆にアタックをクリアにするために少し強調する?
  // 「音の輪郭と解像度が向上」 -> アタック強調 (Expander的な)

  float absInput = std::abs(input);

  float attackCoeff = 1.0f - std::exp(-1.0f / (float(currentSampleRate) *
                                               VT2WConstants::kEnvelopeAttack));
  float releaseCoeff =
      1.0f - std::exp(-1.0f / (float(currentSampleRate) *
                               VT2WConstants::kEnvelopeRelease));

  if (absInput > envelope)
    envelope = envelope + attackCoeff * (absInput - envelope);
  else
    envelope = envelope + releaseCoeff * (absInput - envelope);

  float normalizedDrive = drive / VT2WConstants::kDriveMax;
  float amount = VT2WConstants::kTransientAmountMax * normalizedDrive;

  // エンベロープの変化率が高い（アタック）時に少しブースト
  // 簡易実装として、入力とエンベロープの差分を加算
  float transient = absInput - envelope;
  if (transient > 0) {
    // アタック成分
    return input + input * (transient * amount * 2.0f);
  }

  return input;
}

float VT2WWhiteProcessor::calculateMakeupGain(float drive) {
  // ブースト分を少し下げる
  return 1.0f / (1.0f + (drive / VT2WConstants::kDriveMax) * 0.5f);
}

//==============================================================================
bool VT2WWhiteProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor *VT2WWhiteProcessor::createEditor() {
  return new VT2WWhiteEditor(*this);
}

//==============================================================================
void VT2WWhiteProcessor::getStateInformation(juce::MemoryBlock &destData) {
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void VT2WWhiteProcessor::setStateInformation(const void *data,
                                             int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(parameters.state.getType()))
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new VT2WWhiteProcessor();
}
