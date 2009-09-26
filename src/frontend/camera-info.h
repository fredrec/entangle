/*
 *  Capa: Capa Assists Photograph Aquisition
 *
 *  Copyright (C) 2009 Daniel P. Berrange
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

#ifndef __CAMERA_INFO__
#define __CAMERA_INFO__

#include <glib-object.h>

#include "camera.h"

G_BEGIN_DECLS

#define CAPA_TYPE_CAMERA_INFO            (capa_camera_info_get_type ())
#define CAPA_TYPE_CAMERA_INFO_DATA       (capa_camera_info_data_get_type())
#define CAPA_CAMERA_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CAPA_TYPE_CAMERA_INFO, CapaCameraInfo))
#define CAPA_CAMERA_INFO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CAPA_TYPE_CAMERA_INFO, CapaCameraInfoClass))
#define CAPA_IS_CAMERA_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CAPA_TYPE_CAMERA_INFO))
#define CAPA_IS_CAMERA_INFO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CAPA_TYPE_CAMERA_INFO))
#define CAPA_CAMERA_INFO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CAPA_TYPE_CAMERA_INFO, CapaCameraInfoClass))


typedef struct _CapaCameraInfo CapaCameraInfo;
typedef struct _CapaCameraInfoPrivate CapaCameraInfoPrivate;
typedef struct _CapaCameraInfoClass CapaCameraInfoClass;

struct _CapaCameraInfo
{
  GObject parent;

  CapaCameraInfoPrivate *priv;
};

struct _CapaCameraInfoClass
{
  GObjectClass parent_class;

  void (*info_close)(CapaCameraInfo *info);
};

typedef enum {
  CAPA_CAMERA_INFO_DATA_SUMMARY,
  CAPA_CAMERA_INFO_DATA_MANUAL,
  CAPA_CAMERA_INFO_DATA_DRIVER,
  CAPA_CAMERA_INFO_DATA_SUPPORTED,
} CapaCameraInfoData;

GType capa_camera_info_get_type(void) G_GNUC_CONST;
GType capa_camera_info_data_get_type(void) G_GNUC_CONST;

CapaCameraInfo* capa_camera_info_new(void);

void capa_camera_info_show(CapaCameraInfo *info);
void capa_camera_info_hide(CapaCameraInfo *info);

G_END_DECLS

#endif /* __CAMERA_INFO__ */
