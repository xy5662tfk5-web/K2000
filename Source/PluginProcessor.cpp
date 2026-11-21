#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

K2000AudioProcessor::K2000AudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency(cutoff.load(std::memory_order_relaxed));
    reverbParams.roomSize = 0.45f;
    reverbParams.damping = 0.55f;
    reverbParams.wetLevel = reverbMix.load(std::memory_order_relaxed);
    reverbParams.dryLevel = 1.0f - reverbParams.wetLevel;
    reverb.setParameters(reverbParams);
}

void K2000AudioProcessor::prepareToPlay(double sampleRate, int)
{
    sampleRateHz = juce::jmax(1.0, sampleRate);
    filter.reset();
    filter.prepare({ sampleRateHz, 512, static_cast<juce::uint32>(getTotalNumOutputChannels()) });
    reverb.reset();
    for (auto& voice : voices)
        voice = {};
}

void K2000AudioProcessor::releaseResources() {}

bool K2000AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

void K2000AudioProcessor::noteOn(int note, float velocity)
{
    auto* target = &voices.front();
    for (auto& voice : voices)
        if (!voice.active) { target = &voice; break; }

    *target = {};
    target->note = note;
    target->velocity = juce::jlimit(0.0f, 1.0f, velocity);
    target->held = target->active = true;
}

void K2000AudioProcessor::noteOff(int note)
{
    for (auto& voice : voices)
        if (voice.active && voice.note == note)
            voice.held = false;
}

void K2000AudioProcessor::renderVoice(Voice& voice, juce::AudioBuffer<float>& buffer,
                                      int start, int count)
{
    if (!voice.active || count <= 0 || buffer.getNumChannels() <= 0)
        return;

    const double frequency = juce::MidiMessage::getMidiNoteInHertz(voice.note);
    const double increment = juce::MathConstants<double>::twoPi * frequency / sampleRateHz;
    const float attackSeconds = attack.load(std::memory_order_relaxed);
    const float releaseSeconds = release.load(std::memory_order_relaxed);
    const float attackStep = 1.0f / juce::jmax(1.0f, attackSeconds * static_cast<float>(sampleRateHz));
    const float releaseStep = 1.0f / juce::jmax(1.0f, releaseSeconds * static_cast<float>(sampleRateHz));

    for (int i = 0; i < count; ++i)
    {
        if (voice.held) voice.envelope = juce::jmin(1.0f, voice.envelope + attackStep);
        else voice.envelope = juce::jmax(0.0f, voice.envelope - releaseStep);

        if (!voice.held && voice.envelope <= 0.00001f)
        {
            voice.active = false;
            break;
        }

        const float saw = static_cast<float>((voice.phase / juce::MathConstants<double>::pi) - 1.0);
        const float pulse = std::sin(voice.phase) >= 0.0 ? 0.22f : -0.22f;
        const float sample = (saw * 0.72f + pulse) * voice.envelope * voice.velocity * 0.13f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.addSample(channel, start + i, sample);

        voice.phase += increment;
        if (voice.phase >= juce::MathConstants<double>::twoPi)
            voice.phase -= juce::MathConstants<double>::twoPi;
    }
}

void K2000AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const int numSamples = buffer.getNumSamples();
    int cursor = 0;

    for (const auto metadata : midi)
    {
        const auto message = metadata.getMessage();
        const int position = juce::jlimit(cursor, numSamples, metadata.samplePosition);
        for (auto& voice : voices)
            renderVoice(voice, buffer, cursor, position - cursor);
        cursor = position;

        if (message.isNoteOn())
            noteOn(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff())
            noteOff(message.getNoteNumber());
        else if (message.isProgramChange())
            setCurrentProgram(message.getProgramChangeNumber());
    }

    for (auto& voice : voices)
        renderVoice(voice, buffer, cursor, numSamples - cursor);
    midi.clear();

    const float currentCutoff = cutoff.load(std::memory_order_relaxed);
    const float currentReverbMix = reverbMix.load(std::memory_order_relaxed);
    filter.setCutoffFrequency(currentCutoff);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);

    reverbParams.wetLevel = currentReverbMix;
    reverbParams.dryLevel = 1.0f - currentReverbMix * 0.55f;
    reverb.setParameters(reverbParams);
    if (buffer.getNumChannels() == 2)
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), numSamples);
    else if (buffer.getNumChannels() == 1)
        reverb.processMono(buffer.getWritePointer(0), numSamples);
}

int K2000AudioProcessor::getCurrentProgram()
{
    return currentProgram.load(std::memory_order_relaxed);
}

void K2000AudioProcessor::setCurrentProgram(int index)
{
    currentProgram.store(juce::jlimit(0, 127, index), std::memory_order_relaxed);
}

const juce::String K2000AudioProcessor::getProgramName(int index)
{
    return "Program " + juce::String(juce::jlimit(0, 127, index) + 1).paddedLeft('0', 3);
}

void K2000AudioProcessor::changeProgramName(int, const juce::String&) {}

void K2000AudioProcessor::selectMode(const juce::String& mode)
{
    const juce::ScopedLock lock(modeLock);
    currentMode = mode;
}

juce::String K2000AudioProcessor::getMode() const
{
    const juce::ScopedLock lock(modeLock);
    return currentMode;
}

void K2000AudioProcessor::setCurrentLayer(int layer)
{
    currentLayer.store(juce::jlimit(1, 3, layer), std::memory_order_relaxed);
}

int K2000AudioProcessor::getCurrentLayer() const
{
    return currentLayer.load(std::memory_order_relaxed);
}

void K2000AudioProcessor::selectProgramByStep(int delta)
{
    int previous = currentProgram.load(std::memory_order_relaxed);
    int next = 0;
    do
    {
        next = (previous + delta + 128) % 128;
    }
    while (!currentProgram.compare_exchange_weak(previous, next, std::memory_order_relaxed));
}

void K2000AudioProcessor::setToneParameter(const juce::String& name, float value)
{
    if (name == "CUTOFF") cutoff.store(juce::jlimit(80.0f, 18000.0f, value), std::memory_order_relaxed);
    else if (name == "ATTACK") attack.store(juce::jlimit(0.001f, 3.0f, value), std::memory_order_relaxed);
    else if (name == "RELEASE") release.store(juce::jlimit(0.02f, 5.0f, value), std::memory_order_relaxed);
    else if (name == "REVERB") reverbMix.store(juce::jlimit(0.0f, 1.0f, value), std::memory_order_relaxed);
}

float K2000AudioProcessor::getToneParameter(const juce::String& name) const
{
    if (name == "CUTOFF") return cutoff.load(std::memory_order_relaxed);
    if (name == "ATTACK") return attack.load(std::memory_order_relaxed);
    if (name == "RELEASE") return release.load(std::memory_order_relaxed);
    if (name == "REVERB") return reverbMix.load(std::memory_order_relaxed);
    return 0.0f;
}

void K2000AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state("K2000");
    state.setProperty("program", getCurrentProgram(), nullptr);
    state.setProperty("layer", getCurrentLayer(), nullptr);
    state.setProperty("mode", getMode(), nullptr);
    state.setProperty("cutoff", getToneParameter("CUTOFF"), nullptr);
    state.setProperty("attack", getToneParameter("ATTACK"), nullptr);
    state.setProperty("release", getToneParameter("RELEASE"), nullptr);
    state.setProperty("reverb", getToneParameter("REVERB"), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void K2000AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return;
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml(*xml);
        if (!state.hasType("K2000"))
            return;
        setCurrentProgram(static_cast<int>(state.getProperty("program", 0)));
        setCurrentLayer(static_cast<int>(state.getProperty("layer", 1)));
        selectMode(state.getProperty("mode", "PROGRAM").toString());
        setToneParameter("CUTOFF", static_cast<float>(state.getProperty("cutoff", 12000.0f)));
        setToneParameter("ATTACK", static_cast<float>(state.getProperty("attack", 0.015f)));
        setToneParameter("RELEASE", static_cast<float>(state.getProperty("release", 0.25f)));
        setToneParameter("REVERB", static_cast<float>(state.getProperty("reverb", 0.12f)));
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
