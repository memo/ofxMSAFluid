#pragma once

#include "MSAFluidSolver.h"

#if defined( MSA_HOST_OPENFRAMEWORKS )
#include "MSAFluidDrawerGl-OF.h"

#elif defined( MSA_HOST_CINDER )
#include "MSAFluidDrawerGl-Cinder.h"

#endif
