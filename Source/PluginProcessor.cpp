#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

K2000AudioProcessor::K2000AudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "K2000_STATE", {})
{
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency(cutoff);
    reverbParams.roomSize = 0.45f;
    reverbParams.damping = 0.55f;
    reverbParams.wetLevel = reverbMix;
    reverbParams.dryLevel = 1.0f - reverbMix;
    reverb.setParameters(reverbParams);
}

void K2000AudioProcessor::prepareToPlay(double sampleRate, int)
{
    sampleRateHz = sampleRate;
    filter.reset();
    filter.prepare({ sampleRate, 512, static_cast<juce::uint32>(getTotalNumOutputChannels()) });
    reverb.reset();
    for (auto& voice : voices) voice = {};
}

void K2000AudioProcessor::releaseResources() {}

bool K2000AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

void K2000AudioProcessor::noteOn(int note, float velocity)
{
    auto* target = &voices[0];
    for (auto& voice : voices)
        if (!voice.active) { target = &voice; break; }
    *target = {};
    target->note = note;
    target->velocity = velocity;
    target->held = target->active = true;
}

void K2000AudioProcessor::noteOff(int note)
{
    for (auto& voice : voices)
        if (voice.active && voice.note == note) voice.held = false;
}

void K2000AudioProcessor::renderVoice(Voice& voice, juce::AudioBuffer<float>& buffer, int start, int count)
{
    if (!voice.active) return;
    const double frequency = juce::MidiMessage::getMidiNoteInHertz(voice.note);
    const double increment = juce::MathConstants<double>::twoPi * frequency / sampleRateHz;
    const float attackStep = 1.0f / juce::jmax(1.0f, attack * static_cast<float>(sampleRateHz));
    const float releaseStep = 1.0f / juce::jmax(1.0f, release * static_cast<float>(sampleRateHz));
    for (int i = 0; i < count; ++i)
    {
        if (voice.held) voice.envelope = juce::jmin(1.0f, voice.envelope + attackStep);
        else voice.envelope = juce::jmax(0.0f, voice.envelope - releaseStep);
        if (!voice.held && voice.envelope <= 0.00001f) { voice.active = false; break; }

        // Initial playable oscillator: a bright sawtooth with a little pulse component.
        const float saw = static_cast<float>((voice.phase / juce::MathConstants<double>::pi) - 1.0);
        const float pulse = std::sin(voice.phase) >= 0.0 ? 0.22f : -0.22f;
        const float sample = (saw * 0.72f + pulse) * voice.envelope * voice.velocity * 0.13f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addSample(ch, start + i, sample);
        voice.phase += increment;
        if (voice.phase >= juce::MathConstants<double>::twoPi) voice.phase -= juce::MathConstants<double>::twoPi;
    }
}

void K2000AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    int cursor = 0;
    for (const auto metadata : midi)
    {
        const auto message = metadata.getMessage();
        const int position = juce::jlimit(0, buffer.getNumSamples(), metadata.samplePosition);
        for (auto& voice : voices) renderVoice(voice, buffer, cursor, position - cursor);
        cursor = position;

        if (message.isNoteOn()) noteOn(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff()) noteOff(message.getNoteNumber());
        else if (message.isProgramChange()) setCurrentProgram(message.getProgramChangeNumber());
    }
    for (auto& voice : voices) renderVoice(voice, buffer, cursor, buffer.getNumSamples() - cursor);
    midi.clear();

    filter.setCutoffFrequency(cutoff);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);
    reverbParams.wetLevel = reverbMix;
    reverbParams.dryLevel = 1.0f - reverbMix * 0.55f;
    reverb.setParameters(reverbParams);
    if (buffer.getNumChannels() > 1)
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
    else if (buffer.getNumChannels() == 1)
        reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());
}

void K2000AudioProcessor::setCurrentProgram(int index) { currentProgram = juce::jlimit(0, 127, index); }
const juce::String K2000AudioProcessor::getProgramName(int index) { return "Program " + juce::String(index + 1).paddedLeft('0', 3); }
void K2000AudioProcessor::changeProgramName(int, const juce::String&) {}
void K2000AudioProcessor::selectMode(const juce::String& mode) { currentMode = mode; }
juce::String K2000AudioProcessor::getMode() const { return currentMode; }
void K2000AudioProcessor::setCurrentLayer(int layer) { currentLayer = juce::jlimit(1, 3, layer); }
int K2000AudioProcessor::getCurrentLayer() const { return currentLayer; }
void K2000AudioProcessor::selectProgramByStep(int delta) { setCurrentProgram((currentProgram + delta + 128) % 128); }

void K2000AudioProcessor::setToneParameter(const juce::String& name, float value)
{
    if (name == "CUTOFF") cutoff = juce::jlimit(80.0f, 18000.0f, value);
    else if (name == "ATTACK") attack = juce::jlimit(0.001f, 3.0f, value);
    else if (name == "RELEASE") release = juce::jlimit(0.02f, 5.0f, value);
    else if (name == "REVERB") reverbMix = juce::jlimit(0.0f, 1.0f, value);
}
float K2000AudioProcessor::getToneParameter(const juce::String& name) const
{
    if (name == "CUTOFF") return cutoff;
    if (name == "ATTACK") return attack;
    if (name == "RELEASE") return release;
    if (name == "REVERB") return reverbMix;
    return 0.0f;
}

void K2000AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state("K2000");
    state.setProperty("program", currentProgram, nullptr);
    state.setProperty("layer", currentLayer, nullptr);
    state.setProperty("mode", currentMode, nullptr);
    state.setProperty("cutoff", cutoff, nullptr);
    state.setProperty("attack", attack, nullptr);
    state.setProperty("release", release, nullptr);
    state.setProperty("reverb", reverbMix, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, destData);
}
void K2000AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml(*xml);
        if (state.hasType("K2000"))
        {
            setCurrentProgram(static_cast<int>(state.getProperty("program", 0)));
            setCurrentLayer(static_cast<int>(state.getProperty("layer", 1)));
            currentMode = state.getProperty("mode", "PROGRAM").toString();
            setToneParameter("CUTOFF", state.getProperty("cutoff", 12000.0f));
            setToneParameter("ATTACK", state.getProperty("attack", 0.015f));
            setToneParameter("RELEASE", state.getProperty("release", 0.25f));
            setToneParameter("REVERB", state.getProperty("reverb", 0.12f));
        }
    }
}
juce::AudioProcessorEditor* K2000AudioProcessor::createEditor() { return new K2000AudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new K2000AudioProcessor(); }
