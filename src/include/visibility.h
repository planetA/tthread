#pragma once

// Macros to export symbols
#if defined _WIN32 \
  || defined __CYGWIN__
  # ifdef BUILDING_DLL
    #  ifdef __GNUC__
      #   define _PUBLIC_ __attribute__((dllexport))
    #  else // ifdef __GNUC__
      #   define _PUBLIC_ __declspec(dllexport) // Note: actually gcc seems to
                                                // also supports this syntax.
    #  endif // ifdef __GNUC__
  # else    // ifdef BUILDING_DLL
    #  ifdef __GNUC__
      #   define _PUBLIC_ __attribute__((dllimport))
    #  else // ifdef __GNUC__
      #   define _PUBLIC_ __declspec(dllimport) // Note: actually gcc seems to
                                                // also supports this syntax.
    #  endif // ifdef __GNUC__
  # endif // ifdef BUILDING_DLL
  # define _LOCAL_
#else // if defined _WIN32 || defined __CYGWIN__
  # if __GNUC__ >= 4
    #  define _PUBLIC_ __attribute__((visibility("default")))
    #  define _LOCAL_ __attribute__((visibility("hidden")))
  # else // if __GNUC__ >= 4
    #  define _PUBLIC_
    #  define _LOCAL_
  # endif // if __GNUC__ >= 4
#endif // if defined _WIN32 || defined __CYGWIN__
