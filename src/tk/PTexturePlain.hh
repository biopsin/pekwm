//
// PTexturePlain.hh for pekwm
// Copyright (C) 2004-2023 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#ifndef _PEKWM_PTEXTUREPLAIN_HH_
#define _PEKWM_PTEXTUREPLAIN_HH_

#include "config.h"

#include <map>
#include <string>

#include "PTexture.hh"
#include "X11.hh"

class PImage;

// PTextureEmpty

class PTextureEmpty : public PTexture {
public:
	virtual ~PTextureEmpty() { }

	// START - PTexture interface.
	virtual void doRender(Render &rend,
			      int x, int y, size_t width, size_t height);
	virtual bool getPixel(ulong &pixel) const;
	// END - PTexture interface.
};

class PTextureAreaRender : public PTexture {
public:
	virtual ~PTextureAreaRender() { }

	virtual void renderArea(Render &rend, int x, int y,
				size_t width, size_t height) = 0;
};

// PTextureSolid

class PTextureSolid : public PTexture {
public:
	PTextureSolid(const std::string &color);
	virtual ~PTextureSolid();

	// START - PTexture interface.
	virtual void doRender(Render &rend,
			      int x, int y, size_t width, size_t height);
	virtual bool getPixel(ulong &pixel) const {
		pixel = _xc->pixel;
		return true;
	}
	// END - PTexture interface.

	inline XColor *getColor() { return _xc; }
	bool setColor(const std::string &color);
	void unsetColor();

private:
	XColor *_xc;
};

// PTextureSolidRaised

class PTextureSolidRaised : public PTextureAreaRender {
public:
	PTextureSolidRaised(const std::string &base,
			    const std::string &hi, const std::string &lo);
	virtual ~PTextureSolidRaised();

	// START - PTexture interface.
	virtual void doRender(Render &rend,
			      int x, int y, size_t width, size_t height);
	virtual bool getPixel(ulong&) const { return false; }
	// END - PTexture interface.

	virtual void renderArea(Render &rend, int x, int y,
				size_t width, size_t height);

	inline void setLineOff(size_t loff) { _loff = loff; }
	inline void setDraw(bool top, bool bottom, bool left, bool right) {
		_draw_top = top;
		_draw_bottom = bottom;
		_draw_left = left;
		_draw_right = right;
	}

	void setLineWidth(uint lw);
	bool setColor(const std::string &base,
		      const std::string &hi, const std::string &lo);
	void unsetColor();

private:
	XColor *_xc_base;
	XColor *_xc_hi;
	XColor *_xc_lo;

	uint _lw, _loff;
	bool _draw_top;
	bool _draw_bottom;
	bool _draw_left;
	bool _draw_right;
};

// PTxtureLines

class PTextureLines : public PTextureAreaRender {
public:
	PTextureLines(float line_size, bool size_percent, bool horz,
		      const std::vector<std::string> &colors);
	virtual ~PTextureLines();

	// START - PTexture interface.
	virtual void doRender(Render &rend,
			      int x, int y, size_t width, size_t height);
	virtual bool getPixel(ulong&) const { return false; }
	// END - PTexture interface.

	virtual void renderArea(Render &rend, int x, int y,
				size_t width, size_t height);

private:
	void renderHorz(Render &rend,
			int x, int y, size_t width, size_t height);
	void renderVert(Render &rend,
			int x, int y, size_t width, size_t height);

	void setColors(const std::vector<std::string> &colors);
	void unsetColors();

private:
	/** Line width/height, given in percent (0-100) if _size_percent is
	 * true */
	float _line_size;
	/** If true, size given in percent instead of pixels. */
	bool _size_percent;
	/** If false, vertical mode. */
	bool _horz;
	/** Line colours. */
	std::vector<XColor*> _colors;
};

// PTextureImage

class PTextureImage : public PTexture {
public:
	PTextureImage(PImage *image);
	PTextureImage(const std::string &image, const std::string &colormap);
	virtual ~PTextureImage(void);

	// START - PTexture interface.
	virtual void doRender(Render &rend,
			      int x, int y, size_t width, size_t height);
	virtual bool getPixel(ulong&) const { return false; }
	virtual Pixmap getMask(size_t width, size_t height, bool &do_free);
	// END - PTexture interface.

	bool setImage(const std::string &image, const std::string &colormap);
	void setImage(PImage *image);
	void unsetImage(void);

private:
	PImage *_image;
	std::string _colormap;
};

#endif // _PEKWM_PTEXTUREPLAIN_HH_
