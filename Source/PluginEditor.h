#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class K2000AudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit K2000AudioProcessorEditor(K2000AudioProcessor&);
    ~K2000AudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void refreshDisplay();
    void chooseMode(const juce::String& mode);
    void addButton(juce::TextButton& button, const juce::String& text, std::function<void()> action);

    K2000AudioProcessor& processor;
    juce::Label brand, model, lcdTitle, lcdLine1, lcdLine2, lcdFooter, sectionLabel;
    juce::TextButton programButton, setupButton, midiButton, masterButton, editButton;
    juce::TextButton soft1, soft2, soft3, soft4, soft5, soft6;
    juce::TextButton prevButton, nextButton, enterButton, cancelButton, upButton, downButton;
    juce::TextButton layer1, layer2, layer3;
    std::array<std::unique_ptr<juce::TextButton>, 12> keypad;
    juce::Slider dataWheel;
    juce::Label wheelLabel;
    std::array<juce::Slider, 4> toneControls;
    std::array<juce::Label, 4> toneLabels;
    juce::TextButton diskButton, effectsButton, sampleButton, assignButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(K2000AudioProcessorEditor)
};
