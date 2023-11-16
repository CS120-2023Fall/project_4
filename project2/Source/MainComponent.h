#pragma once

#include <JuceHeader.h>
#include "MAC.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    // stop. transmit and receive simultaneously.
    enum class juce_States_Set {
        STOP,
        T_AND_R
    };
    juce::TextButton stopButton, T_and_R_Button;
    // state of the outer program
    juce_States_Set juceState{ juce_States_Set::STOP };
    juce::Label mes0, mes1;
    juce::AudioFormatManager formatManager;
    MAC_Layer mac;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};




MainComponent::MainComponent() {
    // Make sure you set the size of the component after
    // you add any child components.
    setSize(600, 400);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio)) {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
        // 不应该进这里
        exit(55);
    }
    else {
        // Specify the number of input and output channels that we want to open
        setAudioChannels(2, 1);
    }

    /////////////////////////////////////////////////////////////
    // stop
    stopButton.setButtonText("Stop");
    stopButton.setSize(60, 40);
    stopButton.setCentrePosition(120, 200);
    stopButton.onClick = [this] {juceState = juce_States_Set::STOP; mac.STOP();
    mes0.setText("stop", juce::NotificationType::dontSendNotification); };
    addAndMakeVisible(stopButton);

    // transmit and receive button
    T_and_R_Button.setButtonText("transmit and receive");
    T_and_R_Button.setSize(110, 40);
    T_and_R_Button.setCentrePosition(220, 200);
    T_and_R_Button.onClick = [this] {juceState = juce_States_Set::T_AND_R;
    mes0.setText("transmit and receive", juce::NotificationType::dontSendNotification); };
    addAndMakeVisible(T_and_R_Button);

    // message
    mes0.setText("project2", juce::NotificationType::dontSendNotification);
    mes0.setSize(400, 40);
    addAndMakeVisible(mes0);

    mes1.setText("addition", juce::NotificationType::dontSendNotification);
    mes0.setSize(400, 60);
    addAndMakeVisible(mes1);

    juce::Label *tmp[2] = { &mes0, &mes1 };
    mac = MAC_Layer(tmp, 2);


    formatManager.registerBasicFormats();       // [1]
    auto dev_info = deviceManager.getAudioDeviceSetup();
    dev_info.sampleRate = 48000;
    deviceManager.setAudioDeviceSetup(dev_info, false);

}

MainComponent::~MainComponent() {
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    //bufferToFill.clearActiveBufferRegion();

    if (juceState == juce_States_Set::STOP) {
        return;
    }
    auto *device = deviceManager.getCurrentAudioDevice();
    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
    auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

    for (int channel = 0; channel < maxInputChannels; ++channel) {
        int inputChannelNum = 1;
        if ((!activeInputChannels[channel] || !activeOutputChannels[channel]) || maxInputChannels == 0) {
            bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
            continue;
        } // [2]
        auto *inBuffer = bufferToFill.buffer->getReadPointer(inputChannelNum, bufferToFill.startSample);
        auto *outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
        int num_samples = bufferToFill.buffer->getNumSamples();
        double x = inBuffer[0];
        x = x + 1;
        std::vector<double> p;
        p.push_back(x);
        //////////////////////////////////////////////////////////
        ///// TODO: set a signal to stop automatically///////////
        ///////////////////////////////////////////////////////
        if (juceState == juce_States_Set::T_AND_R) {
            mac.TxPending = false;
            mac.refresh_MAC(inBuffer, outBuffer, num_samples);
        }
        if (false) {
            mac.TxPending = false;
            stopButton.triggerClick();
        }

        break;
    }
}

void MainComponent::releaseResources() {}

//==============================================================================
void MainComponent::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized() {}