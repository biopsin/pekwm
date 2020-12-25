#include "ClientHints.hh"
#include "Workspaces.hh"
#include "Util.hh"
#include "x11.hh"

ClientHints::ClientHints()
    : _size(0),
      _transient_for_window(None),
      _send_focus_message(false),
      _send_close_message(false)
{
}

ClientHints::~ClientHints()
{
}

/**
 * Read "all" hints required during creation of Client.
 */
void
ClientHints::readHints(Window win)
{
    readMwm(win);
    readEwmh(win);
    readEwmhStrut(win);
    readPekwm(win);
    readClientPid(win);
    readClientRemote(win);
    getWMProtocols(win);
}

/**
 * Read MWM atoms.
 */
void
ClientHints::readMwm(Window win)
{
    auto mwm_hints = getMwmHints(win);
    if (mwm_hints == nullptr) {
        return;
    }

    if (mwm_hints->flags&MWM_HINTS_FUNCTIONS) {
#define ALLOW_MWM_FUN(flag) ((mwm_hints->functions&flag) ? state : ! state)
        bool state = ! (mwm_hints->functions&MWM_FUNC_ALL);

        _actions.resize = ALLOW_MWM_FUN(MWM_FUNC_RESIZE);
        _actions.move = ALLOW_MWM_FUN(MWM_FUNC_MOVE);
        _actions.iconify = ALLOW_MWM_FUN(MWM_FUNC_ICONIFY);
        _actions.close = ALLOW_MWM_FUN(MWM_FUNC_CLOSE);
        _actions.maximize_vert = ALLOW_MWM_FUN(MWM_FUNC_MAXIMIZE);
        _actions.maximize_horz = ALLOW_MWM_FUN(MWM_FUNC_MAXIMIZE);
#undef ALLOW_MWM_FUN
    }

    // Check decoration flags
    if (mwm_hints->flags & MWM_HINTS_DECORATIONS) {
        if (mwm_hints->decorations & MWM_DECOR_ALL) {
            _state.decor |= DECOR_TITLEBAR|DECOR_BORDER;
        } else {
            if (! (mwm_hints->decorations & MWM_DECOR_TITLE)) {
                _state.decor &= ~DECOR_TITLEBAR;
            }
            if (! (mwm_hints->decorations & MWM_DECOR_BORDER)) {
                _state.decor &= ~DECOR_BORDER;
            }
            // Do not handle HANDLE, MENU, ICONFIY or MAXIMIZE. Maybe
            // one should set the allowed actions for the client based
            // on this but that might be annoying so ignoring these.
        }
    }
    XFree(mwm_hints);
}

/**
 * Loads the Clients state from EWMH atoms.
 */
void
ClientHints::readEwmh(Window win)
{
    // which workspace do we belong to?
    long workspace = -1;
    X11::getLong(win, NET_WM_DESKTOP, workspace);
    if (workspace < 0) {
        _state.workspace = Workspaces::getActive();
        X11::setLong(win, NET_WM_DESKTOP, _state.workspace);
    } else {
        _state.workspace = workspace;
    }

    // The _NET_WM_STATE overrides the _NET_WM_TYPE
    NetWMStates win_states;
    if (getEwmhStates(win, win_states)) {
        if (win_states.hidden) _state.hidden = true;
        if (win_states.shaded) _state.shaded = true;
        if (win_states.max_vert) _state.maximized_vert = true;
        if (win_states.max_horz) _state.maximized_horz = true;
        if (win_states.skip_taskbar) _state.skip |= SKIP_TASKBAR;
        if (win_states.skip_pager) _state.skip |= SKIP_PAGER;
        if (win_states.sticky) _state.sticky = true;
        if (win_states.above) _layer = LAYER_ABOVE_DOCK;
        if (win_states.below) _layer = LAYER_BELOW;
        if (win_states.fullscreen) _state.fullscreen = true;
        _state.demands_attention = win_states.demands_attention;
    }
}

void
ClientHints::readEwmhStrut(Window win)
{
    int num = 0;
    long *strut = static_cast<long*>(X11::getEwmhPropData(win, NET_WM_STRUT,
                                                          XA_CARDINAL, num));
    if (strut) {
        _strut = strut;
        XFree(strut);
    }
}

/**
 * Read non-standard pekwm hints
 */
void
ClientHints::readPekwm(Window win)
{
    long value;
    std::string str;

    // Get decor state
    if (X11::getLong(win, PEKWM_FRAME_DECOR, value)) {
        _state.decor = value;
    }
    // Get skip state
    if (X11::getLong(win, PEKWM_FRAME_SKIP, value)) {
        _state.skip = value;
    }
    // Get custom title
    if (X11::getString(win, PEKWM_TITLE, str)) {
        _user_title = Util::to_wide_str(str);
    }

    _state.initial_frame_order = getPekwmFrameOrder(win);
}

/**
 * Read _NET_WM_PID.
 */
void
ClientHints::readClientPid(Window win)
{
    X11::getLong(win, NET_WM_PID, _pid);
}

/**
 * Read WM_CLIENT_MACHINE and check against local hostname and set
 * _is_remote if it does not match.
 */
void
ClientHints::readClientRemote(Window win)
{
    X11::getTextProperty(win, XA_WM_CLIENT_MACHINE, _client_machine);
}

/**
 * Read WM protocols.
 */
void
ClientHints::getWMProtocols(Window win)
{
    int count;
    Atom *protocols = 0;
    XGetWMProtocols(X11::getDpy(), win, &protocols, &count);
    if (! protocols) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        if (protocols[i] == X11::getAtom(WM_TAKE_FOCUS)) {
            _send_focus_message = true;
        } else if (protocols[i] == X11::getAtom(WM_DELETE_WINDOW)) {
            _send_close_message = true;
        }
    }

    XFree(protocols);
}

/**
 * Gets a Mwm structure from a window. Doesn't free memory.
 */
ClientHints::Mwm*
ClientHints::getMwmHints(Window win)
{
    Atom real_type;
    int real_format;
    ulong items_read, items_left;
    uchar *udata;
    Atom hints_atom = X11::getAtom(MOTIF_WM_HINTS);

    int status =
        XGetWindowProperty(X11::getDpy(), win, hints_atom, 0L, 20L, False,
                           hints_atom, &real_type, &real_format, &items_read,
                           &items_left, &udata);
    if (status == Success) {
        if (items_read >= MWM_HINTS_NUM) {
            return reinterpret_cast<ClientHints::Mwm*>(udata);
        }
        XFree(udata);
    }

    return nullptr;
}

bool
ClientHints::getEwmhStates(Window win, NetWMStates &win_states)
{
    int num = 0;
    auto states =
        static_cast<Atom*>(X11::getEwmhPropData(win, STATE, XA_ATOM, num));
    if (! states) {
        return false;
    }

    for (int i = 0; i < num; ++i) {
        if (states[i] == X11::getAtom(STATE_MODAL)) {
            win_states.modal = true;
        } else if (states[i] == X11::getAtom(STATE_STICKY)) {
            win_states.sticky = true;
        } else if (states[i] == X11::getAtom(STATE_MAXIMIZED_VERT)) {
            win_states.max_vert = true;
        } else if (states[i] == X11::getAtom(STATE_MAXIMIZED_HORZ)) {
            win_states.max_horz = true;
        } else if (states[i] == X11::getAtom(STATE_SHADED)) {
            win_states.shaded = true;
        } else if (states[i] == X11::getAtom(STATE_SKIP_TASKBAR)) {
            win_states.skip_taskbar = true;
        } else if (states[i] == X11::getAtom(STATE_SKIP_PAGER)) {
            win_states.skip_pager = true;
        } else if (states[i] == X11::getAtom(STATE_DEMANDS_ATTENTION)) {
            win_states.demands_attention = true;
        } else if (states[i] == X11::getAtom(STATE_HIDDEN)) {
            win_states.hidden = true;
        } else if (states[i] == X11::getAtom(STATE_FULLSCREEN)) {
            win_states.fullscreen = true;
        } else if (states[i] == X11::getAtom(STATE_ABOVE)) {
            win_states.above = true;
        } else if (states[i] == X11::getAtom(STATE_BELOW)) {
            win_states.below = true;
        }
    }

    XFree(states);

    return true;
}

/**
 * Get _PEKWM_FRAME_ORDER hint from client, return < 0 on failure.
 */
long
ClientHints::getPekwmFrameOrder(Window win)
{
    long num = -1;
    X11::getLong(win, PEKWM_FRAME_ORDER, num);
    return num;
}
