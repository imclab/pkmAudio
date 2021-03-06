

/*
 *  pkmAudioSpectralFlux.cpp
 *  avSaliency
 *
 *  Created by Mr. Magoo on 8/30/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "pkmAudioSpectralFlux.h"

pkmAudioSpectralFlux::pkmAudioSpectralFlux(int fS, int fftSize, int sampleRate)
{
    bInit = bSegmenting = false;
    frameSize = fS;
    fluxHistorySize = sampleRate / fS * 1.0;
    fft = new pkmFFT(fftSize);
    magnitudes = pkm::Mat(1, fftSize / 2, true);
    phases = pkm::Mat(1, fftSize / 2, true);
    fluxHistory = pkm::Mat(fluxHistorySize, fftSize / 2, true);
    numFramesSinceLastOnset = 0;
    minSegmentLength = sampleRate / fS * 0.05;
    threshold = 1;
    currentFlux = 0;
}

pkmAudioSpectralFlux::~pkmAudioSpectralFlux()
{
    delete fft;
}

float pkmAudioSpectralFlux::getCurrentFlux()
{
    return currentFlux;
}

float pkmAudioSpectralFlux::getFlux(float *audioSignal)
{   
    fft->forward(0, audioSignal, magnitudes.data, phases.data);
    
    return getFlux(magnitudes.data, magnitudes.size());
}


float pkmAudioSpectralFlux::getFlux(float *magnitudes, int length)
{   
    
    if (bInit) {
        pkm::Mat meanFlux = fluxHistory.mean();
        pkm::Mat stdFlux = fluxHistory.stddev();
        
        fluxHistory.insertRowCircularly(magnitudes);
        
        pkm::Mat magnitudesMat(1, length, magnitudes, true);
        
        magnitudesMat.subtract(meanFlux);
        magnitudesMat.divide(stdFlux);
        magnitudesMat.sqr();
        currentFlux = magnitudesMat.sumAll() / magnitudesMat.size();
    }
    else {
        currentFlux = 0;
        bInit = true;
    }
    
    return currentFlux;
}

bool pkmAudioSpectralFlux::detectOnset(float *audioSignal)
{
    fft->forward(0, audioSignal, magnitudes.data, phases.data);
    
    return detectOnset(magnitudes.data, magnitudes.size());
}


bool pkmAudioSpectralFlux::detectOnset(float *magnitudes, int length)
{
    currentFlux = getFlux(magnitudes, length);
    
    if (isnan(currentFlux) | isinf(currentFlux)) {
        numFramesSinceLastOnset = 0;
        return false;
    }
    else {
        numFramesSinceLastOnset++;
    }
    
    if (!bSegmenting && currentFlux > threshold && numFramesSinceLastOnset > minSegmentLength) {
        numFramesSinceLastOnset = 0;
        bSegmenting = true;
        return true;
    }
    if (bSegmenting && currentFlux < threshold && numFramesSinceLastOnset > minSegmentLength) {
        numFramesSinceLastOnset = 0;
        bSegmenting = false;
        return true;
    }
    else {
        numFramesSinceLastOnset++;
        return false;
    }
}

void pkmAudioSpectralFlux::setOnsetThreshold(float thresh)
{
    threshold = thresh;
}

float pkmAudioSpectralFlux::getOnsetThreshold()
{
    return threshold;
}

void pkmAudioSpectralFlux::setMinSegmentLength(int frames)
{
    minSegmentLength = frames;
}

float pkmAudioSpectralFlux::getMinSegmentLength()
{
    return minSegmentLength;
}











