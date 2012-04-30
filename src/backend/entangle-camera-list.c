/*
 *  Entangle: Tethered Camera Control & Capture
 *
 *  Copyright (C) 2009-2012 Daniel P. Berrange
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

#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <string.h>
#include <gphoto2.h>

#include "entangle-debug.h"
#include "entangle-camera-list.h"
#include "entangle-device-manager.h"

#define ENTANGLE_CAMERA_LIST_GET_PRIVATE(obj)                               \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj), ENTANGLE_TYPE_CAMERA_LIST, EntangleCameraListPrivate))

struct _EntangleCameraListPrivate {
    size_t ncamera;
    EntangleCamera **cameras;

    EntangleDeviceManager *devManager;

    GPContext *ctx;
    CameraAbilitiesList *caps;
    GPPortInfoList *ports;
};

G_DEFINE_TYPE(EntangleCameraList, entangle_camera_list, G_TYPE_OBJECT);

enum {
    PROP_0,
    PROP_DEVMANAGER,
};


static void entangle_camera_list_get_property(GObject *object,
                                              guint prop_id,
                                              GValue *value,
                                              GParamSpec *pspec)
{
    EntangleCameraList *list = ENTANGLE_CAMERA_LIST(object);
    EntangleCameraListPrivate *priv = list->priv;

    switch (prop_id) {
    case PROP_DEVMANAGER:
        g_value_set_object(value, priv->devManager);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void entangle_camera_list_set_property(GObject *object,
                                              guint prop_id,
                                              const GValue *value,
                                              GParamSpec *pspec)
{
    EntangleCameraList *list = ENTANGLE_CAMERA_LIST(object);
    EntangleCameraListPrivate *priv = list->priv;

    switch (prop_id) {
    case PROP_DEVMANAGER:
        if (priv->devManager)
            g_object_unref(priv->devManager);
        priv->devManager = g_value_get_object(value);
        g_object_ref(priv->devManager);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}


static void entangle_camera_list_finalize (GObject *object)
{
    EntangleCameraList *list = ENTANGLE_CAMERA_LIST(object);
    EntangleCameraListPrivate *priv = list->priv;
    ENTANGLE_DEBUG("Finalize list");

    for (int i = 0 ; i < priv->ncamera ; i++) {
        ENTANGLE_DEBUG("Unref camera in list %p", priv->cameras[i]);
        g_object_unref(priv->cameras[i]);
    }
    g_free(priv->cameras);

    if (priv->devManager)
        g_object_unref(priv->devManager);

    if (priv->ports)
        gp_port_info_list_free(priv->ports);
    if (priv->caps)
        gp_abilities_list_free(priv->caps);
    gp_context_unref(priv->ctx);

    G_OBJECT_CLASS (entangle_camera_list_parent_class)->finalize (object);
}


static void entangle_camera_list_udev_event(EntangleDeviceManager *manager G_GNUC_UNUSED,
                                            char *port G_GNUC_UNUSED,
                                            gpointer opaque)
{
    EntangleCameraList *list = ENTANGLE_CAMERA_LIST(opaque);

    if (!entangle_camera_list_refresh(list, NULL)) {
        ENTANGLE_DEBUG("Failed to refresh cameras after device hotplug/unplug");
    }
}


static void entangle_camera_list_gphoto_log(GPLogLevel level G_GNUC_UNUSED,
                                            const char *domain,
                                            const char *format,
                                            va_list args,
                                            void *data G_GNUC_UNUSED)
{
    char *msg = g_strdup_vprintf(format, args);
    g_debug("%s: %s", domain, msg);
}


static void entangle_camera_list_class_init(EntangleCameraListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = entangle_camera_list_finalize;
    object_class->get_property = entangle_camera_list_get_property;
    object_class->set_property = entangle_camera_list_set_property;

    g_signal_new("camera-added",
                 G_TYPE_FROM_CLASS(klass),
                 G_SIGNAL_RUN_FIRST,
                 G_STRUCT_OFFSET(EntangleCameraListClass, camera_added),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__OBJECT,
                 G_TYPE_NONE,
                 1,
                 ENTANGLE_TYPE_CAMERA);

    g_signal_new("camera-removed",
                 G_TYPE_FROM_CLASS(klass),
                 G_SIGNAL_RUN_FIRST,
                 G_STRUCT_OFFSET(EntangleCameraListClass, camera_removed),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__OBJECT,
                 G_TYPE_NONE,
                 1,
                 ENTANGLE_TYPE_CAMERA);

    g_object_class_install_property(object_class,
                                    PROP_DEVMANAGER,
                                    g_param_spec_object("device-manager",
                                                        "Device manager",
                                                        "Device manager for detecting cameras",
                                                        ENTANGLE_TYPE_DEVICE_MANAGER,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_BLURB));

    g_type_class_add_private(klass, sizeof(EntangleCameraListPrivate));
}


EntangleCameraList *entangle_camera_list_new(void)
{
    return ENTANGLE_CAMERA_LIST(g_object_new(ENTANGLE_TYPE_CAMERA_LIST, NULL));
}


static void entangle_camera_list_init(EntangleCameraList *list)
{
    EntangleCameraListPrivate *priv;

    priv = list->priv = ENTANGLE_CAMERA_LIST_GET_PRIVATE(list);

    if (entangle_debug_gphoto)
        gp_log_add_func(GP_LOG_DEBUG,
                        entangle_camera_list_gphoto_log,
                        NULL);

    priv->ctx = gp_context_new();

    priv->devManager = entangle_device_manager_new();

    if (gp_abilities_list_new(&priv->caps) != GP_OK)
        g_error(_("Cannot initialize gphoto2 abilities"));

    if (gp_abilities_list_load(priv->caps, priv->ctx) != GP_OK)
        g_error(_("Cannot load gphoto2 abilities"));

    if (gp_port_info_list_new(&priv->ports) != GP_OK)
        g_error(_("Cannot initialize gphoto2 ports"));

    if (gp_port_info_list_load(priv->ports) != GP_OK)
        g_error(_("Cannot load gphoto2 ports"));

    g_signal_connect(priv->devManager, "device-added",
                     G_CALLBACK(entangle_camera_list_udev_event), list);
    g_signal_connect(priv->devManager, "device-removed",
                     G_CALLBACK(entangle_camera_list_udev_event), list);

    entangle_camera_list_refresh(list, NULL);
}

gboolean entangle_camera_list_refresh(EntangleCameraList *list,
                                      GError **error G_GNUC_UNUSED)
{
    EntangleCameraListPrivate *priv = list->priv;
    CameraList *cams = NULL;
    GHashTable *toRemove;
    GHashTableIter iter;
    gpointer key, value;

    if (priv->ports)
        gp_port_info_list_free(priv->ports);
    if (gp_port_info_list_new(&priv->ports) != GP_OK)
        return FALSE;
    if (gp_port_info_list_load(priv->ports) != GP_OK)
        return FALSE;

    ENTANGLE_DEBUG("Detecting cameras");

    if (gp_list_new(&cams) != GP_OK)
        return FALSE;

    gp_abilities_list_detect(priv->caps, priv->ports, cams, priv->ctx);

    for (int i = 0 ; i < gp_list_count(cams) ; i++) {
        const char *model, *port;
        int n;
        EntangleCamera *cam;
        CameraAbilities cap;

        gp_list_get_name(cams, i, &model);
        gp_list_get_value(cams, i, &port);

        cam = entangle_camera_list_find(list, port);

        if (cam)
            continue;

        n = gp_abilities_list_lookup_model(priv->caps, model);
        gp_abilities_list_get_abilities(priv->caps, n, &cap);

        /* For back compat, libgphoto2 always adds a default
         * USB camera called 'usb:'. We ignore that, since we
         * can go for the exact camera entries
         */
        if (strcmp(port, "usb:") == 0)
            continue;

        ENTANGLE_DEBUG("New camera '%s' '%s' %d", model, port, cap.operations);
        cam = entangle_camera_new(model, port,
                              cap.operations & GP_OPERATION_CAPTURE_IMAGE ? TRUE : FALSE,
                              cap.operations & GP_OPERATION_CAPTURE_PREVIEW ? TRUE : FALSE,
                              cap.operations & GP_OPERATION_CONFIG ? TRUE : FALSE);
        entangle_camera_list_add(list, cam);
        g_object_unref(cam);
    }

    toRemove = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    for (int i = 0 ; i < entangle_camera_list_count(list) ; i++) {
        gboolean found = FALSE;
        EntangleCamera *cam = entangle_camera_list_get(list, i);

        ENTANGLE_DEBUG("Checking if %s exists", entangle_camera_get_port(cam));

        for (int j = 0 ; j < gp_list_count(cams) ; j++) {
            const char *port;
            gp_list_get_value(cams, j, &port);

            if (strcmp(port, entangle_camera_get_port(cam)) == 0) {
                found = TRUE;
                break;
            }
        }
        if (!found)
            g_hash_table_insert(toRemove, g_strdup(entangle_camera_get_port(cam)), cam);
    }

    gp_list_unref(cams);

    g_hash_table_iter_init(&iter, toRemove);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        EntangleCamera *cam = value;

        entangle_camera_list_remove(list, cam);
    }
    g_hash_table_unref(toRemove);

    return TRUE;
}


int entangle_camera_list_count(EntangleCameraList *list)
{
    EntangleCameraListPrivate *priv = list->priv;

    return priv->ncamera;
}

void entangle_camera_list_add(EntangleCameraList *list,
                              EntangleCamera *cam)
{
    EntangleCameraListPrivate *priv = list->priv;

    priv->cameras = g_renew(EntangleCamera *, priv->cameras, priv->ncamera+1);
    priv->cameras[priv->ncamera++] = cam;
    g_object_ref(cam);

    g_signal_emit_by_name(list, "camera-added", cam);
    ENTANGLE_DEBUG("Added camera %p", cam);
}

void entangle_camera_list_remove(EntangleCameraList *list,
                                 EntangleCamera *cam)
{
    EntangleCameraListPrivate *priv = list->priv;

    for (int i = 0 ; i < priv->ncamera ; i++) {
        if (priv->cameras[i] == cam) {
            if (i < (priv->ncamera-1))
                memmove(priv->cameras + i,
                        priv->cameras + i + 1,
                        sizeof(*priv->cameras) * (priv->ncamera - i - 1));
            priv->ncamera--;
        }
    }

    ENTANGLE_DEBUG("Removed camera %p from list", cam);
    g_signal_emit_by_name(list, "camera-removed", cam);

    g_object_unref(cam);
}

EntangleCamera *entangle_camera_list_get(EntangleCameraList *list,
                                         int entry)
{
    EntangleCameraListPrivate *priv = list->priv;

    if (entry < 0 || entry >= priv->ncamera)
        return NULL;

    return priv->cameras[entry];
}


/**
 * entangle_camera_list_get_cameras:
 * @list: the camera list
 *
 * Returns: (transfer container): a list of #EntangleCamera objects
 */
GList *entangle_camera_list_get_cameras(EntangleCameraList *list)
{
    EntangleCameraListPrivate *priv = list->priv;
    GList *cameras = NULL;

    for (int i = (priv->ncamera - 1) ; i >= 0 ; i--) {
        cameras = g_list_append(cameras, priv->cameras[i]);
    }
    return cameras;
}


EntangleCamera *entangle_camera_list_find(EntangleCameraList *list,
                                          const char *port)
{
    EntangleCameraListPrivate *priv = list->priv;
    int i;

    for (i = 0 ; i < priv->ncamera ; i++) {
        const char *thisport = entangle_camera_get_port(priv->cameras[i]);

        ENTANGLE_DEBUG("Compare '%s' '%s'", port, thisport);

        if (strcmp(thisport, port) == 0)
            return priv->cameras[i];
    }

    return NULL;
}


/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  indent-tabs-mode: nil
 *  tab-width: 8
 * End:
 */
