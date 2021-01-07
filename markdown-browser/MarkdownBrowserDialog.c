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
 * MarkdownBrowserDialog.c - Dialog containing a MarkdownBrowser widget.
 */
#include <glib/gi18n.h>
#include "MarkdownBrowserDialog.h"

#define DEFAULT_UI_PATH         "."     // Default path to UI file used by MarkdownBrowser widget

#define MARKDOWN_BROWSER_DIALOG_WIDTH  1000
#define MARKDOWN_BROWSER_DIALOG_HEIGHT 600

enum
{
  PROP_0,
  PROP_UI_PATH,
  PROP_HELP_WIDGET
};

typedef struct
{
  GtkWidget *helpWidget;
  char *uiPath;
} MarkdownBrowserDialogPrivate;

static void markdown_browser_dialog_constructed (GObject *object);
static void markdown_browser_dialog_finalize (GObject *object);
static void markdown_browser_dialog_set_property (GObject *object, guint property_id,
                                           const GValue *value, GParamSpec *pspec);
static void markdown_browser_dialog_get_property (GObject *object, guint property_id,
                                           GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE_WITH_PRIVATE (MarkdownBrowserDialog, markdown_browser_dialog, GTK_TYPE_DIALOG)

static void
markdown_browser_dialog_class_init (MarkdownBrowserDialogClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->set_property = markdown_browser_dialog_set_property;
  obj_class->get_property = markdown_browser_dialog_get_property;
  obj_class->constructed = markdown_browser_dialog_constructed;
  obj_class->finalize = markdown_browser_dialog_finalize;

  g_object_class_install_property (obj_class, PROP_UI_PATH,
    g_param_spec_string ("ui-path", "UiPath", "Path to Help.ui interface file",
                         DEFAULT_UI_PATH, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (obj_class, PROP_HELP_WIDGET,
    g_param_spec_object ("help-widget", "HelpWidget", "Child MarkdownBrowser widget",
                         TYPE_MARKDOWN_BROWSER, G_PARAM_READABLE));
}

static void
markdown_browser_dialog_init (MarkdownBrowserDialog *dialog)
{
  MarkdownBrowserDialogPrivate *priv = markdown_browser_dialog_get_instance_private (dialog);

  priv->uiPath = g_strdup (DEFAULT_UI_PATH);
}

static void
markdown_browser_dialog_constructed (GObject *object)
{
  MarkdownBrowserDialog *dialog = MARKDOWN_BROWSER_DIALOG (object);
  MarkdownBrowserDialogPrivate *priv = markdown_browser_dialog_get_instance_private (dialog);

  if (G_OBJECT_CLASS (markdown_browser_dialog_parent_class)->constructed)
    G_OBJECT_CLASS (markdown_browser_dialog_parent_class)->constructed (object);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Help Browser"));
  gtk_window_set_default_size (GTK_WINDOW (dialog), MARKDOWN_BROWSER_DIALOG_WIDTH, MARKDOWN_BROWSER_DIALOG_HEIGHT);

  // Create help widget and add to dialog content area box
  priv->helpWidget = markdown_browser_new (priv->uiPath);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      priv->helpWidget, TRUE, TRUE, 0);
  gtk_widget_show (priv->helpWidget);
}

static void
markdown_browser_dialog_finalize (GObject *object)
{
  MarkdownBrowserDialog *dialog = MARKDOWN_BROWSER_DIALOG (object);
  MarkdownBrowserDialogPrivate *priv = markdown_browser_dialog_get_instance_private (dialog);

  g_free (priv->uiPath);

  if (G_OBJECT_CLASS (markdown_browser_dialog_parent_class)->finalize)
    G_OBJECT_CLASS (markdown_browser_dialog_parent_class)->finalize (object);
}

static void
markdown_browser_dialog_set_property (GObject *object, guint property_id,
                                const GValue *value, GParamSpec *pspec)
{
  MarkdownBrowserDialog *dialog = MARKDOWN_BROWSER_DIALOG (object);
  MarkdownBrowserDialogPrivate *priv = markdown_browser_dialog_get_instance_private (dialog);

  switch (property_id)
  {
    case PROP_UI_PATH:
      g_free (priv->uiPath);
      priv->uiPath = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
markdown_browser_dialog_get_property (GObject *object, guint property_id,
                                GValue *value, GParamSpec *pspec)
{
  MarkdownBrowserDialog *dialog = MARKDOWN_BROWSER_DIALOG (object);
  MarkdownBrowserDialogPrivate *priv = markdown_browser_dialog_get_instance_private (dialog);

  switch (property_id)
  {
    case PROP_UI_PATH:
      g_value_set_string (value, priv->uiPath);
      break;
    case PROP_HELP_WIDGET:
      g_value_set_object (value, priv->helpWidget);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

/**
 * markdown_browser_dialog_new:
 * @uiPath: Directory path to Help.ui file
 *
 * Create a new help dialog widget.
 * 
 * Returns: New help dialog widget
 */
GtkWidget *
markdown_browser_dialog_new (const char *uiPath)
{
  return GTK_WIDGET (g_object_new (TYPE_MARKDOWN_BROWSER_DIALOG, "ui-path", uiPath, NULL));
}

/**
 * markdown_browser_dialog_get_browser:
 * @dialog: Help dialog
 *
 * Get the child MarkdownBrowser widget from a MarkdownBrowserDialog.
 *
 * Returns: (transfer-none): Child MarkdownBrowser widget
 */
MarkdownBrowser *
markdown_browser_dialog_get_browser (MarkdownBrowserDialog *dialog)
{
  MarkdownBrowserDialogPrivate *priv = markdown_browser_dialog_get_instance_private (dialog);
  return MARKDOWN_BROWSER (priv->helpWidget);
}

