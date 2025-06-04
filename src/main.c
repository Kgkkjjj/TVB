#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget *entry;
static GtkWidget *text_view;

static gboolean read_output(GIOChannel *source, GIOCondition cond, gpointer data) {
    gchar buf[256];
    gsize len;
    GIOStatus status = g_io_channel_read_chars(source, buf, sizeof(buf)-1, &len, NULL);
    if(status == G_IO_STATUS_EOF || (cond & (G_IO_HUP | G_IO_ERR))) {
        g_io_channel_unref(source);
        return FALSE;
    }
    if(status == G_IO_STATUS_NORMAL && len > 0) {
        buf[len] = '\0';
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        gtk_text_buffer_insert_at_cursor(buffer, buf, -1);
    }
    return TRUE;
}

static void on_download_clicked(GtkButton *button, gpointer user_data) {
    const gchar *url = gtk_entry_get_text(GTK_ENTRY(entry));
    if(url == NULL || strlen(url) == 0)
        return;

    gchar command[1024];
    snprintf(command, sizeof(command), "youtube-dl \"%s\"", url);

    gchar *argv[] = {"sh", "-c", command, NULL};
    GPid pid;
    gint out_fd;
    GError *error = NULL;

    if(!g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,
                                 &pid, NULL, &out_fd, NULL, &error)) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        gtk_text_buffer_insert_at_cursor(buffer, error->message, -1);
        gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
        g_error_free(error);
        return;
    }

    GIOChannel *out_channel = g_io_channel_unix_new(out_fd);
    g_io_add_watch(out_channel, G_IO_IN | G_IO_HUP | G_IO_ERR, read_output, NULL);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "TVB Downloader");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "YouTube URL");
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 5);

    GtkWidget *button = gtk_button_new_with_label("Download");
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked", G_CALLBACK(on_download_clicked), NULL);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 5);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
