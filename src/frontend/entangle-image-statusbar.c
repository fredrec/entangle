/*
 *  Entangle: Tethered Camera Control & Capture
 *
 *  Copyright (C) 2009-2015 Daniel P. Berrange
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <config.h>

#include <string.h>

#include "entangle-debug.h"
#include "entangle-image-statusbar.h"
#include "entangle-image.h"

#define ENTANGLE_IMAGE_STATUSBAR_GET_PRIVATE(obj)                       \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj), ENTANGLE_TYPE_IMAGE_STATUSBAR, EntangleImageStatusbarPrivate))

struct _EntangleImageStatusbarPrivate {
    gulong imageNotifyID;
    EntangleImage *image;

    GtkWidget *metaBox;

    GtkWidget *metaAperture;
    GtkWidget *metaShutter;
    GtkWidget *metaIso;
    GtkWidget *metaFocal;
    GtkWidget *metaDimensions;
};

G_DEFINE_TYPE(EntangleImageStatusbar, entangle_image_statusbar, GTK_TYPE_EVENT_BOX);

enum {
    PROP_O,
    PROP_IMAGE,
};


static void entangle_image_statusbar_get_property(GObject *object,
                                                  guint prop_id,
                                                  GValue *value,
                                                  GParamSpec *pspec)
{
    EntangleImageStatusbar *statusbar = ENTANGLE_IMAGE_STATUSBAR(object);
    EntangleImageStatusbarPrivate *priv = statusbar->priv;

    switch (prop_id)
        {
        case PROP_IMAGE:
            g_value_set_object(value, priv->image);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        }
}


static void entangle_image_statusbar_set_property(GObject *object,
                                                  guint prop_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec)
{
    EntangleImageStatusbar *statusbar = ENTANGLE_IMAGE_STATUSBAR(object);

    ENTANGLE_DEBUG("Set prop on image statusbar %d", prop_id);

    switch (prop_id)
        {
        case PROP_IMAGE:
            entangle_image_statusbar_set_image(statusbar, g_value_get_object(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        }
}


static void entangle_image_statusbar_finalize(GObject *object)
{
    EntangleImageStatusbar *statusbar = ENTANGLE_IMAGE_STATUSBAR(object);
    EntangleImageStatusbarPrivate *priv = statusbar->priv;

    if (priv->image) {
        g_signal_handler_disconnect(priv->image, priv->imageNotifyID);
        g_object_unref(priv->image);
    }

    G_OBJECT_CLASS(entangle_image_statusbar_parent_class)->finalize(object);
}


static void entangle_image_statusbar_class_init(EntangleImageStatusbarClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = entangle_image_statusbar_finalize;
    object_class->get_property = entangle_image_statusbar_get_property;
    object_class->set_property = entangle_image_statusbar_set_property;

    g_object_class_install_property(object_class,
                                    PROP_IMAGE,
                                    g_param_spec_object("image",
                                                        "Image",
                                                        "Image",
                                                        ENTANGLE_TYPE_IMAGE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_BLURB));

    g_type_class_add_private(klass, sizeof(EntangleImageStatusbarPrivate));
}


EntangleImageStatusbar *entangle_image_statusbar_new(void)
{
    return ENTANGLE_IMAGE_STATUSBAR(g_object_new(ENTANGLE_TYPE_IMAGE_STATUSBAR, NULL));
}


static void entangle_image_statusbar_init(EntangleImageStatusbar *statusbar)
{
    g_return_if_fail(ENTANGLE_IS_IMAGE_STATUSBAR(statusbar));

    EntangleImageStatusbarPrivate *priv;
    GdkRGBA white = { 1.0, 1.0, 1.0, 1.0 };
    GdkRGBA black = { 0.0, 0.0, 0.0, 1.0 };

    priv = statusbar->priv = ENTANGLE_IMAGE_STATUSBAR_GET_PRIVATE(statusbar);

    priv->metaBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    g_object_set(priv->metaBox, "homogeneous", TRUE, NULL);

    gtk_container_set_border_width(GTK_CONTAINER(priv->metaBox), 6);

    priv->metaAperture = gtk_label_new("");
    priv->metaShutter = gtk_label_new("");
    priv->metaFocal = gtk_label_new("");
    priv->metaIso = gtk_label_new("");
    priv->metaDimensions = gtk_label_new("");

    gtk_box_pack_start(GTK_BOX(priv->metaBox), priv->metaAperture, TRUE, TRUE, 6);
    gtk_box_pack_start(GTK_BOX(priv->metaBox), priv->metaShutter, TRUE, TRUE, 6);
    gtk_box_pack_start(GTK_BOX(priv->metaBox), priv->metaFocal, TRUE, TRUE, 6);
    gtk_box_pack_start(GTK_BOX(priv->metaBox), priv->metaIso, TRUE, TRUE, 6);
    gtk_box_pack_start(GTK_BOX(priv->metaBox), priv->metaDimensions, TRUE, TRUE, 6);

    gtk_container_add(GTK_CONTAINER(statusbar), priv->metaBox);

    gtk_widget_override_background_color(GTK_WIDGET(statusbar), GTK_STATE_FLAG_NORMAL, &black);

    gtk_widget_override_color(GTK_WIDGET(priv->metaAperture), GTK_STATE_FLAG_NORMAL, &white);
    gtk_widget_override_color(GTK_WIDGET(priv->metaShutter), GTK_STATE_FLAG_NORMAL, &white);
    gtk_widget_override_color(GTK_WIDGET(priv->metaFocal), GTK_STATE_FLAG_NORMAL, &white);
    gtk_widget_override_color(GTK_WIDGET(priv->metaIso), GTK_STATE_FLAG_NORMAL, &white);
    gtk_widget_override_color(GTK_WIDGET(priv->metaDimensions), GTK_STATE_FLAG_NORMAL, &white);

    gtk_widget_show_all(GTK_WIDGET(statusbar));
}


static void entangle_image_statusbar_update_labels(EntangleImageStatusbar *statusbar)
{
    g_return_if_fail(ENTANGLE_IS_IMAGE_STATUSBAR(statusbar));

    EntangleImageStatusbarPrivate *priv = statusbar->priv;
    GExiv2Metadata *metadata = entangle_image_get_metadata(priv->image);
    gchar *shutter = NULL;
    gchar *aperture = NULL;
    gchar *focal = NULL;
    gchar *iso = NULL;
    gchar *dimensions = NULL;

    if (metadata) {
        gint nom, den;
        gint fnumn, fnumd;
        gdouble fnum;
        guint isonum;
        gdouble focalnum;

        if (gexiv2_metadata_has_tag(metadata, "Exif.Photo.ExposureTime")) {
            gexiv2_metadata_get_exif_tag_rational(metadata, "Exif.Photo.ExposureTime", &nom, &den);
            if (den == 10)
                shutter = g_strdup_printf("%0.1lf secs", (double)nom/10.0);
            else if (nom == 10)
                shutter = g_strdup_printf("1/%0.0lf secs", (double)den/10.0);
            else
                shutter = g_strdup_printf("%d/%d secs", nom, den);
        }

        if ((gexiv2_metadata_has_tag(metadata, "Exif.Photo.FNumber") &&
             gexiv2_metadata_get_exif_tag_rational(metadata, "Exif.Photo.FNumber", &fnumn, &fnumd)) ||
            (gexiv2_metadata_has_tag(metadata, "Exif.Photo.Aperture") &&
             gexiv2_metadata_get_exif_tag_rational(metadata, "Exif.Photo.Aperture", &fnumn, &fnumd))) {
            fnum = (double)fnumn/(double)fnumd;
            if (fnum < 10.0)
                aperture = g_strdup_printf("f/%1.1f", fnum);
            else
                aperture = g_strdup_printf("f/%2.0f", fnum);
        }

        if (gexiv2_metadata_has_tag(metadata, "Exif.Photo.ISOSpeedRatings")) {
            isonum = gexiv2_metadata_get_iso_speed(metadata);
            iso = g_strdup_printf("ISO %d", isonum);
        }

        if (gexiv2_metadata_has_tag(metadata, "Exif.Photo.FocalLength")) {
            gexiv2_metadata_get_exif_tag_rational(metadata, "Exif.Photo.FocalLength", &nom, &den);
            if (den > 0.0001) {
                focalnum = (nom / den);
                focal = g_strdup_printf("%0.0lf mm", focalnum);
            }
        }

        dimensions = g_strdup_printf("%d x %d",
                                     gexiv2_metadata_get_pixel_width(metadata),
                                     gexiv2_metadata_get_pixel_height(metadata));
    }

    gtk_label_set_text(GTK_LABEL(priv->metaShutter), shutter ? shutter : "");
    gtk_label_set_text(GTK_LABEL(priv->metaAperture), aperture ? aperture : "");
    gtk_label_set_text(GTK_LABEL(priv->metaFocal), focal ? focal : "");
    gtk_label_set_text(GTK_LABEL(priv->metaIso), iso ? iso : "");
    gtk_label_set_text(GTK_LABEL(priv->metaDimensions), dimensions ? dimensions : "");

    g_free(shutter);
    g_free(aperture);
    g_free(focal);
    g_free(iso);
    g_free(dimensions);
}


static void entangle_image_statusbar_image_metadata_notify(GObject *image G_GNUC_UNUSED,
                                                           GParamSpec *pspec G_GNUC_UNUSED,
                                                           gpointer data)
{
    g_return_if_fail(ENTANGLE_IS_IMAGE_STATUSBAR(data));

    EntangleImageStatusbar *statusbar = ENTANGLE_IMAGE_STATUSBAR(data);

    entangle_image_statusbar_update_labels(statusbar);
}


/**
 * entangle_image_statusbar_set_image:
 * @statusbar: (transfer none): the status bar widget
 * @image: (transfer none)(allow-none): the image to display status for, or NULL
 *
 * Set the image to display status information for
 */
void entangle_image_statusbar_set_image(EntangleImageStatusbar *statusbar,
                                        EntangleImage *image)
{
    g_return_if_fail(ENTANGLE_IS_IMAGE_STATUSBAR(statusbar));
    g_return_if_fail(!image || ENTANGLE_IS_IMAGE(image));

    EntangleImageStatusbarPrivate *priv = statusbar->priv;

    if (priv->image) {
        g_signal_handler_disconnect(priv->image, priv->imageNotifyID);
        g_object_unref(priv->image);
    }
    priv->image = image;
    if (priv->image) {
        g_object_ref(priv->image);
        priv->imageNotifyID = g_signal_connect(priv->image,
                                               "notify::metadata",
                                               G_CALLBACK(entangle_image_statusbar_image_metadata_notify),
                                               statusbar);
    }

    gtk_widget_queue_draw(GTK_WIDGET(statusbar));
}


/**
 * entangle_image_statusbar_get_image:
 * @statusbar: (transfer none): the status bar widget
 *
 * Retrieve the image that status is currently displayed for
 *
 * Returns: (transfer none): the image displayed
 */
EntangleImage *entangle_image_statusbar_get_image(EntangleImageStatusbar *statusbar)
{
    g_return_val_if_fail(ENTANGLE_IS_IMAGE_STATUSBAR(statusbar), NULL);

    EntangleImageStatusbarPrivate *priv = statusbar->priv;

    return priv->image;
}

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  indent-tabs-mode: nil
 *  tab-width: 8
 * End:
 */
