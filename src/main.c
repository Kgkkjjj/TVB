#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget *entry;
static GtkWidget *text_view;
static GtkWidget *search_entry;
static GtkListStore *video_store;
static GtkTreeModel *filter_model;

static gboolean filter_visible_func(GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
    const gchar *query = gtk_entry_get_text(GTK_ENTRY(search_entry));
    if (query == NULL || *query == '\0')
        return TRUE;

    gchar *name;
    gtk_tree_model_get(model, iter, 0, &name, -1);
    gchar *lower_name = g_utf8_strdown(name, -1);
    gchar *lower_query = g_utf8_strdown(query, -1);
    gboolean visible = strstr(lower_name, lower_query) != NULL;
    g_free(lower_name);
    g_free(lower_query);
    g_free(name);
    return visible;
}

static void on_search_changed(GtkEditable *editable, gpointer data) {
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(filter_model));
}

static gboolean read_output(GIOChannel *source, GIOCondition cond, gpointer data) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GError *error = NULL;

    while (TRUE) {
        gchar *line = NULL;
        gsize len = 0;
        GIOStatus status = g_io_channel_read_line(source, &line, &len, NULL, &error);
        if (status == G_IO_STATUS_EOF) {
            g_free(line);
            g_io_channel_unref(source);
            return FALSE;
        }
        if (status == G_IO_STATUS_ERROR) {
            gtk_text_buffer_insert_at_cursor(buffer, error->message, -1);
            gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
            g_error_free(error);
            g_free(line);
            return FALSE;
        }
        if (status == G_IO_STATUS_NORMAL) {
            gtk_text_buffer_insert_at_cursor(buffer, line, -1);

            if (g_str_has_prefix(line, "[download] Destination: ")) {
                const gchar *dest = line + strlen("[download] Destination: ");
                gchar *filename = g_strchomp(g_strdup(dest));
                GtkTreeIter it;
                gtk_list_store_append(video_store, &it);
                gtk_list_store_set(video_store, &it, 0, filename, -1);
                g_free(filename);
            }
            g_free(line);
        } else {
            g_free(line);
            break;
        }
    }
    return TRUE;
}

static void on_download_clicked(GtkButton *button, gpointer user_data) {
    const gchar *url = gtk_entry_get_text(GTK_ENTRY(entry));
    if(url == NULL || strlen(url) == 0)
        return;

    const gchar *dl = g_getenv("TVB_DOWNLOADER");
    if(dl == NULL || strlen(dl) == 0)
        dl = "youtube-dl";

    gchar command[1024];
    snprintf(command, sizeof(command), "%s \"%s\"", dl, url);

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
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    /* Sidebar */
    GtkWidget *side_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), side_vbox, FALSE, FALSE, 5);

    search_entry = gtk_search_entry_new();
    gtk_box_pack_start(GTK_BOX(side_vbox), search_entry, FALSE, FALSE, 0);
    g_signal_connect(search_entry, "search-changed", G_CALLBACK(on_search_changed), NULL);

    video_store = gtk_list_store_new(1, G_TYPE_STRING);
    filter_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(video_store), NULL);
    gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filter_model), filter_visible_func, NULL, NULL);

    GtkWidget *tree = gtk_tree_view_new_with_model(filter_model);
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Videos", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    GtkWidget *scroll_list = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll_list, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll_list), tree);
    gtk_box_pack_start(GTK_BOX(side_vbox), scroll_list, TRUE, TRUE, 0);

    /* Main area */
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), main_vbox, TRUE, TRUE, 0);

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "YouTube URL");
    gtk_box_pack_start(GTK_BOX(main_vbox), entry, FALSE, FALSE, 0);

    GtkWidget *button = gtk_button_new_with_label("Download");
    gtk_box_pack_start(GTK_BOX(main_vbox), button, FALSE, FALSE, 0);
    g_signal_connect(button, "clicked", G_CALLBACK(on_download_clicked), NULL);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(main_vbox), scroll, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
