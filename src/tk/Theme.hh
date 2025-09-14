//
// Theme.hh for pekwm
// Copyright (C) 2003-2025 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#ifndef _PEKWM_THEME_HH_
#define _PEKWM_THEME_HH_

#include "config.h"

#include "pekwm_types.hh"

#include "CfgParser.hh"
#include "Action.hh" // ActionEvent
#include "PFont.hh" // PFont::Color
#include "Util.hh"
#include "PTexture.hh"

class FontHandler;
class ImageHandler;
class TextureHandler;

#include <string>
#include <map>
#include <vector>

//! @brief Theme data parser and container.
class Theme
{
public:
	/**
	 * Color map for mapping colors in images.
	 */
	class ColorMap {
	public:
		static bool load(CfgParser::Entry *section,
				 std::map<int,int> &map);

	protected:
		static int parseColor(const std::string& str);
		static uchar parseHex(const char *p);
	};

	//! @brief Theme data parser and container for PDecor::Button
	class PDecorButtonData {
	public:
		PDecorButtonData(TextureHandler* th);
		~PDecorButtonData(void);

		/**
		 * Returns whether the shape (derived from the alpha channel)
		 * should be set.
		 */
		inline bool setShape(void) const { return _shape; }
		/**
		 * Returns whether the button is positioned relative to the
		 * left title edge.
		 */
		inline bool isLeft(void) const { return _left; }
		//! @brief Returns width of button.
		inline uint getWidth(void) const { return _width; }
		//! @brief Returns height of button.
		inline uint getHeight(void) const { return _height; }

		//! @brief Returns PTexture used in ButtonState state.
		inline PTexture *getTexture(ButtonState state) {
			return _texture[(state != BUTTON_STATE_NO) ? state : 0];
		}
		//! @brief Returns iterator to the first ActionEvent.
		std::vector<ActionEvent>::iterator begin(void) {
			return _aes.begin();
		}
		//! @brief Return iterator to the last+1 ActionEvent.
		std::vector<ActionEvent>::iterator end(void) {
			return _aes.end();
		}

		bool load(CfgParser::Entry *section);
		void unload(void);
		void check(void);

	private:
		TextureHandler* _th;
		bool _loaded;

		std::vector<ActionEvent> _aes;
		PTexture *_texture[BUTTON_STATE_NO];

		bool _shape:1, _left:1;
		uint _width, _height;
	};

	//! @brief PDecor theme data container and parser.
	class PDecorData {
	public:
		PDecorData(FontHandler* fh, TextureHandler* th,
			   int version, const char *name=0);
		~PDecorData(void);

		//! @brief Returns decor name.
		inline const std::string &getName(void) const { return _name; }
		//! @brief Sets decor name.
		inline void setName(const std::string &name) { _name = name; }

		uint getTitleHeight() const { return _title_height; }
		//! @brief Returns title minimum width (0 for full width
		//title).
		inline int getTitleWidthMin(void) const {
			return _title_width_min;
		}
		/** Returns title maximum width in percent. */
		inline int getTitleWidthMax(void) const {
			return _title_width_max;
		}
		/** Returns title text pad for direction. */
		int getPad(PadType pad) const;

		/** Returns whether all items in the title have same width. */
		inline bool isTitleWidthSymetric(void) const {
			return _title_width_symetric;
		}
		/**
		 * Returns whether titlebar height should be relative the font
		 * height
		 */
		bool isTitleHeightAdapt(void) const {
			return _title_height_adapt;
		}
		/** If true, align titlebar to side border width instead of
		 *  corner width. */
		bool isTitleAlignToSide() const { return _title_align_to_side; }

		// Title textures

		//! @brief Returns background PTexture used in FocusedState
		//state.
		inline PTexture *getTextureMain(FocusedState state) {
			return _texture_main
				[(state < FOCUSED_STATE_FOCUSED_SELECTED)
				 ? state : 0];
		}
		//! @brief Returns tab PTexture used in FocusedState state.
		inline PTexture *getTextureTab(FocusedState state) {
			return _texture_tab
				[(state != FOCUSED_STATE_NO) ? state : 0];
		}
		//! @brief Returns separator PTexture used in FocusedState
		//state.
		inline PTexture *getTextureSeparator(FocusedState state) {
			return _texture_separator
				[(state != FOCUSED_STATE_FOCUSED_SELECTED)
				 ? state : 0];
		}
		/**
		 * Return PTexture rendered behind title text.
		 */
		PTexture *getTextureTitle(FocusedState state) {
			return _texture_title
				[(state != FOCUSED_STATE_NO) ? state : 0];
		}

		// font

		//! @brief Returns PFont used in FocusedState state.
		inline PFont *getFont(FocusedState state) const {
			return _font
				[((state == FOCUSED_STATE_NO)
				  || ! _font[state])
				 ? FOCUSED_STATE_FOCUSED : state];
		}
		//! @brief Return PFont::Color used in FocusedState state.
		inline PFont::Color *getFontColor(FocusedState state) {
			return _font_color
				[(state != FOCUSED_STATE_NO) ? state : 0];
		}

		// border

		/**
		 * Return border PTexture used in FocusedState state for pos.
		 */
		inline PTexture *getBorderTexture(FocusedState state,
						  BorderPosition pos) {
			return _texture_border
				[(state < FOCUSED_STATE_FOCUSED_SELECTED)
				 ? state : 0][pos];
		}

		// button

		std::vector<Theme::PDecorButtonData*>::const_iterator
		buttonBegin(void) {
			return _buttons.begin();
		}
		std::vector<Theme::PDecorButtonData*>::const_iterator
		buttonEnd(void) {
			return _buttons.end();
		}

		bool load(CfgParser::Entry *section);
		void unload(void);
		void check(void);

	private:
		void loadTextures(CfgParser::Entry *section,
				  PTexture **textures,
				  FocusedState max_state);
		void loadBorder(CfgParser::Entry *cs);
		void loadButtons(CfgParser::Entry *cs);

		void checkTextures(void);
		void checkFonts(void);
		void checkBorder(void);
		void checkColors(void);

		FontHandler* _fh;
		TextureHandler* _th;
		int _version;

		bool _loaded;
		std::string _name;

		// size, padding etc
		uint _title_height;
		uint _title_width_min;
		uint _title_width_max;
		int _pad[PAD_NO];
		bool _title_width_symetric;
		bool _title_height_adapt;
		bool _title_align_to_side;

		// elements with FOCUSED_STATE_NO
		PTexture *_texture_tab[FOCUSED_STATE_NO];
		PFont *_font[FOCUSED_STATE_NO];
		PFont::Color *_font_color[FOCUSED_STATE_NO];
		PTexture *_texture_title[FOCUSED_STATE_NO];

		// elements with FOCUSED_STATE_FOCUSED_SELECTED
		PTexture *_texture_main[FOCUSED_STATE_FOCUSED_SELECTED];
		PTexture *_texture_separator[FOCUSED_STATE_FOCUSED_SELECTED];
		PTexture *_texture_border
			[FOCUSED_STATE_FOCUSED_SELECTED][BORDER_NO_POS];

		std::vector<Theme::PDecorButtonData*> _buttons;
	};

	//! @brief PMenu theme data container and parser.
	class PMenuData {
	public:
		PMenuData(FontHandler* fh, TextureHandler* th,
			  int version);
		~PMenuData(void);

		void setVersion(int version) { _version = version; }

		//! @brief Returns PFont used in ObjectState state.
		inline PFont *getFont(ObjectState state) {
			return _font[state];
		}
		/** Returns PFont::Color used in ObjectState state. */
		PFont::Color *getColor(ObjectState state) {
			return _color[state];
		}
		//! @brief Returns menu PTexture used in ObjectState state.
		inline PTexture *getTextureMenu(ObjectState state) {
			return _tex_menu[state];
		}
		//! @brief Returns item PTexture used in ObjectState state.
		inline PTexture *getTextureItem(ObjectState state) {
			return _tex_item[state];
		}
		//! @brief Returns arrow PTexture used in ObjectState state.
		inline PTexture *getTextureArrow(ObjectState state) {
			return _tex_arrow[state];
		}
		//! @brief Returns separator PTexture used in ObjectState state.
		inline PTexture *getTextureSeparator(ObjectState state) {
			return _tex_sep[(state < OBJECT_STATE_SELECTED)
					? state : OBJECT_STATE_FOCUSED];
		}
		//! @brief Returns text pad in PadType dir.
		inline int getPad(PadType dir) const {
			return _pad[(dir != PAD_NO) ? dir : 0];
		}

		bool load(CfgParser::Entry *section);
		void unload(void);
		void check(void);

	private:
		void loadState(CfgParser::Entry *cs, ObjectState state);

	private:
		FontHandler* _fh;
		TextureHandler* _th;
		int _version;
		bool _loaded;

		PFont *_font[OBJECT_STATE_NO + 1];
		PFont::Color *_color[OBJECT_STATE_NO + 1];
		PTexture *_tex_menu[OBJECT_STATE_NO + 1];
		PTexture *_tex_item[OBJECT_STATE_NO + 1];
		PTexture *_tex_arrow[OBJECT_STATE_NO + 1];
		PTexture *_tex_sep[OBJECT_STATE_NO];

		int _pad[PAD_NO];
	};

	//! @brief CmdDialog/StatusWindow theme data container and parser.
	class TextDialogData {
	public:
		TextDialogData(FontHandler* fh, TextureHandler* th,
			       int version);
		~TextDialogData(void);

		void setVersion(int version) { _version = version; }

		//! @brief Returns PFont.
		inline PFont *getFont(void) { return _font; }
		//! @brief Returns PFont::Color.
		inline PFont::Color *getColor(void) { return _color; }
		//! @brief Returns background texture.
		inline PTexture *getTexture(void) { return _tex; }
		//! @brief Returns text pad in PadType dir.
		inline int getPad(PadType dir) const {
			return _pad[(dir != PAD_NO) ? dir : 0];
		}

		bool load(CfgParser::Entry *section);
		void unload(void);
		void check(void);

	private:
		FontHandler* _fh;
		TextureHandler* _th;
		int _version;
		bool _loaded;

		PFont *_font;
		PFont::Color *_color;
		PTexture *_tex;

		int _pad[PAD_NO];
	};

	/**
	 * Class holding WorkspaceIndicator theme data.
	 */
	class WorkspaceIndicatorData {
	public:
		WorkspaceIndicatorData(FontHandler* fh, TextureHandler *th);
		~WorkspaceIndicatorData(void);

		bool load(CfgParser::Entry *section);
		void unload(void);
		void check(void);

	private:
		FontHandler* _fh;
		TextureHandler* _th;
		bool _loaded;

	public:
		PFont *font;
		PFont::Color *font_color;
		PTexture *texture_background;
		PTexture *texture_workspace;
		PTexture *texture_workspace_act;

		uint edge_padding;
		uint workspace_padding;
	};

	/**
	 * Class holding harbour theme data.
	 */
	class HarbourData {
	public:
		HarbourData(TextureHandler* th);
		~HarbourData(void);

		inline PTexture *getTexture(void) const { return _texture; }

		bool load(CfgParser::Entry *section);
		void unload(void);
		void check(void);
	private:
		TextureHandler* _th;

		bool _loaded;
		/**< Texture for rendering dockapps in the harbour. */
		PTexture *_texture;
	};

	/**
	 * Data for pekwm_dialog
	 */
	class DialogData {
	public:
		DialogData(FontHandler* fh, TextureHandler* th);
		~DialogData(void);

		PTexture* getBackground(void) const { return _background; }

		PFont* getButtonFont(void) const { return _button_font; }
		PFont::Color* getButtonColor(void) const {
			return _button_color;
		}
		PTexture* getButton(ButtonState state) const {
			return _button[state < BUTTON_STATE_NO ? state : 0];
		}

		PFont* getTitleFont(void) const { return _title_font; }
		PFont::Color* getTitleColor(void) const { return _title_color; }

		PFont* getTextFont(void) const { return _text_font; }
		PFont::Color* getTextColor(void) const { return _text_color; }

		int getPad(PadType dir) const {
			return _pad[dir < PAD_NO ? dir : 0];
		}
		int padHorz(void) const {
			return getPad(PAD_LEFT) + getPad(PAD_RIGHT);
		}
		int padVert(void) const {
			return getPad(PAD_UP) + getPad(PAD_DOWN);
		}

		bool load(CfgParser::Entry *section);
		void unload(void);
		void check(void);

	private:
		void clear();

	private:
		FontHandler* _fh;
		TextureHandler* _th;
		bool _loaded;

		PTexture *_background;
		PFont *_button_font;
		PFont::Color *_button_color;
		PTexture *_button[BUTTON_STATE_NO];

		PFont *_title_font;
		PFont::Color *_title_color;
		PFont *_text_font;
		PFont::Color *_text_color;

		int _pad[PAD_NO];
	};

	Theme(FontHandler *fh, ImageHandler *ih, TextureHandler *th,
	      const std::string& theme_file, const std::string &theme_variant,
	      bool is_owner = false);
	~Theme();

	bool load(const std::string &dir, const std::string &variant,
		  bool force=false);
	void unload();
	bool variantExists(const std::string &dir, const std::string &variant);

	inline const GC &getInvertGC(void) const { return _invert_gc; }

	const std::string& getThemeDir() const { return _theme_dir; }
	const std::string& getThemeFile() const { return _theme_file; }
	const std::string& getBackground() const { return _background; }

	Util::StringMap<PDecorData*>::const_iterator
	decor_begin(void) {
		return _decors.begin();
	}
	Util::StringMap<PDecorData*>::const_iterator
	decor_end(void) {
		return _decors.end();
	}

	/**
	 * Find PDecorData based on name.
	 */
	PDecorData *getPDecorData(const std::string &name) {
		return _decors.get(name);
	}

	Theme::DialogData *getDialogData(void) { return &_dialog_data; }
	Theme::HarbourData *getHarbourData(void) { return &_harbour_data; }
	Theme::PMenuData *getMenuData(void) { return &_menu_data; }
	Theme::TextDialogData *getCmdDialogData(void) { return &_cmd_d_data; }
	Theme::TextDialogData *getStatusData(void) { return &_status_data; }
	Theme::WorkspaceIndicatorData *getWorkspaceIndicatorData(void) {
		return &_ws_indicator_data;
	}

protected:
	Theme()
		: _fh(nullptr),
		  _ih(nullptr),
		  _th(nullptr),
		  _is_owner(false),
		  _version(0),
		  _loaded(false),
		  _invert_gc(None),
		  _dialog_data(nullptr, nullptr),
		  _menu_data(nullptr, nullptr, 0),
		  _harbour_data(nullptr),
		  _status_data(nullptr, nullptr, 0),
		  _cmd_d_data(nullptr, nullptr, 0),
		  _ws_indicator_data(nullptr, nullptr)
	{
	}

private:
	void loadThemeRequire(CfgParser &theme_cfg, const std::string &file);
	void loadVersion(CfgParser::Entry *root);
	void loadBackground(CfgParser::Entry *section);
	void loadColorMaps(CfgParser::Entry *section);
	void loadDecors(CfgParser::Entry *root);
	void getThemePaths(const std::string &dir, const std::string &variant,
			   std::string &norm_dir,  std::string &file,
			   std::string &variant_file);

	FontHandler* _fh;
	ImageHandler* _ih;
	TextureHandler* _th;

	std::string _theme_dir; /**< Path to theme directory. */
	std::string _theme_file;
	bool _is_owner;
	int _version;
	std::string _background;
	std::map<std::string, ColorMap> _color_maps;
	TimeFiles _cfg_files;

	bool _loaded;

	// gc
	GC _invert_gc;

	// frame decors
	Util::StringMap<Theme::PDecorData*> _decors;

	DialogData _dialog_data;
	PMenuData _menu_data;
	HarbourData _harbour_data;
	TextDialogData _status_data;
	TextDialogData _cmd_d_data;
	WorkspaceIndicatorData _ws_indicator_data;
};

namespace pekwm
{
	Theme* theme();
}

#endif // _PEKWM_THEME_HH_
