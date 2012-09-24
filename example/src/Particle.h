/*
 *  Particle.h
 *  ofxMSAFluid Demo
 *
 *  Created by Mehmet Akten on 02/05/2009.
 *  Copyright 2009 MSA Visuals Ltd.. All rights reserved.
 *
 */

#pragma once

#include "MSACore.h"
#include "MSAFluidSolver.h"

class Particle {
public:	
    ofVec2f	pos, vel;
    float	radius;
    float	alpha;
    float	mass;
	
    void init(float x, float y);
    void update( const msa::fluid::Solver &solver, const ofVec2f &windowSize, const ofVec2f &invWindowSize );
	void updateVertexArrays( bool drawingFluid, const ofVec2f &invWindowSize, int i, float* posBuffer, float* colBuffer);
};

