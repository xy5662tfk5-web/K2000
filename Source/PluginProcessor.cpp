#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

K2000AudioProcessor::K2000AudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "K2000_STATE", {})
{
}

void K2000AudioProcessor::prepareToPlay(double sampleRate, int)
{
    phase = 0.0;
    testLevel = 0.08f;
    sampleRateHz = sampleRate;
}

void K2000AudioProcessor::releaseResources() {}

bool K2000AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

void K2000AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    for (const auto metadata : midi)
    {
        const auto message = metadata.getMessage();
        if (message.isProgramChange())
            setCurrentProgram(message.getProgramChangeNumber());
    }
    midi.clear();

    // Bootstrap-only audition tone. Replace with the V.A.S.T. engine in a later milestone.
    buffer.clear();
    const auto increment = juce::MathConstants<double>::twoPi * 220.0 / sampleRateHz;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const float value = static_cast<float>(std::sin(phase)) * testLevel;
        phase += increment;
        if (phase >= juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, value);
    }
}

void K2000AudioProcessor::setCurrentProgram(int index)
{
    currentProgram = juce::jlimit(0, 127, index);
}

const juce::String K2000AudioProcessor::getProgramName(int index)
{
    return "Program " + juce::String(index + 1).paddedLeft('0', 3);
}

void K2000AudioProcessor::changeProgramName(int, const juce::String&) {}

void K2000AudioProcessor::selectMode(const juce::String& mode) { currentMode = mode; }
juce::String K2000AudioProcessor::getMode() const { return currentMode; }
void K2000AudioProcessor::setCurrentLayer(int layer) { currentLayer = juce::jlimit(1, 3, layer); }
int K2000AudioProcessor::getCurrentLayer() const { return currentLayer; }
void K2000AudioProcessor::selectProgramByStep(int delta) { setCurrentProgram((currentProgram + delta + 128) % 128); }

void K2000AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state("K2000");
    state.setProperty("program", currentProgram, nullptr);
    state.setProperty("layer", currentLayer, nullptr);
    state.setProperty("mode", currentMode, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void K2000AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        const juce::ValueTree state = juce::ValueTree::fromXml(*xml);
        if (state.hasType("K2000"))
        {
            setCurrentProgram(static_cast<int>(state.getProperty("program", 0)));
            setCurrentLayer(static_cast<int>(state.getProperty("layer", 1)));
            currentMode = state.getProperty("mode", "PROGRAM").toString();
        }
    }
}

juce::AudioProcessorEditor* K2000AudioProcessor::createEditor()
{
    return new K2000AudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new K2000AudioProcessor();
}
