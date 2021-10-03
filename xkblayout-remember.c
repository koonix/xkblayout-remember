#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>
#define MAXSTR 1000

unsigned long getActiveWindow(Window root);
void getWindowClass(Window w, char* class_ret);
int getKeyboardLayout();
unsigned long getLongProperty(Window w, const char* propname);
void getStringProperty(Window w, const char* propname);
void checkStatus(int status, Window w);

Display* d;
unsigned char* prop;

int main()
{
    XEvent e;
    Window w, root;
    XkbEvent* xev;
    char class[MAXSTR] = {0};
    int layout, layout_old, layout_main;

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

    layout = layout_old = layout_main = getKeyboardLayout();
    for (;;) {
        XNextEvent(d, &e);
        xev = (XkbEvent*)&e;
        if ((e.type == PropertyNotify && !strcmp(XGetAtomName(d, e.xproperty.atom), "_NET_ACTIVE_WINDOW")) ||
            (e.type == xkbEventType && xev->any.xkb_type == XkbStateNotify)) {
            if (e.type == xkbEventType) {
                layout = getKeyboardLayout();
                if (layout == layout_old)
                    continue;
                layout_old = layout;
            }
            w = getActiveWindow(root);
            getWindowClass(w, class);
            if (!class)
                strcpy(class, "NULL");
            printf("%lu\t%s\t%d\n", w, class, getKeyboardLayout());
        }
    }

    return 0;
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
        class_ret = NULL;
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
