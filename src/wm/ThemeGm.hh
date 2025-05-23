//
// ThemeGm.hh for pekwm
// Copyright (C) 2021-2023 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#ifndef _PEKWM_THEMEGM_HH_
#define _PEKWM_THEMEGM_HH_

#include "tk/PTexturePlain.hh"
#include "tk/Theme.hh"
#include "tk/X11Util.hh"

class ThemeState {
public:
	virtual bool hasBorder(void) const = 0;
	virtual bool hasTitlebar(void) const = 0;
	virtual bool isShaded(void) const = 0;
	virtual FocusedState getFocusedState(bool selected) const = 0;
};

/**
 * Theme state based on MWM hints.
 */
class MwmThemeState : public ThemeState {
public:
	MwmThemeState()
		: ThemeState(),
		  _hints(0, 0, 0)
	{
	}

	void setHints(const MwmHints &hints) { _hints = hints; }

	virtual bool hasBorder(void) const {
		if (_hints.flags & MWM_HINTS_DECORATIONS) {
			return _hints.decorations
				& (MWM_DECOR_ALL | MWM_DECOR_BORDER);
		} else {
			return true;
		}
	}
	virtual bool hasTitlebar(void) const {
		if (_hints.flags & MWM_HINTS_DECORATIONS) {
			return _hints.decorations
				& (MWM_DECOR_ALL | MWM_DECOR_TITLE);
		} else {
			return true;
		}
	}
	virtual bool isShaded(void) const {
		return false;
	}
	virtual FocusedState getFocusedState(bool) const {
		return FOCUSED_STATE_FOCUSED;
	}

private:
	MwmHints _hints;
};

class ThemeGm {
public:
	ThemeGm(Theme::PDecorData *data)
		: _data(data)
	{
	}
	~ThemeGm(void)
	{
	}

	void setData(Theme::PDecorData *data) { _data = data; }

	uint decorWidth(const ThemeState *state) const;
	uint decorHeight(const ThemeState *state) const;

	PFont *getFont(FocusedState state) const;
	uint titleWidth(const ThemeState *state, const std::string& str) const;
	uint titleHeight(const ThemeState *state) const;

	/**
	 * Offset for title position, depends on border size (and shape)
	 */
	uint titleLeftOffset(const ThemeState *state) const {
		// Top left corner has same height as top border, title should
		// be placed right of the left border.
		uint top_left = bdTopLeft(state);
		if (_data->isTitleAlignToSide()
		    || (top_left && bdTopLeftHeight(state) == bdTop(state))) {
			return bdLeft(state);
		}
		return top_left;
	}
	/**
	 * Offset for title position, depends on border size (and shape)
	 */
	uint titleRightOffset(const ThemeState *state) const {
		// Top right corner has same height as top border, title should
		// be placed left of the right border.
		uint top_right = bdTopRight(state);
		if (_data->isTitleAlignToSide()
		    || (top_right && bdTopRightHeight(state) == bdTop(state))) {
			return bdRight(state);
		}
		return top_right;
	}

	uint bdTopOffset(const ThemeState *state) const;
	uint bdTop(const ThemeState *state) const;
	uint bdTopLeft(const ThemeState *state) const;
	uint bdTopLeftHeight(const ThemeState *state) const;
	uint bdTopRight(const ThemeState *state) const;
	uint bdTopRightHeight(const ThemeState *state) const;
	uint bdBottom(const ThemeState *state) const;
	uint bdBottomLeft(const ThemeState *state) const;
	uint bdBottomLeftHeight(const ThemeState *state) const;
	uint bdBottomRight(const ThemeState *state) const;
	uint bdBottomRightHeight(const ThemeState *state) const;
	uint bdLeft(const ThemeState *state) const;
	uint bdRight(const ThemeState *state) const;

private:
	const PTexture* getBorder(const ThemeState *state,
				  BorderPosition pos) const;

private:
	Theme::PDecorData *_data;
	PTextureEmpty _empty;
};

#endif // _PEKWM_THEMEGM_HH_
