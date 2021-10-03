#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#define MAXSTR 1000

unsigned long getActiveWindow(Window root);
void getWindowClass(Window w, char* class_ret);
unsigned int getKeyboardLayout();
unsigned long getLongProperty(Window w, const char* propname);
void getStringProperty(Window w, const char* propname);
void checkStatus(int status, Window w);

typedef struct {
    Window w;
    char class[MAXSTR];
    unsigned int layout;
    unsigned int isfree;
} Record;

Display* d;
GHashTable* t;
unsigned char* prop;
static Record r[256];

int main()
{
    XEvent ev;
    Window w, root;
    XkbEvent* xev;
    char class[MAXSTR] = {0};
    unsigned int layout, layout_prev, layout_main;
    Record db[MAXSTR];

    if (!(d = XOpenDisplay(NULL))) {
        fprintf(stderr, "Cannot Open Display.\n");
        return 1;
    }

    // Subscribe to window events
    root = DefaultRootWindow(d);
    XSelectInput(d, root, PropertyChangeMask);

    // Subscribe to keyboard layout events
    int xkbEventType;
    XKeysymToKeycode(d, XK_F1);
    XkbQueryExtension(d, 0, &xkbEventType, 0, 0, 0);
    XkbSelectEvents(d, XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);
    XSync(d, False);

    // Create HashTable
    t = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);

    layout = layout_prev = layout_main = getKeyboardLayout();
    /* Change Layout ===> XkbLockGroup(d, XkbUseCoreKbd, 1); */
    for (;;) {

        XNextEvent(d, &e);

        xev = (XkbEvent*)&e;
        if (e.type == xkbEventType && xev->any.xkb_type == XkbStateNotify) {
            layout = getKeyboardLayout();
            if (layout == layout_prev)
                continue;
            layout_prev = layout;
            if (layout == layout_main) {
            } else {

            }
            w = getActiveWindow(root);
        }

        else if ((e.type == PropertyNotify && !strcmp(XGetAtomName(d, e.xproperty.atom), "_NET_ACTIVE_WINDOW")) ||
            (e.type == xkbEventType && xev->any.xkb_type == XkbStateNotify)) {
            if (e.type == xkbEventType) {
                layout = getKeyboardLayout();
                if (layout == layout_prev)
                    continue;
                layout_prev = layout;
            }
            w = getActiveWindow(root);
            getWindowClass(w, class);
            printf("%lu\t%s\t%u\n", w, class, getKeyboardLayout());
        }
    }

    XFree(prop);
    XCloseDisplay(d);
    return 0;
}

void record(Window w, const char* class, unsigned int layout) {
    unsigned long* key = NULL;
    key = malloc(sizeof(*key));
    *key = (unsigned long)w;
    g_hash_table_insert(t, key, (gpointer)1);
}

unsigned long fetch(Window w, const char* class) {
    return g_hash_table_lookup(t, (gconstpointer)&w);
}

unsigned long getActiveWindow(Window root)
{
    unsigned long w;
    if (( w = getLongProperty(root, "_NET_ACTIVE_WINDOW") ))
        return w;
    else
        return 0;
}

void getWindowClass(Window w, char* class_ret)
{
    if (!w) {
        class_ret[0] = '\0';
        return;
    }
    XClassHint ch;
    XGetClassHint(d, w, &ch);
    strcpy(class_ret, ch.res_class);
}

int getKeyboardLayout()
{
    XkbStateRec state;
    XkbGetState(d, XkbUseCoreKbd, &state);
    return (int)state.group;
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
