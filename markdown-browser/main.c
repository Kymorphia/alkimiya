/*
 * MIT License
 *
 * Copyright (C) 2021 Kymorphia, PBC - https://www.kymorphia.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/*
 * main.c - main() for Markdown browser test application
 */
#include <gtk/gtk.h>

#include "MarkdownBrowserDialog.h"
#include "MarkdownBrowser.h"

#define CMDLINE_SUMMARY \
  "markdown-browser Test Markdown browser application\n" \
  "Copyright (C) 2021 Kymorphia, PBC\n" \
  "MIT license"

static char *images_path = NULL;
static char *file_match = NULL;
static char *title_match = NULL;
static char *ui_file = NULL;
static GSList *topic_paths = NULL;

static GOptionEntry command_line_options[] =
{
  { "images-path", 'i', 0, G_OPTION_ARG_STRING, &images_path,
    "Path to images referenced in Markdown content", NULL },
  { "file-match", 'f', 0, G_OPTION_ARG_STRING, &file_match,
    "Regex for Markdown file match, capture group is used as topic ID (defaults to '(.*)\\.(md|markdown)$')", NULL },
  { "title-match", 't', 0, G_OPTION_ARG_STRING, &title_match,
    "Regex to extract title from Markdown files, capture group is the title (defeaults to '^ {0,3}\\# (.*)')", NULL },
  { "ui-file", 'u', 0, G_OPTION_ARG_STRING, &ui_file,
    "External UI file to use instead of default builtin interface data", NULL },
  { NULL }
};

static void
app_open (GApplication *app, GFile **files, gint n_files, const gchar *hint)
{
  char *path;
  int i;

  for (i = 0; i < n_files; i++)
  {
    path = g_file_get_path (files[i]);
    topic_paths = g_slist_append (topic_paths, path);
  }

  g_application_activate (app);
}                           

static void
app_activate (GApplication *app, gpointer user_data)
{
  GtkWidget *browserDialog;
  MarkdownBrowser *browser;
  const char *path;
  GError *err = NULL;
  GSList *p;

  // Create Markdown browser dialog and add to application
  browserDialog = markdown_browser_dialog_new (ui_file);
  browser = markdown_browser_dialog_get_browser (MARKDOWN_BROWSER_DIALOG (browserDialog));

  if (topic_paths)
  {
    for (p = topic_paths; p; p = p->next)
    {
      path = (const char *)(p->data);

      if (!markdown_browser_add_files (browser, path, file_match, title_match, &err))
      {
        g_warning ("Failed to load Markdown files from path '%s': %s", path, err->message);
        g_clear_error (&err);
      }
    }
  }
  else
  {
    if (!markdown_browser_add_files (browser, ".", file_match, title_match, &err))
    {
      g_warning ("Failed to load Markdown files from path '.': %s", err->message);
      g_clear_error (&err);
    }
  }

  if (images_path)
    g_object_set (browser, "images-path", images_path, NULL);

  gtk_widget_show (browserDialog);

  gtk_application_add_window (GTK_APPLICATION (app), GTK_WINDOW (browserDialog));
}

int
main (int argc, char **argv)
{
  GApplication *app;
  int status;

  app = G_APPLICATION (gtk_application_new ("com.kymorphia.MarkdownBrowser",
                                            G_APPLICATION_HANDLES_OPEN | G_APPLICATION_NON_UNIQUE));

  g_application_set_option_context_parameter_string (app, "PATH - Markdown content browser");
  g_application_add_main_option_entries (app, command_line_options);
  g_application_set_option_context_summary (app, CMDLINE_SUMMARY);

  g_signal_connect (app, "open", G_CALLBACK (app_open), NULL);
  g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);

  status = g_application_run (app, argc, argv);
  g_object_unref (app);

  return status;
}

