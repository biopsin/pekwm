#ifndef _CLIENT_HINTS_HH_
#define _CLIENT_HINTS_HH_

#include <string>

#include "config.h"

#include "pekwm.hh"
#include "x11.hh"

class ClientHints {
public:
    // MWM
    struct Mwm {
        ulong flags;
        ulong functions;
        ulong decorations;
    };

    enum {
        MWM_HINTS_FUNCTIONS = (1L << 0),
        MWM_HINTS_DECORATIONS = (1L << 1),
        MWM_HINTS_NUM = 3
    };
    enum {
        MWM_FUNC_ALL = (1L << 0),
        MWM_FUNC_RESIZE = (1L << 1),
        MWM_FUNC_MOVE = (1L << 2),
        MWM_FUNC_ICONIFY = (1L << 3),
        MWM_FUNC_MAXIMIZE = (1L << 4),
        MWM_FUNC_CLOSE = (1L << 5)
    };
    enum {
        MWM_DECOR_ALL = (1L << 0),
        MWM_DECOR_BORDER = (1L << 1),
        MWM_DECOR_HANDLE = (1L << 2),
        MWM_DECOR_TITLE = (1L << 3),
        MWM_DECOR_MENU = (1L << 4),
        MWM_DECOR_ICONIFY = (1L << 5),
        MWM_DECOR_MAXIMIZE = (1L << 6)
    };

    // EWMH actions
    class Actions {
    public:
        Actions(void)
            : move(true), resize(true), iconify(true),
              shade(true), stick(true), maximize_horz(true),
              maximize_vert(true), fullscreen(true),
              change_ws(true), close(true) { }
        ~Actions(void) { }

        bool move:1;
        bool resize:1;
        bool iconify:1; // iconify
        bool shade:1;
        bool stick:1;
        bool maximize_horz:1;
        bool maximize_vert:1;
        bool fullscreen:1;
        bool change_ws:1; // workspace
        bool close:1;
    };

    class State {
    public:
        State(void)
            : maximized_vert(false),
              maximized_horz(false),
              shaded(false),
              fullscreen(false),
              demands_attention(false),
              sticky(false),
              workspace(0),
              placed(false),
              initial_frame_order(0),
              skip(0),
              decor(DECOR_TITLEBAR|DECOR_BORDER),
              cfg_deny(CFG_DENY_NO) { }
        ~State(void) { }

        bool hidden:1;
        bool maximized_vert:1;
        bool maximized_horz:1;
        bool shaded:1;
        bool fullscreen:1;
        bool demands_attention:1;
        bool sticky:1;
        uint workspace;

        // pekwm states
        bool placed;
        uint initial_frame_order; /**< Initial frame position */
        uint skip, decor, cfg_deny;
    };

    ClientHints();
    virtual ~ClientHints();

    void readHints(Window win);

    const Actions &getActions() const { return _actions; }
    const State &getState() const { return _state; }

    bool isTransient(void) const { return _transient_for_window != None; }
    Window getTransientForClientWindow(void) const {
        return _transient_for_window;
    }
    bool cameWithPosition(void) {
        return _size->flags & (PPosition|USPosition);
    }
    bool hasTitlebar(void) const { return (_state.decor&DECOR_TITLEBAR); }
    bool hasBorder(void) const { return (_state.decor&DECOR_BORDER); }

    bool hasStrut(void) const { return _strut.isSet(); }
    Strut *getStrut() { return &_strut; }

    bool demandsAttention(void) const { return _state.demands_attention; }
    /** Return PID of client. */
    long getPid() const { return _pid; }

protected:
    void readMwm(Window win);
    void readEwmh(Window win);
    void readEwmhStrut(Window win);
    void readPekwm(Window win);
    void readClientPid(Window win);
    void readClientRemote(Window win);
    void getWMProtocols(Window win);

    Mwm* getMwmHints(Window win);
    bool getEwmhStates(Window win, NetWMStates &win_states);
    long getPekwmFrameOrder(Window win);

protected:
    XSizeHints *_size;

    Actions _actions;
    State _state;
    Strut _strut;

    Window _transient_for_window;

    Layer _layer;
    std::wstring _user_title;

    bool _send_focus_message;
    bool _send_close_message;

    /** _NET_WM_PID of the client, only valid if is_remote is false. */
    long _pid;
    std::string _client_machine;
};

#endif // _CLIENT_HINTS_HH_
