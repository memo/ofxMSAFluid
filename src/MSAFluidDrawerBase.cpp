/***********************************************************************
 
 This class is for drawing a fluidsolver using the openFrameworks texture
 
 ************************************************************************/

#include "MSAFluidDrawerBase.h"


namespace msa {
    namespace fluid {
        
        //--------------------------------------------------------------
        vector<string>& getDrawModeTitles(){
            static vector<string> drawModeTitles;
            if(drawModeTitles.size() == 0) {
                drawModeTitles.push_back("kDrawColor");
                drawModeTitles.push_back("kDrawMotion");
                drawModeTitles.push_back("kDrawSpeed");
                drawModeTitles.push_back("kDrawVectors");
            }
            return drawModeTitles;
        }
        
        
        //--------------------------------------------------------------
        DrawerBase::DrawerBase() {
            _pixels				= NULL;
            _fluidSolver		= NULL;
            _didICreateTheFluid	= false;
            
            enabled				= true;
            useAdditiveBlending = false;
            brightness			= 1;
            doInvert			= false;
            velDrawMult				= 1;
            vectorSkipCount		= 0;
            
            enableAlpha(false);
            
            setDrawMode(kDrawColor);
        }
        
        //--------------------------------------------------------------
        DrawerBase::~DrawerBase() {
            deleteFluidSolver();
        }
        

        //--------------------------------------------------------------
        Solver* DrawerBase::setup(int NX, int NY) {
            deleteFluidSolver();
            _fluidSolver = new Solver;
            _fluidSolver->setup(NX, NY);
            allocatePixels();
            createTexture();
            
            return _fluidSolver;
        }
        

        //--------------------------------------------------------------
        Solver* DrawerBase::setup(Solver* f) {
            deleteFluidSolver();
            _fluidSolver = f;
            allocatePixels();
            createTexture();
            
            return _fluidSolver;
        }
        
        //--------------------------------------------------------------
        Solver* DrawerBase::getFluidSolver() {
            return _fluidSolver;
        }
        
        //--------------------------------------------------------------
        void DrawerBase::enableAlpha(bool b) {
            _alphaEnabled = b;
            if(_alphaEnabled) {
                _glType = GL_RGBA;
                _bpp = 4;
            } else {
                _glType = GL_RGB;
                _bpp = 3;
            }
            
            if(isFluidReady()) {
                allocatePixels();
                createTexture();
            }
        }
        
        
        //--------------------------------------------------------------
        void DrawerBase::allocatePixels() {
            if(_pixels) delete []_pixels;
            int texWidth = _fluidSolver->getWidth()-2;
            int texHeight =_fluidSolver->getHeight()-2;
            _pixels = new unsigned char[texWidth * texHeight * _bpp];
        }
        
        
        //--------------------------------------------------------------
        void DrawerBase::reset() {
            if(!isFluidReady()) {
                printf("DrawerBase::reset() - Fluid not ready\n");
                return;
            }
            _fluidSolver->reset();
        }
        
        //--------------------------------------------------------------
        void DrawerBase::update() {
            if(!isFluidReady()) {
                printf("DrawerBase::updateFluid() - Fluid not ready\n");
                return;
            }
            _fluidSolver->update();
        }
        
        
        //--------------------------------------------------------------
        void DrawerBase::setDrawMode(DrawMode newDrawMode) {
            drawMode = newDrawMode;
            if(drawMode < 0) drawMode = (DrawMode)(kDrawCount-1);
            else if(drawMode >= kDrawCount) drawMode = (DrawMode)0;
        }
        
        //--------------------------------------------------------------
        void DrawerBase::incDrawMode() {
            setDrawMode((DrawMode)((int)drawMode + 1));
        }
        
        //--------------------------------------------------------------
        void DrawerBase::decDrawMode() {
            setDrawMode((DrawMode)(drawMode - 1));
        }
        
        //--------------------------------------------------------------
        DrawMode DrawerBase::getDrawMode() {
            return drawMode;
        }
        
        //--------------------------------------------------------------
        string DrawerBase::getDrawModeName() {
            return getDrawModeTitles().at(drawMode);
        }
        
        
        
        //--------------------------------------------------------------
        void DrawerBase::draw(float x, float y) {
            if(enabled == false) return;
            
            draw(x, y, getWindowWidth(), getWindowHeight());
        }
        
        
        //--------------------------------------------------------------
        void DrawerBase::draw(float x, float y, float renderWidth, float renderHeight) {
            if(enabled == false) return;
            
            switch(drawMode) {
                case kDrawColor:
                    drawColor(x, y, renderWidth, renderHeight);
                    break;
                    
                case kDrawMotion:
                    drawMotion(x, y, renderWidth, renderHeight);
                    break;
                    
                case kDrawSpeed:
                    drawSpeed(x, y, renderWidth, renderHeight);
                    break;
                    
                case kDrawVectors:
                    drawVectors(x, y, renderWidth, renderHeight);
                    break;
                    
            }
        }
        
        
        //--------------------------------------------------------------
        void DrawerBase::drawColor(float x, float y, float renderWidth, float renderHeight, bool withAlpha) {
            if(enabled == false) return;
            
            if(useAdditiveBlending) {
                glBlendFunc(GL_ONE, GL_ONE);
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
            
            int fw = _fluidSolver->getWidth();
            int fh = _fluidSolver->getHeight();
            
            Vec2f vel;
            Color color;
            int index = 0;
            for(int j=1; j < fh-1; j++) {
                for(int i=1; i < fw-1; i++) {
                    _fluidSolver->getInfoAtCell(i, j, &vel, &color);
                    int r = (unsigned char)min(color.r * 255 * brightness, 255.0f);
                    int g = (unsigned char)min(color.g * 255 * brightness, 255.0f);
                    int b = (unsigned char)min(color.b * 255 * brightness, 255.0f);
                    if(doInvert) {
                        r = 255 - r;
                        g = 255 - g;
                        b = 255 - b;
                    }
                    _pixels[index++] = r;
                    _pixels[index++] = g;
                    _pixels[index++] = b;
                    
                    if(_alphaEnabled) _pixels[index++] = withAlpha ? max(b, max(r, g)) : 255;
                }
            }
            
            updateTexture();
            drawTexture(x, y, renderWidth, renderHeight);
        }
        
        
        
        //--------------------------------------------------------------
        void DrawerBase::drawMotion(float x, float y, float renderWidth, float renderHeight, bool withAlpha) {
            if(enabled == false) return;
            
            if(useAdditiveBlending) {
                glBlendFunc(GL_ONE, GL_ONE);
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
            
            int fw = _fluidSolver->getWidth();
            int fh = _fluidSolver->getHeight();
            
            Vec2f vel;
            int index = 0;
            for(int j=1; j < fh-1; j++) {
                for(int i=1; i < fw-1; i++) {
                    _fluidSolver->getInfoAtCell(i, j, &vel, NULL);
                    float speed2 = fabs(vel.x) * fw + fabs(vel.y) * fh;
                    int speed = (int)min(speed2 * 255 * brightness, 255.0f);
                    _pixels[index++] = (unsigned char)min(fabs(vel.x) * fw * 255 * brightness, 255.0f);
                    _pixels[index++] = (unsigned char)min(fabs(vel.y) * fh * 255 * brightness, 255.0f);
                    _pixels[index++] = (unsigned char)0;
                    
                    if(_alphaEnabled) _pixels[index++] = withAlpha ? speed : 255;
                    
                }
            }
            
            updateTexture();
            drawTexture(x, y, renderWidth, renderHeight);
        }
        
        
        //--------------------------------------------------------------
        void DrawerBase::drawSpeed(float x, float y, float renderWidth, float renderHeight, bool withAlpha) {
            if(enabled == false) return;
            
            if(useAdditiveBlending) {
                glBlendFunc(GL_ONE, GL_ONE);
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
            
            int fw = _fluidSolver->getWidth();
            int fh = _fluidSolver->getHeight();
            
            Vec2f vel;
            int index = 0;
            for(int j=1; j < fh-1; j++) {
                for(int i=1; i < fw-1; i++) {
                    _fluidSolver->getInfoAtCell(i, j, &vel, NULL);
                    float speed2 = fabs(vel.x) * fw + fabs(vel.y) * fh;
                    int speed = (int)min(speed2 * 255 * brightness, 255.0f);
                    _pixels[index++] = (unsigned char)speed;
                    _pixels[index++] = (unsigned char)speed;
                    _pixels[index++] = (unsigned char)speed;
                    
                    if(_alphaEnabled) _pixels[index++] = withAlpha ? speed : 255;
                }
            }
            
            updateTexture();
            drawTexture(x, y, renderWidth, renderHeight);
        }
        
        
        //--------------------------------------------------------------
        void DrawerBase::drawVectors(float x, float y, float renderWidth, float renderHeight) {
            if(enabled == false) return;
            
            int fw = _fluidSolver->getWidth();
            int fh = _fluidSolver->getHeight();
            
            if(useAdditiveBlending) {
                glBlendFunc(GL_ONE, GL_ONE);
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
            
            //	int xStep = renderWidth / 10;		// every 10 pixels
            //	int yStep = renderHeight / 10;		// every 10 pixels
            
            glPushMatrix();
            glTranslatef(x, y, 0);
            glScalef(renderWidth/(fw-2), renderHeight/(fh-2), 1.0);
            
            float maxVel = 5.0f/20000;
            
            Vec2f vel;
            float vt = velDrawThreshold * _fluidSolver->getInvWidth() * _fluidSolver->getInvHeight();
            vt *= vt;
            
            for (int j=0; j<fh-2; j+=vectorSkipCount+1 ){
                for (int i=0; i<fw-2; i+=vectorSkipCount+1 ){
                    vel = _fluidSolver->getVelocityAtCell(i+1, j+1);
                    float d2 = vel.lengthSquared();
                    if(d2>vt) {
                        if(d2 > maxVel * maxVel) {
                            float mult = maxVel * maxVel/ d2;
                            //				float mult = (d2 - maxVel * maxVel) / d2;
                            vel.x *= mult;
                            vel.y *= mult;
                        }
                        vel *= velDrawMult * 50000;
                        
#ifndef TARGET_OPENGLES
                        float b = mapRange(d2, vt, maxVel, 0.0f, brightness);
                        b = brightness;
                        glColor3f(b, b, b);
                        
                        glBegin(GL_LINES);
                        glVertex2f(i, j);
                        glVertex2f(i + vel.x, j + vel.y);
                        glEnd();
#endif
                    }
                }
            }
            glPopMatrix();
            
        }
        
        
        
        //--------------------------------------------------------------
        void DrawerBase::deleteFluidSolver() {
            //	printf("DrawerBase::deleteFluidSolver()\n");
            if(_fluidSolver && _didICreateTheFluid) {
                delete _fluidSolver;
                _fluidSolver = NULL;
                
                if(_pixels) delete []_pixels;
                _pixels = NULL;
                
                deleteTexture();
            }
        }
        
        //--------------------------------------------------------------
        bool DrawerBase::isFluidReady() {
            if(!_fluidSolver) {
                printf("DrawerBase::isFluidReady() - No fluid solver\n");
                return false;
            }
            
            if(!_fluidSolver->isInited()) {
                printf("DrawerBase::isFluidReady() - Fluid solver not initialized yet, call init()\n");
                return false;
            }
            
            return true;
        }
    }
}