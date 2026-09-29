#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SYSFS_BASE "/sys/bus/wmi/drivers/acer-wmi-battery"

static GtkWidget* health_switch;
static GtkWidget* calib_switch;
static GtkWidget* temp_label;

int read_sysfs(const char* filename, char* buffer, size_t size)
{
  char filepath[256];
  snprintf(filepath, sizeof(filepath), "%s/%s", SYSFS_BASE, filename);
  FILE* f = fopen(filepath, "r");
  if (!f)
    return -1;
  if (fgets(buffer, size, f) == NULL) {
    fclose(f);
    return -1;
  }
  fclose(f);
  buffer[strcspn(buffer, "\n")] = '\0';
  return 0;
}

int write_sysfs(const char* filename, const char* value)
{
  char filepath[256];
  snprintf(filepath, sizeof(filepath), "%s/%s", SYSFS_BASE, filename);
  FILE* f = fopen(filepath, "w");
  if (!f)
    return -1;
  fprintf(f, "%s\n", value);
  fclose(f);
  return 0;
}

gboolean update_temp(gpointer data)
{
  char buf[64];
  if (read_sysfs("temperature", buf, sizeof(buf)) == 0) {
    int  mC = atoi(buf);
    char label_text[128];
    snprintf(label_text, sizeof(label_text), "<span font_desc='14'><b>Temperature:</b> %.1f °C</span>", mC / 1000.0);
    gtk_label_set_markup(GTK_LABEL(temp_label), label_text);
  }
  else {
    gtk_label_set_markup(GTK_LABEL(temp_label), "<span font_desc='14'><b>Temperature:</b> N/A</span>");
  }
  return TRUE;
}

void on_health_toggled(GObject* gobject, GParamSpec* pspec, gpointer user_data)
{
  gboolean state = gtk_switch_get_active(GTK_SWITCH(gobject));
  write_sysfs("health_mode", state ? "1" : "0");
}

void on_calib_toggled(GObject* gobject, GParamSpec* pspec, gpointer user_data)
{
  gboolean state = gtk_switch_get_active(GTK_SWITCH(gobject));
  write_sysfs("calibration_mode", state ? "1" : "0");
}

void sync_ui_state()
{
  char buf[64];

  // Block signals so we don't trigger sysfs writes while syncing UI from
  // hardware
  g_signal_handlers_block_by_func(health_switch, G_CALLBACK(on_health_toggled), NULL);
  g_signal_handlers_block_by_func(calib_switch, G_CALLBACK(on_calib_toggled), NULL);

  if (read_sysfs("health_mode", buf, sizeof(buf)) == 0) {
    gtk_switch_set_active(GTK_SWITCH(health_switch), strcmp(buf, "1") == 0);
  }

  if (read_sysfs("calibration_mode", buf, sizeof(buf)) == 0) {
    gtk_switch_set_active(GTK_SWITCH(calib_switch), strcmp(buf, "1") == 0);
  }

  g_signal_handlers_unblock_by_func(health_switch, G_CALLBACK(on_health_toggled), NULL);
  g_signal_handlers_unblock_by_func(calib_switch, G_CALLBACK(on_calib_toggled), NULL);
}

static void activate(GtkApplication* app, gpointer user_data)
{
  GtkWidget* window;
  GtkWidget* grid;
  GtkWidget* health_label;
  GtkWidget* calib_label;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Acer Battery");
  gtk_window_set_default_size(GTK_WINDOW(window), 350, 200);
  gtk_container_set_border_width(GTK_CONTAINER(window), 20);

  grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 30);
  gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
  gtk_container_add(GTK_CONTAINER(window), grid);

  temp_label = gtk_label_new("<span font_desc='14'><b>Temperature:</b> -- °C</span>");
  gtk_label_set_use_markup(GTK_LABEL(temp_label), TRUE);
  gtk_widget_set_halign(temp_label, GTK_ALIGN_CENTER);

  health_label = gtk_label_new("Health Mode (80% Limit):");
  gtk_widget_set_halign(health_label, GTK_ALIGN_START);
  health_switch = gtk_switch_new();
  gtk_widget_set_valign(health_switch, GTK_ALIGN_CENTER);

  calib_label = gtk_label_new("Calibration Mode:");
  gtk_widget_set_halign(calib_label, GTK_ALIGN_START);
  calib_switch = gtk_switch_new();
  gtk_widget_set_valign(calib_switch, GTK_ALIGN_CENTER);

  gtk_grid_attach(GTK_GRID(grid), temp_label, 0, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), health_label, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), health_switch, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), calib_label, 0, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), calib_switch, 1, 2, 1, 1);

  g_signal_connect(health_switch, "notify::active", G_CALLBACK(on_health_toggled), NULL);
  g_signal_connect(calib_switch, "notify::active", G_CALLBACK(on_calib_toggled), NULL);

  sync_ui_state();
  update_temp(NULL);
  g_timeout_add(2000, update_temp, NULL);  // Refresh temperature every 2 seconds

  gtk_widget_show_all(window);
}

int main(int argc, char** argv)
{
  GtkApplication* app;
  int             status;

  if (access(SYSFS_BASE, F_OK) != 0) {
    fprintf(stderr, "Error: Acer Battery WMI driver not found at %s.\n", SYSFS_BASE);
  }

  app = gtk_application_new("com.rkkdr.acerbattery", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
