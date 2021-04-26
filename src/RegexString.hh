//
// RegexString.hh for pekwm
// Copyright (C)  2003-2020 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#ifndef _PEKWM_REGEXSTRING_HH_
#define _PEKWM_REGEXSTRING_HH_

#include "config.h"

#include <string>
#include <vector>

extern "C" {
#include <sys/types.h>
#include <regex.h>
}

#include "Compat.hh"
#include "Types.hh"

//! @brief POSIX regular expression wrapper.
class RegexString
{
public:
    //! @brief Part of parsed replace data.
    class Part
    {
    public:
        //! @brief RegexString::Part constructor.
        Part(const std::string &str, int ref = -1)
            : _string(str),
              _ref(ref)
        {
        }
        //! @brief RegexString::Part destructor.
        ~Part(void) { }

        //! @brief Returns string data.
        const std::string &get_string(void) { return _string; }
        //! @brief Returns reference number.
        int get_reference(void) { return _ref; }

    private:
        std::string _string; //!< String data at item.
        int _ref; //!< Reference string should be replaced with.
    };

    RegexString(void);
    RegexString(const std::string &string, bool full = false);
    ~RegexString(void);

    //! @brief Returns parse_match data status.
    bool is_match_ok(void) { return _reg_ok; }
    const std::string& getPattern(void) const { return _pattern; }

    bool ed_s(std::string &str);

    bool parse_match(const std::string &match, bool full = false);
    bool parse_replace(const std::string &replace);
    bool parse_ed_s(const std::string &ed_s);

    bool operator==(const std::string &rhs) const;

private:
    RegexString(const RegexString &);
    RegexString &operator=(const RegexString &);
    void free_regex(void);

private:
    regex_t _regex; //!< Compiled regular expression holder.
    bool _reg_ok; //!< _regex compiled ok flag.
    std::string _pattern; /**< String regex was compiled from. */
    /** If true, a non-matching regexp is considered a match. */
    bool _reg_inverted;

    int _ref_max; //!< Highest reference used.
    /** Vector of RegexString::Part holding data generated by parse_replace. */
    std::vector<RegexString::Part> _refs;
    static const char SEPARATOR; /**< Regular expression seperator. */
};

#endif // _PEKWM_REGEXSTRING_HH_
