/*
  ==============================================================================
    VT-2W White - EMU AUDIO
    Plugin Editor (UI)
  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"

//==============================================================================
/**
 * 画像ベースのノブ
 */
class VT2WImageKnob : public juce::Component {
public:
  VT2WImageKnob();
  ~VT2WImageKnob() override;

  void paint(juce::Graphics &g) override;
  void resized() override;

  void setImage(const juce::Image &knobImage);
  void setRange(double min, double max, double interval = 0.0);
  void
  setValue(double newValue,
           juce::NotificationType notification = juce::sendNotificationAsync);
  double getValue() const;

  void setLabel(const juce::String &labelText);
  void setRotationRange(float startAngleRadians, float endAngleRadians);

  std::function<void()> onValueChange;

private:
  void mouseDown(const juce::MouseEvent &event) override;
  void mouseDrag(const juce::MouseEvent &event) override;
  void mouseUp(const juce::MouseEvent &event) override;
  void mouseDoubleClick(const juce::MouseEvent &event) override;
  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;

  juce::Image knobImage;

  double value = 0.0;
  double minValue = 0.0;
  double maxValue = 10.0;
  double defaultValue = 0.0;
  double dragStartValue = 0.0;
  int dragStartY = 0;

  float startAngle = -2.35619f; // -135 degrees
  float endAngle = 2.35619f;    // 135 degrees

  juce::String label;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VT2WImageKnob)
};

//==============================================================================
/**
 * メインエディター
 */
class VT2WWhiteEditor : public juce::AudioProcessorEditor {
public:
  explicit VT2WWhiteEditor(VT2WWhiteProcessor &);
  ~VT2WWhiteEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  VT2WWhiteProcessor &audioProcessor;

  // 画像
  juce::Image backgroundImage;
  juce::Image knobImage;

  // ノブ
  VT2WImageKnob driveKnob;
  VT2WImageKnob mixKnob;

  // 内部スライダー（アタッチメント用）
  juce::Slider driveSlider;
  juce::Slider mixSlider;

  // パラメータアタッチメント
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      driveAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      mixAttachment;

  // 画像ロード
  void loadImages();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VT2WWhiteEditor)
};
