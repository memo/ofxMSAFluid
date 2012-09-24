/*
 *  ParticleSystem.h
 *  ofxMSAFluid Demo
 *
 *  Created by Mehmet Akten on 02/05/2009.
 *  Copyright 2009 MSA Visuals Ltd.. All rights reserved.
 *
 */
#pragma once

#include "Particle.h"

#define MAX_PARTICLES		50000

class ParticleSystem {
public:	
	
    float posArray[MAX_PARTICLES * 2 * 2];
    float colArray[MAX_PARTICLES * 3 * 2];
    int curIndex;
	
    Particle particles[MAX_PARTICLES];
	
	ParticleSystem();

    void updateAndDraw(const msa::fluid::Solver &aSolver, ofVec2f windowSize, bool drawingFluid);
	void addParticles(const ofVec2f &pos, int count);
	void addParticle(const ofVec2f &pos);
};

