//
// PImageLoaderPng.hh for pekwm
// Copyright (C) 2005-2021 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#pragma once

#include "config.h"

#ifdef HAVE_IMAGE_PNG

#include "pekwm.hh"

#include <cstdio>

/**
 * PNG Loader class.
 */
namespace PImageLoaderPng
{
    const char *getExt(void);

    uchar* load(const std::string &file, uint &width, uint &height,
                bool &use_alpha);
    bool save(const std::string &file,
              uchar *data, uint width, uint height);
}

#endif // HAVE_IMAGE_PNG
