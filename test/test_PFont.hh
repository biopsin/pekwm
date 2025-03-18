//
// test_PFont.hh for pekwm
// Copyright (C) 2021-2023 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#include <iostream>

#include "test.hh"
#include "tk/PFont.hh"

struct WMP {
	const char *str;
	uint w;
};

class MockPFont : public PFont {
public:
	MockPFont(WMP *width_map);
	virtual ~MockPFont();

	virtual bool load(const PFont::Descr&) { return true; }
	virtual void unload() { }
	virtual uint getWidth(const std::string& text, uint max_chars = 0);
	virtual void setColor(PFont::Color*) { }

private:
	virtual std::string toNativeDescr(const PFont::Descr& descr) const {
		return descr.str();
	}
	virtual void drawText(PSurface*, int, int, const std::string&,
			      uint, bool) { }

private:
	std::map<std::string, uint> _width_map;
};

MockPFont::MockPFont(WMP *width_map)
	: PFont(1.0)
{
	for (int i = 0; width_map[i].str != nullptr; ++i) {
		_width_map[width_map[i].str] = width_map[i].w;
	}
}

MockPFont::~MockPFont(void)
{
}

uint
MockPFont::getWidth(const std::string& text, uint max_chars)
{
	std::string key = text + std::to_string(max_chars);
	std::map<std::string, uint>::iterator it = _width_map.find(key);
	if (it == _width_map.end()) {
		std::cerr << "unexpected lookup " << key << std::endl;
		std::cerr << "|" << text.substr(0, max_chars)
			  << "|" << std::endl;
		exit(1);
	}
	return _width_map[key];
}

class TestPFont : public TestSuite {
public:
	TestPFont(void);
	~TestPFont(void);

	bool run_test(TestSpec spec, bool status);

private:
	// parse
	static void testParseFamily();
	static void testParseSize();
	static void testParseProp();
	static void testParseFull();

	// trim
	static void testTrimEndNoTrim(void);
	static void testTrimEndTrim(void);
	static void testTrimEndNoSpace(void);
	static void testTrimEndUTF8(void);
	static void testTrimMiddle(void);
};

TestPFont::TestPFont(void)
	: TestSuite("PFont")
{
}

TestPFont::~TestPFont(void)
{
}

bool
TestPFont::run_test(TestSpec spec, bool status)
{
	TEST_FN(spec, "parseFamily", testParseFamily());
	TEST_FN(spec, "parseSize", testParseSize());
	TEST_FN(spec, "parseProp", testParseProp());
	TEST_FN(spec, "parseFull", testParseFull());

	TEST_FN(spec, "trimEndNoTrim", testTrimEndNoTrim());
	TEST_FN(spec, "trimEndTrim", testTrimEndTrim());
	TEST_FN(spec, "trimEndNoSpace", testTrimEndNoSpace());
	TEST_FN(spec, "trimEndUTF8", testTrimEndUTF8());
	TEST_FN(spec, "trimMiddle", testTrimMiddle());
	return status;
}

void
TestPFont::testParseFamily()
{
	PFont::Descr descr("Sans", false);
	ASSERT_EQUAL("family count", 1, descr.getFamilies().size());
	ASSERT_EQUAL("family", "Sans", descr.getFamilies()[0].getFamily());
	ASSERT_EQUAL("size", 0, descr.getFamilies()[0].getSize());
	ASSERT_EQUAL("prop count", 0, descr.getProperties().size());
}

void
TestPFont::testParseSize()
{
	PFont::Descr descr("-12", false);
	ASSERT_EQUAL("family count", 1, descr.getFamilies().size());
	ASSERT_EQUAL("family", "", descr.getFamilies()[0].getFamily());
	ASSERT_EQUAL("size", 12, descr.getFamilies()[0].getSize());
	ASSERT_EQUAL("prop count", 0, descr.getProperties().size());
}

void
TestPFont::testParseProp()
{
	PFont::Descr descr(":weight=bold", false);
	ASSERT_EQUAL("family count", 0, descr.getFamilies().size());
	ASSERT_EQUAL("prop count", 1, descr.getProperties().size());
	ASSERT_EQUAL("prop weight", "bold",
		     descr.getProperty("weight")->getValue());
}

void
TestPFont::testParseFull()
{
	PFont::Descr descr("Sans-10,Serif-20:weight=bold:slant=normal", false);
	ASSERT_EQUAL("family count", 2, descr.getFamilies().size());
	ASSERT_EQUAL("family", "Sans", descr.getFamilies()[0].getFamily());
	ASSERT_EQUAL("size", 10, descr.getFamilies()[0].getSize());
	ASSERT_EQUAL("family", "Serif", descr.getFamilies()[1].getFamily());
	ASSERT_EQUAL("size", 20, descr.getFamilies()[1].getSize());
	ASSERT_EQUAL("prop count", 2, descr.getProperties().size());
	ASSERT_EQUAL("prop weight", "bold",
		     descr.getProperty("weight")->getValue());
	ASSERT_EQUAL("prop slant", "normal",
		     descr.getProperty("slant")->getValue());
}

void
TestPFont::testTrimEndNoTrim(void)
{
	WMP fontd[] = {{"test0", 100}, {nullptr, 0}};
	MockPFont font(fontd);
	std::string str("test");
	font.trim(str, PFont::FONT_TRIM_END, 100);
	ASSERT_EQUAL("no trim", "test", str);
}

void
TestPFont::testTrimEndTrim(void)
{
	WMP fontd[] = {
		{"test0", 100},
		{"test3", 75},
		{"test2", 50},
		{nullptr, 0}
	};
	MockPFont font(fontd);
	std::string str("test");
	font.trim(str, PFont::FONT_TRIM_END, 50);
	ASSERT_EQUAL("trim end", "te", str);
}

void
TestPFont::testTrimEndNoSpace(void)
{
	WMP fontd[] ={{"test0", 100}, {"test3", 75},
		      {"test2", 50}, {"test1", 25},
		      {nullptr, 0}};
	MockPFont font(fontd);
	std::string str("test");
	font.trim(str, PFont::FONT_TRIM_END, 10);
	ASSERT_EQUAL("trim end, does not fit", "", str);
}

void
TestPFont::testTrimEndUTF8(void)
{
	WMP fontd[] = {{"Räksmörgås — M0", 100},
		       {"Räksmörgås — M17", 90},
		       {"Räksmörgås — M15", 80},
		       {"Räksmörgås — M14", 70},
		       {"Räksmörgås — M13", 60},
		       {"Räksmörgås — M12", 50},
		       {"Räksmörgås — M10", 40},
		       {nullptr, 0}};
	MockPFont font(fontd);
	std::string str("Räksmörgås — M");
	font.trim(str, PFont::FONT_TRIM_END, 45);
	ASSERT_EQUAL("trim end, UTF8", "Räksmörg", str);
}

void
TestPFont::testTrimMiddle(void)
{
	WMP fontd[] = {{"testtest0", 80},
		       {"..0", 20},

		       {"testtest7", 70},
		       {"testtest6", 60},
		       {"testtest5", 50},
		       {"testtest4", 40},
		       {"testtest3", 30},
		       {"testtest2", 20},

		       {"ttest0", 50},
		       {"test0", 40},
		       {"est0", 30},
		       {"st0", 20},

		       {"test2", 20},

		       {"te..st0", 60},
		       {nullptr, 0}};
	MockPFont font(fontd);
	std::string str("testtest");
	font.setTrimString("..");
	font.trim(str, PFont::FONT_TRIM_MIDDLE, 60);
	ASSERT_EQUAL("trim middle", "te..st", str);
}
