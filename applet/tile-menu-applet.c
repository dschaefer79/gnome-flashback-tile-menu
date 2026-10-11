/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 dschaefer79
 * Written entirely with Claude (Anthropic's AI assistant).
 *
 * gnome-panel module with two applets, Window Tiling and Resolution: a
 * panel icon that opens a menu. The menus are key files that
 * flashback-tile and flashback-resolution-settings write from the settings
 * (~/.config/gnome-flashback-tile-menu/tiling.menu and resolution.menu),
 * read each time a menu opens:
 *
 *   [Item 0]
 *   Type=item            item, check or separator
 *   Label=Tile in 2 columns
 *   Icon=flashback-tile-columns-2-symbolic
 *   Accel=<Super><Alt>2  shown on the right of the label
 *   Comment=Tile the front windows into 2 columns
 *   Active=true          check items only
 *   Exec=flashback-tile 2
 */
#include <libgnome-panel/gp-module.h>
#include <libgnome-panel/gp-applet.h>
#include <libgnome-panel/gp-image-menu-item.h>

#define MODULE_ID "org.dschaefer79.tile-menu"
#define CONFIG_NAME "gnome-flashback-tile-menu"

#ifndef VERSION
#define VERSION "0"
#endif
#ifndef DATADIR
#define DATADIR "/usr/share"
#endif

/* The base type of both applets; each subclass names its menu and icon. */
typedef struct
{
  GpApplet   parent;
  GtkWidget *image;
  GtkWidget *menu;
} FbtmApplet;

typedef struct
{
  GpAppletClass parent_class;
  const char   *menu_file;      /* key file in the config and data dirs */
  const char   *icon_name;
  const char   *tooltip;
  const char   *refresh_command; /* rewrites the menu file */
  const char   *removed_command; /* run when removed from the panel */
} FbtmAppletClass;

static GType fbtm_applet_get_type (void);
G_DEFINE_TYPE (FbtmApplet, fbtm_applet, GP_TYPE_APPLET)

#define FBTM_APPLET(o) ((FbtmApplet *) (o))
#define FBTM_APPLET_GET_CLASS(o) ((FbtmAppletClass *) G_OBJECT_GET_CLASS (o))

static void
run_command (const char *command)
{
  g_autoptr (GError) error = NULL;
  g_auto (GStrv) argv = NULL;

  if (!g_shell_parse_argv (command, NULL, &argv, &error) ||
      !g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
                      NULL, NULL, NULL, &error))
    g_warning ("%s: %s", command, error->message);
}

static void
item_activated (GtkMenuItem *item,
                gpointer     user_data)
{
  const char *command = g_object_get_data (G_OBJECT (item), "exec");

  if (command != NULL && *command != '\0')
    run_command (command);
}

static void
show_accel (GtkWidget  *item,
            const char *accel)
{
  GtkWidget *label = gtk_bin_get_child (GTK_BIN (item));
  guint key = 0;
  GdkModifierType mods = 0;

  gtk_accelerator_parse (accel, &key, &mods);
  if (key != 0 && GTK_IS_ACCEL_LABEL (label))
    gtk_accel_label_set_accel (GTK_ACCEL_LABEL (label), key, mods);
}

static GtkWidget *
menu_item (FbtmApplet *self,
           GKeyFile   *file,
           const char *group)
{
  g_autofree char *type = g_key_file_get_string (file, group, "Type", NULL);
  g_autofree char *label = g_key_file_get_string (file, group, "Label", NULL);
  g_autofree char *icon = g_key_file_get_string (file, group, "Icon", NULL);
  g_autofree char *accel = g_key_file_get_string (file, group, "Accel", NULL);
  g_autofree char *comment = g_key_file_get_string (file, group, "Comment", NULL);
  GtkWidget *item;

  if (g_strcmp0 (type, "separator") == 0)
    return gtk_separator_menu_item_new ();

  if (g_strcmp0 (type, "check") == 0)
    {
      item = gtk_check_menu_item_new_with_label (label ? label : "");
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                      g_key_file_get_boolean (file, group, "Active", NULL));
    }
  else
    {
      item = gp_image_menu_item_new_with_label (label ? label : "");
      if (icon != NULL && *icon != '\0')
        {
          GtkWidget *image = gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_MENU);

          gtk_image_set_pixel_size (GTK_IMAGE (image),
                                    gp_applet_get_menu_icon_size (GP_APPLET (self)));
          gp_image_menu_item_set_image (GP_IMAGE_MENU_ITEM (item), image);
        }
    }

  if (accel != NULL && *accel != '\0')
    show_accel (item, accel);
  if (comment != NULL && *comment != '\0')
    gtk_widget_set_tooltip_text (item, comment);
  if (g_key_file_has_key (file, group, "Sensitive", NULL))
    gtk_widget_set_sensitive (item, g_key_file_get_boolean (file, group, "Sensitive", NULL));

  g_object_set_data_full (G_OBJECT (item), "exec",
                          g_key_file_get_string (file, group, "Exec", NULL), g_free);
  g_signal_connect (item, "activate", G_CALLBACK (item_activated), NULL);
  return item;
}

/* The menu from the user's menu file, else the one the package ships. */
static GtkWidget *
build_menu (FbtmApplet *self)
{
  FbtmAppletClass *klass = FBTM_APPLET_GET_CLASS (self);
  g_autoptr (GKeyFile) file = g_key_file_new ();
  g_autofree char *user = g_build_filename (g_get_user_config_dir (), CONFIG_NAME,
                                            klass->menu_file, NULL);
  g_autofree char *system = g_build_filename (DATADIR, CONFIG_NAME,
                                              klass->menu_file, NULL);
  g_auto (GStrv) groups = NULL;
  GtkWidget *menu = gtk_menu_new ();

  if (!g_key_file_load_from_file (file, user, G_KEY_FILE_NONE, NULL))
    g_key_file_load_from_file (file, system, G_KEY_FILE_NONE, NULL);

  groups = g_key_file_get_groups (file, NULL);
  for (int i = 0; groups[i] != NULL; i++)
    if (g_str_has_prefix (groups[i], "Item"))
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item (self, file, groups[i]));

  gtk_widget_show_all (menu);
  return menu;
}

static gboolean
fbtm_applet_button_press_event (GtkWidget      *widget,
                                GdkEventButton *event)
{
  FbtmApplet *self = FBTM_APPLET (widget);
  FbtmAppletClass *klass = FBTM_APPLET_GET_CLASS (self);

  if (event->button != GDK_BUTTON_PRIMARY || event->type != GDK_BUTTON_PRESS)
    return GTK_WIDGET_CLASS (fbtm_applet_parent_class)->button_press_event (widget, event);

  /* The last menu stays until now: its items may still be activating. */
  g_clear_pointer (&self->menu, gtk_widget_destroy);
  self->menu = build_menu (self);
  gp_applet_popup_menu_at_widget (GP_APPLET (self), GTK_MENU (self->menu),
                                  widget, (GdkEvent *) event);

  /* Bring the menu file up to date for next time, for instance with
   * shortcuts changed in the Keyboard settings. */
  if (klass->refresh_command != NULL)
    run_command (klass->refresh_command);
  return TRUE;
}

static void
update_icon_size (FbtmApplet *self)
{
  gtk_image_set_pixel_size (GTK_IMAGE (self->image),
                            gp_applet_get_panel_icon_size (GP_APPLET (self)));
}

static void
fbtm_applet_constructed (GObject *object)
{
  FbtmApplet *self = FBTM_APPLET (object);
  FbtmAppletClass *klass = FBTM_APPLET_GET_CLASS (self);

  G_OBJECT_CLASS (fbtm_applet_parent_class)->constructed (object);

  self->image = gtk_image_new_from_icon_name (klass->icon_name, GTK_ICON_SIZE_MENU);
  gtk_widget_set_margin_start (self->image, 4);
  gtk_widget_set_margin_end (self->image, 4);
  gtk_container_add (GTK_CONTAINER (self), self->image);
  gtk_widget_set_tooltip_text (GTK_WIDGET (self), klass->tooltip);
  update_icon_size (self);
  g_signal_connect_swapped (self, "notify::panel-icon-size",
                            G_CALLBACK (update_icon_size), self);
  gtk_widget_show_all (GTK_WIDGET (self));
}

static void
fbtm_applet_remove_from_panel (GpApplet *applet)
{
  FbtmAppletClass *klass = FBTM_APPLET_GET_CLASS (applet);

  /* Taken off with the panel's own menu: do not put it back at login. */
  if (klass->removed_command != NULL)
    run_command (klass->removed_command);
}

static void
fbtm_applet_dispose (GObject *object)
{
  FbtmApplet *self = FBTM_APPLET (object);

  g_clear_pointer (&self->menu, gtk_widget_destroy);
  G_OBJECT_CLASS (fbtm_applet_parent_class)->dispose (object);
}

static void
fbtm_applet_class_init (FbtmAppletClass *klass)
{
  G_OBJECT_CLASS (klass)->constructed = fbtm_applet_constructed;
  G_OBJECT_CLASS (klass)->dispose = fbtm_applet_dispose;
  GTK_WIDGET_CLASS (klass)->button_press_event = fbtm_applet_button_press_event;
  GP_APPLET_CLASS (klass)->remove_from_panel = fbtm_applet_remove_from_panel;
}

static void
fbtm_applet_init (FbtmApplet *self)
{
}

/* Window Tiling */
typedef FbtmApplet FbtmTilingApplet;
typedef FbtmAppletClass FbtmTilingAppletClass;
static GType fbtm_tiling_applet_get_type (void);
G_DEFINE_TYPE (FbtmTilingApplet, fbtm_tiling_applet, fbtm_applet_get_type ())

static void
fbtm_tiling_applet_class_init (FbtmTilingAppletClass *klass)
{
  klass->menu_file = "tiling.menu";
  klass->icon_name = "view-grid-symbolic";
  klass->tooltip = "Window Tiling";
  klass->refresh_command = "flashback-tile menu";
  klass->removed_command = "flashback-tile-menu-setup tiling --mark-removed";
}

static void
fbtm_tiling_applet_init (FbtmTilingApplet *self)
{
}

/* Resolution */
typedef FbtmApplet FbtmResolutionApplet;
typedef FbtmAppletClass FbtmResolutionAppletClass;
static GType fbtm_resolution_applet_get_type (void);
G_DEFINE_TYPE (FbtmResolutionApplet, fbtm_resolution_applet, fbtm_applet_get_type ())

static void
fbtm_resolution_applet_class_init (FbtmResolutionAppletClass *klass)
{
  klass->menu_file = "resolution.menu";
  klass->icon_name = "video-display-symbolic";
  klass->tooltip = "Resolution";
  klass->refresh_command = NULL;
  klass->removed_command = "flashback-tile-menu-setup resolution --mark-removed";
}

static void
fbtm_resolution_applet_init (FbtmResolutionApplet *self)
{
}

static GpAppletInfo *
get_applet_info (const char *id)
{
  if (g_strcmp0 (id, "tiling") == 0)
    return gp_applet_info_new (fbtm_tiling_applet_get_type, "Window Tiling",
                               "Tile the windows into columns, rows or a grid",
                               "view-grid-symbolic");
  if (g_strcmp0 (id, "resolution") == 0)
    return gp_applet_info_new (fbtm_resolution_applet_get_type, "Resolution",
                               "Switch the screen resolution",
                               "video-display-symbolic");
  return NULL;
}

void
gp_module_load (GpModule *module)
{
  gp_module_set_abi_version (module, GP_MODULE_ABI_VERSION);
  gp_module_set_gettext_domain (module, CONFIG_NAME);
  gp_module_set_id (module, MODULE_ID);
  gp_module_set_version (module, VERSION);
  gp_module_set_applet_ids (module, "tiling", "resolution", NULL);
  gp_module_set_get_applet_info (module, get_applet_info);
}
