/*
  ==============================================================================
    VT-2W White - EMU AUDIO
    Plugin Editor (UI) Implementation
  ==============================================================================
*/

#include "PluginEditor.h"
#include "BinaryData.h"
#include "PluginProcessor.h"
#include <iostream>

//==============================================================================
// VT2WImageKnob
//==============================================================================

VT2WImageKnob::VT2WImageKnob() { setRepaintsOnMouseActivity(true); }
VT2WImageKnob::~VT2WImageKnob() {}

void VT2WImageKnob::setImage(const juce::Image &image) {
  knobImage = image;
  repaint();
}

void VT2WImageKnob::paint(juce::Graphics &g) {
  auto bounds = getLocalBounds().toFloat();
  auto centre = bounds.getCentre();

  if (knobImage.isValid()) {
    float normalizedValue =
        static_cast<float>((value - minValue) / (maxValue - minValue));
    float angle = startAngle + normalizedValue * (endAngle - startAngle);

    float knobSize = juce::jmin(bounds.getWidth(), bounds.getHeight());
    float scale = knobSize / static_cast<float>(knobImage.getWidth());

    // 透過PNGを使用するため、追加のクリッピングは不要

    juce::AffineTransform transform =
        juce::AffineTransform::rotation(
            angle, static_cast<float>(knobImage.getWidth()) / 2.0f,
            static_cast<float>(knobImage.getHeight()) / 2.0f)
            .scaled(scale)
            .translated(centre.x - (knobImage.getWidth() * scale) / 2.0f,
                        centre.y - (knobImage.getHeight() * scale) / 2.0f);

    g.drawImageTransformed(knobImage, transform, false);
  }
}

void VT2WImageKnob::resized() {}

void VT2WImageKnob::setRange(double min, double max, double interval) {
  minValue = min;
  maxValue = max;
  juce::ignoreUnused(interval);
  defaultValue = min;
}

void VT2WImageKnob::setValue(double newValue,
                             juce::NotificationType notification) {
  value = juce::jlimit(minValue, maxValue, newValue);
  repaint();
  if (notification != juce::dontSendNotification && onValueChange)
    onValueChange();
}

double VT2WImageKnob::getValue() const { return value; }

void VT2WImageKnob::setLabel(const juce::String &labelText) {
  label = labelText;
}

void VT2WImageKnob::setRotationRange(float startAngleRadians,
                                     float endAngleRadians) {
  startAngle = startAngleRadians;
  endAngle = endAngleRadians;
}

void VT2WImageKnob::mouseDown(const juce::MouseEvent &event) {
  dragStartValue = value;
  dragStartY = event.y;
}

void VT2WImageKnob::mouseDrag(const juce::MouseEvent &event) {
  float sensitivity = event.mods.isShiftDown() ? 0.002f : 0.01f;
  double delta = static_cast<double>(dragStartY - event.y) * sensitivity *
                 (maxValue - minValue);
  setValue(dragStartValue + delta);
}

void VT2WImageKnob::mouseUp(const juce::MouseEvent &) {}
void VT2WImageKnob::mouseDoubleClick(const juce::MouseEvent &) {
  setValue(defaultValue);
}

void VT2WImageKnob::mouseWheelMove(const juce::MouseEvent &,
                                   const juce::MouseWheelDetails &wheel) {
  double delta = wheel.deltaY * (maxValue - minValue) * 0.05;
  setValue(value + delta);
}

//==============================================================================
// VT2WWhiteEditor
//==============================================================================

VT2WWhiteEditor::VT2WWhiteEditor(VT2WWhiteProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {

  loadImages();

  // ウィンドウサイズを設定
  // 背景画像に合わせて自動設定
  if (backgroundImage.isValid()) {
    setSize(backgroundImage.getWidth(), backgroundImage.getHeight());
  } else {
    setSize(800, 600); // フォールバック
  }

  // Driveノブ
  driveKnob.setImage(knobImage);
  driveKnob.setLabel("DRIVE");
  driveKnob.setRange(0.0, 100.0, 0.1);
  driveKnob.setValue(0.0);
  driveKnob.setRotationRange(-2.35619f, 2.35619f);
  addAndMakeVisible(driveKnob);

  // Mixノブ
  mixKnob.setImage(knobImage);
  mixKnob.setLabel("MIX");
  mixKnob.setRange(0.0, 100.0, 1.0);
  mixKnob.setValue(100.0);
  mixKnob.setRotationRange(-2.35619f, 2.35619f);
  addAndMakeVisible(mixKnob);

  // パラメータ連携用スライダー
  driveSlider.setRange(0.0, 10.0);
  mixSlider.setRange(0.0, 100.0);

  // バインディング
  driveKnob.onValueChange = [this]() {
    // 0-100 -> 0-10
    driveSlider.setValue(driveKnob.getValue() / 10.0,
                         juce::sendNotificationSync);
  };
  mixKnob.onValueChange = [this]() {
    mixSlider.setValue(mixKnob.getValue(), juce::sendNotificationSync);
  };

  driveSlider.onValueChange = [this]() {
    driveKnob.setValue(driveSlider.getValue() * 10.0,
                       juce::dontSendNotification);
  };
  mixSlider.onValueChange = [this]() {
    mixKnob.setValue(mixSlider.getValue(), juce::dontSendNotification);
  };

  // Attachment
  driveAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.getParameters(), "drive", driveSlider);
  mixAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.getParameters(), "mix", mixSlider);

  // 初期値反映
  driveKnob.setValue(driveSlider.getValue() * 10.0, juce::dontSendNotification);
  mixKnob.setValue(mixSlider.getValue(), juce::dontSendNotification);
}

VT2WWhiteEditor::~VT2WWhiteEditor() {
  driveAttachment.reset();
  mixAttachment.reset();
}

void VT2WWhiteEditor::loadImages() {
  // BinaryData名は CMakeLists.txt の juce_add_binary_data
  // で指定されたファイル名に基づく
  backgroundImage = juce::ImageCache::getFromMemory(
      VT2WData::background_jpg, VT2WData::background_jpgSize);

  // 新しいノブ画像 (PNG透過あり) をロード
  knobImage = juce::ImageCache::getFromMemory(VT2WData::knob_png,
                                              VT2WData::knob_pngSize);

  // ノブにセット
  driveKnob.setImage(knobImage);
  mixKnob.setImage(knobImage);
}

void VT2WWhiteEditor::paint(juce::Graphics &g) {
  if (backgroundImage.isValid()) {
    g.drawImage(backgroundImage, getLocalBounds().toFloat());
  } else {
    g.fillAll(juce::Colours::white);
    g.setColour(juce::Colours::black);
    g.drawText("Background Image Not Found", getLocalBounds(),
               juce::Justification::centred);
  }
}

void VT2WWhiteEditor::resized() {
  // ユーザー指定座標(Centerで設定)
  int knobSize = 206;
  int radius = knobSize / 2;

  // DRIVE: Center(216, 626) -> TopLeft(216-103, 626-103)
  driveKnob.setBounds(216 - radius, 626 - radius, knobSize, knobSize);

  // MIX: Center(809, 626) -> TopLeft(809-103, 626-103)
  mixKnob.setBounds(809 - radius, 626 - radius, knobSize, knobSize);
}
