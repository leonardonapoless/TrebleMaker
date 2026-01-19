#include "PluginProcessor.h"
#include "../UI/PluginEditor.h"

TrebleMakerAudioProcessor::TrebleMakerAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
{
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
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // freq 2k-20k
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("freq", 1), "Frequency", 
        juce::NormalisableRange<float>(2000.0f, 20000.0f, 1.0f, 0.4f), 8000.0f));

    // gain 0-8db
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gain", 1), "Gain", 
        juce::NormalisableRange<float>(0.0f, 8.0f, 0.1f, 1.0f), 2.0f));

    // q 0.1-1.5
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("q", 1), "Width (Q)", 
        juce::NormalisableRange<float>(0.1f, 1.5f, 0.01f, 1.0f), 0.7f));

    // boost/cut mode
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("mode", 1), "Reduce Mode", false));

    return { params.begin(), params.end() };
}

void TrebleMakerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (uint32_t) samplesPerBlock;
    spec.numChannels = (uint32_t) getTotalNumOutputChannels();

    filters.clear();
    for (int i = 0; i < getTotalNumOutputChannels(); ++i)
    {
        auto filter = std::make_unique<juce::dsp::StateVariableTPTFilter<float>>();
        filter->prepare(spec);
        // juce's tpt filter doesn't have a shelf mode, so i use a highpass
        // and mix it in later (dry + hp = boost, dry - hp = cut)
        filter->setType(juce::dsp::StateVariableTPTFilterType::highpass);
        filters.push_back(std::move(filter));
    }
    
    dryBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    
    driftPhase = 0.0;
}

void TrebleMakerAudioProcessor::releaseResources()
{
    dryBuffer.setSize(0, 0);
    filters.clear();
}

void TrebleMakerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    const double sampleRate = getSampleRate();
    if (sampleRate <= 0.0)
        return;

    float currentCutoff = *apvts.getRawParameterValue("freq");
    float driveAmount   = *apvts.getRawParameterValue("gain");
    float currentQ      = *apvts.getRawParameterValue("q");
    
    bool isReduceMode = *apvts.getRawParameterValue("mode") > 0.5f;

    const double driftInc = (sampleRate > 0.0) ? (2.0 * juce::MathConstants<double>::pi * 0.2) / sampleRate : 0.0;
    
    driftPhase += driftInc * buffer.getNumSamples();
    
    if (driftPhase > juce::MathConstants<double>::twoPi) 
        driftPhase -= juce::MathConstants<double>::twoPi;

    float driftAmount = (float)std::sin(driftPhase) * 0.005f;
    float analogFreq = currentCutoff * (1.0f + driftAmount);
    
    analogFreq = juce::jlimit(20.0f, (float)(sampleRate * 0.49), analogFreq);
    
    float analogQ = currentQ + (driveAmount * 0.02f); 

    for (auto& filter : filters)
    {
        filter->setCutoffFrequency(analogFreq);
        filter->setResonance(analogQ);
    }

    // copy dry
    int channelsToCopy = juce::jmin(buffer.getNumChannels(), dryBuffer.getNumChannels());
    int samplesToCopy = juce::jmin(buffer.getNumSamples(), dryBuffer.getNumSamples());
    
    for (int ch = 0; ch < channelsToCopy; ++ch)
    {
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, samplesToCopy);
    }

    // process filters
    juce::dsp::AudioBlock<float> block(buffer);
    
    for (size_t ch = 0; ch < (size_t) buffer.getNumChannels(); ++ch)
    {
        if (ch < filters.size())
        {
            auto singleChannelBlock = block.getSingleChannelBlock(ch);
            juce::dsp::ProcessContextReplacing<float> context(singleChannelBlock);
            filters[ch]->process(context);
        }
    }
    
    // input (dry) channels might be fewer than output channels (mono in -> stereo out)
    const int numChannelsToProcess = juce::jmin(buffer.getNumChannels(), dryBuffer.getNumChannels());

    // mix
    if (!isReduceMode)
    {
        // boost
        float boostAmount = juce::Decibels::decibelsToGain(driveAmount) - 1.0f;
        
        for (int ch = 0; ch < numChannelsToProcess; ++ch)
        {
            buffer.applyGain(ch, 0, buffer.getNumSamples(), boostAmount);
            buffer.addFrom(ch, 0, dryBuffer, ch, 0, buffer.getNumSamples());
        }
    }
    else
    {
        // cut
        float cutAmount = juce::Decibels::decibelsToGain(driveAmount); 
        
        for (int ch = 0; ch < numChannelsToProcess; ++ch)
        {
            buffer.applyGain(ch, 0, buffer.getNumSamples(), cutAmount);
            
            auto* dryData = dryBuffer.getReadPointer(ch);
            auto* wetData = buffer.getWritePointer(ch);
            
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                wetData[s] = dryData[s] - wetData[s];
            }
        }
    }

    if (!isReduceMode && driveAmount > 0.1f)
    {
        const float targetDrive = 1.0f + (driveAmount * 0.08f);
        const float dcBias = 0.15f;
        const float invScale = 1.0f / (std::tanh(targetDrive + dcBias) - std::tanh(dcBias));
        const float blend = juce::jmin(driveAmount / 12.0f, 1.0f);

        const float startDrive = smoothDrive;

        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            float currentDrive = startDrive;
            
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                currentDrive += (targetDrive - currentDrive) * 0.001f;

                float in = channelData[sample];
                float x = in * currentDrive + dcBias;
                float out = (std::tanh(x) - std::tanh(dcBias)) * invScale;
                
                channelData[sample] = out * blend + in * (1.0f - blend);
            }
            
            if (channel == totalNumInputChannels - 1)
                smoothDrive = currentDrive;
        }
    }
}

TrebleMakerAudioProcessor::~TrebleMakerAudioProcessor()
{
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


