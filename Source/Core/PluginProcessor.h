#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace PID
{
    static const juce::String freq { "freq" };
    static const juce::String gain { "gain" };
    static const juce::String q    { "q" };
    static const juce::String mode { "mode" };
}

class TrebleMakerAudioProcessor  : public juce::AudioProcessor,
                                   public juce::AudioProcessorValueTreeState::Listener
{
public:
    TrebleMakerAudioProcessor();
    ~TrebleMakerAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;

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

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void updateParameters(float& outCutoff, float& outRes, float& outDrive, bool& outMode);
    float getAnalogDrift();
    void updateFilters(float cutoff, float q, float drift);
    void processSaturation(juce::AudioBuffer<float>& buffer, float drive, bool reduceMode);

    std::vector<std::unique_ptr<juce::dsp::StateVariableTPTFilter<float>>> filters;
    juce::AudioBuffer<float> dryBuffer;

    juce::Random random;
    float driftValue = 0.0f;


    float smoothDrive = 0.0f;
    
    // mode memory
    struct ModeSettings
    {
        float gain = 2.0f;
        float q    = 0.7f;
    };
    
    ModeSettings boostSettings;
    ModeSettings reduceSettings { 3.5f, 0.7f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrebleMakerAudioProcessor)
};
