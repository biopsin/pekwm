#ifndef _PEKWM_OPENBSD34_CONFIG_H_
#define _PEKWM_OPENBSD34_CONFIG_H_

#define FEATURES "XShape image-xpm"

#undef PEKWM_HAVE_LIMITS
#define PEKWM_HAVE_SYS_LIMITS_H
#undef PEKWM_HAVE_STDINT_H
#undef PEKWM_HAVE_LOCALE
#define PEKWM_HAVE_LOCALE_H

// #define PEKWM_HAVE_GCC_DIAGNOSTICS_PUSH

// #define PEKWM_HAVE_PUT_TIME
// #define PEKWM_HAVE_TO_STRING
// #define PEKWM_HAVE_STOI
// #define PEKWM_HAVE_STOF

#define PEKWM_HAVE_SETENV
#define PEKWM_HAVE_UNSETENV
#define PEKWM_HAVE_DAEMON
#define PEKWM_HAVE_TIMERSUB
#define PEKWM_HAVE_CLOCK_GETTIME

#define PEKWM_HAVE_SHAPE
// #define PEKWM_HAVE_XINERAMA
// #define PEKWM_HAVE_XFT
// #define PEKWM_HAVE_XRANDR

// #define PEKWM_HAVE_IMAGE_PNG
// #define PEKWM_HAVE_IMAGE_JPEG
#define PEKWM_HAVE_IMAGE_XPM

// #define PEKWM_HAVE_XUTF8

#endif // _PEKWM_OPENBSD34_CONFIG_H_
