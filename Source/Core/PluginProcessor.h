#pragma once

#include <JuceHeader.h>

class TrebleMakerAudioProcessor  : public juce::AudioProcessor
{
public:
    TrebleMakerAudioProcessor();
    ~TrebleMakerAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // parameter state
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // tpt filters for better modulation response
    std::vector<std::unique_ptr<juce::dsp::StateVariableTPTFilter<float>>> filters;
    
    // dry buffer for mix
    juce::AudioBuffer<float> dryBuffer;

    // drift lfo
    double driftPhase = 0.0;
    
    // smoothed drive
    float  smoothDrive = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrebleMakerAudioProcessor)
};
