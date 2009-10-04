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

#ifndef __CAPA_CAMERA_H__
#define __CAPA_CAMERA_H__

#include <glib-object.h>

#include "control-group.h"
#include "image.h"
#include "session.h"

G_BEGIN_DECLS

#define CAPA_TYPE_CAMERA            (capa_camera_get_type ())
#define CAPA_CAMERA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CAPA_TYPE_CAMERA, CapaCamera))
#define CAPA_CAMERA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CAPA_TYPE_CAMERA, CapaCameraClass))
#define CAPA_IS_CAMERA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CAPA_TYPE_CAMERA))
#define CAPA_IS_CAMERA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CAPA_TYPE_CAMERA))
#define CAPA_CAMERA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CAPA_TYPE_CAMERA, CapaCameraClass))


typedef struct _CapaCamera CapaCamera;
typedef struct _CapaCameraPrivate CapaCameraPrivate;
typedef struct _CapaCameraClass CapaCameraClass;

struct _CapaCamera
{
  GObject parent;

  CapaCameraPrivate *priv;
};

struct _CapaCameraClass
{
  GObjectClass parent_class;

  void (*camera_image)(CapaCamera *cam, CapaImage *image);
  void (*camera_error)(CapaCamera *cam, const char *err);

  void (*camera_op_begin)(CapaCamera *cam);
  void (*camera_op_end)(CapaCamera *cam);
};


GType capa_camera_get_type(void) G_GNUC_CONST;

CapaCamera *capa_camera_new(const char *model,
			    const char *port,
			    gboolean hasCapture,
			    gboolean hasPreview,
			    gboolean hasSettings);

const char *capa_camera_model(CapaCamera *cam);
const char *capa_camera_port(CapaCamera *cam);

int capa_camera_connect(CapaCamera *cap);

char *capa_camera_summary(CapaCamera *cam);
char *capa_camera_manual(CapaCamera *cam);
char *capa_camera_driver(CapaCamera *cam);

int capa_camera_capture(CapaCamera *cam);
int capa_camera_preview(CapaCamera *cam);
int capa_camera_monitor(CapaCamera *cam);


gboolean capa_camera_has_capture(CapaCamera *cam);
gboolean capa_camera_has_preview(CapaCamera *cam);
gboolean capa_camera_has_settings(CapaCamera *cam);

CapaSession *capa_camera_session(CapaCamera *cam);
CapaControlGroup *capa_camera_controls(CapaCamera *cam);

G_END_DECLS

#endif /* __CAPA_CAMERA_H__ */
