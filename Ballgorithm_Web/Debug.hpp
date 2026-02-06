#pragma once

# include <Siv3D.hpp>

#define VAR_NAME(var) #var

#define PrintDebug(x) Print << Unicode::Widen(#x) << U": " << x;
#define ConsoleDebug(x) Console << Unicode::Widen(#x) << U": " << x;
