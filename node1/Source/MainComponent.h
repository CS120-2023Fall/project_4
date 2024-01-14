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
class MainContentComponent;
#include "MAC.h"
#include <JuceHeader.h>
#define PI juce::MathConstants<double>::pi

std::vector<float> in_data;

namespace my_log {
#include <iostream>
#include <Windows.h>

    class PrintDebugConsole {
    public:
        PrintDebugConsole() {
            AllocConsole();
            SetConsoleTitle(TEXT("Juce Debug Window"));
            freopen("conin$", "r", stdin);
            freopen("conout$", "w", stdout);
            freopen("conout$", "w", stderr);

            std::cout << "Welcome..." << std::endl;
        }

        ~PrintDebugConsole() {
            fclose(stdin);
            fclose(stdout);
            fclose(stderr);
            FreeConsole();
        }
    };

    static const PrintDebugConsole staticPrintConsole = PrintDebugConsole();
}
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
          start_csma = false;
          mac.macState = MAC_Layer::MAC_States_Set::Idle;

          mes0.setText("stop", juce::NotificationType::dontSendNotification);
          if (RECORD_IN_LIVE) {
            Write("inBuffer_log.txt", in_data);
          }
          in_data.clear();

        };
        addAndMakeVisible(stopButton);

        testButton.setButtonText("Test");
        testButton.setSize(60, 40);
        testButton.setCentrePosition(120, 240);
        testButton.onClick = [this] {juceState = juce_States_Set::TEST;
        mes0.setText("testing", juce::NotificationType::dontSendNotification);
        std::cout << "test button clicked" << std::endl; };
        addAndMakeVisible(testButton);


        // transmit and receive button
        T_and_R_Button.setButtonText("transmit and receive");
        T_and_R_Button.setSize(110, 40);
        T_and_R_Button.setCentrePosition(220, 200);
        T_and_R_Button.onClick = [this] {juceState = juce_States_Set::T_AND_R; mac.Start();
        mes0.setText("transmit and receive", juce::NotificationType::dontSendNotification); 
        mes1.setText("null", juce::NotificationType::dontSendNotification);
        mes2.setText("null", juce::NotificationType::dontSendNotification);
        mes3.setText("null", juce::NotificationType::dontSendNotification);
            };
        addAndMakeVisible(T_and_R_Button);


        // transmit and receive button
        //csmaWithJamButton.setButtonText("csma_task");
        //csmaWithJamButton.setSize(110, 40);
        //csmaWithJamButton.setCentrePosition(330, 200);
        //csmaWithJamButton.onClick = [this] {start_csma = true; juceState = juce_States_Set::T_AND_R ; mac.Start();
        //mes0.setText("csma_task", juce::NotificationType::dontSendNotification); };
        //addAndMakeVisible(csmaWithJamButton);

        recordButton.setButtonText("reflect the sound");
        recordButton.setSize(130, 40);
        recordButton.setCentrePosition(460, 200);
        recordButton.onClick = [this] {
            juceState = juce_States_Set::RECORD;
            record_max = 0;
        };
        addAndMakeVisible(recordButton);

        if (!CORNER_LOG) {
            //mes0.setCentrePosition(800, 40);
            mes1.setCentrePosition(800, 40);
            mes2.setCentrePosition(800, 40);
            mes3.setCentrePosition(800, 40);
        }


        // message
        mes0.setText("project2", juce::NotificationType::dontSendNotification);
        mes0.setSize(400, 40);
        addAndMakeVisible(mes0);

        mes1.setText("addition mes", juce::NotificationType::dontSendNotification);
        mes1.setSize(400, 90);
        addAndMakeVisible(mes1);

        mes2.setText("addition mes2", juce::NotificationType::dontSendNotification);
        mes2.setSize(400, 130);
        addAndMakeVisible(mes2);

        mes3.setText("addition mes3", juce::NotificationType::dontSendNotification);
        mes3.setSize(400, 170);
        addAndMakeVisible(mes3);

        juce::Label* tmp[4] = { &mes0, &mes1, &mes2, &mes3};
        mac = MAC_Layer(tmp, 4);
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

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {}

    void releaseResources() override {}

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
            if (juceState == juce_States_Set::T_AND_R) {
                mac.TxPending = false;
                if (mac.startTransmitting && mac.transmitter.transmitted_packet * NUM_PACKET_DATA_BITS  < 50000 && !mac.wait) {
                    mac.TxPending = true;
                }
               
                // Record the inBuffer. Watch out memory overflow.
                if (RECORD_IN_LIVE)
                for (int i = 0; i < num_samples; ++i) {
                    in_data.emplace_back(inBuffer[i]);
                }
                mac.refresh_MAC(inBuffer, outBuffer, num_samples);

            }
            else if (juceState == juce_States_Set::STOP) {
                for (int i = 0; i < num_samples; i++) {
                    outBuffer[i] = 0;
                }
            }
            else if (juceState == juce_States_Set::TEST) {
                double pi = juce::MathConstants<double>::pi;
                for (int i = 0; i < num_samples; ++i) {
                    outBuffer[i] = (float)std::sin(2 * pi * 100 * i/48000);
                }
                return;
            }
            else if (juceState == juce_States_Set::RECORD) {
                record_max = 0;
                for (int i = 0; i < num_samples; ++i) {
                    record_max = record_max > abs(inBuffer[i]) ? record_max : abs(inBuffer[i]);
                }
                mes0.setText("the record_max is :" + std::to_string(record_max),juce::dontSendNotification);
            }
            if (mac.macState == MAC_Layer::MAC_States_Set::LinkError) {
                std::cout << "received: " << mac.receiver.received_packet << std::endl;
                std::cout << "transitted: " << mac.transmitter.transmitted_packet << std::endl;
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
    juce::AudioFormatManager formatManager;
    float record_max=0;
    enum class juce_States_Set {
        STOP,
        T_AND_R,
        TEST,
        RECORD,
    };
    juce::TextButton T_and_R_Button;
    // state of the outer program
    juce_States_Set juceState{ juce_States_Set::STOP };
    juce::Label mes0, mes1, mes2, mes3;
    MAC_Layer mac;
    // a buffer to save the result of the fft, sized 512.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
