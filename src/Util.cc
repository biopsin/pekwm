//
// Util.cc for pekwm
// Copyright (C) 2002-2020 Claes Nästén <pekdon@gmail.com>
//
// misc.cc for aewm++
// Copyright (C) 2000 Frank Hale <frankhale@yahoo.com>
// http://sapphire.sourceforge.net/
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#include "config.h"

#include <cerrno>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <iterator>

extern "C" {
#include <assert.h>
#include <fcntl.h>
#include <iconv.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
}

#include "CfgParser.hh"
#include "Debug.hh"
#include "Util.hh"

#define THEME_DEFAULT DATADIR "/pekwm/themes/default/theme"

namespace String {
    /** Get safe version of position */
    size_t safe_position(size_t pos, size_t fallback, size_t add) {
        return pos == std::wstring::npos ? fallback : (pos + add);
    }

    /**
     * Split string into tokens similar to how a shell interprets
     * escape and quote characets.
     */
    std::vector<std::string>
    shell_split(const std::string& str)
    {
        bool in_tok = false;
        bool in_escape = false;
        char in_quote = 0;
        std::vector<std::string> toks;

        std::string tok;
        for (auto c : str) {
            if (! in_tok && ! isspace(c)) {
                in_tok = true;
            }

            if (in_tok) {
                if (in_escape) {
                    tok.push_back(c);
                    in_escape = false;
                } else if (c == in_quote) {
                    in_quote = 0;
                    toks.push_back(tok);
                    tok.clear();
                    in_tok = false;
                } else if (c == '\\') {
                    in_escape = true;
                } else if (in_quote) {
                    tok.push_back(c);
                } else if (c == '"' || c == '\'') {
                    in_quote = c;
                } else if (isspace(c)) {
                    toks.push_back(tok);
                    tok.clear();
                    in_tok = false;
                } else {
                    tok.push_back(c);
                }
            }
        }

        if (in_tok && tok.size()) {
            toks.push_back(tok);
        }

        return toks;
    }
}

namespace Util {

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif // HOST_NAME_MAX

static iconv_t do_iconv_open(const char **from_names, const char **to_names);
static size_t do_iconv(iconv_t ic, const char **inp, size_t *in_bytes,
                       char **outp, size_t *out_bytes);

// Initializers, members used for shared buffer
unsigned int WIDE_STRING_COUNT = 0;
iconv_t IC_TO_WC = reinterpret_cast<iconv_t>(-1);
iconv_t IC_TO_UTF8 = reinterpret_cast<iconv_t>(-1);
char *ICONV_BUF = 0;
size_t ICONV_BUF_LEN = 0;

// Constants, name of iconv internal names
const char *ICONV_WC_NAMES[] = {"WCHAR_T", "UCS-4", 0};
const char *ICONV_UTF8_NAMES[] = {"UTF-8", "UTF8", 0};

const char *ICONV_UTF8_INVALID_STR = "<INVALID>";
const wchar_t *ICONV_WIDE_INVALID_STR = L"<INVALID>";

// Constants, maximum number of bytes a single UTF-8 character can use.
const size_t UTF8_MAX_BYTES = 6;

/**
 * Return environment variabel as string.
 */
std::string getEnv(const std::string& key)
{
    auto val = getenv(key.c_str());
    return val ? val : "";
}

/**
 * Fork and execute command with /bin/sh and execlp
 */
void
forkExec(std::string command)
{
    if (command.length() == 0) {
        ERR("command length == 0");
        return;
    }

    pid_t pid = fork();
    switch (pid) {
    case 0:
        setsid();
        execlp("/bin/sh", "sh", "-c", command.c_str(), (char *) 0);
        ERR("execlp failed: " << strerror(errno));
        exit(1);
    case -1:
        ERR("fork failed: " << strerror(errno));
        break;
    default:
        TRACE("started child " << pid);
        break;
    }
}

static void
forkExecLog(const char* fun, int line, const char *msg,
            const std::vector<std::string>& args)
{
    DebugErrObj dobj;
    dobj << _PEK_DEBUG_START(fun, line)
         << " " << msg  << ": " << strerror(errno)
         << ". args";
    auto it = args.begin();
    for (; it != args.end(); ++it) {
        dobj << " " << *it;
    }
}

pid_t
forkExec(const std::vector<std::string>& args)
{
    assert(! args.empty());

    pid_t pid = fork();
    switch (pid) {
    case 0: {
        int i = 0;
        auto argv = new char*[args.size() + 1];
        auto it = args.begin();
        for (; it != args.end(); ++it) {
            argv[i++] = const_cast<char*>(it->c_str());
        }
        argv[i] = nullptr;

        setsid();
        execvp(argv[0], argv);
        forkExecLog(__PRETTY_FUNCTION__, __LINE__, "execvp failed", args);
        exit(1);
    }
    case -1:
        forkExecLog(__PRETTY_FUNCTION__, __LINE__, "fork failed", args);
    default:
        TRACE("started child " << pid);
        return pid;
    }
}

/**
 * Wrapper for gethostname returning a string instead of populating
 * char buffer.
 */
std::string
getHostname(void)
{
    std::string hostname;

    // Set WM_CLIENT_MACHINE
    char hostname_buf[HOST_NAME_MAX + 1];
    if (! gethostname(hostname_buf, HOST_NAME_MAX)) {
      // Make sure it is null terminated
      hostname_buf[HOST_NAME_MAX] = '\0';
      hostname = hostname_buf;
    }

    return hostname;
}

/**
 * Set file descriptor in non-blocking mode.
 */
bool
setNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        ERR("failed to get flags from fd " << fd << ": " << strerror(errno));
        return false;
    }
    int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (ret == -1) {
        ERR("failed to set O_NONBLOCK on fd " << fd << ": " << strerror(errno));
        return false;
    }
    return true;
}

//! @brief Determines if the file exists
bool
isFile(const std::string &file)
{
    if (file.size() == 0) {
        return false;
    }

    struct stat stat_buf;
    if (stat(file.c_str(), &stat_buf) == 0) {
        return (S_ISREG(stat_buf.st_mode));
    }

    return false;
}

//! @brief Determines if the file is executable for the current user.
bool
isExecutable(const std::string &file)
{
    if (file.size() == 0) {
        ERR("file length == 0");
        return false;
    }

    struct stat stat_buf;
    if (! stat(file.c_str(), &stat_buf)) {
        if (stat_buf.st_uid == getuid()) { // user readable and executable
            if ((stat_buf.st_mode&S_IRUSR) && (stat_buf.st_mode&S_IXUSR)) {
                return true;
            }
        }
        if (getgid() == stat_buf.st_gid) { // group readable and executable
            if ((stat_buf.st_mode&S_IRGRP) && (stat_buf.st_mode&S_IXGRP)) {
                return true;
            }
        }
        if ((stat_buf.st_mode&S_IROTH) && (stat_buf.st_mode&S_IXOTH)) {
            return true; // other readable and executable
        }
    }

    return false;
}

/**
 * Get file mtime.
 */
time_t
getMtime(const std::string &file)
{
    struct stat stat_buf;

    if (! stat(file.c_str(), &stat_buf)) {
        return stat_buf.st_mtime;
    } else {
        return 0;
    }
}

/**
 * Copies a single text file.
 */
bool
copyTextFile(const std::string &from, const std::string &to)
{
    if ((from.length() == 0) || (to.length() == 0)) {
        return false;
    }

    std::ifstream stream_from(from.c_str());
    if (! stream_from.good()) {
        USER_WARN("can not copy: " << from << " to: " << to);
        return false;
    }

    std::ofstream stream_to(to.c_str());
    if (! stream_to.good()) {
        USER_WARN("can not copy: " << from << " to: " << to);
        return false;
    }

    stream_to << stream_from.rdbuf();

    return true;
}

/**
 * Get name of the current user.
 */
std::string
getUserName(void)
{
    // Try to lookup current user with
    struct passwd *entry = getpwuid(geteuid());

    if (entry && entry->pw_name) {
        return entry->pw_name;
    } else {
        if (getenv("USER")) {
            return getenv("USER");
        } else {
            return "UNKNOWN";
        }
    }
}


//! @brief Returns .extension of file
std::string
getFileExt(const std::string &file)
{
    std::string::size_type pos = file.find_last_of('.');
    if ((pos != std::string::npos) && (pos < file.size())) {
        return file.substr(pos + 1, file.size() - pos - 1);
    } else {
        return std::string("");
    }
}

//! @brief Returns dir part of file
std::string
getDir(const std::string &file)
{
    std::string::size_type pos = file.find_last_of('/');
    if ((pos != std::string::npos) && (pos < file.size())) {
        return file.substr(0, pos);
    } else {
        return std::string("");
    }
}

//! @brief Replaces the ~ with the complete homedir path.
void
expandFileName(std::string &file)
{
    if (file.size() > 0) {
        if (file[0] == '~') {
            file.replace(0, 1, getEnv("HOME"));
        }
    }
}

void
getThemeDir(const std::string& config_file,
            std::string& dir, std::string& variant,
            std::string& theme_file)
{
    CfgParser cfg;
    cfg.parse(config_file, CfgParserSource::SOURCE_FILE, true);
    auto files = cfg.getEntryRoot()->findSection("FILES");
    if (files != nullptr) {
        std::vector<CfgParserKey*> keys;
        keys.push_back(new CfgParserKeyPath("THEME", dir, THEME_DEFAULT));
        keys.push_back(new CfgParserKeyString("THEMEVARIANT", variant));
        files->parseKeyValues(keys.begin(), keys.end());
        std::for_each(keys.begin(), keys.end(), Util::Free<CfgParserKey*>());
    } else {
        dir = THEME_DEFAULT;
        variant = "";
    }

    std::string norm_dir(dir);
    if (dir.size() && dir.at(dir.size() - 1) == '/') {
        norm_dir.erase(norm_dir.end() - 1);
    }

    theme_file = norm_dir + "/theme";
    if (! variant.empty()) {
        auto theme_file_variant = theme_file + "-" + variant;
        if (isFile(theme_file_variant)) {
            theme_file = theme_file_variant;
        } else {
            DBG("theme variant " << variant << " does not exist");
        }
    }
}

const char*
spaceChars(char escape)
{
    return " \t\n";
}

const wchar_t*
spaceChars(wchar_t escape)
{
    return L" \t\n";
}

//! @brief Converts wide-character string to multibyte version
//! @param str String to convert.
//! @return Returns multibyte version of string.
std::string
to_mb_str(const std::wstring &str)
{
    size_t ret, num = str.size() * 6 + 1;
    char *buf = new char[num];
    memset(buf, '\0', num);

    ret = wcstombs(buf, str.c_str(), num);
    if (ret == static_cast<size_t>(-1)) {
        USER_WARN("failed to convert wide string to multibyte string");
    }
    std::string ret_str(buf);

    delete [] buf;

    return ret_str;
}

//! @brief Converts multibyte string to wide-character version
//! @param str String to convert.
//! @return Returns wide-character version of string.
std::wstring
to_wide_str(const std::string &str)
{
    size_t ret, num = str.size() + 1;
    wchar_t *buf = new wchar_t[num];
    wmemset(buf, L'\0', num);

    ret = mbstowcs(buf, str.c_str(), num);
    if (ret == static_cast<size_t>(-1)) {
        USER_WARN("failed to convert multibyte string to wide string");
    }
    std::wstring ret_str(buf);

    delete [] buf;

    return ret_str;
}

//! @brief Open iconv handle with to/from names.
//! @param from_names null terminated list of from name alternatives.
//! @param to_names null terminated list of to name alternatives.
//! @return iconv_t handle on success, else -1.
iconv_t
do_iconv_open(const char **from_names, const char **to_names)
{
    iconv_t ic = reinterpret_cast<iconv_t>(-1);

    // Try all combinations of from/to name's to get a working
    // conversion handle.
    for (unsigned int i = 0; from_names[i]; ++i) {
        for (unsigned int j = 0; to_names[j]; ++j) {
            ic = iconv_open(to_names[j], from_names[i]);
            if (ic != reinterpret_cast<iconv_t>(-1)) {
#ifdef HAVE_ICONVCTL
                int int_value_one = 1;
                iconvctl(ic, ICONV_SET_DISCARD_ILSEQ, &int_value_one);
#endif // HAVE_ICONVCTL
                return ic;
            }
        }
    }

  return ic;
}

//! @brief Iconv wrapper to hide different definitions of iconv.
//! @param ic iconv handle.
//! @param inp Input pointer.
//! @param in_bytes Input bytes.
//! @param outp Output pointer.
//! @param out_bytes Output bytes.
//! @return number of bytes converted irreversible or -1 on error.
size_t
do_iconv(iconv_t ic, const char **inp, size_t *in_bytes,
         char **outp, size_t *out_bytes)
{
#ifdef ICONV_CONST
    return iconv(ic, inp, in_bytes, outp, out_bytes);
#else // !ICONV_CONST
    return iconv(ic, const_cast<char**>(inp), in_bytes, outp, out_bytes);
#endif // ICONV_CONST
}

//! @brief Init iconv conversion.
void
iconv_init(void)
{
    // Cleanup previous init if any, being paranoid.
    iconv_deinit();

    // Raise exception if this fails
    IC_TO_WC = do_iconv_open(ICONV_UTF8_NAMES, ICONV_WC_NAMES);
    IC_TO_UTF8 = do_iconv_open(ICONV_WC_NAMES, ICONV_UTF8_NAMES);

    // Equal mean
    if (IC_TO_WC != reinterpret_cast<iconv_t>(-1)
        && IC_TO_UTF8 != reinterpret_cast<iconv_t>(-1)) {
        // Create shared buffer.
        ICONV_BUF_LEN = 1024;
        ICONV_BUF = new char[ICONV_BUF_LEN];
    }
}

//! @brief Deinit iconv conversion.
void
iconv_deinit(void)
{
    // Cleanup resources
    if (IC_TO_WC != reinterpret_cast<iconv_t>(-1)) {
        iconv_close(IC_TO_WC);
    }

    if (IC_TO_UTF8 != reinterpret_cast<iconv_t>(-1)) {
        iconv_close(IC_TO_UTF8);
    }

    if (ICONV_BUF) {
        delete [] ICONV_BUF;
    }

    // Set members to safe values
    IC_TO_WC = reinterpret_cast<iconv_t>(-1);
    IC_TO_UTF8 = reinterpret_cast<iconv_t>(-1);
    ICONV_BUF = 0;
    ICONV_BUF_LEN = 0;
}

//! @brief Validate buffer size, grow if required.
//! @param size Required size.
void
iconv_buf_grow(size_t size)
{
    if (ICONV_BUF_LEN < size) {
        // Free resources, if any.
        if (ICONV_BUF) {
            delete [] ICONV_BUF;
        }

        // Calculate new buffer length and allocate new buffer
        for (; ICONV_BUF_LEN < size; ICONV_BUF_LEN *= 2)
            ;
        ICONV_BUF = new char[ICONV_BUF_LEN];
    }
}

//! @brief Converts wide-character string to UTF-8
//! @param str String to convert.
//! @return Returns UTF-8 representation of string.
std::string
to_utf8_str(const std::wstring &str)
{
    std::string utf8_str;

    // Calculate length
    size_t in_bytes = str.size() * sizeof(wchar_t);
    size_t out_bytes = str.size() * UTF8_MAX_BYTES + 1;

    iconv_buf_grow(out_bytes);

    // Convert
    const char *inp = reinterpret_cast<const char*>(str.c_str());
    char *outp = ICONV_BUF;
    size_t len = do_iconv(IC_TO_UTF8, &inp, &in_bytes, &outp, &out_bytes);
    if (len != static_cast<size_t>(-1)) {
        // Terminate string and cache result
        *outp = '\0';
        utf8_str = ICONV_BUF;
    } else {
        USER_WARN("to_utf8_str, failed with error " << strerror(errno));
        utf8_str = ICONV_UTF8_INVALID_STR;
    }

    return utf8_str;
}

//! @brief Converts to wide string from UTF-8
//! @param str String to convert.
//! @return Returns wide representation of string.
std::wstring
from_utf8_str(const std::string &str)
{
    std::wstring wide_str;

    // Calculate length
    size_t in_bytes = str.size();
    size_t out_bytes = (in_bytes + 1) * sizeof(wchar_t);

    iconv_buf_grow(out_bytes);

    // Convert
    const char *inp = str.c_str();
    char *outp = ICONV_BUF;
    size_t len = do_iconv(IC_TO_WC, &inp, &in_bytes, &outp, &out_bytes);
    if (len != static_cast<size_t>(-1)) {
        // Terminate string and cache result
        *reinterpret_cast<wchar_t*>(outp) = L'\0';

        wide_str = reinterpret_cast<wchar_t*>(ICONV_BUF);
    } else {
        USER_WARN("from_utf8_str, failed on string \"" << str << "\"");
        wide_str = ICONV_WIDE_INVALID_STR;
    }

    return wide_str;
}

} // end namespace Util.
