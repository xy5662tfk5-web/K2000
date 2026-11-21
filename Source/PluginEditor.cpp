#include "PluginEditor.h"

namespace
{
const juce::Colour caseBlack(0xff17191b);
const juce::Colour panelGrey(0xff34383b);
const juce::Colour lcdGreen(0xffb8c79a);
const juce::Colour lcdInk(0xff1b2b20);
const juce::Colour amber(0xffd99b42);
}

K2000AudioProcessorEditor::K2000AudioProcessorEditor(K2000AudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1000, 620);
    brand.setText("KURZWEIL", juce::dontSendNotification);
    brand.setFont(juce::Font(25.0f, juce::Font::bold));
    brand.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(brand);
    model.setText("K2000R  •  V.A.S.T. WORKSTATION", juce::dontSendNotification);
    model.setFont(juce::Font(13.0f, juce::Font::bold));
    model.setColour(juce::Label::textColourId, amber);
    addAndMakeVisible(model);

    for (auto* label : { &lcdTitle, &lcdLine1, &lcdLine2, &lcdFooter })
    {
        label->setColour(juce::Label::textColourId, lcdInk);
        label->setFont(juce::Font(16.0f, juce::Font::bold));
        label->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*label);
    }
    lcdTitle.setFont(juce::Font(14.0f, juce::Font::bold));
    lcdFooter.setFont(juce::Font(12.0f));
    sectionLabel.setText("FRONT PANEL  /  PROGRAM EDITOR", juce::dontSendNotification);
    sectionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(sectionLabel);

    addButton(programButton, "PROGRAM", [this] { chooseMode("PROGRAM"); });
    addButton(setupButton, "SETUP", [this] { chooseMode("SETUP"); });
    addButton(midiButton, "MIDI", [this] { chooseMode("MIDI"); });
    addButton(masterButton, "MASTER", [this] { chooseMode("MASTER"); });
    addButton(editButton, "EDIT", [this] { chooseMode("EDIT"); });
    addButton(diskButton, "DISK", [this] { chooseMode("DISK"); });
    addButton(effectsButton, "EFFECTS", [this] { chooseMode("EFFECTS"); });
    addButton(sampleButton, "SAMPLE", [this] { chooseMode("SAMPLE"); });
    addButton(assignButton, "ASSIGN", [this] { chooseMode("ASSIGN"); });

    std::array<juce::TextButton*, 6> soft { &soft1, &soft2, &soft3, &soft4, &soft5, &soft6 };
    for (int i = 0; i < 6; ++i)
        addButton(*soft[static_cast<size_t>(i)], "F" + juce::String(i + 1), [this, i] {
            processor.selectMode("SOFT KEY " + juce::String(i + 1));
            refreshDisplay();
        });

    addButton(prevButton, "PREV", [this] { processor.selectProgramByStep(-1); refreshDisplay(); });
    addButton(nextButton, "NEXT", [this] { processor.selectProgramByStep(1); refreshDisplay(); });
    addButton(enterButton, "ENTER", [this] { processor.selectMode("ENTER"); refreshDisplay(); });
    addButton(cancelButton, "CANCEL", [this] { processor.selectMode("PROGRAM"); refreshDisplay(); });
    addButton(upButton, "▲", [this] { processor.setCurrentLayer(processor.getCurrentLayer() - 1); refreshDisplay(); });
    addButton(downButton, "▼", [this] { processor.setCurrentLayer(processor.getCurrentLayer() + 1); refreshDisplay(); });

    addButton(layer1, "LAYER 1", [this] { processor.setCurrentLayer(1); refreshDisplay(); });
    addButton(layer2, "LAYER 2", [this] { processor.setCurrentLayer(2); refreshDisplay(); });
    addButton(layer3, "LAYER 3", [this] { processor.setCurrentLayer(3); refreshDisplay(); });

    const std::array<juce::String, 12> keys { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-", "." };
    for (size_t i = 0; i < keys.size(); ++i)
    {
        keypad[i] = std::make_unique<juce::TextButton>(keys[i]);
        keypad[i]->onClick = [this, key = keys[i]] {
            processor.selectMode("KEY " + key);
            refreshDisplay();
        };
        addAndMakeVisible(*keypad[i]);
        allButtons.push_back(keypad[i].get());
    }

    dataWheel.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dataWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    dataWheel.setRange(0, 127, 1);
    dataWheel.setValue(0);
    dataWheel.onValueChange = [this] {
        processor.setCurrentProgram(static_cast<int>(dataWheel.getValue()));
        refreshDisplay();
    };
    addAndMakeVisible(dataWheel);
    wheelLabel.setText("ALPHA WHEEL / DATA", juce::dontSendNotification);
    wheelLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    wheelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wheelLabel);

    refreshDisplay();
}

void K2000AudioProcessorEditor::addButton(juce::TextButton& button, const juce::String& text, std::function<void()> action)
{
    button.setButtonText(text);
    button.onClick = std::move(action);
    button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff555b60));
    button.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    button.setColour(juce::TextButton::buttonOnColourId, amber);
    addAndMakeVisible(button);
    allButtons.push_back(&button);
}

void K2000AudioProcessorEditor::chooseMode(const juce::String& mode)
{
    processor.selectMode(mode);
    refreshDisplay();
}

void K2000AudioProcessorEditor::refreshDisplay()
{
    lcdTitle.setText("K2000  /  " + processor.getMode(), juce::dontSendNotification);
    lcdLine1.setText("PROGRAM  " + juce::String(processor.getCurrentProgram() + 1).paddedLeft('0', 3)
                     + "     " + processor.getProgramName(processor.getCurrentProgram()), juce::dontSendNotification);
    lcdLine2.setText("LAYER " + juce::String(processor.getCurrentLayer())
                     + "     V.A.S.T. EDITOR  •  ENGINE UNDER DEVELOPMENT", juce::dontSendNotification);
    lcdFooter.setText("F1 PROGRAM   F2 KEYMAP   F3 PITCH   F4 AMP   F5 OUTPUT   F6 MORE",
                      juce::dontSendNotification);
    dataWheel.setValue(processor.getCurrentProgram(), juce::dontSendNotification);
}

void K2000AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(caseBlack);
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff080909));
    g.fillRoundedRectangle(bounds.reduced(8.0f), 14.0f);
    g.setColour(panelGrey);
    g.fillRoundedRectangle(bounds.reduced(16.0f), 10.0f);
    g.setColour(juce::Colour(0xff777d80));
    g.drawRoundedRectangle(bounds.reduced(16.0f), 10.0f, 1.5f);

    g.setColour(juce::Colour(0xff202427));
    g.fillRoundedRectangle(juce::Rectangle<float>(35, 105, 930, 180), 8.0f);
    g.setColour(juce::Colour(0xff090c0b));
    g.fillRoundedRectangle(juce::Rectangle<float>(65, 125, 870, 138), 5.0f);
    g.setColour(lcdGreen);
    g.fillRoundedRectangle(juce::Rectangle<float>(78, 138, 844, 112), 3.0f);
    g.setColour(juce::Colour(0xffcbd8b0));
    g.drawRoundedRectangle(juce::Rectangle<float>(78, 138, 844, 112), 3.0f, 1.0f);

    g.setColour(juce::Colours::white.withAlpha(0.18f));
    g.drawHorizontalLine(90, 32.0f, 968.0f);
    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.drawHorizontalLine(590, 32.0f, 968.0f);
}

void K2000AudioProcessorEditor::resized()
{
    brand.setBounds(38, 28, 250, 42);
    model.setBounds(300, 36, 500, 26);
    sectionLabel.setBounds(40, 76, 400, 22);
    lcdTitle.setBounds(92, 144, 820, 20);
    lcdLine1.setBounds(92, 169, 820, 24);
    lcdLine2.setBounds(92, 195, 820, 22);
    lcdFooter.setBounds(92, 224, 820, 18);

    programButton.setBounds(42, 315, 105, 36);
    setupButton.setBounds(154, 315, 92, 36);
    midiButton.setBounds(253, 315, 82, 36);
    masterButton.setBounds(342, 315, 92, 36);
    editButton.setBounds(441, 315, 82, 36);
    diskButton.setBounds(530, 315, 82, 36);
    effectsButton.setBounds(619, 315, 92, 36);
    sampleButton.setBounds(718, 315, 92, 36);
    assignButton.setBounds(817, 315, 92, 36);

    std::array<juce::TextButton*, 6> soft { &soft1, &soft2, &soft3, &soft4, &soft5, &soft6 };
    for (int i = 0; i < 6; ++i)
        soft[static_cast<size_t>(i)]->setBounds(78 + i * 142, 270, 112, 28);

    prevButton.setBounds(45, 385, 76, 34);
    nextButton.setBounds(127, 385, 76, 34);
    enterButton.setBounds(45, 426, 76, 34);
    cancelButton.setBounds(127, 426, 76, 34);
    upButton.setBounds(225, 385, 62, 34);
    downButton.setBounds(225, 426, 62, 34);

    layer1.setBounds(320, 385, 92, 34);
    layer2.setBounds(320, 426, 92, 34);
    layer3.setBounds(320, 467, 92, 34);

    for (size_t i = 0; i < keypad.size(); ++i)
    {
        const int col = static_cast<int>(i % 3);
        const int row = static_cast<int>(i / 3);
        keypad[i]->setBounds(470 + col * 52, 380 + row * 42, 44, 34);
    }
    dataWheel.setBounds(700, 370, 130, 130);
    wheelLabel.setBounds(670, 500, 190, 24);
}
