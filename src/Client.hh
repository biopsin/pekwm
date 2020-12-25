//
// Client.hh for pekwm
// Copyright (C) 2003-2020 Claes Nästén <pekdon@gmail.com>
//
// client.hh for aewm++
// Copyright (C) 2002 Frank Hale <frankhale@yahoo.com>
// http://sapphire.sourceforge.net/
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#pragma once

#include "config.h"

#include "pekwm.hh"
#include "ClientHints.hh"
#include "PWinObj.hh"
#include "Observer.hh"
#include "PTexturePlain.hh"
#include "PDecor.hh"

class PScreen;
class Strut;
class ClassHint;
class AutoProperty;
class Frame;

#include <string>

extern "C" {
#include <X11/Xutil.h>
}

class LayerObservation : public Observation {
public:
    LayerObservation(enum Layer _layer) : layer(_layer) { };
    virtual ~LayerObservation(void) { };
public:
    const enum Layer layer; /**< Layer client changed to. */
};

class ClientInitConfig {
public:
    bool focus;
    bool focus_parent;
    bool map;
    bool parent_is_new;
};

class Client : public ClientHints, public PWinObj, public Observer
{
    // FIXME: This relationship should end as soon as possible, but I need to
    // figure out a good way of sharing. :)
    friend class Frame;

public: // Public Member Functions
    Client(Window new_client, ClientInitConfig &initConfig,
           bool is_new = false);
    virtual ~Client(void);

    // START - PWinObj interface.
    virtual void mapWindow(void) override;
    virtual void unmapWindow(void) override;
    virtual void iconify(void) override;
    virtual void stick(void) override;

    virtual void move(int x, int y) override;
    virtual void resize(uint width, uint height) override;
    virtual void moveResize(int x, int y, uint width, uint height) override;

    virtual void setWorkspace(uint workspace) override;

    virtual void giveInputFocus(void) override;
    virtual void reparent(PWinObj *parent, int x, int y) override;

    virtual ActionEvent *handleButtonPress(XButtonEvent *ev)  override {
        if (_parent) { return _parent->handleButtonPress(ev); }
        return 0;
    }
    virtual ActionEvent *handleButtonRelease(XButtonEvent *ev) override {
        if (_parent) { return _parent->handleButtonRelease(ev); }
        return 0;
    }

    virtual ActionEvent *handleMapRequest(XMapRequestEvent *ev) override;
    virtual ActionEvent *handleUnmapEvent(XUnmapEvent *ev) override;
    // END - PWinObj interface.

#ifdef HAVE_SHAPE
    void handleShapeEvent(XShapeEvent *);
#endif // HAVE_SHAPE

    // START - Observer interface.
    virtual void notify(Observable *observable,
                        Observation *observation) override;
    // END - Observer interface.

    static Client *findClient(Window win);
    static Client *findClientFromWindow(Window win);
    static Client *findClientFromHint(const ClassHint *class_hint);
    static Client *findClientFromID(uint id);
    static void findFamilyFromWindow(std::vector<Client*> &client_list,
                                     Window win);

    static void mapOrUnmapTransients(Window win, bool hide);

    // START - Iterators
    static uint client_size(void) { return _clients.size(); }
    static std::vector<Client*>::const_iterator client_begin(void) {
        return _clients.begin();
    }
    static std::vector<Client*>::const_iterator client_end(void) {
        return _clients.end();
    }
    static std::vector<Client*>::const_reverse_iterator client_rbegin(void) {
        return _clients.rbegin();
    }
    static std::vector<Client*>::const_reverse_iterator client_rend(void) {
        return _clients.rend();
    }

    bool hasTransients() const { return ! _transients.empty(); }
    std::vector<Client*>::const_iterator getTransientsBegin(void) const {
        return _transients.begin();
    }
    std::vector<Client*>::const_iterator getTransientsEnd(void) const {
        return _transients.end();
    }
    // END - Iterators

    bool validate(void);

    inline uint getClientID(void) { return _id; }
    /**< Return title item for client name. */
    inline PDecor::TitleItem *getTitle(void) { return &_title; }

    inline const ClassHint* getClassHint(void) const { return _class_hint; }

    Client *getTransientForClient(void) const { return _transient_for; }
    void findAndRaiseIfTransient(void);

    inline XSizeHints* getXSizeHints(void) const { return _size; }

    bool isViewable(void);

    PTexture *getIcon(void) const { return _icon; }

    /** Return true if client is remote. */
    bool isRemote(void) const { return _is_remote; }

    // State accessors
    bool isMaximizedVert(void) const { return _state.maximized_vert; }
    bool isMaximizedHorz(void) const { return _state.maximized_horz; }
    bool isShaded(void) const { return _state.shaded; }
    bool isFullscreen(void) const { return _state.fullscreen; }
    bool isPlaced(void) const { return _state.placed; }
    uint getInitialFrameOrder(void) const { return _state.initial_frame_order; }
    uint getSkip(void) const { return _state.skip; }
    bool isSkip(Skip skip) const { return (_state.skip&skip); }
    uint getDecorState(void) const { return _state.decor; }
    bool isCfgDeny(uint deny) { return (_state.cfg_deny&deny); }

    bool allowMove(void) const { return _actions.move; }
    bool allowResize(void) const { return _actions.resize; }
    bool allowIconify(void) const { return _actions.iconify; }
    bool allowShade(void) const { return _actions.shade; }
    bool allowStick(void) const { return _actions.stick; }
    bool allowMaximizeHorz(void) const { return _actions.maximize_horz; }
    bool allowMaximizeVert(void) const { return _actions.maximize_vert; }
    bool allowFullscreen(void) const { return _actions.fullscreen; }
    bool allowChangeWorkspace(void) const { return _actions.change_ws; }
    bool allowClose(void) const { return _actions.close; }

    bool isAlive(void) const { return _alive; }
    bool isMarked(void) const { return _marked; }

    // We have this public so that we can reload button actions.
    void grabButtons(void);

    void setStateCfgDeny(StateAction sa, uint deny);
    inline void setStateMarked(StateAction sa) {
        if (ActionUtil::needToggle(sa, _marked)) {
            _marked = !_marked;
            if (_marked) {
                _title.infoAdd(PDecor::TitleItem::INFO_MARKED);
            } else {
                _title.infoRemove(PDecor::TitleItem::INFO_MARKED);
            }
            _title.updateVisible();
        }
    }

    // toggles
    void alwaysOnTop(bool top);
    void alwaysBelow(bool bottom);

    void setSkip(uint skip);

    inline void setStateSkip(StateAction sa, Skip skip) {
        if ((isSkip(skip) && (sa == STATE_SET))
            || (! isSkip(skip) && (sa == STATE_UNSET))) {
            return;
        }
        _state.skip ^= skip;
    }

    inline void setTitlebar(bool titlebar) {
        if (titlebar) {
            _state.decor |= DECOR_TITLEBAR;
        } else {
            _state.decor &= ~DECOR_TITLEBAR;
        }
    }

    inline void setBorder(bool border) {
        if (border) {
            _state.decor |= DECOR_BORDER;
        } else {
            _state.decor &= ~DECOR_BORDER;
        }
    }

    void setDemandsAttention(bool attention) {
        _state.demands_attention = attention;
    }

    std::string getAPDecorName(void);

    void close(void);
    void kill(void);

    void handleDestroyEvent(XDestroyWindowEvent *ev);
    void handleColormapChange(XColormapEvent *ev);

    inline bool setConfigureRequestLock(bool lock) {
        bool old_lock = _cfg_request_lock;
        _cfg_request_lock = lock;
        return old_lock;
    }

    void configureRequestSend(void);
    void sendTakeFocusMessage(void);

    bool getAspectSize(uint *r_w, uint *r_h, uint w, uint h);
    bool getIncSize(uint *r_w, uint *r_h, uint w, uint h, bool incr=false);

    void updateEwmhStates(void);

    inline AtomName getWinType(void) const { return _window_type; }
    void updateWinType(bool set);

    ulong getWMHints(void);
    void getWMNormalHints(void);
    void getTransientForHint(void);
    void updateParentLayerAndRaiseIfActive(void);
    void readName(void);
    void setStrutHint(Strut* strut);
    void removeStrutHint(Strut* strut);

    long getPekwmFrameOrder(void);

    void setPekwmFrameOrder(long num);
    bool getPekwmFrameActive(void);
    void setPekwmFrameActive(bool active);

    void setFrameExtents(long left, long right, long top, long bottom);
    static void setClientEnvironment(Client *client);
    AutoProperty* readAutoprops(ApplyOn type = APPLY_ON_ALWAYS);

private:
    bool getAndUpdateWindowAttributes(void);

    bool findOrCreateFrame(AutoProperty *autoproperty);
    bool findTaggedFrame(void);
    bool findPreviousFrame(void);
    bool findAutoGroupFrame(AutoProperty *autoproperty);

    void setInitialState(void);
    void setClientInitConfig(ClientInitConfig &initConfig, bool is_new,
                             AutoProperty *autoproperty);

    bool titleApplyRule(std::wstring &wtitle);
    uint titleFindID(std::wstring &wtitle);

    void setWmState(ulong state);
    long getWmState(void);

    // these are used by frame
    inline void setMaximizedVert(bool m) { _state.maximized_vert = m; }
    inline void setMaximizedHorz(bool m) { _state.maximized_horz = m; }
    inline void setShade(bool s) { _state.shaded = s; }
    inline void setFullscreen(bool f) { _state.fullscreen = f; }

    // Grabs button with Caps,Num and so on
    void grabButton(int button, int mod, int mask, Window win);

    void readClassRoleHints(void);
    void readIcon(void);
    void applyAutoprops(AutoProperty *ap);
    void applyActionAccessMask(uint mask, bool value);

    static uint findClientID(void);
    static void returnClientID(uint id);

private: // Private Member Variables
    uint _id; //<! Unique ID of the Client.

    Colormap _cmap;

    /** Client for which this client is transient for */
    Client *_transient_for;
    std::vector<Client*> _transients; /**< Vector of transient clients. */

    PDecor::TitleItem _title; /**< Name of the client. */
    PTextureImage *_icon;
    bool _is_remote; /**< Boolean flag  */

    ClassHint *_class_hint;
    AtomName _window_type; /**< _NET_WM_WINDOW_TYPE */

    bool _alive, _marked;
    bool _wm_hints_input;
    bool _cfg_request_lock;
    bool _extended_net_name;

    static const long _clientEventMask;

    static std::vector<Client*> _clients; //!< Vector of all Clients.
    static std::vector<uint> _clientids; //!< Vector of free Client IDs.
};
