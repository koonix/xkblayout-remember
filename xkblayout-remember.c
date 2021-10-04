#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#define MAXSTR 1000

unsigned long getActiveWindow();
/* void getWindowClass(Window w, char* class_ret); */
unsigned int getKeyboardLayout();
unsigned long getLongProperty(Window w, const char* propname);
void getStringProperty(Window w, const char* propname);
void checkStatus(int status, Window w);
int is_xkb_event(XEvent ev);
int is_focus_event(XEvent ev);
void record(Window w, unsigned int layout);
unsigned int fetch(Window w);
void subscribe_xkb();
void subscribe_focus();

Display* d;
Window root;
GHashTable* t;
unsigned char* prop;
int xkbEventType;

int main()
{
    /* char class[MAXSTR] = {0}; */
    unsigned int layout, layout_prev, layout_main;

    if (!(d = XOpenDisplay(NULL))) {
        fprintf(stderr, "Cannot Open Display.\n");
        return 1;
    }

    subscribe_xkb();
    subscribe_focus();

    /* create the hashtable */
    t = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, NULL);

    layout = layout_prev = layout_main = getKeyboardLayout();
    record(getActiveWindow(), layout_main);

    /* Change Layout ===> XkbLockGroup(d, XkbUseCoreKbd, 1); */

    for (;;) {

        XEvent ev;
        XNextEvent(d, &ev);

        if (is_xkb_event(ev)) {
            layout = getKeyboardLayout();
            if (layout == layout_prev)
                continue;
            layout_prev = layout;
            record(getActiveWindow(), getKeyboardLayout());
        }
        else if (is_focus_event(ev)) {
            XkbLockGroup(d, XkbUseCoreKbd, fetch(getActiveWindow()));
        }
    }

    XFree(prop);
    XCloseDisplay(d);
    return 0;
}

/* void record(Window w, const char* class, unsigned int layout) { */
void record(Window w, unsigned int layout) {
    unsigned long* key = NULL;
    key = malloc(sizeof(*key));
    *key = (unsigned long)w;
    g_hash_table_insert(t, key, GINT_TO_POINTER((int)layout));
}

unsigned int fetch(Window w) {
    return (unsigned int)GPOINTER_TO_INT(g_hash_table_lookup(t, &w));
}

unsigned long getActiveWindow()
{
    unsigned long w;
    if (( w = getLongProperty(root, "_NET_ACTIVE_WINDOW") ))
        return w;
    else
        return 0;
}

/* void getWindowClass(Window w, char* class_ret) */
/* { */
/*     if (!w) { */
/*         class_ret[0] = '\0'; */
/*         return; */
/*     } */
/*     XClassHint ch; */
/*     XGetClassHint(d, w, &ch); */
/*     strcpy(class_ret, ch.res_class); */
/* } */

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

// Subscribe to keyboard layout events
void subscribe_xkb()
{
    XKeysymToKeycode(d, XK_F1);
    XkbQueryExtension(d, 0, &xkbEventType, 0, 0, 0);
    XkbSelectEvents(d, XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);
    XSync(d, False);
}

// Subscribe to window events
void subscribe_focus()
{
    root = DefaultRootWindow(d);
    XSelectInput(d, root, PropertyChangeMask);
}

int is_xkb_event(XEvent ev) {
    XkbEvent* xev = (XkbEvent*)&ev;
    return (ev.type == xkbEventType && xev->any.xkb_type == XkbStateNotify);
}

int is_focus_event(XEvent ev) {
    return (ev.type == PropertyNotify && !strcmp(XGetAtomName(d, ev.xproperty.atom), "_NET_ACTIVE_WINDOW"));
}
