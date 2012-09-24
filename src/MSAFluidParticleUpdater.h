/***********************************************************************
 
 This class interfaces the fluid solver with ofxMSAPhysics (applies fluid velocity as forces to particles)
 
 ***********************************************************************/

// only include this if you have MSAPhysics as well


#pragma once

#include "MSAFluid.h"
#include "MSAPhysicsParticleUpdater.h"

namespace msa {
	
	template <typename T>
	class FluidParticleUpdater : public Physics::ParticleUpdaterT<T> {
	public:
		float strength;
		Solver *fluidSolver;
		
		FluidParticleUpdater() {
			fluidSolver = NULL;
		}
		
		void update(Physics::ParticleT<T> *p) {
			if(fluidSolver) {
				Vec2f fluidVel = fluidSolver->getVelocityAtPos(p->getPosition()* p->getParams()->worldSizeInv);
				float invMass = p->getInvMass();
				p->addVelocity(fluidVel.x * invMass * strength, fluidVel.y * invMass * strength, 0);
			}
		}
		
	};
	
}
