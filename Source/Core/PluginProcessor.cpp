#include "PluginProcessor.h"
#include "../UI/PluginEditor.h"

TrebleMakerAudioProcessor::TrebleMakerAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    apvts.addParameterListener(PID::mode, this);
}

TrebleMakerAudioProcessor::~TrebleMakerAudioProcessor()
{
    apvts.removeParameterListener(PID::mode, this);
}

const juce::String TrebleMakerAudioProcessor::getName() const { return "TrebleMaker"; }

bool TrebleMakerAudioProcessor::acceptsMidi() const { return false; }
bool TrebleMakerAudioProcessor::producesMidi() const { return false; }
bool TrebleMakerAudioProcessor::isMidiEffect() const { return false; }
double TrebleMakerAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int TrebleMakerAudioProcessor::getNumPrograms() { return 1; }
int TrebleMakerAudioProcessor::getCurrentProgram() { return 0; }
void TrebleMakerAudioProcessor::setCurrentProgram (int) {}
const juce::String TrebleMakerAudioProcessor::getProgramName (int) { return "Default"; }
void TrebleMakerAudioProcessor::changeProgramName (int, const juce::String&) {}

juce::AudioProcessorValueTreeState::ParameterLayout TrebleMakerAudioProcessor::createParameterLayout()
{
    auto freqRange  = juce::NormalisableRange<float>(2000.0f, 20000.0f, 1.0f, 0.4f);
    auto gainRange  = juce::NormalisableRange<float>(0.0f, 8.0f, 0.1f, 1.0f);
    auto qRange     = juce::NormalisableRange<float>(0.1f, 1.5f, 0.01f, 1.0f);

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(PID::freq, 1),  "Frequency", freqRange, 2000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(PID::gain, 1),  "Gain",      gainRange, 3.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(PID::q, 1),     "Q Factor",  qRange,    0.7f));
    params.push_back(std::make_unique<juce::AudioParameterBool> (juce::ParameterID(PID::mode, 1),  "Reduce Mode", false));

    return { params.begin(), params.end() };
}

void TrebleMakerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec { sampleRate, (uint32_t) samplesPerBlock, (uint32_t) getTotalNumOutputChannels() };

    filters.clear();
    for (int i = 0; i < getTotalNumOutputChannels(); ++i)
    {
        auto f = std::make_unique<juce::dsp::StateVariableTPTFilter<float>>();
        f->prepare(spec);
        f->setType(juce::dsp::StateVariableTPTFilterType::highpass);
        filters.push_back(std::move(f));
    }
    
    dryBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    
    driftValue = (random.nextFloat() * 2.0f - 1.0f) * 0.1f;
}

void TrebleMakerAudioProcessor::releaseResources()
{
    dryBuffer.setSize(0, 0);
    filters.clear();
}

void TrebleMakerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (getSampleRate() <= 0.0) return;

    float cutoff, res, drive;
    bool isReduceMode;
    
    updateParameters(cutoff, res, drive, isReduceMode);
    
    float drift = getAnalogDrift();
    updateFilters(cutoff, res, drift);

    int numSamples = buffer.getNumSamples();
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    juce::dsp::AudioBlock<float> block(buffer);
    for (size_t ch = 0; ch < filters.size(); ++ch)
    {
        if (ch >= (size_t)buffer.getNumChannels()) break;
        
        auto singleChannelBlock = block.getSingleChannelBlock(ch);
        juce::dsp::ProcessContextReplacing<float> context(singleChannelBlock);
        filters[ch]->process(context);
    }
    
    processSaturation(buffer, drive, isReduceMode);
}

void TrebleMakerAudioProcessor::updateParameters(float& outCutoff, float& outRes, float& outDrive, bool& outMode)
{
    outCutoff = apvts.getRawParameterValue(PID::freq)->load();
    outDrive  = apvts.getRawParameterValue(PID::gain)->load();
    outRes    = apvts.getRawParameterValue(PID::q)->load();
    outMode   = apvts.getRawParameterValue(PID::mode)->load() > 0.5f;
}

float TrebleMakerAudioProcessor::getAnalogDrift()
{
    const float nudge = (random.nextFloat() - 0.5f) * 0.002f;
    driftValue = std::clamp(driftValue + nudge, -0.05f, 0.05f);
    
    driftValue *= 0.999f;
    
    return driftValue;
}

void TrebleMakerAudioProcessor::updateFilters(float cutoff, float q, float drift)
{
    const float modCutoff = juce::jlimit(20.0f, (float)(getSampleRate() * 0.49), cutoff * (1.0f + drift));
    
    const float compensatedQ = q * (1.0f + (smoothDrive * 0.05f));

    for (auto& f : filters)
    {
        f->setCutoffFrequency(modCutoff);
        f->setResonance(compensatedQ);
    }
}

void TrebleMakerAudioProcessor::processSaturation(juce::AudioBuffer<float>& buffer, float drive, bool reduceMode)
{
    float gainFactor = juce::Decibels::decibelsToGain(drive);
    
    smoothDrive += (gainFactor - smoothDrive) * 0.05f;
    
    const float saturationAmount = juce::jmax(0.0f, (smoothDrive - 1.0f) * 0.5f);
    
    auto* dryReader = dryBuffer.getArrayOfReadPointers();
    auto* wetWriter = buffer.getArrayOfWritePointers();
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float wet = wetWriter[ch][i]; 
            float dry = dryReader[ch][i];
            
            if (saturationAmount > 0.01f)
            {
                wet = std::tanh(wet * smoothDrive);
            }
            else
            {
                wet *= smoothDrive;
            }
            
            if (reduceMode)
            {
                wetWriter[ch][i] = dry - wet;
            }
            else
            {
                float boostGain = juce::jmax(0.0f, smoothDrive - 1.0f); 
                wetWriter[ch][i] = dry + (wet * boostGain); 
            }
        }
    }
}

bool TrebleMakerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TrebleMakerAudioProcessor::createEditor()
{
    return new TrebleMakerEditor (*this);
}

void TrebleMakerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TrebleMakerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TrebleMakerAudioProcessor();
}

void TrebleMakerAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == PID::mode)
    {
        bool isReduce = (newValue > 0.5f);
        
        auto* gainParam = apvts.getParameter(PID::gain);
        auto* qParam    = apvts.getParameter(PID::q);
        
        if (isReduce)
        {
            boostSettings.gain = gainParam->convertFrom0to1(gainParam->getValue());
            boostSettings.q    = qParam->convertFrom0to1(qParam->getValue());
            
            // restore reduce settings
            gainParam->setValueNotifyingHost(gainParam->convertTo0to1(reduceSettings.gain));
            qParam->setValueNotifyingHost(qParam->convertTo0to1(reduceSettings.q));
        }
        else
        {
            reduceSettings.gain = gainParam->convertFrom0to1(gainParam->getValue());
            reduceSettings.q    = qParam->convertFrom0to1(qParam->getValue());
            
            // restore boost settings
            gainParam->setValueNotifyingHost(gainParam->convertTo0to1(boostSettings.gain));
            qParam->setValueNotifyingHost(qParam->convertTo0to1(boostSettings.q));
        }
    }
}


