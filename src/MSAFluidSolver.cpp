/***********************************************************************
 
 This class is for the actual solver (doesn't draw anything)
 
 ************************************************************************/

#include "MSAFluidSolver.h"

namespace msa {
    namespace fluid {
        
        Solver::Solver()
        :density(NULL)
        ,densityOld(NULL)
        ,uv(NULL)
        ,uvOld(NULL)
        ,color(NULL)
        ,colorOld(NULL)
        ,curl(NULL)
        ,_isInited(false)
        {
        }
        
        //--------------------------------------------------------------
        Solver& Solver::setSize(int NX, int NY)
        {
            _NX = NX;
            _NY = NY;
            _numCells = (_NX + 2) * (_NY + 2);
            
            _invNX = 1.0f / _NX;
            _invNY = 1.0f / _NY;
            _invNumCells = 1.0f / _numCells;
            
            width           = getWidth();
            height          = getHeight();
            invWidth        = 1.0f/width;
            invHeight       = 1.0f/height;
            
            reset();
            return *this;
        }
        
        
        //--------------------------------------------------------------
        Solver& Solver::setup(int NX, int NY)
        {
            setDeltaT();
            setFadeSpeed();
            setSolverIterations();
            enableVorticityConfinement(false);
            setWrap(false, false);
            
            //maa
            viscocity =  FLUID_DEFAULT_VISC;
            colorDiffusion = FLUID_DEFAULT_COLOR_DIFFUSION;
            
            return setSize(NX, NY);
        }
        
        //--------------------------------------------------------------
        Solver&  Solver::setDeltaT(float deltaT) {
            this->deltaT = deltaT;
            return *this;
        }
        
        //--------------------------------------------------------------
        Solver&  Solver::setFadeSpeed(float fadeSpeed) {
            this->fadeSpeed = fadeSpeed;
            return *this;
        }
        
        //--------------------------------------------------------------
        Solver&  Solver::setSolverIterations(int solverIterations) {
            this->solverIterations = solverIterations;
            return *this;
        }
        
        
        //--------------------------------------------------------------
        // whether fluid is RGB or monochrome (if only pressure / velocity is needed no need to update 3 channels)
        Solver&  Solver::enableRGB(bool doRGB) {
            this->doRGB = doRGB;
            return *this;
        }
        
        //--------------------------------------------------------------
        Solver&  Solver::enableVorticityConfinement(bool b) {
            doVorticityConfinement = b;
            return *this;
        }
        
        //--------------------------------------------------------------
        bool Solver::getVorticityConfinement() {
            return doVorticityConfinement;
        }
        
        //--------------------------------------------------------------
        Solver& Solver::setWrap(bool bx, bool by) {
            wrap_x = bx;
            wrap_y = by;
            return *this;
        }
        
        //--------------------------------------------------------------
        bool Solver::isInited() const {
            return _isInited;
        }
        
        //--------------------------------------------------------------
        Solver::~Solver() {
            destroy();
        }
        
        //--------------------------------------------------------------
        void Solver::destroy() {
            _isInited = false;
            
            if(density)		delete []density;
            if(densityOld)	delete []densityOld;
            if(color)		delete []color;
            if(colorOld)	delete []colorOld;
            if(uv)		delete []uv;
            if(uvOld)	delete []uvOld;
            if(curl)       delete []curl;
        }
        
        
        //--------------------------------------------------------------
        void Solver::reset() {
            destroy();
            _isInited = true;
            
            density		= new float[_numCells];
            densityOld	= new float[_numCells];
            color		= new Vec3f[_numCells];
            colorOld	= new Vec3f[_numCells];
            uv    = new Vec2f[_numCells];
            uvOld = new Vec2f[_numCells];
            curl = new float[_numCells];
            
            for (int i = _numCells-1; i>=0; --i) {
                density[i] = 0;
                densityOld[i] = 0;
                color[i] = Vec3f::zero();
                colorOld[i] = Vec3f::zero();
                uv[i] = Vec2f::zero();
                uvOld[i] = Vec2f::zero();
                curl[i] = 0.0f;
            }
        }
        
        //--------------------------------------------------------------
        // return total number of cells (_NX+2) * (_NY+2)
        int Solver::getNumCells() const {
            return _numCells;
        }
        
        //--------------------------------------------------------------
        int Solver::getWidth() const {
            return _NX + 2;
        }
        
        
        //--------------------------------------------------------------
        int Solver::getHeight() const {
            return _NY + 2;
        }
        
        //--------------------------------------------------------------
        float Solver::getInvWidth() const {
            return invWidth;
        }
        
        
        //--------------------------------------------------------------
        float Solver::getInvHeight() const {
            return invHeight;
        }
        
        //--------------------------------------------------------------
        Vec2f Solver::getSize() {
            return Vec2f(getWidth(), getHeight());
        }
        
        //--------------------------------------------------------------
        Vec2f Solver::getInvSize() {
            return Vec2f(invWidth, invHeight);
        }
        
        
        
        //--------------------------------------------------------------
        Solver& Solver::setVisc(float newVisc) {
            viscocity = newVisc;
            return *this;
        }
        
        //--------------------------------------------------------------
        // returns current viscocity
        float Solver::getVisc() const {
            return viscocity;
        }
        
        //--------------------------------------------------------------
        Solver& Solver::setColorDiffusion(float diff)
        {
            colorDiffusion = diff;
            return *this;
        }
        
        //--------------------------------------------------------------
        float	Solver::getColorDiffusion()
        {
            return colorDiffusion;
        }
        
        //--------------------------------------------------------------
        // returns average density of fluid
        float Solver::getAvgDensity() const {
            return _avgDensity;
        }
        
        //--------------------------------------------------------------
        // returns average uniformity
        float Solver::getUniformity() const {
            return _uniformity;
        }
        
        //--------------------------------------------------------------
        float Solver::getAvgSpeed() const {
            return _avgSpeed;
        }
        
        //--------------------------------------------------------------
        // Curl and vorticityConfinement based on code by Alexander McKenzie
        float Solver::calcCurl(int i, int j)
        {
            float du_dy = uv[FLUID_IX(i, j + 1)].x - uv[FLUID_IX(i, j - 1)].x;
            float dv_dx = uv[FLUID_IX(i + 1, j)].y - uv[FLUID_IX(i - 1, j)].y;
            return (du_dy - dv_dx) * 0.5f;	// for optimization should be moved to later and done with another operation
        }
        
        //--------------------------------------------------------------
        void Solver::vorticityConfinement(Vec2f* Fvc_xy) {
            float dw_dx, dw_dy;
            float length;
            float v;
            
            // Calculate magnitude of calcCurl(u,v) for each cell. (|w|)
            for (int j = _NY; j > 0; --j)
            {
                for (int i = _NX; i > 0; --i)
                {
                    curl[FLUID_IX(i, j)] = fabs(calcCurl(i, j));
                }
            }
            
            for (int j = _NY-1; j > 1; --j)	//for (int j = 2; j < _NY; j++)
            {
                for (int i = _NX-1; i > 1; --i)		//for (int i = 2; i < _NX; i++)
                {
                    // Find derivative of the magnitude (_N = del |w|)
                    dw_dx = (curl[FLUID_IX(i + 1, j)] - curl[FLUID_IX(i - 1, j)]);	// was * 0.5f; now done later with 2./lenght
                    dw_dy = (curl[FLUID_IX(i, j + 1)] - curl[FLUID_IX(i, j - 1)]);	// was * 0.5f;
                    
                    // Calculate vector length. (|_N|)
                    // Add small factor to prevent divide by zeros.
                    length = (float) sqrt(dw_dx * dw_dx + dw_dy * dw_dy) + 0.000001f;
                    
                    // N = (_N/|_N|)
                    length = 2./length;	// the 2. come from the previous * 0.5
                    dw_dx *= length;
                    dw_dy *= length;
                    
                    v = calcCurl(i, j);
                    
                    // N x w
                    Fvc_xy[FLUID_IX(i, j)].x = dw_dy * -v;
                    Fvc_xy[FLUID_IX(i, j)].y = dw_dx *  v;
                }
            }
        }
        
        //--------------------------------------------------------------
        void Solver::update() {
            addSource(uv, uvOld);
            
            if(doVorticityConfinement)
            {
                vorticityConfinement(uvOld);
                addSource(uv, uvOld);
            }
            
            SWAP(uv, uvOld);
            
            diffuseUV(viscocity);
            
            project(uv, uvOld);
            
            SWAP(uv, uvOld);
            
            advect2d(uv, uvOld);
            
            project(uv, uvOld);
            
            if(doRGB)
            {
                addSource(color, colorOld);
                SWAP(color, colorOld);
                
                if(colorDiffusion!=0. && deltaT!=0.)
                {
                    diffuseRGB(0, colorDiffusion);
                    SWAP(color, colorOld);
                }
                
                advectRGB(0, uv);
                fadeRGB();
            }
            else
            {
                addSource(density, densityOld);
                SWAP(density, densityOld);
                
                if(colorDiffusion!=0. && deltaT!=0.) {
                    diffuse(0, density, densityOld, colorDiffusion);
                    SWAP(density, densityOld);
                }
                
                advect(0, density, densityOld, uv);
                fadeDensity();
            }
        }
        
#define ZERO_THRESH		1e-9			// if value falls under this, set to zero (to avoid denormal slowdown)
#define CHECK_ZERO(p)	if(fabsf(p)<ZERO_THRESH) p = 0
        
        //--------------------------------------------------------------
        void Solver::fadeDensity() {
            // I want the fluid to gradually fade out so the screen doesn't fill. the amount it fades out depends on how full it is, and how uniform (i.e. boring) the fluid is...
            //		float holdAmount = 1 - _avgDensity * _avgDensity * fadeSpeed;	// this is how fast the density will decay depending on how full the screen currently is
            float holdAmount = 1 - fadeSpeed;
            
            _avgDensity = 0;
            _avgSpeed = 0;
            
            float totalDeviations = 0;
            float currentDeviation;
            float tmp;
            _avgSpeed = 0;
            
            for (int i = _numCells-1; i >=0; --i) {
                
                // clear old values
                uvOld[i] = Vec2f::zero();
                densityOld[i] = 0;
                
                // calc avg speed
                _avgSpeed += uv[i].x * uv[i].x + uv[i].y * uv[i].y;
                
                // calc avg density
                tmp = min(1.0f, density[i]);
                _avgDensity += tmp;	// add it up
                
                // calc deviation (for uniformity)
                currentDeviation = tmp - _avgDensity;
                totalDeviations += currentDeviation * currentDeviation;
                
                // fade out old
                density[i] = tmp * holdAmount;
                
                CHECK_ZERO(density[i]);
                CHECK_ZERO(uv[i].x);
                CHECK_ZERO(uv[i].y);
                if(doVorticityConfinement) CHECK_ZERO(curl[i]);
                
            }
            _avgDensity *= _invNumCells;
            _avgSpeed *= _invNumCells;
            
            //	println("%.3f\n", _avgSpeed);
            _uniformity = 1.0f / (1 + totalDeviations * _invNumCells);		// 0: very wide distribution, 1: very uniform
        }
        
        
        //--------------------------------------------------------------
        void Solver::fadeRGB() {
            // I want the fluid to gradually fade out so the screen doesn't fill. the amount it fades out depends on how full it is, and how uniform (i.e. boring) the fluid is...
            //		float holdAmount = 1 - _avgDensity * _avgDensity * fadeSpeed;	// this is how fast the density will decay depending on how full the screen currently is
            float holdAmount = 1 - fadeSpeed;
            
            _avgDensity = 0;
            _avgSpeed = 0;
            
            float totalDeviations = 0;
            float currentDeviation;
            Vec3f tmp;
            _avgSpeed = 0;
            
            for (int i = _numCells-1; i >=0; --i)
            {
                // clear old values
                uvOld[i] = Vec2f::zero();
                colorOld[i] = Vec3f::zero();
                
                // calc avg speed
                _avgSpeed += uv[i].x * uv[i].x + uv[i].y * uv[i].y;
                
                // calc avg density
                tmp.x = min(1.0f, color[i].x);
                tmp.y = min(1.0f, color[i].y);
                tmp.z = min(1.0f, color[i].z);
                //			tmp.a = min(1.0f, color[i].a);
                
                //			float density = max(tmp.a, max(tmp.r, max(tmp.g, tmp.b)));
                //			float density = max(tmp.r, max(tmp.g, tmp.b));
                float density = max(tmp.x, max(tmp.y, tmp.z));
                _avgDensity += density;	// add it up
                
                // calc deviation (for _uniformity)
                currentDeviation = density - _avgDensity;
                totalDeviations += currentDeviation * currentDeviation;
                
                // fade out old
                color[i] = tmp * holdAmount;
                
                CHECK_ZERO(color[i].x);
                CHECK_ZERO(color[i].y);
                CHECK_ZERO(color[i].z);
                //			CHECK_ZERO(color[i].a);
                CHECK_ZERO(uv[i].x);
                CHECK_ZERO(uv[i].y);
                if(doVorticityConfinement) CHECK_ZERO(curl[i]);
            }
            _avgDensity *= _invNumCells;
            _avgSpeed *= _invNumCells;
            
            //println("%.3f\n", _avgDensity);
            _uniformity = 1.0f / (1 + totalDeviations * _invNumCells);		// 0: very wide distribution, 1: very uniform
        }
        
        
        //	void Solver::addSourceUV()
        //	{
        //		for (int i = _numCells-1; i >=0; --i) {
        //			uv[i] += deltaT * uvOld[i];
        //		}
        //	}
        //
        //	void Solver::addSourceRGB()
        //	{
        //		for (int i = _numCells-1; i >=0; --i) {
        //			color[i] += deltaT * colorOld[i];
        //		}
        //	}
        //
        //	void Solver::addSource(float* x, float* x0) {
        //		for (int i = _numCells-1; i >=0; --i) {
        //			x[i] += deltaT * x0[i];
        //		}
        //	}
        
        //--------------------------------------------------------------
        void Solver::advect(int bound, float* d, const float* d0, const Vec2f* duv) {
            int i0, j0, i1, j1;
            float x, y, s0, t0, s1, t1;
            int	index;
            
            const float dt0x = deltaT * _NX;
            const float dt0y = deltaT * _NY;
            
            for (int j = _NY; j > 0; --j)
            {
                for (int i = _NX; i > 0; --i)
                {
                    index = FLUID_IX(i, j);
                    x = i - dt0x * duv[index].x;
                    y = j - dt0y * duv[index].y;
                    
                    if (x > _NX + 0.5) x = _NX + 0.5f;
                    if (x < 0.5)     x = 0.5f;
                    
                    i0 = (int) x;
                    i1 = i0 + 1;
                    
                    if (y > _NY + 0.5) y = _NY + 0.5f;
                    if (y < 0.5)     y = 0.5f;
                    
                    j0 = (int) y;
                    j1 = j0 + 1;
                    
                    s1 = x - i0;
                    s0 = 1 - s1;
                    t1 = y - j0;
                    t0 = 1 - t1;
                    
                    d[index] = s0 * (t0 * d0[FLUID_IX(i0, j0)] + t1 * d0[FLUID_IX(i0, j1)])
                    + s1 * (t0 * d0[FLUID_IX(i1, j0)] + t1 * d0[FLUID_IX(i1, j1)]);
                    
                }
            }
            setBoundary(bound, d);
        }
        
        //--------------------------------------------------------------
        //          d    d0    du    dv
        // advect(1, u, uOld, uOld, vOld);
        // advect(2, v, vOld, uOld, vOld);
        void Solver::advect2d(Vec2f *uv, const Vec2f *duv) {
            int i0, j0, i1, j1;
            float s0, t0, s1, t1;
            int	index;
            
            const float dt0x = deltaT * _NX;
            const float dt0y = deltaT * _NY;
            
            for (int j = _NY; j > 0; --j)
            {
                for (int i = _NX; i > 0; --i)
                {
                    index = FLUID_IX(i, j);
                    float x = i - dt0x * duv[index].x;
                    float y = j - dt0y * duv[index].y;
                    
                    if (x > _NX + 0.5) x = _NX + 0.5f;
                    if (x < 0.5)     x = 0.5f;
                    
                    i0 = (int) x;
                    i1 = i0 + 1;
                    
                    if (y > _NY + 0.5) y = _NY + 0.5f;
                    if (y < 0.5)     y = 0.5f;
                    
                    j0 = (int) y;
                    j1 = j0 + 1;
                    
                    s1 = x - i0;
                    s0 = 1 - s1;
                    t1 = y - j0;
                    t0 = 1 - t1;
                    
                    uv[index].x = s0 * (t0 * duv[FLUID_IX(i0, j0)].x + t1 * duv[FLUID_IX(i0, j1)].x)
                    + s1 * (t0 * duv[FLUID_IX(i1, j0)].x + t1 * duv[FLUID_IX(i1, j1)].x);
                    uv[index].y = s0 * (t0 * duv[FLUID_IX(i0, j0)].y + t1 * duv[FLUID_IX(i0, j1)].y)
                    + s1 * (t0 * duv[FLUID_IX(i1, j0)].y + t1 * duv[FLUID_IX(i1, j1)].y);
                    
                }
            }
            setBoundary2d(1, uv);
            setBoundary2d(2, uv);
        }
        
        //--------------------------------------------------------------
        void Solver::advectRGB(int bound, const Vec2f* duv) {
            int i0, j0;
            float x, y, s0, t0, s1, t1, dt0x, dt0y;
            int	index;
            
            dt0x = deltaT * _NX;
            dt0y = deltaT * _NY;
            
            for (int j = _NY; j > 0; --j)
            {
                for (int i = _NX; i > 0; --i)
                {
                    index = FLUID_IX(i, j);
                    x = i - dt0x * duv[index].x;
                    y = j - dt0y * duv[index].y;
                    
                    if (x > _NX + 0.5) x = _NX + 0.5f;
                    if (x < 0.5)     x = 0.5f;
                    
                    i0 = (int) x;
                    
                    if (y > _NY + 0.5) y = _NY + 0.5f;
                    if (y < 0.5)     y = 0.5f;
                    
                    j0 = (int) y;
                    
                    s1 = x - i0;
                    s0 = 1 - s1;
                    t1 = y - j0;
                    t0 = 1 - t1;
                    
                    i0 = FLUID_IX(i0, j0);	//we don't need col/row index any more but index in 1 dimension
                    j0 = i0 + (_NX + 2);
                    color[index] = (colorOld[i0] * t0 + colorOld[j0] * t1) * s0 + (colorOld[i0+1] * t0 + colorOld[j0+1] * t1) * s1;
                }
            }
            setBoundaryRGB();
        }
        
        //--------------------------------------------------------------
        void Solver::diffuse(int bound, float* c, float* c0, float diff)
        {
            float a = deltaT * diff * _NX * _NY;	//todo find the exact strategy for using _NX and _NY in the factors
            linearSolver(bound, c, c0, a, 1.0 + 4 * a);
        }
        
        //--------------------------------------------------------------
        void Solver::diffuseRGB(int bound, float diff)
        {
            float a = deltaT * diff * _NX * _NY;
            linearSolverRGB(a, 1.0 + 4 * a);
        }
        
        //--------------------------------------------------------------
        void Solver::diffuseUV(float diff)
        {
            float a = deltaT * diff * _NX * _NY;
            linearSolverUV(a, 1.0 + 4 * a);
        }
        
        //--------------------------------------------------------------
        void Solver::project(Vec2f* xy, Vec2f* pDiv)
        {
            float	h;
            int		index;
            int		step_x = _NX + 2;
            
            h = - 0.5f / _NX;
            for (int j = _NY; j > 0; --j)
            {
                index = FLUID_IX(_NX, j);
                for (int i = _NX; i > 0; --i)
                {
                    pDiv[index].x = h * (xy[index+1].x - xy[index-1].x + xy[index+step_x].y - xy[index-step_x].y);
                    pDiv[index].y = 0;
                    --index;
                }
            }
            
            setBoundary02d(reinterpret_cast<Vec2f*>(&pDiv[0].x));
            setBoundary02d(reinterpret_cast<Vec2f*>(&pDiv[0].y));
            
            linearSolverProject(pDiv);
            
            float fx = 0.5f * _NX;
            float fy = 0.5f * _NY;	//maa	change it from _NX to _NY
            for (int j = _NY; j > 0; --j)
            {
                index = FLUID_IX(_NX, j);
                for (int i = _NX; i > 0; --i)
                {
                    xy[index].x -= fx * (pDiv[index+1].x - pDiv[index-1].x);
                    xy[index].y -= fy * (pDiv[index+step_x].x - pDiv[index-step_x].x);
                    --index;
                }
            }
            
            setBoundary2d(1, xy);
            setBoundary2d(2, xy);
        }
        
        
        //--------------------------------------------------------------
        //	Gauss-Seidel relaxation
        void Solver::linearSolver(int bound, float* __restrict x, const float* __restrict x0, float a, float c)
        {
            int	step_x = _NX + 2;
            int index;
            c = 1. / c;
            for (int k = solverIterations; k > 0; --k)	// MEMO
            {
                for (int j = _NY; j > 0 ; --j)
                {
                    index = FLUID_IX(_NX, j);
                    for (int i = _NX; i > 0 ; --i)
                    {
                        x[index] = ((x[index-1] + x[index+1] + x[index - step_x] + x[index + step_x]) * a + x0[index]) * c;
                        --index;
                    }
                }
                setBoundary(bound, x);
            }
        }
        
        //--------------------------------------------------------------
        void Solver::linearSolverProject(Vec2f* __restrict pdiv)
        {
            int	step_x = _NX + 2;
            int index;
            for (int k = solverIterations; k > 0; --k) {
                for (int j = _NY; j > 0 ; --j) {
                    index = FLUID_IX(_NX, j);
                    float prev = pdiv[index+1].x;
                    for (int i = _NX; i > 0 ; --i)
                    {
                        prev = (pdiv[index-1].x + prev + pdiv[index - step_x].x + pdiv[index + step_x].x + pdiv[index].y) * .25;
                        pdiv[index].x = prev;
                        --index;
                    }
                }
                setBoundary02d(reinterpret_cast<Vec2f*>(&pdiv[0].x));
            }
        }
        
        //--------------------------------------------------------------
        void Solver::linearSolverRGB(float a, float c)
        {
            int index3, index4, index;
            int	step_x = _NX + 2;
            c = 1. / c;
            for (int k = solverIterations; k > 0; --k)	// MEMO
            {           
                for (int j = _NY; j > 0 ; --j)
                {
                    index = FLUID_IX(_NX, j);
                    //index1 = index - 1;		//FLUID_IX(i-1, j);
                    //index2 = index + 1;		//FLUID_IX(i+1, j);
                    index3 = index - step_x;	//FLUID_IX(i, j-1);
                    index4 = index + step_x;	//FLUID_IX(i, j+1);
                    for (int i = _NX; i > 0 ; --i)
                    {	
                        color[index] = ((color[index-1] + color[index+1]  +  color[index3] + color[index4]) * a  +  colorOld[index]) * c;                                
                        --index;
                        --index3;
                        --index4;
                    }
                }
                setBoundaryRGB();	
            }
        }
        
        //--------------------------------------------------------------
        void Solver::linearSolverUV(float a, float c)
        {
            int index;
            int	step_x = _NX + 2;
            c = 1. / c;
            Vec2f* __restrict localUV = uv;
            const Vec2f* __restrict localOldUV = uvOld;
            
            for (int k = solverIterations; k > 0; --k)	// MEMO
            {           
                for (int j = _NY; j > 0 ; --j)
                {
                    index = FLUID_IX(_NX, j);
                    float prevU = localUV[index+1].x;
                    float prevV = localUV[index+1].y;
                    for (int i = _NX; i > 0 ; --i)
                    {
                        prevU = ((localUV[index-1].x + prevU + localUV[index - step_x].x + localUV[index + step_x].x) * a  + localOldUV[index].x) * c;
                        prevV = ((localUV[index-1].y + prevV + localUV[index - step_x].y + localUV[index + step_x].y) * a  + localOldUV[index].y) * c;
                        localUV[index].x = prevU;
                        localUV[index].y = prevV;
                        --index;
                    }
                }
                setBoundary2d(1, uv);
            }
        }
        
        //--------------------------------------------------------------
        void Solver::setBoundary(int bound, float* x)
        {
            int dst1, dst2, src1, src2;
            int step = FLUID_IX(0, 1) - FLUID_IX(0, 0);
            
            dst1 = FLUID_IX(0, 1);
            src1 = FLUID_IX(1, 1);
            dst2 = FLUID_IX(_NX+1, 1);
            src2 = FLUID_IX(_NX, 1);
            if(wrap_x)
                SWAP(src1, src2);
            if(bound == 1 && !wrap_x)
                for (int i = _NY; i > 0; --i)
                {
                    x[dst1] = -x[src1];	dst1 += step;	src1 += step;	
                    x[dst2] = -x[src2];	dst2 += step;	src2 += step;	
                }
            else
                for (int i = _NY; i > 0; --i)
                {
                    x[dst1] = x[src1];	dst1 += step;	src1 += step;	
                    x[dst2] = x[src2];	dst2 += step;	src2 += step;	
                }
            
            dst1 = FLUID_IX(1, 0);
            src1 = FLUID_IX(1, 1);
            dst2 = FLUID_IX(1, _NY+1);
            src2 = FLUID_IX(1, _NY);
            if(wrap_y)
                SWAP(src1, src2);
            if(bound == 2 && !wrap_y)
                for (int i = _NX; i > 0; --i)
                {
                    x[dst1++] = -x[src1++];	
                    x[dst2++] = -x[src2++];	
                }
            else
                for (int i = _NX; i > 0; --i)
                {
                    x[dst1++] = x[src1++];
                    x[dst2++] = x[src2++];	
                }
            
            x[FLUID_IX(0,   0)] = 0.5f * (x[FLUID_IX(1, 0)] + x[FLUID_IX(0, 1)]);
            x[FLUID_IX(0, _NY+1)] = 0.5f * (x[FLUID_IX(1, _NY+1)] + x[FLUID_IX(0, _NY)]);
            x[FLUID_IX(_NX+1,   0)] = 0.5f * (x[FLUID_IX(_NX, 0)] + x[FLUID_IX(_NX+1, 1)]);
            x[FLUID_IX(_NX+1, _NY+1)] = 0.5f * (x[FLUID_IX(_NX, _NY+1)] + x[FLUID_IX(_NX+1, _NY)]);
        }
        
        //--------------------------------------------------------------
        void Solver::setBoundary02d(Vec2f* x)
        {
            int dst1, dst2, src1, src2;
            int step = FLUID_IX(0, 1) - FLUID_IX(0, 0);
            
            dst1 = FLUID_IX(0, 1);
            src1 = FLUID_IX(1, 1);
            dst2 = FLUID_IX(_NX+1, 1);
            src2 = FLUID_IX(_NX, 1);
            if(wrap_x)
                SWAP(src1, src2);
            for (int i = _NY; i > 0; --i)
            {
                x[dst1].x = x[src1].x;	dst1 += step;	src1 += step;	
                x[dst2].x = x[src2].x;	dst2 += step;	src2 += step;	
            }
            
            dst1 = FLUID_IX(1, 0);
            src1 = FLUID_IX(1, 1);
            dst2 = FLUID_IX(1, _NY+1);
            src2 = FLUID_IX(1, _NY);
            if(wrap_y)
                SWAP(src1, src2);
            for (int i = _NX; i > 0; --i)
            {
                x[dst1++] = x[src1++];
                x[dst2++] = x[src2++];	
            }
            
            x[FLUID_IX(0,   0)].x = 0.5f * (x[FLUID_IX(1, 0)].x + x[FLUID_IX(0, 1)].x);
            x[FLUID_IX(0, _NY+1)].x = 0.5f * (x[FLUID_IX(1, _NY+1)].x + x[FLUID_IX(0, _NY)].x);
            x[FLUID_IX(_NX+1,   0)].x = 0.5f * (x[FLUID_IX(_NX, 0)].x + x[FLUID_IX(_NX+1, 1)].x);
            x[FLUID_IX(_NX+1, _NY+1)].x = 0.5f * (x[FLUID_IX(_NX, _NY+1)].x + x[FLUID_IX(_NX+1, _NY)].x);
        }
        
        //--------------------------------------------------------------
        void Solver::setBoundary2d(int bound, Vec2f *xy)
        {
            int dst1, dst2, src1, src2;
            int step = FLUID_IX(0, 1) - FLUID_IX(0, 0);
            
            dst1 = FLUID_IX(0, 1);
            src1 = FLUID_IX(1, 1);
            dst2 = FLUID_IX(_NX+1, 1);
            src2 = FLUID_IX(_NX, 1);
            if(wrap_x)
                SWAP(src1, src2);
            if(bound == 1 && !wrap_x)
                for (int i = _NY; i > 0; --i)
                {
                    xy[dst1].x = -xy[src1].x;	dst1 += step;	src1 += step;	
                    xy[dst2].x = -xy[src2].x;	dst2 += step;	src2 += step;	
                }
            else
                for (int i = _NY; i > 0; --i)
                {
                    xy[dst1].x = xy[src1].x;	dst1 += step;	src1 += step;	
                    xy[dst2].x = xy[src2].x;	dst2 += step;	src2 += step;	
                }
            
            dst1 = FLUID_IX(1, 0);
            src1 = FLUID_IX(1, 1);
            dst2 = FLUID_IX(1, _NY+1);
            src2 = FLUID_IX(1, _NY);
            if(wrap_y)
                SWAP(src1, src2);
            if(bound == 2 && !wrap_y)
                for (int i = _NX; i > 0; --i)
                {
                    xy[dst1++].y = -xy[src1++].y;	
                    xy[dst2++].y = -xy[src2++].y;	
                }
            else
                for (int i = _NX; i > 0; --i)
                {
                    xy[dst1++].y = xy[src1++].y;
                    xy[dst2++].y = xy[src2++].y;	
                }
            
            xy[FLUID_IX(0,   0)][bound-1] = 0.5f * (xy[FLUID_IX(1, 0)][bound-1] + xy[FLUID_IX(0, 1)][bound-1]);
            xy[FLUID_IX(0, _NY+1)][bound-1] = 0.5f * (xy[FLUID_IX(1, _NY+1)][bound-1] + xy[FLUID_IX(0, _NY)][bound-1]);
            xy[FLUID_IX(_NX+1,   0)][bound-1] = 0.5f * (xy[FLUID_IX(_NX, 0)][bound-1] + xy[FLUID_IX(_NX+1, 1)][bound-1]);
            xy[FLUID_IX(_NX+1, _NY+1)][bound-1] = 0.5f * (xy[FLUID_IX(_NX, _NY+1)][bound-1] + xy[FLUID_IX(_NX+1, _NY)][bound-1]);
        }
        
        //#define CPY_RGB(d, s)		{	r[d] = r[s];	g[d] = g[s];	b[d] = b[s]; }
        //#define CPY_RGB_NEG(d, s)	{	r[d] = -r[s];	g[d] = -g[s];	b[d] = -b[s]; }
        
        //--------------------------------------------------------------
        void Solver::setBoundaryRGB()
        {
            int dst1, dst2, src1, src2;
            int step = FLUID_IX(0, 1) - FLUID_IX(0, 0);
            
            dst1 = FLUID_IX(0, 1);
            src1 = FLUID_IX(1, 1);
            dst2 = FLUID_IX(_NX+1, 1);
            src2 = FLUID_IX(_NX, 1);
            if(wrap_x)
                SWAP(src1, src2);
            for (int i = _NY; i > 0; --i)
            {
                color[dst1] = color[src1];
                dst1 += step;
                src1 += step;	
                
                color[dst2] = color[src2];
                dst2 += step;
                src2 += step;	
            }
            
            dst1 = FLUID_IX(1, 0);
            src1 = FLUID_IX(1, 1);
            dst2 = FLUID_IX(1, _NY+1);
            src2 = FLUID_IX(1, _NY);
            if(wrap_y)
                SWAP(src1, src2);
            for (int i = _NX; i > 0; --i)
            {
                color[dst1] = color[src1];
                ++dst1;
                ++src1;	
                
                color[dst2] = color[src2];
                ++dst2;
                ++src2;	
            }
		}
		
        
        //--------------------------------------------------------------
        void Solver::randomizeColor() {
            for (int i = getWidth()-1; i > 0; --i)
            {
                for (int j = getHeight()-1; j > 0; --j)
                {
                    int index = FLUID_IX(i, j);
                    color[index] = Vec3f(Rand::randFloat(), Rand::randFloat(), Rand::randFloat());
                    density[index] = Rand::randFloat();
                }
            }
        }
        
    }
}
