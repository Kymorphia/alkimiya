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
 * MarkdownBrowser.h - Markdown browser widget derived from GtkBox.
 */
#ifndef MARKDOWN_BROWSER_H
#define MARKDOWN_BROWSER_H

#include <gtk/gtk.h>

typedef struct _MarkdownBrowser MarkdownBrowser;
typedef struct _MarkdownBrowserClass MarkdownBrowserClass;

#define TYPE_MARKDOWN_BROWSER   (markdown_browser_get_type ())
#define MARKDOWN_BROWSER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MARKDOWN_BROWSER, MarkdownBrowser))
#define IS_MARKDOWN_BROWSER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MARKDOWN_BROWSER))

struct _MarkdownBrowser
{
  GtkBox parent_instance;
};

struct _MarkdownBrowserClass
{
  GtkBoxClass parent_class;
};

/**
 * MarkdownBrowserTopic:
 * @name: Name identifier
 * @title: Topic title
 * @content: Markdown topic content
 *
 * Markdown browser topic information.
 */
typedef struct
{
  char *name;
  char *title;
  char *content;
} MarkdownBrowserTopic;

/**
 * MarkdownBrowserVisit:
 * @topic: Topic index for this visit
 * @line: Line number of text view
 *
 * Defines data for a visit in the markdown link history.
 */
typedef struct
{
  int topic;
  int line;
} MarkdownBrowserVisit;

/**
 * MARKDOWN_BROWSER_TOPIC_NONE:
 *
 * Value indicating no topic selected.
 */
#define MARKDOWN_BROWSER_TOPIC_NONE    (-1)

GType markdown_browser_get_type (void);
GtkWidget *markdown_browser_new (const char *uiFile);
void markdown_browser_navigate (MarkdownBrowser *help, int historyOfs, int topicIndex);
gboolean markdown_browser_navigate_to_topic_by_name (MarkdownBrowser *help, const char *name);
int markdown_browser_get_topic_by_name (MarkdownBrowser *help, const char *name);
MarkdownBrowserTopic *markdown_browser_get_topics (MarkdownBrowser *browser, guint *count);
MarkdownBrowserVisit *markdown_browser_get_history (MarkdownBrowser *browser, guint *count);
void markdown_browser_add_topic (MarkdownBrowser *help, const char *name, const char *title, const char *content);
gboolean markdown_browser_add_files (MarkdownBrowser *help, const char *path, const char *fileMatch,
                              const char *titleMatch, GError **err);
#endif

