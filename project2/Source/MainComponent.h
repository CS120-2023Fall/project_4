#pragma once

/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             ProcessingAudioInputTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Performs processing on an input signal.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#include<array>
#include<omp.h>
#include<deque>
#define _USE_MATH_DEFINES
#include<cmath>

#pragma once
#define PI acos(-1)
class MainContentComponent;
#include "MAC.h"
#include <JuceHeader.h>
/*
This project refers to the JUCE official examples. (www.JUCE.com)

*/
bool start_csma = false;
class MainContentComponent : public juce::AudioAppComponent {
    friend class Record;
public:
    //==============================================================================
    MainContentComponent() {
        setSize(600, 400);

        // playButton 
      
        stopButton.setButtonText("Stop");
        stopButton.setSize(60, 40);
        stopButton.setCentrePosition(120, 200);
        stopButton.onClick = [this] {
          juceState = juce_States_Set::STOP;

          mac.STOP();
          mac.macState = MAC_Layer::MAC_States_Set::Idle;
          mes0.setText("stop", juce::NotificationType::dontSendNotification);
        };
        addAndMakeVisible(stopButton);

        // transmit and receive button
        T_and_R_Button.setButtonText("transmit and receive");
        T_and_R_Button.setSize(110, 40);
        T_and_R_Button.setCentrePosition(220, 200);
        T_and_R_Button.onClick = [this] {juceState = juce_States_Set::T_AND_R; mac.Start();
        mes0.setText("transmit and receive", juce::NotificationType::dontSendNotification); };
        addAndMakeVisible(T_and_R_Button);




        // transmit and receive button
        csmaWithJamButton.setButtonText("csma_task");
        csmaWithJamButton.setSize(110, 40);
        csmaWithJamButton.setCentrePosition(330, 200);
        csmaWithJamButton.onClick = [this] {start_csma = true;
        mes0.setText("csma_task", juce::NotificationType::dontSendNotification); };
        addAndMakeVisible(csmaWithJamButton);

        csmaButton.setButtonText("csma_with_jam");
        csmaButton.setSize(110, 40);
        csmaButton.setCentrePosition(440, 200);
        csmaButton.onClick = [this] {start_csma = true;
        mes0.setText("csma_with_jam_task", juce::NotificationType::dontSendNotification); };
        addAndMakeVisible(csmaButton);
        // message
        mes0.setText("project2", juce::NotificationType::dontSendNotification);
        mes0.setSize(400, 40);
        addAndMakeVisible(mes0);

        mes1.setText("addition", juce::NotificationType::dontSendNotification);
        mes0.setSize(400, 60);
        addAndMakeVisible(mes1);

        juce::Label* tmp[2] = { &mes0, &mes1 };
        mac = MAC_Layer(tmp, 2);
        // recordButton
       
        addAndMakeVisible(receiver_with_wireButton);
        setAudioChannels(2, 1);

        formatManager.registerBasicFormats();       // [1]
        auto dev_info = deviceManager.getAudioDeviceSetup();
        dev_info.sampleRate = 48000;
        deviceManager.setAudioDeviceSetup(dev_info, false);
    }

   

    ~MainContentComponent() override {
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void releaseResources() override {
        transportSource.releaseResources();
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* device = deviceManager.getCurrentAudioDevice();
        auto activeInputChannels = device->getActiveInputChannels();
        auto activeOutputChannels = device->getActiveOutputChannels();
        auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
        auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

        for (int channel = 0; channel < maxInputChannels; ++channel) {
            //int channel = 0;
            int actualInputChannel = 1; // [1]
            if ((!activeInputChannels[channel] || !activeOutputChannels[channel]) || maxInputChannels == 0) {
                bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
                continue;
            } // [2]
            auto* inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannel,
                bufferToFill.startSample);
            int num_samples = bufferToFill.buffer->getNumSamples();
            auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
            KeepSilence( inBuffer, outBuffer,  num_samples);
            if (start_csma) {
                juceState = juce_States_Set::T_AND_R;
                mac.TxPending = true;
                if (mac.wait) {
                    mac.TxPending = false;
                }
                mac.refresh_MAC(inBuffer, outBuffer, num_samples);
                return;
            }
            if (juceState == juce_States_Set::T_AND_R) {
                mac.TxPending = true;
                if (mac.wait) {
                    mac.TxPending = false;
                }
                mac.refresh_MAC(inBuffer, outBuffer, num_samples);
            }
            else
            {

                for (int i = 0; i < num_samples; i++) {
                    outBuffer[i] = 0;
                }
            }
            for (int i = 0; i < num_samples; i++) {
                double tmp = outBuffer[i];
                if (i >= PREAMBLE_SIZE) {
                    int xxxxx1 = 111;
                    xxxxx1++;
                }
            }
            if (false) {
                mac.TxPending = false;
                stopButton.triggerClick();
            }
            if (mac.macState == MAC_Layer::MAC_States_Set::LinkError) {
            
                stopButton.triggerClick();
            }   
            break;
        }
    }
    void resized() override {}


private:
    juce::Label mes;
    juce::Random random;
    juce::TextButton playButton, stopButton, recordButton, recordWithPredefinedButton, openButton, testButton, transmitButton, transmit_with_wireButton, receiverButton, receiver_with_wireButton;
    juce::TextButton csmaButton,csmaWithJamButton;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    bool isPlayingPredefined{ false };
    enum class juce_States_Set {
        STOP,
        T_AND_R
    };
    juce::TextButton T_and_R_Button;
    // state of the outer program
    juce_States_Set juceState{ juce_States_Set::STOP };
    juce::Label mes0, mes1;
    MAC_Layer mac;
    // a buffer to save the result of the fft, sized 512.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
