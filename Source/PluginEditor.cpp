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
    : AudioProcessorEditor(&p), proc(p)
{
    setSize(1200, 500);
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
        label->setFont(juce::Font("Menlo", 15.0f, juce::Font::bold));
        label->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*label);
    }
    lcdTitle.setFont(juce::Font("Menlo", 13.0f, juce::Font::bold));
    lcdLine1.setFont(juce::Font("Menlo", 16.0f, juce::Font::bold));
    lcdLine2.setFont(juce::Font("Menlo", 13.0f, juce::Font::plain));
    lcdFooter.setFont(juce::Font("Menlo", 11.0f, juce::Font::bold));
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
            proc.selectMode("SOFT KEY " + juce::String(i + 1));
            refreshDisplay();
        });

    addButton(prevButton, "PREV", [this] { proc.selectProgramByStep(-1); refreshDisplay(); });
    addButton(nextButton, "NEXT", [this] { proc.selectProgramByStep(1); refreshDisplay(); });
    addButton(enterButton, "ENTER", [this] { proc.selectMode("ENTER"); refreshDisplay(); });
    addButton(cancelButton, "CANCEL", [this] { proc.selectMode("PROGRAM"); refreshDisplay(); });
    addButton(upButton, "▲", [this] { proc.setCurrentLayer(proc.getCurrentLayer() - 1); refreshDisplay(); });
    addButton(downButton, "▼", [this] { proc.setCurrentLayer(proc.getCurrentLayer() + 1); refreshDisplay(); });

    addButton(layer1, "LAYER 1", [this] { proc.setCurrentLayer(1); refreshDisplay(); });
    addButton(layer2, "LAYER 2", [this] { proc.setCurrentLayer(2); refreshDisplay(); });
    addButton(layer3, "LAYER 3", [this] { proc.setCurrentLayer(3); refreshDisplay(); });

    const std::array<juce::String, 12> keys { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-", "." };
    for (size_t i = 0; i < keys.size(); ++i)
    {
        keypad[i] = std::make_unique<juce::TextButton>(keys[i]);
        keypad[i]->onClick = [this, key = keys[i]] {
            proc.selectMode("KEY " + key);
            refreshDisplay();
        };
        addAndMakeVisible(*keypad[i]);
    }

    dataWheel.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dataWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    dataWheel.setRange(0, 127, 1);
    dataWheel.setValue(0);
    dataWheel.onValueChange = [this] {
        proc.setCurrentProgram(static_cast<int>(dataWheel.getValue()));
        refreshDisplay();
    };
    addAndMakeVisible(dataWheel);
    wheelLabel.setText("ALPHA WHEEL / DATA", juce::dontSendNotification);
    wheelLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    wheelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wheelLabel);

    const std::array<juce::String, 4> controlNames { "CUTOFF", "ATTACK", "RELEASE", "REVERB" };
    const std::array<juce::String, 4> controlTitles { "FILTER CUTOFF", "AMP ATTACK", "AMP RELEASE", "REVERB MIX" };
    const std::array<float, 4> controlMins { 80.0f, 0.001f, 0.02f, 0.0f };
    const std::array<float, 4> controlMaxs { 18000.0f, 3.0f, 5.0f, 1.0f };
    for (size_t i = 0; i < toneControls.size(); ++i)
    {
        auto& slider = toneControls[i];
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
        slider.setRange(controlMins[i], controlMaxs[i], i == 0 ? 1.0 : 0.001);
        slider.setValue(proc.getToneParameter(controlNames[i]));
        slider.setColour(juce::Slider::rotarySliderFillColourId, amber);
        slider.onValueChange = [this, i, controlNames] {
            proc.setToneParameter(controlNames[i], static_cast<float>(toneControls[i].getValue()));
        };
        addAndMakeVisible(slider);
        toneLabels[i].setText(controlTitles[i], juce::dontSendNotification);
        toneLabels[i].setColour(juce::Label::textColourId, juce::Colours::white);
        toneLabels[i].setJustificationType(juce::Justification::centred);
        toneLabels[i].setFont(juce::Font(10.0f, juce::Font::bold));
        addAndMakeVisible(toneLabels[i]);
    }

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
}

void K2000AudioProcessorEditor::chooseMode(const juce::String& mode)
{
    proc.selectMode(mode);
    refreshDisplay();
}

void K2000AudioProcessorEditor::refreshDisplay()
{
    lcdTitle.setText("K2000R     " + proc.getMode() + " MODE       MIDI: CH 1",
                     juce::dontSendNotification);
    lcdLine1.setText("PROGRAM  " + juce::String(proc.getCurrentProgram() + 1).paddedLeft('0', 3)
                     + "   " + proc.getProgramName(proc.getCurrentProgram()),
                     juce::dontSendNotification);
    lcdLine2.setText("LAYER " + juce::String(proc.getCurrentLayer())
                     + "     KEYMAP: --      ALG: --      OUTPUT: MAIN",
                     juce::dontSendNotification);
    lcdFooter.setText("PITCH     KEYMAP     PITCH     AMP      OUTPUT     MORE",
                      juce::dontSendNotification);
    dataWheel.setValue(proc.getCurrentProgram(), juce::dontSendNotification);
}

void K2000AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff090b0c));

    const auto panel = getLocalBounds().toFloat().reduced(24.0f, 18.0f);

    // Flat 19-inch rack chassis: square corners, metal faceplate and rack ears.
    g.setColour(juce::Colour(0xff080909));
    g.fillRect(panel);
    g.setColour(juce::Colour(0xff777d80));
    g.drawRect(panel, 1.5f);

    g.setColour(juce::Colour(0xff252a2d));
    g.fillRect(panel.reduced(18.0f, 8.0f));
    g.setColour(juce::Colour(0xff50575b));
    g.drawRect(panel.reduced(18.0f, 8.0f), 1.0f);

    // Rack mounting flanges and four recessed screw heads.
    g.setColour(juce::Colour(0xff171b1e));
    g.fillRect(24.0f, 18.0f, 42.0f, getHeight() - 36.0f);
    g.fillRect((float)getWidth() - 66.0f, 18.0f, 42.0f, getHeight() - 36.0f);
    g.setColour(juce::Colour(0xff858b8e));
    g.drawRect(24.0f, 18.0f, 42.0f, getHeight() - 36.0f, 1.0f);
    g.drawRect((float)getWidth() - 66.0f, 18.0f, 42.0f, getHeight() - 36.0f, 1.0f);

    for (float x : { 45.0f, (float)getWidth() - 45.0f })
        for (float y : { 48.0f, (float)getHeight() - 48.0f })
        {
            g.setColour(juce::Colour(0xff080a0b));
            g.fillEllipse(x - 7.0f, y - 7.0f, 14.0f, 14.0f);
            g.setColour(juce::Colour(0xff92999c));
            g.drawEllipse(x - 7.0f, y - 7.0f, 14.0f, 14.0f, 1.2f);
            g.drawLine(x - 3.5f, y, x + 3.5f, y, 1.0f);
        }

    // Recessed rectangular LCD bezel.
    g.setColour(juce::Colour(0xff101416));
    g.fillRect(110.0f, 106.0f, (float)getWidth() - 220.0f, 150.0f);
    g.setColour(juce::Colour(0xff080a0b));
    g.fillRect(126.0f, 120.0f, (float)getWidth() - 252.0f, 122.0f);
    g.setColour(juce::Colour(0xffb8c79a));
    g.fillRect(138.0f, 132.0f, (float)getWidth() - 276.0f, 98.0f);
    g.setColour(juce::Colour(0xffcbd8b0));
    g.drawRect(138.0f, 132.0f, (float)getWidth() - 276.0f, 98.0f, 1.0f);

    g.setColour(juce::Colour(0xff718268).withAlpha(0.22f));
    for (int y = 155; y < 230; y += 24)
        g.drawHorizontalLine(y, 145.0f, (float)getWidth() - 145.0f);
    g.setColour(juce::Colour(0xff40533d).withAlpha(0.45f));
    g.drawRect(144.0f, 138.0f, (float)getWidth() - 288.0f, 86.0f, 1.0f);

    // Fine horizontal faceplate highlights.
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawHorizontalLine(82, 70.0f, (float)getWidth() - 70.0f);
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.drawHorizontalLine(getHeight() - 24, 70.0f, (float)getWidth() - 70.0f);
}

void K2000AudioProcessorEditor::resized()
{
    const int w = getWidth();
    brand.setBounds(82, 28, 250, 42);
    model.setBounds(350, 36, 520, 26);
    sectionLabel.setBounds(82, 76, 420, 22);

    lcdTitle.setBounds(152, 137, w - 304, 18);
    lcdLine1.setBounds(152, 160, w - 304, 22);
    lcdLine2.setBounds(152, 184, w - 304, 20);
    lcdFooter.setBounds(152, 211, w - 304, 18);

    // Main mode keys sit below the display in one straight rack-style row.
    const int modeY = 280;
    const int modeW = 94;
    const int modeGap = 8;
    const int modeStart = 82;
    std::array<juce::TextButton*, 9> modes {
        &programButton, &setupButton, &midiButton, &masterButton, &editButton,
        &diskButton, &effectsButton, &sampleButton, &assignButton
    };
    for (int i = 0; i < 9; ++i)
        modes[static_cast<size_t>(i)]->setBounds(modeStart + i * (modeW + modeGap), modeY, modeW, 34);

    std::array<juce::TextButton*, 6> soft { &soft1, &soft2, &soft3, &soft4, &soft5, &soft6 };
    for (int i = 0; i < 6; ++i)
        soft[static_cast<size_t>(i)]->setBounds(138 + i * ((w - 276) / 6), 244, (w - 300) / 6, 25);

    prevButton.setBounds(90, 352, 70, 32);
    nextButton.setBounds(166, 352, 70, 32);
    enterButton.setBounds(90, 390, 70, 32);
    cancelButton.setBounds(166, 390, 70, 32);
    upButton.setBounds(250, 352, 58, 32);
    downButton.setBounds(250, 390, 58, 32);

    layer1.setBounds(350, 352, 86, 32);
    layer2.setBounds(350, 390, 86, 32);
    layer3.setBounds(350, 428, 86, 32);

    for (size_t i = 0; i < keypad.size(); ++i)
    {
        const int col = static_cast<int>(i % 3);
        const int row = static_cast<int>(i / 3);
        keypad[i]->setBounds(500 + col * 48, 346 + row * 38, 42, 30);
    }

    dataWheel.setBounds(w - 270, 330, 112, 112);
    wheelLabel.setBounds(w - 302, 442, 176, 20);

    for (size_t i = 0; i < toneControls.size(); ++i)
    {
        const int x = 680 + static_cast<int>(i) * 118;
        toneLabels[i].setBounds(x, 346, 112, 18);
        toneControls[i].setBounds(x + 8, 365, 96, 96);
    }
}
