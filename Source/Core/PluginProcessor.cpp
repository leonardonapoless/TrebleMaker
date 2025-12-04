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

juce::AudioProcessorValueTreeState::ParameterLayout TrebleMakerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // freq 2k-20k
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "freq", "Frequency", 
        juce::NormalisableRange<float>(2000.0f, 20000.0f, 1.0f, 0.4f), 8000.0f));

    // gain 0-8db
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gain", "Gain", 
        juce::NormalisableRange<float>(0.0f, 8.0f, 0.1f, 1.0f), 2.0f));

    // q 0.1-1.5
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "q", "Width (Q)", 
        juce::NormalisableRange<float>(0.1f, 1.5f, 0.01f, 1.0f), 0.7f));

    // boost/cut mode
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "mode", "Reduce Mode", false));

    return { params.begin(), params.end() };
}

void TrebleMakerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

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
    
    dryBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    
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

    float currentCutoff = *apvts.getRawParameterValue("freq");
    float driveAmount   = *apvts.getRawParameterValue("gain");
    float currentQ      = *apvts.getRawParameterValue("q");
    
    bool isReduceMode = *apvts.getRawParameterValue("mode") > 0.5f;

    // analog drift
    double driftAmount = std::sin(driftPhase) * 0.005;
    
    // lfo increment
    driftPhase += (2.0 * juce::MathConstants<double>::pi * 0.2) / getSampleRate() * buffer.getNumSamples();
    
    if (driftPhase > juce::MathConstants<double>::twoPi) 
        driftPhase -= juce::MathConstants<double>::twoPi;
    
    float analogFreq = currentCutoff * (1.0f + (float)driftAmount);
    
    // link q to gain
    float analogQ = currentQ + (driveAmount * 0.02f); 

    for (auto& filter : filters)
    {
        filter->setCutoffFrequency(analogFreq);
        filter->setResonance(analogQ);
    }

    // copy dry
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, buffer.getNumSamples());
    }

    // process filters
    juce::dsp::AudioBlock<float> block(buffer);
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        if (ch < filters.size())
        {
            auto singleChannelBlock = block.getSingleChannelBlock(ch);
            juce::dsp::ProcessContextReplacing<float> context(singleChannelBlock);
            filters[ch]->process(context);
        }
    }
    
    // mix
    if (!isReduceMode)
    {
        // boost
        float boostAmount = juce::Decibels::decibelsToGain(driveAmount) - 1.0f;
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            buffer.applyGain(ch, 0, buffer.getNumSamples(), boostAmount);
            buffer.addFrom(ch, 0, dryBuffer, ch, 0, buffer.getNumSamples());
        }
    }
    else
    {
        // cut
        float cutAmount = juce::Decibels::decibelsToGain(driveAmount); 
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
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

    // saturation
    if (!isReduceMode && driveAmount > 0.1f)
    {
        float targetDrive = 1.0f + (driveAmount * 0.08f); 
        smoothDrive = smoothDrive * 0.95f + targetDrive * 0.05f;

        const float dcBias = 0.15f; 

        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float in = channelData[sample];

                float x = in * smoothDrive;
                x += dcBias;

                // soft clip tanh
                float saturated = std::tanh(x);
                float out = saturated - std::tanh(dcBias);
                
                out /= (std::tanh(smoothDrive + dcBias) - std::tanh(dcBias));

                float blend = juce::jmin(driveAmount / 12.0f, 1.0f);
                
                channelData[sample] = (out * blend) + (in * (1.0f - blend));
            }
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

const juce::String TrebleMakerAudioProcessor::getName() const { return "TrebleMaker"; }

bool TrebleMakerAudioProcessor::acceptsMidi() const { return false; }
bool TrebleMakerAudioProcessor::producesMidi() const { return false; }
bool TrebleMakerAudioProcessor::isMidiEffect() const { return false; }
double TrebleMakerAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int TrebleMakerAudioProcessor::getNumPrograms() { return 1; }
int TrebleMakerAudioProcessor::getCurrentProgram() { return 0; }
void TrebleMakerAudioProcessor::setCurrentProgram (int index) {}
const juce::String TrebleMakerAudioProcessor::getProgramName (int index) { return "Default"; }
void TrebleMakerAudioProcessor::changeProgramName (int index, const juce::String& newName) {}
