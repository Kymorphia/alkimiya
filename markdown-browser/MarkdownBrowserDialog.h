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
 * MarkdownBrowserDialog.h - Dialog containing a MarkdownBrowser widget.
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

