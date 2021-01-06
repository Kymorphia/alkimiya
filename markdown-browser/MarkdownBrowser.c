/*
 * MIT License
 *
 * Copyright (C) 2021 Kymorphia, PBC - http://www.kymorphia.com
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
 * MarkdownBrowser.c - Markdown browser widget derived from GtkBox.
 */
#include "MarkdownBrowser.h"

// C source data for default interface
#include "MarkdownBrowser-ui.h"

#define DEFAULT_HISTORY_MAX     10      // Default topic visit history size

#define DEFAULT_FILE_MATCH      "(.*)\\.(md|markdown)$" // Default Markdown file match regex (first group capture is used as topic ID name)
#define DEFAULT_TITLE_MATCH     "^ {0,3}\\# (.*)"       // Default regular expression to extract title from content

#define MAX_LIST_LEVELS         10      // Maximum number of nested list levels (1 = bullet list without children)
#define MAX_HEADER_NUMBER       6       // Maximum header number (h6)
#define MAX_FIRST_LEVEL_SPACES  3       // Maximum spaces at beginning of line for first level of list or header
#define MAX_LEVEL_SPACES        5       // Maximum number of additional spaces at start of line per level
#define MIN_LEVEL_SPACING       2       // Minimum additional spaces for each list level

#define DEFAULT_IMAGES_PATH     "."     // Default data path for images
#define DEFAULT_BULLET_CHARS    "●○■"   // Default bullet characters for level 1, 2, and 3+
#define DEFAULT_HOME_TOPIC      "README"        // Default home topic name ID
#define DEFAULT_ICON_SIZE       24

enum
{
  PROP_0,
  PROP_UI_FILE,
  PROP_IMAGES_PATH,
  PROP_TOPIC_INDEX,
  PROP_HISTORY_POSITION,
  PROP_HISTORY_SIZE,
  PROP_HISTORY_MAX,
  PROP_BULLET_CHARS,
  PROP_HOME_TOPIC
};

// Enum of tags defined in UI file
typedef enum
{
  MARKDOWN_BROWSER_TAG_BOLD,
  MARKDOWN_BROWSER_TAG_ITALIC,
  MARKDOWN_BROWSER_TAG_LINK,
  MARKDOWN_BROWSER_TAG_L1,
  MARKDOWN_BROWSER_TAG_L2,
  MARKDOWN_BROWSER_TAG_L3,
  MARKDOWN_BROWSER_TAG_L4,
  MARKDOWN_BROWSER_TAG_L5,
  MARKDOWN_BROWSER_TAG_L6,
  MARKDOWN_BROWSER_TAG_L7,
  MARKDOWN_BROWSER_TAG_L8,
  MARKDOWN_BROWSER_TAG_L9,
  MARKDOWN_BROWSER_TAG_L10,
  MARKDOWN_BROWSER_TAG_H1,
  MARKDOWN_BROWSER_TAG_H2,
  MARKDOWN_BROWSER_TAG_H3,
  MARKDOWN_BROWSER_TAG_H4,
  MARKDOWN_BROWSER_TAG_H5,
  MARKDOWN_BROWSER_TAG_H6,
  MARKDOWN_BROWSER_TAG_COUNT
} MarkdownBrowserTag;

// Columns for data in topic list store
enum
{
  TOPIC_COLUMN_TITLE,           // Topic title
  TOPIC_COLUMN_INDEX            // Index of topic in topic array
};

typedef struct
{
  GtkBuilder *builder;                  // GTK builder for preferences interface
  GtkTreeSelection *treeSelection;      // Topic tree view selection
  GtkListStore *topicListStore;         // Topic list store
  GtkTextView *textView;                // Content text view
  GtkTextBuffer *textBuffer;            // Content text buffer

  GArray *topics;                   // Array of MarkdownBrowserTopic structures
  GtkTextTag *tags[MARKDOWN_BROWSER_TAG_COUNT];        // Tag array for quick access
  GArray *history;                      // Array of MarkdownBrowserVisit for visit history
  int historyPos;                       // Current history position (next index in history to store to which may be off the end)
  gboolean scrollToLine;                // TRUE to scroll to line in size-request signal of GtkTextView
  guint idleId;                         // Idle callback ID

  char *uiFile;                         // External UI file name (or NULL to use internal data)
  char *imagesPath;                     // Path to images
  char *bulletChars;                    // Bullet characters, last one is used for all following levels
  char *homeTopic;                      // Home topic name or NULL if disabled
  int topicIndex;                       // Current topic index (-1 if none)
  int historyMax;                      // Maximum history size
  gboolean onLink;                      // TRUE when mouse cursor is over link (changed to pointer cursor)
} MarkdownBrowserPrivate;

static void markdown_browser_topic_clear (gpointer data);
static void markdown_browser_finalize (GObject *object);
static void markdown_browser_constructed (GObject *object);
static void markdown_browser_key_press_event (MarkdownBrowser *browser, GdkEventKey *keyEvent, gpointer user_data);
static void markdown_browser_text_view_size_allocate (GtkTextView *textView,
                                               GdkRectangle *rect, MarkdownBrowser *browser);
static gboolean markdown_browser_text_view_motion_notify (GtkTextView *textView,
                                                   GdkEventMotion *motionEvent, MarkdownBrowser *browser);
static gboolean markdown_browser_text_view_leave_notify (GtkTextView *textView,
                                                  GdkEventCrossing *crossingEvent, MarkdownBrowser *browser);
static gboolean markdown_browser_text_view_query_tooltip (GtkTextView *textView, int x, int y, gboolean keyboard_mode,
                                                          GtkTooltip *tooltip, MarkdownBrowser *browser);
static gboolean markdown_browser_text_view_button_press (GtkTextView *textView,
                                                  GdkEventButton *buttonEvent, MarkdownBrowser *browser);
static void markdown_browser_set_property (GObject *object, guint property_id,
                                     const GValue *value, GParamSpec *pspec);
static void markdown_browser_get_property (GObject *object, guint property_id,
                                     GValue *value, GParamSpec *pspec);
static void markdown_browser_topic_selection_changed (GtkTreeSelection *selection, gpointer user_data);
static int markdown_browser_topic_sort (gconstpointer a, gconstpointer b);
static void markdown_browser_render_topic (MarkdownBrowser *browser, MarkdownBrowserTopic *topic);
static gboolean markdown_browser_real_navigate (MarkdownBrowser *browser, int historyOfs, int topicIndex);
static void markdown_browser_back_clicked (GtkWidget *widget, MarkdownBrowser *browser);
static void markdown_browser_forward_clicked (GtkWidget *widget, MarkdownBrowser *browser);
static void markdown_browser_home_clicked (GtkWidget *widget, MarkdownBrowser *browser);
static void markdown_browser_clicker_clicked (GtkWidget *widget, MarkdownBrowser *browser);
static gboolean markdown_browser_topics_update (gpointer data);

G_DEFINE_TYPE_WITH_PRIVATE (MarkdownBrowser, markdown_browser, GTK_TYPE_BOX)

// Tag names for MarkdownBrowserTag enums
const char *markdown_browser_tag_names[MARKDOWN_BROWSER_TAG_COUNT] =
{
  "B",
  "I",
  "A",
  "L1",
  "L2",
  "L3",
  "L4",
  "L5",
  "L6",
  "L7",
  "L8",
  "L9",
  "L10",
  "H1",
  "H2",
  "H3",
  "H4",
  "H5",
  "H6"
};

typedef enum
{
  REGEX_EMPHASIS_START,         // Emphasis (bold/italic) start regular expression
  REGEX_HEADER_START,           // h1-h6 Markdown header
  REGEX_BULLET_ITEM_START,      // Start of a bullet list item
  REGEX_NUMERIC_ITEM_START,     // Start of a numeric list item
  REGEX_IMAGE,                  // Image in the form: ![Alt Text](/images/image.jpg)
  REGEX_LINK,                   // Link in the form: [Alt Text](http://link)

  REGEX_EMPHASIS_END,           // Emphasis end regular expression
  REGEX_HEADER_OR_LIST_ITEM_END // End of header
} MarkdownBrowserRegexEnum;

#define REGEX_COUNT             REGEX_HEADER_OR_LIST_ITEM_END + 1

// Set to the first END tag (only searched if a START tag has matched)
#define REGEX_FIRST_END_MATCH   REGEX_EMPHASIS_END

// Match a string in brackets or parenthesis, including escaped end bracket or parenthesis using Friedl's unrolling-the-loop technique
#define BRACKET_STR             "\\[([^]\\\\]*(?:\\\\.[^]\\\\]*)*)\\]"
#define PARENTH_STR             "\\(([^)\\\\]+(?:\\\\.[^)\\\\]*)*)\\)"

// Markdown parsing regular expressions
// Look behind/ahead is used to not consume surrounding character assertions
// Backslash escape characters are handled here as well, to ensure regex does not match
static const char *regex_strings[REGEX_COUNT] =
{
  "(?<!\\\\)(\\*{1,3})(?![* ])",  // REGEX_EMPHASIS_START
  "^ {0,3}(\\#{1,6}) ",         // REGEX_HEADER_START
  "^( *)\\* ",                  // REGEX_BULLET_ITEM_START
  "^( *)\\d+\\. ",              // REGEX_NUMERIC_ITEM_START
  "(?<!\\\\)!" BRACKET_STR PARENTH_STR,         // REGEX_IMAGE
  "(?<![!\\\\])" BRACKET_STR PARENTH_STR,       // REGEX_LINK

  "(?<![\\\\ ])(\\*{1,3})",     // REGEX_EMPHASIS_END - The look behind causes character preceeding end match to be processed normally
  "(?=\\R)"                     // REGEX_HEADER_OR_LIST_ITEM_END
};

// Characters in Markdown which can be escaped, others will result in the backslash not being considered an escape character
static const char *valid_escaped_chars = "\\`*_{}[]<>()#+-.!|";

// Compiled regular expressions for Markdown matching
static GRegex *regexes[REGEX_COUNT];

static void
markdown_browser_class_init (MarkdownBrowserClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);
  GError *err = NULL;
  int i;

  obj_class->constructed = markdown_browser_constructed;
  obj_class->finalize = markdown_browser_finalize;
  obj_class->set_property = markdown_browser_set_property;
  obj_class->get_property = markdown_browser_get_property;

  g_object_class_install_property (obj_class, PROP_UI_FILE,
    g_param_spec_string ("ui-file", "UiFile", "External .ui user interface file (NULL to use builtin data)",
                         NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (obj_class, PROP_IMAGES_PATH,
    g_param_spec_string ("images-path", "ImagesPath", "Path to image files",
                         DEFAULT_IMAGES_PATH, G_PARAM_READWRITE));
  g_object_class_install_property (obj_class, PROP_TOPIC_INDEX,
    g_param_spec_int ("topic-index", "TopicIndex", "Current topic index (-1 if none)",
                      MARKDOWN_BROWSER_TOPIC_NONE, G_MAXINT, MARKDOWN_BROWSER_TOPIC_NONE, G_PARAM_READWRITE));
  g_object_class_install_property (obj_class, PROP_HISTORY_POSITION,
    g_param_spec_int ("history-position", "HistoryPosition", "Current position to store to in topic visit history (can be after the last index in history array)",
                      0, 100, 0, G_PARAM_READWRITE));
  g_object_class_install_property (obj_class, PROP_HISTORY_SIZE,
    g_param_spec_int ("history-size", "HistorySize", "Current size of topic visit history",
                      0, 100, 0, G_PARAM_READABLE));
  g_object_class_install_property (obj_class, PROP_HISTORY_MAX,
    g_param_spec_int ("history-max", "HistoryMax", "Maximum size of topic visit history",
                      1, 100, DEFAULT_HISTORY_MAX, G_PARAM_READWRITE));
  g_object_class_install_property (obj_class, PROP_BULLET_CHARS,
    g_param_spec_string ("bullet-chars", "BulletChars", "Bullet characters (last one used for remaining levels)",
                         DEFAULT_BULLET_CHARS, G_PARAM_READWRITE));
  g_object_class_install_property (obj_class, PROP_HOME_TOPIC,
    g_param_spec_string ("home-topic", "HomeTopic", "Home topic or NULL to disable",
                         DEFAULT_HOME_TOPIC, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  // Compile regular expressions into GRegex structures
  for (i = 0; i < REGEX_COUNT; i++)
  {
    regexes[i] = g_regex_new (regex_strings[i], G_REGEX_MULTILINE, 0, &err);

    if (!regexes[i])
      g_error ("Invalid regex '%s': %s", regex_strings[i], err->message);
  }
}

static void
markdown_browser_init (MarkdownBrowser *browser)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);

  priv->topics = g_array_new (FALSE, FALSE, sizeof (MarkdownBrowserTopic));
  g_array_set_clear_func (priv->topics, markdown_browser_topic_clear);

  priv->history = g_array_new (FALSE, FALSE, sizeof (MarkdownBrowserVisit));

  priv->imagesPath = g_strdup (DEFAULT_IMAGES_PATH);
  priv->topicIndex = MARKDOWN_BROWSER_TOPIC_NONE;
  priv->historyMax = DEFAULT_HISTORY_MAX;
  priv->bulletChars = g_strdup (DEFAULT_BULLET_CHARS);
  priv->homeTopic = g_strdup (DEFAULT_HOME_TOPIC);
}

static void
markdown_browser_finalize (GObject *object)
{
  MarkdownBrowser *browser = MARKDOWN_BROWSER (object);
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);

  g_array_free (priv->topics, TRUE);
  g_array_free (priv->history, TRUE);
  g_object_unref (priv->builder);               // -- unref builder
  g_free (priv->homeTopic);

  if (priv->idleId)
    g_source_remove (priv->idleId);

  if (G_OBJECT_CLASS (markdown_browser_parent_class)->finalize)
    G_OBJECT_CLASS (markdown_browser_parent_class)->finalize (object);
}

static void
markdown_browser_topic_clear (gpointer data)
{
  MarkdownBrowserTopic *topic = data;

  g_free (topic->name);
  g_free (topic->title);
  g_free (topic->content);
  memset (topic, 0, sizeof (MarkdownBrowserTopic));
}

static void
markdown_browser_constructed (GObject *object)
{
  MarkdownBrowser *browser = MARKDOWN_BROWSER (object);
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  GtkWidget *widg;
  GtkTextTagTable *tagTable;
  int i;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (browser), GTK_ORIENTATION_VERTICAL);

  // Use external Help.ui if ui-file property set, builtin data otherwise
  if (priv->uiFile)
    priv->builder = gtk_builder_new_from_file (priv->uiFile);           // ++ ref builder
  else priv->builder = gtk_builder_new_from_string ((const char *)MarkdownBrowser_ui, MarkdownBrowser_ui_len);

  // Hook up the signals from the UI builder file
  gtk_builder_add_callback_symbols (priv->builder,
                                    "back", G_CALLBACK (markdown_browser_back_clicked),
                                    "forward", G_CALLBACK (markdown_browser_forward_clicked),
                                    "home", G_CALLBACK (markdown_browser_home_clicked),
                                    "clicker", G_CALLBACK (markdown_browser_clicker_clicked),
                                    NULL);
  gtk_builder_connect_signals (priv->builder, browser);

  widg = GTK_WIDGET (gtk_builder_get_object (priv->builder, "HelpToolbar"));
  g_object_ref (widg);                          // ++ ref first child to prevent destruction when changing parent
  gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (widg)), widg);
  gtk_box_pack_start (GTK_BOX (browser), widg, FALSE, FALSE, 0);
  g_object_unref (widg);                        // -- unref first child widget

  widg = GTK_WIDGET (gtk_builder_get_object (priv->builder, "HelpPaned"));
  g_object_ref (widg);                          // ++ ref first child to prevent destruction when changing parent
  gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (widg)), widg);
  gtk_box_pack_start (GTK_BOX (browser), widg, TRUE, TRUE, 0);
  g_object_unref (widg);                        // -- unref first child widget

  g_signal_connect (browser, "key-press-event", G_CALLBACK (markdown_browser_key_press_event), browser);

  priv->topicListStore = GTK_LIST_STORE (gtk_builder_get_object (priv->builder, "TopicListStore"));

  widg = GTK_WIDGET (gtk_builder_get_object (priv->builder, "TopicTreeView"));
  priv->treeSelection = GTK_TREE_SELECTION (gtk_tree_view_get_selection (GTK_TREE_VIEW (widg)));
  g_signal_connect (priv->treeSelection, "changed", G_CALLBACK (markdown_browser_topic_selection_changed), browser);

  priv->textBuffer = GTK_TEXT_BUFFER (gtk_builder_get_object (priv->builder, "HelpTextBuffer"));

  priv->textView = GTK_TEXT_VIEW (gtk_builder_get_object (priv->builder, "HelpTextView"));

  gtk_widget_set_has_tooltip (GTK_WIDGET (priv->textView), TRUE);

  g_signal_connect (priv->textView, "size-allocate", G_CALLBACK (markdown_browser_text_view_size_allocate), browser);
  g_signal_connect (priv->textView, "motion-notify-event", G_CALLBACK (markdown_browser_text_view_motion_notify), browser);
  g_signal_connect (priv->textView, "button-press-event", G_CALLBACK (markdown_browser_text_view_button_press), browser);
  g_signal_connect (priv->textView, "leave-notify-event", G_CALLBACK (markdown_browser_text_view_leave_notify), browser);
  g_signal_connect (priv->textView, "query-tooltip", G_CALLBACK (markdown_browser_text_view_query_tooltip), browser);

  // Hide home button if home topic is NULL
  if (!priv->homeTopic)
    gtk_widget_hide (GTK_WIDGET (gtk_builder_get_object (priv->builder, "HomeBtn")));

  gtk_widget_show (GTK_WIDGET (browser));

  tagTable = GTK_TEXT_TAG_TABLE (gtk_builder_get_object (priv->builder, "TagTable"));

  // Populate tags array
  for (i = 0; i < MARKDOWN_BROWSER_TAG_COUNT; i++)
    priv->tags[i] = gtk_text_tag_table_lookup (tagTable, markdown_browser_tag_names[i]);
}

static void
markdown_browser_key_press_event (MarkdownBrowser *browser, GdkEventKey *keyEvent, gpointer user_data)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);

  if ((keyEvent->state & GDK_MODIFIER_MASK) == GDK_MOD1_MASK)
  {
    if (keyEvent->keyval == GDK_KEY_Left)
      markdown_browser_navigate (browser, -1, 0);
    else if (keyEvent->keyval == GDK_KEY_Right)
      markdown_browser_navigate (browser, 1, 0);
    else if (keyEvent->keyval == GDK_KEY_Home && priv->homeTopic)
      markdown_browser_navigate_to_topic_by_name (browser, priv->homeTopic);
  }
}

// FIXME - This seems like a hack
// Scrolling to a line marker after updating the GtkTextBuffer does not work right.
// Presumably because the idle() update handle for GtkTextBuffer has not yet run.
// Doing this in the GtkTextView size-allocate signal appears to work.
static void
markdown_browser_text_view_size_allocate (GtkTextView *textView, GdkRectangle *rect, MarkdownBrowser *browser)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  GtkTextMark *mark;

  if (priv->scrollToLine)
  {
    mark = gtk_text_buffer_get_mark (priv->textBuffer, "scroll");

    if (mark)
      gtk_text_view_scroll_to_mark (priv->textView, mark, 0.0, TRUE, 0.5, 0.0);

    priv->scrollToLine = TRUE;
  }
}

static gboolean
markdown_browser_text_view_motion_notify (GtkTextView *textView, GdkEventMotion *motionEvent, MarkdownBrowser *browser)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  GtkWidget *widg = (GtkWidget *)textView;
  GdkCursor *cursor;
  GtkTextIter iter;
  gboolean onLink = FALSE;
  int x, y;

  // Convert window coordinates of motion position to buffer coordinates
  gtk_text_view_window_to_buffer_coords (textView, GTK_TEXT_WINDOW_TEXT,
                                         motionEvent->x, motionEvent->y, &x, &y);

  // Get a buffer iterator at motion position and check if cursor on text
  if (gtk_text_view_get_iter_at_location (textView, &iter, x, y))
  { // Is this a link?
    if (gtk_text_iter_has_tag (&iter, priv->tags[MARKDOWN_BROWSER_TAG_LINK]))
      onLink = TRUE;
  }

  // If onLink state changed, change the cursor to/from pointer
  if (onLink != priv->onLink)
  {
    cursor = onLink ? gdk_cursor_new_from_name (gtk_widget_get_display (widg), "pointer") : NULL;
    gdk_window_set_cursor (gtk_text_view_get_window (textView, GTK_TEXT_WINDOW_TEXT), cursor);
    priv->onLink = onLink;

    if (cursor)
      g_object_unref (cursor);
  }

  // If hovering on link, don't let default handles process event (they'll undo our mouse cursor change)
  return priv->onLink ? TRUE : FALSE;
}

static gboolean
markdown_browser_text_view_leave_notify (GtkTextView *textView, GdkEventCrossing *crossingEvent, MarkdownBrowser *browser)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);

  if (priv->onLink)
  {
    gdk_window_set_cursor (gtk_text_view_get_window (textView, GTK_TEXT_WINDOW_TEXT), NULL);
    priv->onLink = FALSE;
  }

  return FALSE;
}

static gboolean
markdown_browser_text_view_query_tooltip (GtkTextView *textView, int x, int y, gboolean keyboard_mode,
                                          GtkTooltip *tooltip, MarkdownBrowser *browser)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  GdkPixbuf *pixbuf;
  GtkTextIter iter;
  GSList *marks, *p;
  char *link;
  int bufx, bufy;
  char *alt;

  // Convert window coordinates to buffer coordinates
  gtk_text_view_window_to_buffer_coords (textView, GTK_TEXT_WINDOW_TEXT, x, y, &bufx, &bufy);

  // Get a buffer iterator at motion position and check if cursor on text
  if (gtk_text_view_get_iter_at_location (textView, &iter, bufx, bufy))
  { // Is this an image?
    if ((pixbuf = gtk_text_iter_get_pixbuf (&iter)) && (alt = g_object_get_data (G_OBJECT (pixbuf), "alt")))
    {
      gtk_tooltip_set_text (tooltip, alt);
      return TRUE;
    }
    else if (gtk_text_iter_has_tag (&iter, priv->tags[MARKDOWN_BROWSER_TAG_LINK]))
    { // If position is already at the beginning of link tag or the beginning of the link tag is found..
      if (gtk_text_iter_starts_tag (&iter, priv->tags[MARKDOWN_BROWSER_TAG_LINK])
          || gtk_text_iter_backward_to_tag_toggle (&iter, priv->tags[MARKDOWN_BROWSER_TAG_LINK]))
      {
        marks = gtk_text_iter_get_marks (&iter);        // ++ allocate list of marks

        for (p = marks; p; p = p->next)
        { // Is this the link mark?
          if ((link = g_object_get_data (G_OBJECT (p->data), "link")))
          {
            if (g_str_has_prefix (link, "http") || g_str_has_prefix (link, "mailto"))
            {
              g_slist_free (marks);                     // -- free list of marks
              gtk_tooltip_set_text (tooltip, link);
              return TRUE;
            }
          }
        }

        g_slist_free (marks);                           // -- free list of marks
      }
    }
  }

  return FALSE;         // Don't show tooltip
}

static gboolean
markdown_browser_text_view_button_press (GtkTextView *textView, GdkEventButton *buttonEvent, MarkdownBrowser *browser)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  GtkWidget *widg = GTK_WIDGET (textView);
  GError *err = NULL;
  GtkTextIter iter;
  GSList *marks, *p;
  char *link;
  int x, y, topic;

  // Convert window coordinates of motion position to buffer coordinates
  gtk_text_view_window_to_buffer_coords (textView, GTK_TEXT_WINDOW_TEXT,
                                         buttonEvent->x, buttonEvent->y, &x, &y);

  // Get a buffer iterator at motion position and check if cursor on text
  if (gtk_text_view_get_iter_at_location (textView, &iter, x, y))
  { // Is this a link?
    if (gtk_text_iter_has_tag (&iter, priv->tags[MARKDOWN_BROWSER_TAG_LINK]))
    { // If position is already at the beginning of link tag or the beginning of the link tag is found..
      if (gtk_text_iter_starts_tag (&iter, priv->tags[MARKDOWN_BROWSER_TAG_LINK])
          || gtk_text_iter_backward_to_tag_toggle (&iter, priv->tags[MARKDOWN_BROWSER_TAG_LINK]))
      {
        marks = gtk_text_iter_get_marks (&iter);        // ++ allocate list of marks

        for (p = marks; p; p = p->next)
        { // Is this the link mark?
          if ((link = g_object_get_data (G_OBJECT (p->data), "link")))
          {
            if (g_str_has_prefix (link, "http") || g_str_has_prefix (link, "mailto"))
            {
              if (!gtk_show_uri_on_window (GTK_WINDOW (gtk_widget_get_ancestor (widg, GTK_TYPE_WINDOW)),
                                           link, GDK_CURRENT_TIME, &err))
              {
                g_warning ("Unable to show URI '%s': %s", link, err->message);
                g_clear_error (&err);
              }
            }
            else if ((topic = markdown_browser_get_topic_by_name (browser, link)) != -1)          // Local URI
              markdown_browser_navigate (browser, 0, topic);

            g_slist_free (marks);                       // -- free list of marks
            return TRUE;        // We handled this event
          }
        }

        g_slist_free (marks);                           // -- free list of marks
      }
    }
  }

  return FALSE;         // Let others handle this event
}

static void
markdown_browser_set_property (GObject *object, guint property_id,
                        const GValue *value, GParamSpec *pspec)
{
  MarkdownBrowser *browser = MARKDOWN_BROWSER (object);
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);

  switch (property_id)
  {
    case PROP_UI_FILE:
      g_free (priv->uiFile);
      priv->uiFile = g_value_dup_string (value);
      break;
    case PROP_IMAGES_PATH:
      g_free (priv->imagesPath);
      priv->imagesPath = g_value_dup_string (value);
      break;
    case PROP_TOPIC_INDEX:
      markdown_browser_real_navigate (browser, 0, g_value_get_int (value));
      break;
    case PROP_HISTORY_POSITION:
      markdown_browser_real_navigate (browser, g_value_get_int (value), MARKDOWN_BROWSER_TOPIC_NONE);
      break;
    case PROP_HISTORY_MAX:
      priv->historyMax = g_value_get_int (value);
      break;
    case PROP_BULLET_CHARS:
      g_free (priv->bulletChars);
      priv->bulletChars = g_value_dup_string (value);
      break;
    case PROP_HOME_TOPIC:
      g_free (priv->homeTopic);
      priv->homeTopic = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
markdown_browser_get_property (GObject *object, guint property_id,
                        GValue *value, GParamSpec *pspec)
{
  MarkdownBrowser *browser = MARKDOWN_BROWSER (object);
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);

  switch (property_id)
  {
    case PROP_UI_FILE:
      g_value_set_string (value, priv->uiFile);
      break;
    case PROP_IMAGES_PATH:
      g_value_set_string (value, priv->imagesPath);
      break;
    case PROP_TOPIC_INDEX:
      g_value_set_uint (value, priv->topicIndex);
      break;
    case PROP_HISTORY_POSITION:
      g_value_set_int (value, priv->historyPos);
      break;
    case PROP_HISTORY_SIZE:
      g_value_set_int (value, priv->history->len);
      break;
    case PROP_HISTORY_MAX:
      g_value_set_int (value, priv->historyMax);
      break;
    case PROP_BULLET_CHARS:
      g_value_set_string (value, priv->bulletChars);
      break;
    case PROP_HOME_TOPIC:
      g_value_set_string (value, priv->homeTopic);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
markdown_browser_topic_selection_changed (GtkTreeSelection *selection, gpointer user_data)
{
  MarkdownBrowser *browser = MARKDOWN_BROWSER (user_data);
  GtkTreeModel *model;
  GtkTreeIter iter;
  guint index;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, TOPIC_COLUMN_INDEX, &index, -1);

    g_signal_handlers_block_by_func (G_OBJECT (selection), markdown_browser_topic_selection_changed, user_data);
    markdown_browser_navigate (browser, 0, index);
    g_signal_handlers_unblock_by_func (G_OBJECT (selection), markdown_browser_topic_selection_changed, user_data);
  }
}

// Markdown parse bag (for passing between functions)
typedef struct
{
  MarkdownBrowserPrivate *priv;
  GtkTextBuffer *textBuf;       // The text content buffer
  GtkTextIter iter;             // GtkTextBuffer append iterator

  GMatchInfo *nextRegexMatch[REGEX_COUNT];      // Match info for next match for each regular expression or NULL if no matches remaining

  gboolean listItem;            // TRUE if currently in a list item
  int listLevel;                // Current list level (0=none)
  gboolean isBullet;            // TRUE if currently appending bullet or number for list item
  guint8 listSpacing[MAX_LIST_LEVELS];          // Number of spaces used for each level
  guint8 numListCounts[MAX_LIST_LEVELS];        // Current counts for numbered list levels ([0] == 0 if unnumbered list or no list active)

  int headerSize;               // Current header size (0 for none)
  gboolean italic;              // True if italic active
  gboolean bold;                // True if bold active
  gboolean link;                // True if link active
} MarkdownBrowserParseBag;

static void
markdown_browser_buffer_append (MarkdownBrowserParseBag *bag, const char *string, int len)
{
  GtkTextIter start;
  char *unescaped = NULL, *dest = NULL;
  const char *src, *prev, *end;
  int startOfs, count;

  if (len == -1)
    len = strlen (string);

  end = string + len;

  // Unescape string (optimize case where no escaping occurs)
  for (src = prev = string; src < end; )
  { // Is this a backslash and next char is a valid character to escape?
    if (*src == '\\' && strchr (valid_escaped_chars, src[1]))
    {
      if (!unescaped)
      {
        unescaped = g_malloc (len + 1);
        dest = unescaped;
      }

      // Any previous string data to copy?
      if (src > prev)
      {
        count = src - prev;
        memcpy (dest, prev, count);
        dest += count;
      }

      prev = src + 1;
      src += 2;
    }
    else src = g_utf8_next_char (src);
  }

  // Did any escaping occur?
  if (unescaped)
  {
    if (src > prev)
    {
      count = src - prev;
      memcpy (dest, prev, count);
      dest += count;
    }

    string = unescaped;
    len = dest - unescaped;
  }

  // Append string to text buffer and get a start iterator to apply tags to
  startOfs = gtk_text_iter_get_offset (&bag->iter);
  gtk_text_buffer_insert (bag->textBuf, &bag->iter, string, len);
  gtk_text_buffer_get_iter_at_offset (bag->textBuf, &start, startOfs);

  // Apply list tag for each level (accumulative)
  if (bag->listItem)
    gtk_text_buffer_apply_tag (bag->textBuf, bag->priv->tags[MARKDOWN_BROWSER_TAG_L1 + bag->listLevel - 1], &start, &bag->iter);

  // Apply header tag
  if (bag->headerSize > 0)
    gtk_text_buffer_apply_tag (bag->textBuf, bag->priv->tags[MARKDOWN_BROWSER_TAG_H1 + bag->headerSize - 1],
                               &start, &bag->iter);
  // Italic
  if (bag->italic)
    gtk_text_buffer_apply_tag (bag->textBuf, bag->priv->tags[MARKDOWN_BROWSER_TAG_ITALIC], &start, &bag->iter);

  // Bold
  if (bag->bold)
    gtk_text_buffer_apply_tag (bag->textBuf, bag->priv->tags[MARKDOWN_BROWSER_TAG_BOLD], &start, &bag->iter);

  // Link
  if (bag->link)
    gtk_text_buffer_apply_tag (bag->textBuf, bag->priv->tags[MARKDOWN_BROWSER_TAG_LINK], &start, &bag->iter);

  if (unescaped)
    g_free (unescaped);
}

static void
markdown_browser_render_topic (MarkdownBrowser *browser, MarkdownBrowserTopic *topic)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  MarkdownBrowserParseBag bag = { 0 };
  GMatchInfo *matchInfo, *nextMatchInfo;
  GtkTextIter endIter;
  GtkTextMark *mark;
  MarkdownBrowserRegexEnum nextRegexEnum;      // Enum of next match
  int startPos, endPos;
  int nextMatchPos;                     // Next position in content of a regular expression match
  const char *content;
  int contentLen;
  int contentPos;               // Current content position
  gboolean newLevel;
  GError *err = NULL;
  MarkdownBrowserRegexEnum regexEnum;
  int spaceCount, count, i;
  char *s;

  // Clear out text buffer contents and get end iterator to append content
  gtk_text_buffer_get_bounds (priv->textBuffer, &bag.iter, &endIter);
  gtk_text_buffer_delete (priv->textBuffer, &bag.iter, &endIter);
  gtk_text_buffer_get_end_iter (priv->textBuffer, &bag.iter);

  if (topic == NULL)
    return;

  bag.priv = priv;
  bag.textBuf = priv->textBuffer;

  content = topic->content;
  contentLen = strlen (content);

  nextMatchPos = contentLen;
  nextMatchInfo = NULL;

  // Initialize nextRegexPosition array with first matches of each regular expression and find first match position
  for (regexEnum = 0; regexEnum < REGEX_FIRST_END_MATCH; regexEnum++)
  {
    if (g_regex_match (regexes[regexEnum], content, 0, &matchInfo))     // ++ allocate match info
    {
      bag.nextRegexMatch[regexEnum] = matchInfo;                        // !! nextRegexMatch array takes over match info
      g_match_info_fetch_pos (matchInfo, 0, &startPos, NULL);

      if (startPos < nextMatchPos)
      {
        nextMatchInfo = matchInfo;
        nextMatchPos = startPos;
        nextRegexEnum = regexEnum;
      }
    }
    else g_match_info_free (matchInfo);         // -- free match info
  }

  // No matches?  Just append the entire content
  if (!nextMatchInfo)
  {
    gtk_text_buffer_insert (priv->textBuffer, &bag.iter, content, contentLen);
    return;
  }

  contentPos = 0;

  // Process content and convert Markdown to GtkTextBuffer
  while (contentPos < contentLen)
  { // Any content prior to next match? - Append it
    if (nextMatchPos > contentPos)
      markdown_browser_buffer_append (&bag, content + contentPos, nextMatchPos - contentPos);

    switch (nextRegexEnum)
    {
      case REGEX_EMPHASIS_START:
        g_match_info_fetch_pos (nextMatchInfo, 1, &startPos, &endPos);         // Get start/end position of emphasis chars
        count = endPos - startPos;
        bag.italic |= (count & 1) ? 1 : 0;      // 1 or 3 * or _ characters enables italic
        bag.bold |= (count & 2) ? 1 : 0;        // 2 or 3 * or _ characters enables bold
        break;
      case REGEX_HEADER_START:
        g_match_info_fetch_pos (nextMatchInfo, 1, &startPos, &endPos);         // Get start/end position of header chars
        bag.headerSize = endPos - startPos;     // Count header characters
        break;
      case REGEX_BULLET_ITEM_START:
      case REGEX_NUMERIC_ITEM_START:
        g_match_info_fetch_pos (nextMatchInfo, 1, &startPos, &endPos);         // Get start/end position of space chars
        spaceCount = endPos - startPos;

        // Too many spaces for first level item?
        if (bag.listLevel == 0 && spaceCount > MAX_FIRST_LEVEL_SPACES)
          break;

        bag.listItem = TRUE;

        if (bag.listLevel > 0)
        { // Loop over levels looking for matching spacing
          for (i = 0; i < bag.listLevel - 1; i++)
            if (spaceCount - bag.listSpacing[i] < bag.listSpacing[i + 1] - spaceCount)
              break;

          newLevel = (i == bag.listLevel - 1 && spaceCount >= bag.listSpacing[i] + MIN_LEVEL_SPACING
                      && bag.listLevel < MAX_LIST_LEVELS);

          if (!newLevel)
            bag.listLevel = i + 1;
        }
        else newLevel = TRUE;

        // Is this a new level?
        if (newLevel)
        {
          bag.listSpacing[bag.listLevel] = spaceCount;
          bag.numListCounts[bag.listLevel] = 0;
          bag.listLevel++;
        }

        bag.isBullet = TRUE;

        // Bullet list?
        if (nextRegexEnum == REGEX_BULLET_ITEM_START)
        { // Append bullet character, taking into account UTF-8 characters
          if (priv->bulletChars && (i = g_utf8_strlen (priv->bulletChars, -1)) > 0)
          {
            if (i > bag.listLevel)
              i = bag.listLevel;

            s = g_utf8_offset_to_pointer (priv->bulletChars, i - 1);
            s = g_strdup_printf ("%.*s ", (int)(g_utf8_find_next_char (s, NULL) - s), s);        // ++ alloc bullet string and space
            markdown_browser_buffer_append (&bag, s, -1);
            g_free (s);         // -- free bullet string
          }
        }
        else    // Numeric list
        {
          bag.numListCounts[bag.listLevel - 1]++;

          s = g_strdup_printf ("%d. ", bag.numListCounts[bag.listLevel - 1]);   // ++ allocate number string
          markdown_browser_buffer_append (&bag, s, -1);
          g_free (s);           // -- free number string
        }

        bag.isBullet = FALSE;

        break;
      case REGEX_IMAGE:
      {
        char *basename, *imageName;
        GdkPixbuf *pixbuf;
        char **fields;
        int size;

        imageName = g_match_info_fetch (nextMatchInfo, 2);      // ++ allocate image name

        // Request for an icon?
        if (g_str_has_prefix (imageName, "icon:"))
        { // Split icon name into fields
          fields = g_strsplit (imageName, ":", -1);     // ++ allocate array of string fields
          count = g_strv_length (fields);

          // Was size provided?
          if (count > 2)
          {
            size = strtol (fields[1], &s, 10);

            if (*s || size < 8 || size > 1024)
              size = DEFAULT_ICON_SIZE;
          }
          else size = DEFAULT_ICON_SIZE;

          // FIXME - Might want to attach to Widget::style-set signal to update icon after theme changes (as per docs for this function)
          pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), fields[count - 1], size, 0, NULL);      // ++ new pixbuf image

          g_strfreev (fields);          // -- free array of string fields
        }
        else
        { // Strip off path component of image name for potential security issue if content not trusted
          basename = g_path_get_basename (imageName);     // ++ allocate basename

          s = g_build_filename (priv->imagesPath, basename, NULL);        // ++ allocate image file name
          g_free (basename);                              // -- free basename

          pixbuf = gdk_pixbuf_new_from_file (s, &err);    // ++ new pixbuf image

          if (!pixbuf)
          {
            g_warning ("Failed to load image file '%s': %s", s, err->message);
            g_clear_error (&err);

            pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "image-missing",  // ++ new pixbuf image
                                               DEFAULT_ICON_SIZE, 0, NULL);
          }

          g_free (s);           // -- free image file name
        }

        g_free (imageName);     // -- free image name

        // Insert the pixbuf and update current iterator
        if (pixbuf)
        { // Add marker for image alt tooltip
          s = g_match_info_fetch (nextMatchInfo, 1);    // ++ allocate image alt text

          if (strlen (s) > 0)
            g_object_set_data_full (G_OBJECT (pixbuf), "alt", s, g_free);
          else g_free (s);      // -- Free empty alt text

          // Insert the pixbuf
          gtk_text_buffer_insert_pixbuf (priv->textBuffer, &bag.iter, pixbuf);
          g_object_unref (pixbuf);              // -- unref pixbuf
        }

        break;
      }
      case REGEX_LINK:
        // Add mark for link URL
        s = g_match_info_fetch (nextMatchInfo, 2);      // ++ allocate link URL
        mark = gtk_text_buffer_create_mark (priv->textBuffer, NULL, &bag.iter, TRUE);
        g_object_set_data_full (G_OBJECT (mark), "link", s, g_free);            // !! Mark object takes over link URL

        bag.link = TRUE;
        s = g_match_info_fetch (nextMatchInfo, 1);      // ++ allocate linked text
        markdown_browser_buffer_append (&bag, s, -1);
        g_free (s);                                     // -- free linked text
        bag.link = FALSE;
        break;
      case REGEX_EMPHASIS_END:
        g_match_info_fetch_pos (nextMatchInfo, 0, &startPos, &endPos);          // Get start/end position of emphasis chars
        count = endPos - startPos;
        bag.italic &= (count & 1) ? 0 : 1;      // 1 or 3 * or _ characters disables italic
        bag.bold &= (count & 2) ? 0 : 1;        // 2 or 3 * or _ characters enables bold
        break;
      case REGEX_HEADER_OR_LIST_ITEM_END:
        bag.headerSize = 0;
        bag.listItem = FALSE;
        break;
    }

    // Skip "matched" portion of last match pattern (does not include look ahead patterns)
    g_match_info_fetch_pos (nextMatchInfo, 0, NULL, &contentPos);

    // Free last processed match and search for next match of the same type
    g_match_info_free (nextMatchInfo);
    nextMatchInfo = NULL;

    if (nextRegexEnum < REGEX_FIRST_END_MATCH)
    {
      if (!g_regex_match_full (regexes[nextRegexEnum], content, contentLen, contentPos, 0, &matchInfo, NULL))
      {
        g_match_info_free (matchInfo);
        matchInfo = NULL;
      }

      bag.nextRegexMatch[nextRegexEnum] = matchInfo;
    }
    else bag.nextRegexMatch[nextRegexEnum] = NULL;     // End tags are checked conditionally below

    // Process next END matches according to current open tags
    for (regexEnum = REGEX_FIRST_END_MATCH; regexEnum < REGEX_COUNT; regexEnum++)
    {
      if (!bag.nextRegexMatch[regexEnum]
          && ((regexEnum == REGEX_EMPHASIS_END && (bag.italic || bag.bold))
              || (regexEnum == REGEX_HEADER_OR_LIST_ITEM_END && (bag.headerSize > 0 || bag.listItem))))
      {
        if (!g_regex_match_full (regexes[regexEnum], content, contentLen, contentPos, 0, &matchInfo, NULL))
        {
          g_match_info_free (matchInfo);
          matchInfo = NULL;
        }

        bag.nextRegexMatch[regexEnum] = matchInfo;
      }
    }

    nextMatchPos = contentLen;

    // Find next tag
    for (regexEnum = 0; regexEnum < REGEX_COUNT; regexEnum++)
    {
      if (bag.nextRegexMatch[regexEnum])
      {
        g_match_info_fetch_pos (bag.nextRegexMatch[regexEnum], 0, &startPos, NULL);

        if (startPos < nextMatchPos)
        {
          nextMatchInfo = bag.nextRegexMatch[regexEnum];
          nextMatchPos = startPos;
          nextRegexEnum = regexEnum;
        }
      }
    }

    // No more tags?
    if (!nextMatchInfo)
    {
      if (contentPos < contentLen)
        markdown_browser_buffer_append (&bag, content + contentPos, contentLen - contentPos);

      break;
    }

    // If list is active and end of list item, but this is not the start of another list item, deactivate list
    if (bag.listLevel > 0 && !bag.listItem && nextRegexEnum != REGEX_BULLET_ITEM_START
        && nextRegexEnum != REGEX_NUMERIC_ITEM_START)
      bag.listLevel = 0;
  }     // for content
}

/**
 * markdown_browser_navigate:
 * @browser: Markdown browser
 * @historyOfs: History position offset (0: go to new @topicIndex, -1: back 1 in history, 1: forward 1 in history, etc)
 * @topicIndex: Topic index to go to (if @historyOfs == 0)
 *
 * Navigate to new topic or forward/back in history.
 */
void
markdown_browser_navigate (MarkdownBrowser *browser, int historyOfs, int topicIndex)
{
  if (markdown_browser_real_navigate (browser, historyOfs, topicIndex))
    g_object_notify (G_OBJECT (browser), "topic-index");
}

static gboolean
markdown_browser_real_navigate (MarkdownBrowser *browser, int historyOfs, int topicIndex)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  MarkdownBrowserTopic *topic;
  MarkdownBrowserVisit *visit;
  GtkTreeIter treeIter;
  GtkTextIter textIter;
  GdkRectangle rect;
  int newHistoryPos;

  newHistoryPos = priv->historyPos + historyOfs;

  // Ignore invalid history offset or topicIndex values
  if ((historyOfs != 0 && (newHistoryPos < 0 || newHistoryPos >= priv->history->len))
      || (historyOfs == 0 && ((topicIndex < 0 && topicIndex != MARKDOWN_BROWSER_TOPIC_NONE)
                              || topicIndex >= priv->topics->len)))
    return FALSE;

  // Save current topic state in history if set
  if (priv->topicIndex != MARKDOWN_BROWSER_TOPIC_NONE)
  {
    if (priv->historyPos < priv->history->len)
    { // Overwrite the entry at the current position
      visit = &g_array_index (priv->history, MarkdownBrowserVisit, priv->historyPos);

      // Go to operation truncates forward history
      if (historyOfs == 0)
        g_array_set_size (priv->history, priv->historyPos + 1);
    }
    else        // Append entry on end of history
    {
      g_array_set_size (priv->history, priv->history->len + 1);
      visit = &g_array_index (priv->history, MarkdownBrowserVisit, priv->history->len - 1);
    }

    visit->topic = priv->topicIndex;

    gtk_text_view_get_visible_rect (priv->textView, &rect);
    gtk_text_view_get_iter_at_location (priv->textView, &textIter, 0, rect.y);
    visit->line = gtk_text_iter_get_line (&textIter);

    // Remove old entries if history size exceeded
    if (priv->history->len > priv->historyMax)
    {
      newHistoryPos -= priv->history->len - priv->historyMax;          // Adjust new history position after deletion
      g_array_remove_range (priv->history, 0, priv->history->len - priv->historyMax);

      // Could happen..  Original history being navigated to is now gone..  Just stay where we are (adjust for deletion)
      if (newHistoryPos < 0)
      {
        priv->historyPos -= priv->historyMax - priv->history->len;
        return FALSE;
      }
    }
  }

  // A go forward or back operation?
  if (historyOfs != 0)
  {
    priv->historyPos = newHistoryPos;
    visit = &g_array_index (priv->history, MarkdownBrowserVisit, priv->historyPos);
    topicIndex = visit->topic;
  }
  else priv->historyPos = priv->history->len;   // Go to operation, set history position to after end of array

  priv->topicIndex = topicIndex;
  topic = topicIndex >= 0 ? &g_array_index (priv->topics, MarkdownBrowserTopic, topicIndex) : NULL;

  // Render the Markdown topic content to the GtkTextBuffer
  markdown_browser_render_topic (browser, topic);

  // Restore vertical position if this is a forward/back operation
  if (historyOfs != 0)
  { // Add a marker and scroll to it, supposedly this is the most reliable method to scroll in a text view
    // FIXME - Unfortunately attempting to scroll at this point does not work, we try later in the size-allocate signal of GtkTextView
    visit = &g_array_index (priv->history, MarkdownBrowserVisit, priv->historyPos);
    gtk_text_buffer_get_iter_at_line (priv->textBuffer, &textIter, visit->line);
    gtk_text_buffer_create_mark (priv->textBuffer, "scroll", &textIter, TRUE);
  }

  priv->scrollToLine = historyOfs != 0;

  // Update topic tree selection
  if (topicIndex >= 0)
  {
    g_signal_handlers_block_by_func (priv->treeSelection, markdown_browser_topic_selection_changed, browser);
    gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (priv->topicListStore), &treeIter, NULL, topicIndex);       // Get the nth node in list (parent == NULL)
    gtk_tree_selection_select_iter (priv->treeSelection, &treeIter);
    g_signal_handlers_unblock_by_func (priv->treeSelection, markdown_browser_topic_selection_changed, browser);
  }

  return TRUE;
}

/**
 * markdown_browser_navigate_to_topic_by_name:
 * @browser: Markdown browser
 * @name: Topic name or NULL to unset
 *
 * Navigate to a topic by name ID.
 *
 * Returns: TRUE on success, FALSE if a topic by @name was not found
 */
gboolean
markdown_browser_navigate_to_topic_by_name (MarkdownBrowser *browser, const char *name)
{
  int index;

  g_return_val_if_fail (IS_MARKDOWN_BROWSER (browser), FALSE);

  if (name)
  {
    index = markdown_browser_get_topic_by_name (browser, name);

    if (index == MARKDOWN_BROWSER_TOPIC_NONE)
      return FALSE;
  }
  else index = MARKDOWN_BROWSER_TOPIC_NONE;

  markdown_browser_navigate (browser, 0, index);

  return TRUE;
}

static void
markdown_browser_back_clicked (GtkWidget *widget, MarkdownBrowser *browser)
{
  markdown_browser_navigate (browser, -1, 0);
}

static void
markdown_browser_forward_clicked (GtkWidget *widget, MarkdownBrowser *browser)
{
  markdown_browser_navigate (browser, 1, 0);
}

static void
markdown_browser_home_clicked (GtkWidget *widget, MarkdownBrowser *browser)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);

  if (priv->homeTopic)
    markdown_browser_navigate_to_topic_by_name (browser, priv->homeTopic);
}

static void
markdown_browser_clicker_clicked (GtkWidget *widget, MarkdownBrowser *browser)
{
  // FIXME - Implement click for topic feature
}

/**
 * markdown_browser_new:
 * @uiFile: (optional): Optional external user interface (.ui) file to use or NULL to use default builtin data
 *
 * Create a new markdown browser widget.
 * 
 * Returns: New browser widget
 */
GtkWidget *
markdown_browser_new (const char *uiFile)
{
  return GTK_WIDGET (g_object_new (TYPE_MARKDOWN_BROWSER, "ui-file", uiFile, NULL));
}

/**
 * markdown_browser_get_topic_by_name:
 * @browser: Markdown browser
 * @name: Name of topic
 *
 * Get topic index by name ID.
 *
 * Returns: Topic index or -1 if not found
 */
int
markdown_browser_get_topic_by_name (MarkdownBrowser *browser, const char *name)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  int i;

  g_return_val_if_fail (IS_MARKDOWN_BROWSER (browser), -1);

  if (name)
  {
    for (i = 0; i < priv->topics->len; i++)
      if (strcmp (name, g_array_index (priv->topics, MarkdownBrowserTopic, i).name) == 0)
        return i;
  }

  return -1;
}

/**
 * markdown_browser_get_topics:
 * @browser: Markdown browser
 * @count: Location to store length of returned topic array
 *
 * Get array of topics in a markdown browser widget.
 *
 * Returns: (transfer-none): Array of topic information which is internal to @browser
 *   and is only valid for the widget lifetime and provided the topic data is not changed.
 */
MarkdownBrowserTopic *
markdown_browser_get_topics (MarkdownBrowser *browser, guint *count)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);

  g_return_val_if_fail (IS_MARKDOWN_BROWSER (browser), NULL);
  g_return_val_if_fail (count != NULL, NULL);

  *count = priv->topics->len;
  return (MarkdownBrowserTopic *)(priv->topics->data);
}

/**
 * markdown_browser_get_history:
 * @browser: Markdown browser
 * @count: Location to store length of returned history visit array
 *
 * Get array of topic history visits in a markdown browser widget.
 *
 * Returns: (transfer-none): Array of topic visit history information which is internal to @browser
 *   and is only valid for the widget lifetime and provided the topic data is not changed.
 */
MarkdownBrowserVisit *
markdown_browser_get_history (MarkdownBrowser *browser, guint *count)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);

  g_return_val_if_fail (IS_MARKDOWN_BROWSER (browser), NULL);
  g_return_val_if_fail (count != NULL, NULL);

  *count = priv->history->len;
  return (MarkdownBrowserVisit *)(priv->history->data);
}

/**
 * markdown_browser_add_topic:
 * @browser: Markdown browser
 * @name: Name identifier of the topic
 * @title: Title of the topic
 * @content: Markdown content of the topic
 *
 * Add a single topic to a browser widget.
 */
void
markdown_browser_add_topic (MarkdownBrowser *browser, const char *name, const char *title, const char *content)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  MarkdownBrowserTopic *topic;

  g_return_if_fail (IS_MARKDOWN_BROWSER (browser));

  g_array_set_size (priv->topics, priv->topics->len + 1);
  topic = &g_array_index (priv->topics, MarkdownBrowserTopic, priv->topics->len - 1);
  topic->name = g_strdup (name);
  topic->title = g_strdup (title);
  topic->content = g_strdup (content);

  // Do the topic update in an idle function for optimization purposes with multiple topic adds
  if (!priv->idleId)
    priv->idleId = g_idle_add (markdown_browser_topics_update, browser);
}

/**
 * markdown_browser_add_files:
 * @browser: Markdown browser
 * @path: Path to add markdown content files from
 * @fileMatch: File matching Perl compatible regular expression
 *   (NULL for default of "(.*)\\.(md|markdown)$", group capture is used as topic name)
 * @titleMatch: Perl compatible regular expression to extract title from file content
 *   (NULL for default of "^ {0,3}\# (.*)" for h1 header), group capture is the title text
 *
 * Add markdown files in a directory (not recursive).
 */
gboolean
markdown_browser_add_files (MarkdownBrowser *browser, const char *path, const char *fileMatch,
                     const char *titleMatch, GError **err)
{
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  GRegex *fileRegex, *titleRegex;
  GMatchInfo *fileMatchInfo, *titleMatchInfo;
  char *fullpath, *name, *title, *content;
  const char *filename;
  GError *local_err = NULL;
  GDir *dir;

  g_return_val_if_fail (IS_MARKDOWN_BROWSER (browser), FALSE);
  g_return_val_if_fail (path != NULL, FALSE);
  g_return_val_if_fail (!err || !*err, FALSE);

  if (!(dir = g_dir_open (path, 0, err)))       // ++ open GDir
    return FALSE;

  if (!(fileRegex = g_regex_new (fileMatch ? fileMatch : DEFAULT_FILE_MATCH, 0, 0, err)))       // ++ new fileRegex
  {
    g_dir_close (dir);          // -- close GDir
    return FALSE;
  }

  if (!(titleRegex = g_regex_new (titleMatch ? titleMatch : DEFAULT_TITLE_MATCH,        // ++ new titleRegex
                                  G_REGEX_MULTILINE, 0, err)))
  {
    g_regex_unref (fileRegex);  // -- unref fileRegex
    g_dir_close (dir);          // -- close GDir
    return FALSE;
  }

  // Loop over files
  while ((filename = g_dir_read_name (dir)))
  {
    if (g_regex_match (fileRegex, filename, 0, &fileMatchInfo)) // ++ allocate file
    {
      fullpath = g_build_filename (path, filename, NULL);       // ++ alloc full path

      if (g_file_get_contents (fullpath, &content, NULL, &local_err))   // ++ allocate content
      {
        name = g_match_info_fetch (fileMatchInfo, 1);           // ++ alloc topic name in file match

        if (g_regex_match (titleRegex, content, 0, &titleMatchInfo))    // ++ allocate title match info
          title = g_match_info_fetch (titleMatchInfo, 1);       // ++ allocate title
        else title = NULL;

        g_match_info_free (titleMatchInfo);                     // -- free title match info

        markdown_browser_add_topic (browser, name, title, content);
        g_free (name);          // -- free name
        g_free (title);         // -- free title
        g_free (content);       // -- free content
      }
      else
      {
        g_warning ("Failed to load markdown file '%s': %s", fullpath, local_err->message);
        g_clear_error (&local_err);
      }

      g_free (fullpath);        // -- free full path
    }

    g_match_info_free (fileMatchInfo);  // -- free file match info
  }

  g_regex_unref (titleRegex);   // -- unref titleRegex
  g_regex_unref (fileRegex);    // -- unref fileRegex
  g_dir_close (dir);            // -- close GDir

  // Sort topics alphabetically
  g_array_sort (priv->topics, markdown_browser_topic_sort);

  // Reset selected topic and history which is no longer valid
  g_array_set_size (priv->history, 0);
  priv->historyPos = 0;
  priv->topicIndex = -1;

  // Do the topic update in an idle function for optimization purposes with multiple topic adds
  if (!priv->idleId)
    priv->idleId = g_idle_add (markdown_browser_topics_update, browser);

  return TRUE;
}

static int
markdown_browser_topic_sort (gconstpointer a, gconstpointer b)
{
  const MarkdownBrowserTopic *atopic = a, *btopic = b;
  return strcmp (atopic->name, btopic->name);
}

// Idle callback to update topics tree view and select home topic (if no topic selected)
static gboolean
markdown_browser_topics_update (gpointer data)
{
  MarkdownBrowser *browser = MARKDOWN_BROWSER (data);
  MarkdownBrowserPrivate *priv = markdown_browser_get_instance_private (browser);
  MarkdownBrowserTopic *topic;
  GtkTreeIter iter;
  int i;

  gtk_list_store_clear (priv->topicListStore);

  // Populate topic list store
  for (i = priv->topics->len - 1; i >= 0; i--)
  {
    topic = &g_array_index (priv->topics, MarkdownBrowserTopic, i);
    gtk_list_store_insert (priv->topicListStore, &iter, 0);

    gtk_list_store_set (priv->topicListStore, &iter,
                        TOPIC_COLUMN_TITLE, topic->title,
                        TOPIC_COLUMN_INDEX, i,
                        -1);
  }

  if (priv->topicIndex == MARKDOWN_BROWSER_TOPIC_NONE && priv->homeTopic)
    markdown_browser_navigate_to_topic_by_name (browser, priv->homeTopic);

  // Clear idle callback ID and return FALSE to remove idle
  priv->idleId = 0;
  return FALSE;
}

