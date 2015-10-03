/*
   Copyright 2009 Brain Research Institute, Melbourne, Australia

   Written by J-Donald Tournier, 13/11/09.

   This file is part of MRtrix.

   MRtrix is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   MRtrix is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __gui_mrview_image_h__
#define __gui_mrview_image_h__

#include "image.h"
#include "types.h"
#include "gui/opengl/gl.h"
#include "gui/mrview/volume.h"
#include "interp/linear.h"
#include "interp/nearest.h"

//#ifndef __image_h__
//#error File that #includes "gui/mrview/image.h" must explicitly #include "image.h" beforehand
//#endif


namespace MR
{
  namespace GUI
  {

    class Projection;

    namespace MRView
    {

      class Window;

      class ImageBase : public Volume
      {
        public:
          ImageBase (MR::Header&&);

          void render2D (Displayable::Shader& shader_program, const Projection& projection, const int plane, const int slice);
          void render3D (Displayable::Shader& shader_program, const Projection& projection, const float depth);

          virtual void update_texture2D (const int plane, const int slice) = 0;
          virtual void update_texture3D() = 0;

          void get_axes (const int plane, int& x, int& y) const;

        protected:
          GL::Texture texture2D[3];
          std::vector<ssize_t> position;

      };

      class Image : public ImageBase
      {
        public:
          Image (MR::Header&&);

          void update_texture2D (const int plane, const int slice) override;
          void update_texture3D() override;

          void request_render_colourbar (DisplayableVisitor& visitor) override
          { if (show_colour_bar) visitor.render_image_colourbar (*this); }

          MR::Image<cfloat> image;
          mutable MR::Interp::Linear <MR::Image<cfloat>> linear_interp;
          mutable MR::Interp::Nearest<MR::Image<cfloat>> nearest_interp;
          cfloat trilinear_value (const Eigen::Vector3f&) const;
          cfloat nearest_neighbour_value (const Eigen::Vector3f&) const;

        private:
          bool volume_unchanged ();
          bool format_unchanged ();
          size_t guess_colourmap () const;

          template <typename T> void copy_texture_3D ();
          void copy_texture_3D_complex ();

      };


    }
  }
}

#endif

