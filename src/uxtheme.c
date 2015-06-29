/*
 * This file is part of UxThemeGTK.
 *
 * Copyright © 2015 Ivan Akulinchev
 *
 * UxThemeGTK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * UxThemeGTK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with UxThemeGTK.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <wchar.h>

#include <windef.h>
#include <winbase.h>
#include <wingdi.h>
#include <winuser.h>
#include <winerror.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>

#include <wine/debug.h>

#include <gtk/gtk.h>

#include "button.h"
#include "combobox.h"
#include "edit.h"
#include "header.h"
#include "listbox.h"
#include "listview.h"
#include "menu.h"
#include "rebar.h"
#include "status.h"
#include "tab.h"
#include "toolbar.h"
#include "trackbar.h"
#include "window.h"

WINE_DEFAULT_DEBUG_CHANNEL(uxthemegtk);

typedef void (*init_func_t)(GdkScreen *screen);
typedef void (*uninit_func_t)(void);
typedef HRESULT (*get_color_func_t)(int part_id, int state_id,
                                    int prop_id, GdkRGBA *rgba);
typedef void (*draw_background_func_t)(cairo_t *cr,
                                       int part_id, int state_id,
                                       int width, int height);
typedef HRESULT (*get_part_size_func_t)(int part_id, int state_id,
                                        RECT *rect, SIZE *size);
typedef BOOL (*is_part_defined_func_t)(int part_id, int state_id);

typedef struct {
    const wchar_t *classname;
    init_func_t init;
    uninit_func_t uninit;
    get_color_func_t get_color;
    draw_background_func_t draw_background;
    get_part_size_func_t get_part_size;
    is_part_defined_func_t is_part_defined;
} theme_t;

static /* const */ theme_t themes[] = {
    { VSCLASS_BUTTON,
      uxgtk_button_init,
      uxgtk_button_uninit,
      uxgtk_button_get_color,
      uxgtk_button_draw_background,
      uxgtk_button_get_part_size,
      uxgtk_button_is_part_defined },
    { VSCLASS_COMBOBOX,
      uxgtk_combobox_init,
      uxgtk_combobox_uninit,
      NULL, /* get_color */
      uxgtk_combobox_draw_background,
      NULL, /* get_part_size */
      uxgtk_combobox_is_part_defined },
    { VSCLASS_EDIT,
      uxgtk_edit_init,
      uxgtk_edit_uninit,
      uxgtk_edit_get_color,
      uxgtk_edit_draw_background,
      NULL, /* get_part_size */
      uxgtk_edit_is_part_defined },
    { VSCLASS_HEADER,
      uxgtk_header_init,
      uxgtk_header_uninit,
      NULL, /* get_color */
      uxgtk_header_draw_background,
      NULL, /* get_part_size */
      uxgtk_header_is_part_defined },
    { VSCLASS_LISTBOX,
      uxgtk_listbox_init,
      uxgtk_listbox_uninit,
      NULL, /* get_color */
      uxgtk_listbox_draw_background,
      NULL, /* get_part_size */
      uxgtk_listbox_is_part_defined },
    { VSCLASS_LISTVIEW,
      uxgtk_listview_init,
      uxgtk_listview_uninit,
      NULL, /* get_color */
      uxgtk_listview_draw_background,
      NULL, /* get_part_size */
      uxgtk_listview_is_part_defined },
    { VSCLASS_MENU,
      uxgtk_menu_init,
      uxgtk_menu_uninit,
      uxgtk_menu_get_color,
      NULL, /* draw_background */
      NULL, /* get_part_size */
      NULL }, /* is_part_defined */
    { VSCLASS_REBAR,
      uxgtk_rebar_init,
      uxgtk_rebar_uninit,
      NULL, /* get_color */
      uxgtk_rebar_draw_background,
      NULL, /* get_part_size */
      uxgtk_rebar_is_part_defined },
    { VSCLASS_STATUS,
      uxgtk_status_init,
      uxgtk_status_uninit,
      NULL, /* get_color */
      uxgtk_status_draw_background,
      uxgtk_status_get_part_size,
      uxgtk_status_is_part_defined },
    { VSCLASS_TAB,
      uxgtk_tab_init,
      uxgtk_tab_uninit,
      NULL, /* get_color */
      uxgtk_tab_draw_background,
      NULL, /* get_part_size */
      uxgtk_tab_is_part_defined },
    { VSCLASS_TOOLBAR,
      uxgtk_toolbar_init,
      uxgtk_toolbar_uninit,
      NULL, /* get_color */
      uxgtk_toolbar_draw_background,
      NULL, /* get_part_size */
      uxgtk_toolbar_is_part_defined },
    { VSCLASS_TRACKBAR,
      uxgtk_trackbar_init,
      uxgtk_trackbar_uninit,
      NULL, /* get_color */
      uxgtk_trackbar_draw_background,
      NULL, /* get_part_size */
      uxgtk_trackbar_is_part_defined },
    { VSCLASS_WINDOW,
      uxgtk_window_init,
      uxgtk_window_uninit,
      uxgtk_window_get_color,
      uxgtk_window_draw_background,
      NULL, /* get_part_size */
      uxgtk_window_is_part_defined }
};

#define THEMES_SIZE (sizeof(themes) / sizeof(theme_t))

#define GET_THEME(htheme) ((theme_t *)(htheme))

#define NUM_SYS_COLORS (COLOR_MENUBAR + 1)
#define MENU_HEIGHT 20
#define CLASSLIST_MAXLEN 128
#define PIXEL_SIZE 4 /* red + green + blue + alpha = 4 bytes */
#define THEME_PROPERTY L"uxgtk_theme"

#define GDKRGBA_TO_COLORREF(rgba) RGB( \
    (int)(0.5 + CLAMP(rgba.red, 0.0, 1.0) * 255.0), \
    (int)(0.5 + CLAMP(rgba.green, 0.0, 1.0) * 255.0), \
    (int)(0.5 + CLAMP(rgba.blue, 0.0, 1.0) * 255.0))

static void apply_colors(void)
{
    int i, colors[NUM_SYS_COLORS];
    COLORREF refs[NUM_SYS_COLORS];

    for (i = 0; i < NUM_SYS_COLORS; i++) {
        refs[i] = GetThemeSysColor(NULL, i);
        colors[i] = i;
    }

    SetSysColors(NUM_SYS_COLORS, colors, refs);
}

static void fix_sys_params(void)
{
    NONCLIENTMETRICSW metrics;

    memset(&metrics, 0, sizeof(metrics));
    metrics.cbSize = sizeof(metrics);

    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,
                          sizeof(metrics), &metrics, 0);

    metrics.iMenuHeight = MENU_HEIGHT;

    SystemParametersInfoW(SPI_SETNONCLIENTMETRICS,
                          sizeof(metrics), &metrics, 0);

    SystemParametersInfo(SPI_SETCLEARTYPE, 0, (LPVOID)TRUE, 0);
    SystemParametersInfo(SPI_SETFONTSMOOTHING, 0, (LPVOID)TRUE, 0);
    SystemParametersInfo(SPI_SETFLATMENU, 0, (LPVOID)TRUE, 0);
}

static void init(void)
{
    int i;
    GdkScreen *screen;

    gtk_init(0, NULL); /* Otherwise every call to GTK will fail */

    screen = gdk_screen_get_default();

    for (i = 0; i < THEMES_SIZE; i++)
        themes[i].init(screen);

    apply_colors();
    fix_sys_params();
}

static void uninit(void)
{
    int i;

    for (i = 0; i < THEMES_SIZE; i++)
        themes[i].uninit();
}

static void paint_cairo_surface(cairo_surface_t *surface, HDC target_hdc,
                                int x, int y, int width, int height)
{
    int i;
    HDC bitmap_hdc;
    HBITMAP bitmap;
    BITMAPINFO info;
    BLENDFUNCTION bf;
    unsigned char *bitmap_data, *surface_data;

    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height; /* top-down, see MSDN */
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB; /* no compression */
    info.bmiHeader.biSizeImage = 0;
    info.bmiHeader.biXPelsPerMeter = 1;
    info.bmiHeader.biYPelsPerMeter = 1;
    info.bmiHeader.biClrUsed = 0;
    info.bmiHeader.biClrImportant = 0;

    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 0xff;
    bf.AlphaFormat = AC_SRC_ALPHA;

    bitmap_hdc = CreateCompatibleDC(target_hdc);
    bitmap = CreateDIBSection(bitmap_hdc, &info, DIB_RGB_COLORS,
                              (void **)&bitmap_data, NULL, 0);

    cairo_surface_flush(surface);

    surface_data = cairo_image_surface_get_data(surface);

    for (i = 0; i < width * height * PIXEL_SIZE; i++)
        bitmap_data[i] = surface_data[i];

    SelectObject(bitmap_hdc, bitmap);

    GdiAlphaBlend(target_hdc, x, y, width, height,
                  bitmap_hdc, 0, 0, width, height, bf);

    DeleteObject(bitmap);
    DeleteDC(bitmap_hdc);
}

static BOOL match_class(LPCWSTR classlist, LPCWSTR classname)
{
    const wchar_t *seps = L";";
    wchar_t *last, *tok, buf[CLASSLIST_MAXLEN];

    if (classlist == NULL || classname == NULL)
        return FALSE;

    lstrcpynW(buf, classlist, CLASSLIST_MAXLEN - 1);

    /* Ensure we have a null character at the end of buffer */
    buf[CLASSLIST_MAXLEN - 1] = L'\0';

    for (tok = wcstok(buf, seps, &last); tok != NULL;
         tok = wcstok(NULL, seps, &last))
        if (lstrcmpiW(tok, classname) == 0)
            return TRUE;

    return FALSE;
}

HRESULT WINAPI CloseThemeData(HTHEME htheme)
{
    return S_OK; /* Do nothing */
}

HRESULT WINAPI EnableThemeDialogTexture(HWND hwnd, DWORD flags)
{
    if (flags & ETDT_USETABTEXTURE)
        OpenThemeData(hwnd, VSCLASS_TAB); /* No CloseThemeData is needed */

    return S_OK; /* Always enabled */
}

HRESULT WINAPI EnableTheming(BOOL enable)
{
    return S_OK; /* Always enabled */
}

HRESULT WINAPI GetCurrentThemeName(LPWSTR filename, int filename_maxlen,
                                   LPWSTR color, int color_maxlen,
                                   LPWSTR size, int size_maxlen)
{
    return E_FAIL; /* To prevent calling EnumThemeColors and so on */
}

DWORD WINAPI GetThemeAppProperties(void)
{
    return STAP_ALLOW_CONTROLS; /* Non-client drawing is not supported */
}

HTHEME WINAPI GetWindowTheme(HWND hwnd)
{
    return GetPropW(hwnd, THEME_PROPERTY);
}

BOOL WINAPI IsAppThemed(void)
{
    return TRUE; /* Always themed */
}

BOOL WINAPI IsThemeActive(void)
{
    return TRUE; /* Always active */
}

BOOL WINAPI IsThemeDialogTextureEnabled(HWND hwnd)
{
    return TRUE; /* Always enabled */
}

HTHEME WINAPI OpenThemeData(HWND hwnd, LPCWSTR classlist)
{
    int i;

    /* comctl32.dll likes to send NULL */
    if (classlist == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    for (i = 0; i < THEMES_SIZE; i++) {
        if (match_class(classlist, themes[i].classname)) {
            WINE_TRACE("Using %s for %s.\n",
                       wine_dbgstr_w(themes[i].classname),
                       wine_dbgstr_w(classlist));

            SetPropW(hwnd, THEME_PROPERTY, &themes[i]);

            return &themes[i];
        }
    }

    WINE_FIXME("No matching theme for %s.\n", wine_dbgstr_w(classlist));

    SetLastError(ERROR_NOT_FOUND);

    return NULL;
}

void WINAPI SetThemeAppProperties(DWORD flags)
{
    /* Do nothing */
}

HRESULT WINAPI SetWindowTheme(HWND hwnd, LPCWSTR sub_app_name,
                              LPCWSTR sub_id_list)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeBool(HTHEME htheme,
                            int part_id, int state_id,
                            int prop_id, BOOL *value)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeColor(HTHEME htheme,
                             int part_id, int state_id,
                             int prop_id, COLORREF *color)
{
    HRESULT hr;
    GdkRGBA rgba;

    theme_t *theme = GET_THEME(htheme);

    if (theme == NULL || theme->get_color == NULL)
        return E_HANDLE;

    if (color == NULL)
        return E_INVALIDARG;

    hr = theme->get_color(part_id, state_id, prop_id, &rgba);

    if (SUCCEEDED(hr) && rgba.alpha > 0) {
        *color = GDKRGBA_TO_COLORREF(rgba);
        return S_OK;
    }

    return E_FAIL;
}

HRESULT WINAPI GetThemeEnumValue(HTHEME htheme,
                                 int part_id, int state_id,
                                 int prop_id, int *value)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeFilename(HTHEME htheme,
                                int part_id, int state_id,
                                int prop_id, LPWSTR filename, int maxlen)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeFont(HTHEME htheme, HDC hdc,
                            int part_id, int state_id,
                            int prop_id, LOGFONTW *font)
{
    /* I have tried to implement this, but it looks very ugly, since not all
     * controls use this themed font.
     */
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeInt(HTHEME htheme,
                           int part_id, int state_id,
                           int prop_id, int *value)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeIntList(HTHEME htheme,
                               int part_id, int state_id,
                               int prop_id, INTLIST *intlist)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeMargins(HTHEME htheme, HDC hdc,
                               int part_id, int state_id,
                               int prop_id, LPRECT rect, MARGINS *margins)
{
    memset(margins, 0, sizeof(MARGINS)); /* Just set all margins to 0 */

    return S_OK;
}

HRESULT WINAPI GetThemeMetric(HTHEME htheme, HDC hdc,
                              int part_id, int state_id,
                              int prop_id, int *value)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemePosition(HTHEME htheme,
                                int part_id, int state_id,
                                int prop_id, POINT *point)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemePropertyOrigin(HTHEME htheme,
                                      int part_id, int state_id,
                                      int prop_id, PROPERTYORIGIN *origin)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeRect(HTHEME htheme,
                            int part_id, int state_id,
                            int prop_id, RECT *rect)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeString(HTHEME htheme,
                              int part_id, int state_id,
                              int prop_id, LPWSTR buffer, int maxlen)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeTransitionDuration(HTHEME htheme, int part_id,
                                          int state_id_from, int state_id_to,
                                          int prop_id, DWORD *duration)
{
    return E_NOTIMPL;
}

BOOL WINAPI GetThemeSysBool(HTHEME htheme, int bool_id)
{
    SetLastError(ERROR_NOT_SUPPORTED);

    return FALSE;
}

COLORREF WINAPI GetThemeSysColor(HTHEME htheme, int color_id)
{
    GdkRGBA rgba;

    switch (color_id) {
    case COLOR_BTNFACE:
    case COLOR_SCROLLBAR:
    case COLOR_WINDOWFRAME:
    case COLOR_INACTIVECAPTION:
    case COLOR_GRADIENTINACTIVECAPTION:
    case COLOR_3DDKSHADOW:
    case COLOR_BTNHIGHLIGHT:
    case COLOR_ACTIVEBORDER:
    case COLOR_INACTIVEBORDER:
    case COLOR_APPWORKSPACE:
    case COLOR_BACKGROUND:
    case COLOR_ACTIVECAPTION:
    case COLOR_GRADIENTACTIVECAPTION:
    case COLOR_ALTERNATEBTNFACE:
    case COLOR_INFOBK: /* FIXME */
        uxgtk_window_get_color(WP_DIALOG, 0, TMT_FILLCOLOR, &rgba);
        break;

    case COLOR_3DLIGHT:
    case COLOR_BTNSHADOW:
        uxgtk_button_get_color(BP_PUSHBUTTON, PBS_NORMAL,
                               TMT_BORDERCOLOR, &rgba);
        break;

    case COLOR_BTNTEXT:
    case COLOR_INFOTEXT:
    case COLOR_WINDOWTEXT:
    case COLOR_CAPTIONTEXT:
        uxgtk_window_get_color(WP_DIALOG, 0, TMT_TEXTCOLOR, &rgba);
        break;

    case COLOR_HIGHLIGHTTEXT:
        uxgtk_edit_get_color(EP_EDITTEXT, ETS_SELECTED, TMT_TEXTCOLOR, &rgba);
        break;

    case COLOR_GRAYTEXT:
    case COLOR_INACTIVECAPTIONTEXT:
        uxgtk_button_get_color(BP_PUSHBUTTON, PBS_DISABLED,
                               TMT_TEXTCOLOR, &rgba);
        break;

    case COLOR_HIGHLIGHT:
    case COLOR_MENUHILIGHT:
    case COLOR_HOTLIGHT:
        uxgtk_edit_get_color(EP_EDITTEXT, ETS_SELECTED, TMT_FILLCOLOR, &rgba);
        break;

    case COLOR_MENUBAR:
        uxgtk_menu_get_color(MENU_BARBACKGROUND, MB_ACTIVE,
                             TMT_FILLCOLOR, &rgba);
        break;

    case COLOR_MENU:
        uxgtk_menu_get_color(MENU_POPUPBACKGROUND, 0, TMT_FILLCOLOR, &rgba);
        break;

    case COLOR_MENUTEXT:
        uxgtk_menu_get_color(MENU_POPUPITEM, MPI_NORMAL, TMT_TEXTCOLOR, &rgba);
        break;

    case COLOR_WINDOW:
        uxgtk_edit_get_color(EP_EDITTEXT, ETS_NORMAL, TMT_FILLCOLOR, &rgba);
        break;

    default:
        WINE_FIXME("Unknown color %d.\n", color_id);
        return GetSysColor(color_id);
    }

    if (rgba.alpha <= 0)
        return GetSysColor(color_id);

    return GDKRGBA_TO_COLORREF(rgba);
}

HBRUSH WINAPI GetThemeSysColorBrush(HTHEME htheme, int color_id)
{
    return CreateSolidBrush(GetThemeSysColor(htheme, color_id));
}

HRESULT WINAPI GetThemeSysFont(HTHEME htheme, int font_id, LOGFONTW *font)
{
    return E_NOTIMPL; /* See GetThemeFont */
}

HRESULT WINAPI GetThemeSysInt(HTHEME htheme, int int_id, int *value)
{
    return E_NOTIMPL;
}

int WINAPI GetThemeSysSize(HTHEME htheme, int size_id)
{
    SetLastError(ERROR_NOT_SUPPORTED);

    return -1;
}

HRESULT WINAPI GetThemeSysString(HTHEME htheme, int string_id,
                                 LPWSTR buffer, int maxlen)
{
    return E_NOTIMPL;
}

HRESULT WINAPI DrawThemeBackground(HTHEME htheme, HDC hdc,
                                   int part_id, int state_id,
                                   LPCRECT rect, LPCRECT clip_rect)
{
    DTBGOPTS opts;
    opts.dwSize = sizeof(DTBGOPTS);
    opts.dwFlags = 0;

    if (clip_rect != NULL) {
        opts.dwFlags = DTBG_CLIPRECT;
        CopyRect(&opts.rcClip, clip_rect);
    }

    return DrawThemeBackgroundEx(htheme, hdc, part_id, state_id, rect, &opts);
}

HRESULT WINAPI DrawThemeBackgroundEx(HTHEME htheme, HDC hdc,
                                     int part_id, int state_id,
                                     LPCRECT rect, const DTBGOPTS *options)
{
    cairo_t *cr;
    cairo_surface_t *surface;
    int x, y, width, height;

    theme_t *theme = GET_THEME(htheme);

    if (theme == NULL || theme->draw_background == NULL)
        return E_HANDLE;

    width = rect->right - rect->left;
    height = rect->bottom - rect->top;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create(surface);

    theme->draw_background(cr, part_id, state_id, width, height);

    x = rect->left;
    y = rect->top;

    paint_cairo_surface(surface, hdc, x, y, width, height);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return S_OK;
}

HRESULT WINAPI DrawThemeEdge(HTHEME htheme, HDC hdc,
                             int part_id, int state_id,
                             LPCRECT dest_rect, UINT edge, UINT flags,
                             LPRECT content_rect)
{
    return E_NOTIMPL;
}

HRESULT WINAPI DrawThemeIcon(HTHEME htheme, HDC hdc,
                             int part_id, int state_id,
                             LPCRECT rect, HIMAGELIST list, int index)
{
    return E_NOTIMPL;
}

HRESULT WINAPI DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT *rect)
{
    HWND parent = GetParent(hwnd);

    if (parent == NULL) {
        WINE_ERR("Window has no parent.\n");
        return E_FAIL;
    }

    SendMessageW(parent, WM_ERASEBKGND, (WPARAM)hdc, 0);
    SendMessageW(parent, WM_PRINTCLIENT, (WPARAM)hdc, PRF_CLIENT);

    return S_OK;
}

HRESULT WINAPI DrawThemeText(HTHEME htheme, HDC hdc,
                             int part_id, int state_id,
                             LPCWSTR text, int lenght,
                             DWORD flags, DWORD flags2,
                             LPCRECT rect)
{
    RECT rt;
    HRESULT hr;
    COLORREF color = RGB(0, 0, 0), oldcolor;

    hr = GetThemeColor(htheme, part_id, state_id, TMT_TEXTCOLOR, &color);

    if (FAILED(hr)) {
        theme_t *theme = GET_THEME(htheme);

        WINE_FIXME("No color for (%s, %d, %d).\n",
                   wine_dbgstr_w(theme == NULL ? NULL : theme->classname),
                   part_id, state_id);

        /*return hr;*/
    }

    oldcolor = SetTextColor(hdc, color);

    CopyRect(&rt, rect);

    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, text, lenght, &rt, flags);

    SetTextColor(hdc, oldcolor);

    return S_OK;
}

HRESULT WINAPI GetThemeBackgroundContentRect(HTHEME htheme, HDC hdc,
                                             int part_id, int state_id,
                                             LPCRECT bounding_rect,
                                             LPRECT content_rect)
{
    HRESULT hr;
    MARGINS margins;

    if (bounding_rect == NULL || content_rect == NULL)
        return E_INVALIDARG;

    hr = GetThemeMargins(htheme, hdc, part_id, state_id,
                         TMT_CONTENTMARGINS, NULL, &margins);

    if (FAILED(hr))
        return hr;

    content_rect->left = bounding_rect->left + margins.cxLeftWidth;
    content_rect->top = bounding_rect->top + margins.cyTopHeight;
    content_rect->right = bounding_rect->right - margins.cxRightWidth;
    content_rect->bottom = bounding_rect->bottom - margins.cyBottomHeight;

    return S_OK;
}

HRESULT WINAPI GetThemeBackgroundExtent(HTHEME htheme, HDC hdc,
                                        int part_id, int state_id,
                                        LPCRECT content_rect, RECT *extent_rect)
{
    HRESULT hr;
    MARGINS margins;

    if (content_rect == NULL || extent_rect == NULL)
        return E_INVALIDARG;

    hr = GetThemeMargins(htheme, hdc, part_id, state_id,
                         TMT_CONTENTMARGINS, NULL, &margins);

    if (FAILED(hr))
        return hr;

    extent_rect->left = content_rect->left - margins.cxLeftWidth;
    extent_rect->top = content_rect->top - margins.cyTopHeight;
    extent_rect->right = content_rect->right + margins.cxRightWidth;
    extent_rect->bottom = content_rect->bottom + margins.cyBottomHeight;

    return S_OK;
}

HRESULT WINAPI GetThemeBackgroundRegion(HTHEME htheme, HDC hdc,
                                        int part_id, int state_id,
                                        LPCRECT rect, HRGN *region)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemePartSize(HTHEME htheme, HDC hdc,
                                int part_id, int state_id,
                                RECT *rect, THEMESIZE type, SIZE *size)
{
    theme_t *theme = GET_THEME(htheme);

    if (theme == NULL || theme->get_part_size == NULL)
        return E_HANDLE;

    if (rect == NULL || size == NULL)
        return E_INVALIDARG;

    return theme->get_part_size(part_id, state_id, rect, size);
}

HRESULT WINAPI GetThemeTextExtent(HTHEME htheme, HDC hdc,
                                  int part_id, int state_id,
                                  LPCWSTR text, int lenght, DWORD flags,
                                  LPCRECT bounding_rect, LPRECT extent_rect)
{
    return E_NOTIMPL;
}

HRESULT WINAPI GetThemeTextMetrics(HTHEME htheme, HDC hdc,
                                   int part_id, int state_id,
                                   TEXTMETRICW *metric)
{
    if (!GetTextMetricsW(hdc, metric))
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}

HRESULT WINAPI HitTestThemeBackground(HTHEME htheme, HDC hdc,
                                      int part_id, int state_id,
                                      DWORD options, LPCRECT rect, HRGN hrgn,
                                      POINT point, WORD *hit_test_code)
{
    return E_NOTIMPL;
}

BOOL WINAPI IsThemeBackgroundPartiallyTransparent(HTHEME htheme,
                                                  int part_id, int state_id)
{
    return TRUE; /* The most widgets are partially transparent */
}

BOOL WINAPI IsThemePartDefined(HTHEME htheme, int part_id, int state_id)
{
    theme_t *theme = GET_THEME(htheme);

    if (theme == NULL || theme->is_part_defined == NULL) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return theme->is_part_defined(part_id, state_id);
}

BOOL WINAPI DllMain(HINSTANCE hinstance, DWORD reason, LPVOID reserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        init();
        return TRUE;

    case DLL_PROCESS_DETACH:
        uninit();
        return TRUE;
    }

    return FALSE;
}

/* vim: set expandtab tabstop=8 shiftwidth=4 softtabstop=4: */
