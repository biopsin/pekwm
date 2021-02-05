/**
 * Test application for _NET_REQUEST_FRAME_EXTENTS (and _NET_FRAME_EXTENTS)
 *
 * https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html#idm45075509148864
 *
 * _NET_REQUEST_FRAME_EXTENTS
 *   window = window for which to set _NET_FRAME_EXTENTS
 *   message_type = _NET_REQUEST_FRAME_EXTENTS
 *
 * A Client whose window has not yet been mapped can request of the
 * Window Manager an estimate of the frame extents it will be given
 * upon mapping. To retrieve such an estimate, the Client MUST send a
 * _NET_REQUEST_FRAME_EXTENTS message to the root window. The Window
 * Manager MUST respond by estimating the prospective frame extents
 * and setting the window's _NET_FRAME_EXTENTS property
 * accordingly. The Client MUST handle the resulting
 * _NET_FRAME_EXTENTS PropertyNotify event. So that the Window Manager
 * has a good basis for estimation, the Client MUST set any window
 * properties it intends to set before sending this message. The
 * Client MUST be able to cope with imperfect estimates.
 *
 * Rationale: A client cannot calculate the dimensions of its window's
 * frame before the window is mapped, but some toolkits need this
 * information. Asking the window manager for an estimate of the
 * extents is a workable solution. The estimate may depend on the
 * current theme, font sizes or other window properties. The client
 * can track changes to the frame's dimensions by listening for
 * _NET_FRAME_EXTENTS PropertyNotify events.
 */

#include <iostream>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
}

int
main(int argc, char *argv[])
{
    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cerr << "ERROR: unable to open display" << std::endl;
        return 1;
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    Colormap colormap = DefaultColormap(dpy, screen);

    XColor blue;
    XAllocNamedColor(dpy, colormap, "blue", &blue, &blue);

    XSetWindowAttributes attrs = {0};
    attrs.event_mask = PropertyChangeMask;
    attrs.background_pixel = blue.pixel;
    unsigned long attrs_mask = CWEventMask|CWBackPixel;

    Window win = XCreateWindow(dpy, root,
                               0, 0, 100, 100, 0,
                               CopyFromParent, //depth
                               InputOutput, // class
                               CopyFromParent, // visual
                               attrs_mask,
                               &attrs);

    Atom request_frame_extents =
        XInternAtom(dpy, "_NET_REQUEST_FRAME_EXTENTS", False);

    // request extents on window to be mapped
    XEvent ev = { ClientMessage };
    ev.xclient.window = win;
    ev.xclient.format = 32;
    ev.xclient.message_type = request_frame_extents;
    std::cout << "PROGRESS: send _NET_REQUEST_FRAME_EXTENTS" << std::endl;
    long mask = SubstructureNotifyMask | SubstructureRedirectMask;
    XSendEvent(dpy, root, False, mask, &ev);

    // wait for property notify on window, then read the _NET_FRAME_EXTENTS
    std::cout << "PROGRESS: wait for PropertyNotify" << std::endl;
    XNextEvent(dpy, &ev);
    if (ev.type != PropertyNotify) {
        std::cerr << "ERROR: expected a PropertyNotify, got " << ev.type
                  << std::endl;
        exit(1);
    }

    Atom frame_extents = XInternAtom(dpy, "_NET_FRAME_EXTENTS", False);

    Atom type_ret;
    int format_ret;
    ulong items_ret, after_ret;
    unsigned char *data = 0;
    std::cout << "PROGRESS: read _NET_FRAME_EXTENTS" << std::endl;
    XGetWindowProperty(dpy, win, frame_extents,
                       0L, 4,
                       False, // delete
                       XA_CARDINAL,
                       &type_ret, &format_ret,
                       &items_ret, &after_ret,
                       &data);

    std::cout << "PROGRESS: "
              << "type_ret " << type_ret <<  " "
              << "format_ret " << format_ret << " "
              << "items_ret " << items_ret << " "
              << "after_ret " << after_ret << std::endl;
    if (items_ret == 4) {
        long *extents = reinterpret_cast<long*>(data);
        std::cout << "OK: extents "
                  << "left " << extents[0] << " "
                  << "right " << extents[1] << " "
                  << "top " << extents[2] << " "
                  << "bottom " << extents[3] << std::endl;
    } else {
        std::cerr << "ERROR: expected 4 items returned, got " << items_ret
                  << std::endl;
    }

    XDestroyWindow(dpy, win);
    XFreeColors(dpy, colormap, &blue.pixel, 1, 0);
    XCloseDisplay(dpy);

    return items_ret != 4;
}
