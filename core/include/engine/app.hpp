#pragma once

#include "engine/system.hpp"

#include "graphics/graphics.hpp"

namespace core
{
/**
 * \brief App is an interface for applications that need to init/update/destroy, draw and get event from OS.
 */
class App : public DrawInterface, public DrawImGuiInterface, public SystemInterface, public OnEventInterface
{
};
}
