//
// TextureHandler.cc for pekwm
// Copyright (C) 2004-2023 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#include "config.h"

#include "Debug.hh"
#include "PTexture.hh"
#include "PTexturePlain.hh"
#include "TextureHandler.hh"
#include "Util.hh"
#include "X11.hh"

#include <iostream>

static Util::StringTo<PTexture::Type> parse_map[] =
	{{"SOLID", PTexture::TYPE_SOLID},
	 {"SOLIDRAISED", PTexture::TYPE_SOLID_RAISED},
	 {"LINESHORZ", PTexture::TYPE_LINES_HORZ},
	 {"LINESVERT", PTexture::TYPE_LINES_VERT},
	 {"IMAGE", PTexture::TYPE_IMAGE},
	 {"IMAGEMAPPED", PTexture::TYPE_IMAGE_MAPPED},
	 {"EMPTY", PTexture::TYPE_EMPTY},
	 {nullptr, PTexture::TYPE_NO}};

static bool
parseSize(const std::string &str, uint &width, uint &height)
{
	std::vector<std::string> tok;
	if ((Util::splitString(str, tok, "x", 2, true)) != 2) {
		return false;
	}

	try {
		width = std::stoi(tok[0]);
		height = std::stoi(tok[1]);
	} catch (std::invalid_argument&) {
		return false;
	}
	return true;
}

TextureHandler::TextureHandler(void)
	: _length_min(5)
{
}

TextureHandler::~TextureHandler(void)
{
}

/**
 * Gets or creates a PTexture
 */
PTexture*
TextureHandler::getTexture(const std::string &texture)
{
	if (texture.size() < _length_min) {
		// name to short, can not be a valid texture.
		P_TRACE("texture " << texture << " name too short");
		return nullptr;
	}

	// check for already existing entry
	entry_vector::iterator it(_textures.begin());
	for (; it != _textures.end(); ++it) {
		if (*(*it) == texture) {
			(*it)->incRef();
			return (*it)->getTexture();
		}
	}

	// parse texture
	PTexture *ptexture = parse(texture);
	if (ptexture) {
		// create new entry
		TextureHandler::Entry *entry =
			new TextureHandler::Entry(texture, ptexture);
		entry->incRef();
		_textures.push_back(entry);
	}

	return ptexture;
}

/**
 * Add/Increment reference cont for texture.
 *
 *  @return Pointer to texture referenced.
 */
PTexture*
TextureHandler::referenceTexture(PTexture *texture)
{
	// Check for already existing entry
	entry_vector::iterator it(_textures.begin());
	for (; it != _textures.end(); ++it) {
		if ((*it)->getTexture() == texture) {
			(*it)->incRef();
			return texture;
		}
	}

	// Create new entry
	TextureHandler::Entry *entry = new TextureHandler::Entry("@", texture);
	entry->incRef();
	_textures.push_back(entry);

	return texture;
}

/**
 * Return/free a texture.
 */
void
TextureHandler::returnTexture(PTexture *texture)
{
	bool found = false;

	entry_vector::iterator it(_textures.begin());
	for (; it != _textures.end(); ++it) {
		if ((*it)->getTexture() == texture) {
			found = true;

			(*it)->decRef();
			if ((*it)->getRef() == 0) {
				delete *it;
				_textures.erase(it);
			}
			break;
		}
	}

	if (! found) {
		delete texture;
	}
}

/**
 * Log all referenced textures as trace messages.
 */
void
TextureHandler::logTextures(const std::string& msg) const
{
	std::ostringstream oss;
	oss << msg << " " << _textures.size() << " texture entries";
	entry_vector::const_iterator it(_textures.begin());
	for (; it != _textures.end(); ++it) {
		oss << std::endl
		    << "  " << (*it)->getRef()
		    << ": " << (*it)->getName();
	}
	P_TRACE(oss.str());
}

/**
 * Parses the string, and creates a texture
 */
PTexture*
TextureHandler::parse(const std::string &texture)
{
	PTexture *ptexture = 0;
	std::vector<std::string> tok;

	PTexture::Type type;
	if (Util::splitString(texture, tok, " \t")) {
		type = Util::StringToGet(parse_map, tok[0]);
	} else {
		type = Util::StringToGet(parse_map, texture);
	}

	// need at least type and parameter, except for EMPTY type
	if (tok.size() > 1) {
		tok.erase(tok.begin());

		switch (type) {
		case PTexture::TYPE_SOLID:
			ptexture = parseSolid(tok);
			break;
		case PTexture::TYPE_SOLID_RAISED:
			ptexture = parseSolidRaised(tok);
			break;
		case PTexture::TYPE_LINES_HORZ:
			ptexture = parseLines(true, tok);
			break;
		case PTexture::TYPE_LINES_VERT:
			ptexture = parseLines(false, tok);
			break;
		case PTexture::TYPE_IMAGE:
			ptexture = parseImage(texture);
			break;
		case PTexture::TYPE_IMAGE_MAPPED:
			ptexture = parseImageMapped(texture);
			break;
		case PTexture::TYPE_NO:
		default:
			break;
		}

		// If it fails to load, set clean resources and set it to 0.
		if (ptexture && ! ptexture->isOk()) {
			delete ptexture;
			ptexture = nullptr;
		}

	} else if (type == PTexture::TYPE_EMPTY) {
		ptexture = new PTextureEmpty();
	}

	return ptexture;
}

/**
 * Parse and create PTextureSolid
 */
PTexture*
TextureHandler::parseSolid(std::vector<std::string> &tok)
{
	if (tok.size() < 1) {
		USER_WARN("missing parameter to texture Solid");
		return 0;
	}

	PTextureSolid *tex = new PTextureSolid(tok[0]);
	tok.erase(tok.begin());

	// check if we have size
	if (tok.size() == 1) {
		parseSize(tex, tok[0]);
	}

	return tex;
}

/**
 * Parse and create PTextureSolidRaised
 */
PTexture*
TextureHandler::parseSolidRaised(const std::vector<std::string> &tok)
{
	if (tok.size() < 3) {
		USER_WARN("not enough parameters to texture SolidRaised "
			  "(3 required)");
		return 0;
	}

	size_t i = 0;
	PTextureSolidRaised *tex =
		new PTextureSolidRaised(tok[i], tok[i + 1], tok[i + 2]);
	i += 3;

	// Check if we have line width and offset.
	if (tok.size() > (i + 1)) {
		tex->setLineWidth(strtol(tok[i].c_str(), 0, 10));
		tex->setLineOff(strtol(tok[i + 1].c_str(), 0, 10));
		i += 2;
	}
	// Check if have side draw specified.
	if (tok.size() > (i + 3)) {
		tex->setDraw(Util::isTrue(tok[i]),
			     Util::isTrue(tok[i + 1]),
			     Util::isTrue(tok[i + 2]),
			     Util::isTrue(tok[i + 3]));
		i += 4;
	}

	// Check if we have size
	if (tok.size() > i) {
		parseSize(tex, tok[i]);
	}

	return tex;
}

/**
 * Parse and create PTextureLines
 */
PTexture*
TextureHandler::parseLines(bool horz, std::vector<std::string> &tok)
{
	if (tok.size() < 2) {
		USER_WARN("not enough parameters to texture Lines"
			  << (horz ? "Horz" : "Vert") << " (2 required)");
		return 0;
	}

	float line_size;
	bool size_percent;
	try {
		if (tok[0][tok[0].size()-1] == '%') {
			size_percent = true;
			tok[0].erase(tok[0].end() - 1);
			line_size = std::stof(tok[0]) / 100;
		} else {
			size_percent = false;
			line_size = std::stoi(tok[0]);
		}
	} catch (const std::invalid_argument&) {
		return nullptr;
	}

	tok.erase(tok.begin());

	uint width, height;
	if (::parseSize(tok.back(), width, height)) {
		tok.pop_back();
	} else {
		width = 0;
		height = 0;
	}

	PTextureLines *tex =
		new PTextureLines(line_size, size_percent, horz, tok);
	tex->setWidth(width);
	tex->setHeight(height);
	return tex;
}

/**
 * Parse Image texture, w
 */
PTexture*
TextureHandler::parseImage(const std::string& texture)
{
	// 6==strlen("IMAGE ")
	PTextureImage *image = new PTextureImage(texture.substr(6), "");
	if (! image->isOk()) {
		size_t pos = texture.find_first_not_of(" \t", 6);
		image->setImage(texture.substr(pos), "");
	}
	return image;
}

/**
 * Parse Image texture with colormap.
 */
PTexture*
TextureHandler::parseImageMapped(const std::string& texture)
{
	// 12==strlen("IMAGEMAPPED ")
	size_t map_start = texture.find_first_not_of(" \t", 12);
	size_t map_end = texture.find_first_of(" \t", map_start);
	if (map_end == std::string::npos) {
		USER_WARN("not enough parameters to texture ImageMapped "
			  "(2 required)");
		return nullptr;
	}
	size_t image_start = texture.find_first_not_of(" \t", map_end + 1);
	if (image_start == std::string::npos) {
		USER_WARN("not enough parameters to texture ImageMapped "
			  "(2 required)");
		return nullptr;
	}
	std::string colormap = texture.substr(map_start, map_end - map_start);
	return new PTextureImage(texture.substr(image_start), colormap);
}

/**
 * Parses size parameter, i.e. 10x20
 */
bool
TextureHandler::parseSize(PTexture *tex, const std::string &size)
{
	uint width, height;
	if (::parseSize(size, width, height)) {
		tex->setWidth(width);
		tex->setHeight(height);
		return true;
	}
	return false;
}
