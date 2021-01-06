/*
 * MarkdownBrowserDialog.h - Help browser dialog
 *
 * Kymorphia - Making beautiful waves
 * Copyright (C) 2018-2021 Kymorphia, PBC - http://www.kymorphia.com
 * License: Proprietary - see LICENSE
 */
#ifndef MARKDOWN_BROWSER_DIALOG_H
#define MARKDOWN_BROWSER_DIALOG_H

#include <gtk/gtk.h>

typedef struct _MarkdownBrowserDialog MarkdownBrowserDialog;
typedef struct _MarkdownBrowserDialogClass MarkdownBrowserDialogClass;

#include "MarkdownBrowser.h"

#define TYPE_MARKDOWN_BROWSER_DIALOG   (markdown_browser_dialog_get_type ())
#define MARKDOWN_BROWSER_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MARKDOWN_BROWSER_DIALOG, MarkdownBrowserDialog))
#define MARKDOWN_BROWSER_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MARKDOWN_BROWSER_DIALOG, MarkdownBrowserDialogClass))
#define IS_MARKDOWN_BROWSER_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MARKDOWN_BROWSER_DIALOG))

struct _MarkdownBrowserDialog
{
  GtkDialog parent_instance;
};

struct _MarkdownBrowserDialogClass
{
  GtkDialogClass parent_class;
};

GType markdown_browser_dialog_get_type (void);
GtkWidget *markdown_browser_dialog_new (const char *uiPath);
MarkdownBrowser *markdown_browser_dialog_get_browser (MarkdownBrowserDialog *dialog);

#endif

