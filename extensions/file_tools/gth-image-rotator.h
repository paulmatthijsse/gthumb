/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  GThumb
 *
 *  Copyright (C) 2011 Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTH_IMAGE_ROTATOR_H
#define GTH_IMAGE_ROTATOR_H

#include <glib.h>
#include <glib-object.h>
#include <gthumb.h>
#include "gth-transform-resize.h"

G_BEGIN_DECLS

#define GTH_TYPE_IMAGE_ROTATOR            (gth_image_rotator_get_type ())
#define GTH_IMAGE_ROTATOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTH_TYPE_IMAGE_ROTATOR, GthImageRotator))
#define GTH_IMAGE_ROTATOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTH_TYPE_IMAGE_ROTATOR, GthImageRotatorClass))
#define GTH_IS_IMAGE_ROTATOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTH_TYPE_IMAGE_ROTATOR))
#define GTH_IS_IMAGE_ROTATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTH_TYPE_IMAGE_ROTATOR))
#define GTH_IMAGE_ROTATOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GTH_TYPE_IMAGE_ROTATOR, GthImageRotatorClass))

typedef struct _GthImageRotator         GthImageRotator;
typedef struct _GthImageRotatorClass    GthImageRotatorClass;
typedef struct _GthImageRotatorPrivate  GthImageRotatorPrivate;

struct _GthImageRotator
{
	GObject __parent;
	GthImageRotatorPrivate *priv;
};

struct _GthImageRotatorClass
{
	GObjectClass __parent_class;

	/*< signals >*/

	void (* changed)        (GthImageRotator *rotator);
	void (* center_changed) (GthImageRotator *rotator);
};

GType                 gth_image_rotator_get_type        (void);
GthImageViewerTool *  gth_image_rotator_new             (GthImageViewer     *viewer);
void                  gth_image_rotator_set_center      (GthImageRotator    *rotator,
						         int                 x,
						         int                 y);
void                  gth_image_rotator_get_center      (GthImageRotator    *rotator,
							 int                *x,
							 int                *y);
void                  gth_image_rotator_set_angle       (GthImageRotator    *rotator,
						         double              angle);
void                  gth_image_rotator_set_grid        (GthImageRotator    *self,
						         gboolean            use_grid,
						         int                 lines);
void                  gth_image_rotator_set_resize      (GthImageRotator    *self,
						         GthTransformResize  resize);
void                  gth_image_rotator_set_background  (GthImageRotator    *self,
							 cairo_color_t      *color);
cairo_surface_t *     gth_image_rotator_get_result      (GthImageRotator    *self);

G_END_DECLS

#endif /* GTH_IMAGE_ROTATOR_H */