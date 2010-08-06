#include <glib.h>
#include <gio/gio.h>
#include <stdlib.h>

#include <bognor-regis/br-queue.h>

GMainLoop *mainloop;

static void
list_uris_cb (BrQueue      *queue,
              const char  **uris,
              const GError *error,
              gpointer      userdata)
{
    int i;

    if (error != NULL) {
        g_warning ("Error listing uris: %s", error->message);
    }

    if (uris == NULL || uris[0] == NULL) {
        g_print ("No tracks queued\n");
        g_main_loop_quit (mainloop);

        return;
    }

    for (i = 0; uris[i]; i++) {
        g_print ("   [%d] - %s\n", i, uris[i]);
    }

    g_main_loop_quit (mainloop);
}

static void
get_int_cb(BrQueue      *queue,
           const int     value,
           const GError *error,
           gpointer      userdata)
{
    g_print("return:%d\n", value);
    g_main_loop_quit (mainloop);
}

static void
get_gboolean_cb(BrQueue      *queue,
               const gboolean value,
               const GError  *error,
               gpointer       userdata)
{
    g_print("return:%d\n", value);
    g_main_loop_quit (mainloop);
}

static void
get_double_cb(BrQueue        *queue,
              const double    value,
              const GError   *error,
              gpointer        userdata)
{
    g_print("return:%f\n", value);
    g_main_loop_quit (mainloop);
}

int
main (int    argc,
      char **argv)
{
    BrQueue *local_queue;
    GError *error = NULL;
    int i;

    g_type_init ();

    local_queue = br_queue_new_local ();

    if (argv[1] == NULL) {
        g_print ("Usage: bognor-regis <uri | play | stop | ls |list-queues | remove |\n"
                 "                     get-index | get-mode | get-duration | set-mode [mode] >\n"
                 "                     get-mute | set-mute | get-volume | set-volume [volume] >\n");
        /* probably also print current list */
        return -1;
    }

    if (g_str_equal (argv[1], "play")) {
        br_queue_play (local_queue);
    } else if (g_str_equal (argv[1], "stop")) {
        br_queue_stop (local_queue);
    } else if (g_str_equal (argv[1], "ls")) {
        char **uris;

        br_queue_list_uris (local_queue, list_uris_cb, NULL);

        /* Run a mainloop to get a response */
        mainloop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (mainloop);

        return 0;
    } else if (g_str_equal (argv[1], "list-queues")) {
        g_print ("**FIXME**\n");
    } else if (g_str_equal (argv[1], "set-mute")) {
        br_queue_set_mute(local_queue, atoi(argv[2]));
    } else if (g_str_equal (argv[1], "set-volume")) {
        br_queue_set_volume(local_queue, atof(argv[2]));
    } else if (g_str_equal (argv[1], "set-mode")) {
        br_queue_set_repeat_mode(local_queue, atoi(argv[2]));
    } else if (g_str_equal (argv[1], "get-mute")) {
        br_queue_get_mute(local_queue, get_gboolean_cb, NULL);
        mainloop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (mainloop);
        return 0;
    } else if (g_str_equal (argv[1], "get-volume")) {
        br_queue_get_volume(local_queue, get_double_cb, NULL);
        mainloop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (mainloop);
        return 0;
    } else if (g_str_equal (argv[1], "get-mode")) {
        br_queue_get_repeat_mode(local_queue, get_int_cb, NULL);
        mainloop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (mainloop);
        return 0;
    } else if (g_str_equal (argv[1], "get-duration")) {
        br_queue_get_duration(local_queue, get_int_cb, NULL);
        mainloop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (mainloop);
        return 0;
    } else if (g_str_equal (argv[1], "get-index")) {
        br_queue_get_index(local_queue, get_int_cb, NULL);
        mainloop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (mainloop);
        return 0;
    } else if (g_str_equal (argv[1], "next")) {
        br_queue_next (local_queue);
    } else if (g_str_equal (argv[1], "previous")) {
        br_queue_previous (local_queue);
    } else if (g_str_equal (argv[1], "remove")) {
        int index, count;

        if (argc != 4) {
            g_print ("Usage: %s remove <index> <count>\n", argv[0]);
            return 0;
        }

        index = atoi (argv[2]);
        count = atoi (argv[3]);
        br_queue_remove_range (local_queue, index, count);
    } else {
        char **uris, **mimetypes;

        uris = g_new0 (char *, argc);
        mimetypes = g_new0 (char *, argc);

        for (i = 1; i < argc; i++) {
            GFile *f = g_file_new_for_commandline_arg (argv[i]);
            GFileInfo *info;
            GError *error = NULL;

            info = g_file_query_info (f, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                      G_FILE_QUERY_INFO_NONE, NULL, &error);
            if (info == NULL) {
                g_warning ("Error finding mimetype for %s: %s", argv[i],
                           error->message);
                g_error_free (error);
                g_object_unref (f);
                return 0;
            }

            uris[i - 1] = g_file_get_uri (f);;
            mimetypes[i - 1] = g_strdup (g_file_info_get_content_type (info));

            g_object_unref (info);
            g_object_unref (f);
        }

        br_queue_append_uris (local_queue, argc - 1,
                              (const char **) uris,
                              (const char **) mimetypes);

        /* This will have leaked mimetypes[i] but we're quitting now
           so it doesn't matter */
    }

    return 0;
}
