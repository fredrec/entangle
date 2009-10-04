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

#ifndef __CONTROL_BUTTON__
#define __CONTROL_BUTTON__

#include <glib-object.h>

#include "control.h"

G_BEGIN_DECLS

#define CAPA_TYPE_CONTROL_BUTTON            (capa_control_button_get_type ())
#define CAPA_CONTROL_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CAPA_TYPE_CONTROL_BUTTON, CapaControlButton))
#define CAPA_CONTROL_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CAPA_TYPE_CONTROL_BUTTON, CapaControlButtonClass))
#define CAPA_IS_CONTROL_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CAPA_TYPE_CONTROL_BUTTON))
#define CAPA_IS_CONTROL_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CAPA_TYPE_CONTROL_BUTTON))
#define CAPA_CONTROL_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CAPA_TYPE_CONTROL_BUTTON, CapaControlButtonClass))


typedef struct _CapaControlButton CapaControlButton;
typedef struct _CapaControlButtonPrivate CapaControlButtonPrivate;
typedef struct _CapaControlButtonClass CapaControlButtonClass;

struct _CapaControlButton
{
  CapaControl parent;

  CapaControlButtonPrivate *priv;
};

struct _CapaControlButtonClass
{
  CapaControlClass parent_class;
};


GType capa_control_button_get_type(void) G_GNUC_CONST;
CapaControlButton* capa_control_button_new(const char *path,
					   int id,
					   const char *label,
					   const char *info);


G_END_DECLS

#endif /* __CONTROL_BUTTON__ */
