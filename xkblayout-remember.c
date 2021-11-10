#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#define MAXSTR 1000

static unsigned long getActiveWindowUID();
static unsigned long getActiveWindow();
static unsigned long getWindowPID(Window w);
static unsigned int getKeyboardLayout();
static int is_xkb_event(XEvent ev);
static int is_focus_event(XEvent ev);
static void recordLayout(Window w, unsigned int layout);
static unsigned int fetchLayout(Window w);
static void init_xkb();
static void init_xfocusev();
static void init_hashtable();
static void destroy_hashtable();
static unsigned long getLongProperty(Window w, const char* propname);
static void getStringProperty(Window w, const char* propname);
static void checkStatus(int status, Window w);

static Display* d;
static Window root;
static GHashTable* t;
static unsigned char* prop;
static int xkbEventType;

int main()
{
    unsigned int layout, layout_prev;

    if (!(d = XOpenDisplay(NULL))) {
        fprintf(stderr, "Cannot Open Display.\n");
        return 1;
    }

    init_xkb();
    init_xfocusev();
    init_hashtable();

    layout = layout_prev = getKeyboardLayout();
    recordLayout(getActiveWindowUID(), layout);

    while(1) {
        XEvent ev;
        XNextEvent(d, &ev);
        if (is_xkb_event(ev)) {
            layout = getKeyboardLayout();
            if (layout == layout_prev)
                continue;
            layout_prev = layout;
            recordLayout(getActiveWindowUID(), getKeyboardLayout());
        }
        else if (is_focus_event(ev))
            XkbLockGroup(d, XkbUseCoreKbd, fetchLayout(getActiveWindowUID()));
    }

    destroy_hashtable();
    XFree(prop);
    XCloseDisplay(d);
    return 0;
}

unsigned long getActiveWindowUID()
{
    unsigned long w = getActiveWindow();
    return w + getWindowPID(w);
}

unsigned long getActiveWindow()
{
    unsigned long w;
    if (w = getLongProperty(root, "_NET_ACTIVE_WINDOW"))
        return w;
    else
        return 0;
}

unsigned long getWindowPID(Window w)
{
    if (!w)
        return 0;
    return getLongProperty(w, "_NET_WM_PID");
}

unsigned int getKeyboardLayout()
{
    XkbStateRec state;
    XkbGetState(d, XkbUseCoreKbd, &state);
    return (unsigned int)state.group;
}

unsigned long getLongProperty(Window w, const char* propname)
{
    if (!w)
        return 0;
    getStringProperty(w, propname);
    if (!prop)
        return 0;
    return (unsigned long)(prop[0] + (prop[1] << 8) + (prop[2] << 16) + (prop[3] << 24));
}

void getStringProperty(Window w, const char* propname)
{
    Atom actual_type, filter_atom;
    int actual_format, status;
    unsigned long nitems, bytes_after;

    filter_atom = XInternAtom(d, propname, True);
    status = XGetWindowProperty(d, w, filter_atom, 0, MAXSTR, False, AnyPropertyType,
                                &actual_type, &actual_format, &nitems, &bytes_after, &prop);
    checkStatus(status, w);
}

void checkStatus(int status, Window w)
{
    if (status == BadWindow) {
        fprintf(stderr, "Window ID #0x%lx does not exist.", w);
    }
    if (status != Success) {
        fprintf(stderr, "XGetWindowProperty failed.");
    }
}

/* setup keyboard layout events */
void init_xkb()
{
    XKeysymToKeycode(d, XK_F1);
    XkbQueryExtension(d, 0, &xkbEventType, 0, 0, 0);
    XkbSelectEvents(d, XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);
    XSync(d, False);
}

/* setup window focus change events */
void init_xfocusev()
{
    root = DefaultRootWindow(d);
    XSelectInput(d, root, PropertyChangeMask);
}

int is_xkb_event(XEvent ev)
{
    XkbEvent* xev = (XkbEvent*)&ev;
    return (ev.type == xkbEventType && xev->any.xkb_type == XkbStateNotify);
}

int is_focus_event(XEvent ev)
{
    return (ev.type == PropertyNotify && !strcmp(XGetAtomName(d, ev.xproperty.atom), "_NET_ACTIVE_WINDOW"));
}

/* create the hashtable */
void init_hashtable()
{
    t = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, NULL);
}

void destroy_hashtable()
{
    g_hash_table_destroy(t);
}

void recordLayout(Window w, unsigned int layout)
{
    unsigned long* key = NULL;
    key = malloc(sizeof(*key));
    *key = (unsigned long)w;
    g_hash_table_insert(t, key, GINT_TO_POINTER((int)layout));
}

unsigned int fetchLayout(Window w)
{
    return (unsigned int)GPOINTER_TO_INT(g_hash_table_lookup(t, &w));
}
