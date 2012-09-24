/***********************************************************************
 
 This class is for drawing a fluidsolver using the OpenFrameworks texture
 
 ************************************************************************/

#pragma once

#include "MSAFluidDrawerBase.h"

namespace msa {
    namespace fluid {
        
        class DrawerGl : public DrawerBase {
        public:
            float getWidth() {
                return tex.getWidth();
            }
            
            float getHeight() {
                return tex.getHeight();
            }
            
            ofTexture&	getTextureReference() {
                return tex;
            }
            
            
        protected:
            ofTexture			tex;
            
            void createTexture() {
                int texWidth = _fluidSolver->getWidth()-2;
                int texHeight =_fluidSolver->getHeight()-2;
                tex.allocate(texWidth, texHeight, _glType);
            }
            
            
            void updateTexture() {
                tex.loadData(_pixels, (int)tex.getWidth(), (int)tex.getHeight(), _glType);
            }
            
            void deleteTexture() {
                tex.clear();
            }
            
            void drawTexture(float x, float y, float w, float h) {
                tex.draw(x, y, w, h);
            }		
        };
    }
}
