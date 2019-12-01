#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <unistd.h>
//#include <dos.h>
//#include <windows.h>

//#include <vulkan/vulkan.hpp>
#include <misc/args.hxx>
#include <misc/half.hpp>
#include <misc/pcg_random.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <iomanip>
#include <cmath>
#include <cfenv>
#include <ios>
#include <sstream>
#include <chrono>

#ifdef USE_CIMG
#include "tinyexr.h"
#define cimg_plugin "CImg/tinyexr_plugin.hpp"
//#define cimg_use_png
//#define cimg_use_jpeg
#include "CImg.h"
#endif

#ifndef NSM
#define NSM api
#endif

#include <lancer/lancer.hpp>

namespace vkt {

};
