/*
 *  ParticleSystem.cpp
 *  ofxMSAFluid Demo
 *
 *  Created by Mehmet Akten on 02/05/2009.
 *  Copyright 2009 MSA Visuals Ltd.. All rights reserved.
 *
 */

#include "ParticleSystem.h"

ParticleSystem::ParticleSystem() {
	curIndex = 0;
}

void ParticleSystem::updateAndDraw(const msa::fluid::Solver &solver, ofVec2f windowSize, bool drawingFluid) {
    ofVec2f invWindowSize(1.0f / windowSize.x, 1.0f / windowSize.y);

	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_ONE,GL_ONE);
//	glEnable(GL_LINE_SMOOTH);
    ofSetLineWidth(1);
	
	for(int i=0; i<MAX_PARTICLES; i++) {
		if(particles[i].alpha > 0) {
			particles[i].update(solver, windowSize, invWindowSize);
			particles[i].updateVertexArrays(drawingFluid, invWindowSize, i, posArray, colArray);
		}
	}    
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, posArray);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, 0, colArray);
	
	glDrawArrays(GL_LINES, 0, MAX_PARTICLES * 2);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	glDisable(GL_BLEND);
}


void ParticleSystem::addParticles(const ofVec2f &pos, int count){
	for(int i=0; i<count; i++)
		addParticle(pos + msa::Rand::randVec2f() * 15);
}


void ParticleSystem::addParticle(const ofVec2f &pos) {
	particles[curIndex].init(pos.x, pos.y);
	curIndex++;
	if(curIndex >= MAX_PARTICLES) curIndex = 0;
}
