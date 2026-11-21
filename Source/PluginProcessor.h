#pragma once
#include <JuceHeader.h>
#include <array>

class K2000AudioProcessor final : public juce::AudioProcessor
{
public:
    K2000AudioProcessor();
    ~K2000AudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "K2000"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 128; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void selectMode(const juce::String& mode);
    juce::String getMode() const;
    void setCurrentLayer(int layer);
    int getCurrentLayer() const;
    void selectProgramByStep(int delta);

    void setToneParameter(const juce::String& name, float value);
    float getToneParameter(const juce::String& name) const;

private:
    struct Voice
    {
        int note = -1;
        float velocity = 0.0f;
        double phase = 0.0;
        float envelope = 0.0f;
        bool held = false;
        bool active = false;
    };

    void noteOn(int note, float velocity);
    void noteOff(int note);
    void renderVoice(Voice& voice, juce::AudioBuffer<float>& buffer, int start, int count);

    juce::AudioProcessorValueTreeState parameters;
    int currentProgram = 0;
    int currentLayer = 1;
    juce::String currentMode { "PROGRAM" };
    double sampleRateHz = 44100.0;
    std::array<Voice, 16> voices {};
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    float attack = 0.015f, release = 0.25f, cutoff = 12000.0f, reverbMix = 0.12f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(K2000AudioProcessor)
};
