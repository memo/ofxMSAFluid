/*
 *  Particle.cpp
 *  ofxMSAFluid Demo
 *
 *  Created by Mehmet Akten on 02/05/2009.
 *  Copyright 2009 MSA Visuals Ltd.. All rights reserved.
 *
 */

#include "Particle.h"

static const float MOMENTUM = 0.5f;
static const float FLUID_FORCE = 0.6f;

void Particle::init(float x, float y) {
	pos = ofVec2f( x, y );
	vel = ofVec2f(0, 0);
	radius = 5;
	alpha  = msa::Rand::randFloat( 0.3f, 1 );
	mass = msa::Rand::randFloat( 0.1f, 1 );
}

void Particle::update( const msa::fluid::Solver &solver, const ofVec2f &windowSize, const ofVec2f &invWindowSize ) {
	// only update if particle is visible
	if( alpha == 0 )
		return;
	
	vel = solver.getVelocityAtPos( pos * invWindowSize ) * (mass * FLUID_FORCE ) * windowSize + vel * MOMENTUM;
	pos += vel;	
	
	// bounce of edges
	if( pos.x < 0 ) {
		pos.x = 0;
		vel.x *= -1;
	}
	else if( pos.x > windowSize.x ) {
		pos.x = windowSize.x;
		vel.x *= -1;
	}
	
	if( pos.y < 0 ) {
		pos.y = 0;
		vel.y *= -1;
	}
	else if( pos.y > windowSize.y ) {
		pos.y = windowSize.y;
		vel.y *= -1;
	}
	
	// hackish way to make particles glitter when the slow down a lot
//	if( vel.squareLength() < 1 ) {
//		vel += msa::Rand::randVec2f() * 0.5f;
//	}
	
	// fade out a bit (and kill if alpha == 0);
	alpha *= 0.999f;
	if( alpha < 0.01f )
		alpha = 0;
}



void Particle::updateVertexArrays( bool drawingFluid, const ofVec2f &invWindowSize, int i, float* posBuffer, float* colBuffer) {
	int vi = i * 4;
	posBuffer[vi++] = pos.x - vel.x;
	posBuffer[vi++] = pos.y - vel.y;
	posBuffer[vi++] = pos.x;
	posBuffer[vi++] = pos.y;
	
	int ci = i * 6;
	if( drawingFluid ) {
		// if drawing fluid, draw lines as black & white
		colBuffer[ci++] = alpha;
		colBuffer[ci++] = alpha;
		colBuffer[ci++] = alpha;
		colBuffer[ci++] = alpha;
		colBuffer[ci++] = alpha;
		colBuffer[ci++] = alpha;
	} else {
		// otherwise, use color
		float vxNorm = vel.x * invWindowSize.x;
		float vyNorm = vel.y * invWindowSize.y;
		float v2 = vxNorm * vxNorm + vyNorm * vyNorm;
#define VMAX 0.013f
		if(v2>VMAX*VMAX) v2 = VMAX*VMAX;
		float satInc = mass > 0.5 ? mass * mass * mass : 0;
		satInc *= satInc * satInc * satInc;
		ofColor color;
		color.setHsb(0, v2 * 255.0f / ( VMAX * VMAX ) + satInc, ofLerp(0.5, 1, mass) * alpha * 255.0f);
		
		colBuffer[ci++] = color.r;
		colBuffer[ci++] = color.g;
		colBuffer[ci++] = color.b;
		colBuffer[ci++] = color.r;
		colBuffer[ci++] = color.g;
		colBuffer[ci++] = color.b;
	}
}