/***********************************************************************
 
 This class is for drawing a fluidsolver using the OpenFrameworks texture
 
 ************************************************************************/

#pragma once

#include "MSAFluidDrawerBase.h"

namespace msa {
    namespace fluid {
        
        class DrawerGl : public DrawerBase {
        public:
            float getWidth() const override {
                return tex.getWidth();
            }
            
            float getHeight() const override {
                return tex.getHeight();
            }
            
            ofTexture&	getTextureReference() {
                return tex;
            }
            
            
        protected:
            mutable ofTexture			tex;
            
            void createTexture() override {
                int texWidth = _fluidSolver->getWidth()-2;
                int texHeight =_fluidSolver->getHeight()-2;
                tex.allocate(texWidth, texHeight, _glType);
            }
            
            
            void updateTexture() const override {
                tex.loadData(_pixels, (int)tex.getWidth(), (int)tex.getHeight(), _glType);
            }
            
            void deleteTexture() override {
                tex.clear();
            }
            
            void drawTexture(float x, float y, float w, float h) const override {
                tex.draw(x, y, w, h);
            }		
        };
    }
}
