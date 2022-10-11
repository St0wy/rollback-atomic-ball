#pragma once

#include <cstdlib>

#include <fmt/format.h>

#include "utils/log.hpp"

namespace core
{
/**
 * \brief AssertException is an exception type used for the project assertation when the user need the application to close quietly. It is caught by the core::Engine in the game loop to close the update loop.
 */
class AssertException final : public std::exception
{
public:
	// ReSharper disable once CppNonExplicitConvertingConstructor
	AssertException(const std::string_view msg)
		: _msg(msg) {}

	[[nodiscard]] const char* what() const noexcept override { return _msg.c_str(); }
private:
	std::string _msg;
};
}

#ifdef GPR_ASSERT
# ifdef GPR_ABORT
inline void gpr_assert(bool Expr, std::string_view Msg)
{
    if (!(Expr))
    {
        core::LogError(fmt::format("Assert failed:\t{}\nSource:\t\t{}, line {}", Msg, __FILE__, __LINE__));
        std::abort();
    }
}
#  ifdef GPR_ABORT_WARN

inline void gpr_warn(bool Expr, std::string_view Msg)
{
    if (!(Expr))
    {
        core::LogWarning(fmt::format("Warning Assert failed:\t{}\nSource:\t\t{}, line {}",
            Msg, __FILE__, __LINE__));
        std::abort();
    }
}

#  else

inline void gpr_warn(bool Expr, std::string_view Msg)
{
    if (!(Expr))
    {
        core::LogWarning(fmt::format("Warning Assert failed:\t{}\nSource:\t\t{}, line {}",
            Msg, __FILE__, __LINE__));
    }
}
#  endif
# else
inline void gpr_assert(const bool expr, std::string_view msg)
{
	if (!(expr))
	{
		core::LogError(fmt::format("Assert failed:\t{}\nSource:\t\t{}, line {}", msg, __FILE__, __LINE__));
		throw core::AssertException(msg);
	}
}
#  ifdef GPR_ABORT_WARN

inline void gpr_warn(const bool expr, std::string_view msg)
{
	if (!(expr))
	{
		core::LogWarning(fmt::format("Warning Assert failed:\t{}\nSource:\t\t{}, line {}",
			msg,
			__FILE__,
			__LINE__));
		throw core::AssertException(msg);
	}
}

#  else

inline void gpr_warn(bool Expr, std::string_view Msg)
{
    if (!(Expr))
    {
        core::LogWarning(fmt::format("Warning Assert failed:\t{}\nSource:\t\t{}, line {}",
            Msg, __FILE__, __LINE__));
    }
}
#  endif
# endif
#else
inline void gpr_assert(bool Expr, std::string_view Msg)
{
    
}
inline void gpr_warn(bool Expr, std::string_view Msg)
{
}
#endif
