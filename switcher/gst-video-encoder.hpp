/*
 * This file is part of libswitcher.
 *
 * libswitcher is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __SWITCHER_GST_VIDEO_ENCODER_H__
#define __SWITCHER_GST_VIDEO_ENCODER_H__

#include <memory>
#include "switcher/quiddity.hpp"
#include "switcher/shmdata-connector.hpp"
#include "switcher/gst-video-codec.hpp"

namespace switcher {
class GstVideoEncoder: public Quiddity {
 public:
  SWITCHER_DECLARE_QUIDDITY_PUBLIC_MEMBERS(GstVideoEncoder);
  GstVideoEncoder(const std::string &);
  ~GstVideoEncoder() = default;
  GstVideoEncoder(const GstVideoEncoder &) = delete;
  GstVideoEncoder &operator=(const GstVideoEncoder &) = delete;

 private:
  // registering connect/disconnect/can_sink_caps:
  ShmdataConnector shmcntr_;
  CustomPropertyHelper::ptr custom_props_;
  std::unique_ptr<GstVideoCodec> codecs_{nullptr};
  bool init() final;
  bool on_shmdata_disconnect();
  bool on_shmdata_connect(const std::string &shmdata_sochet_path);
  bool can_sink_caps(const std::string &caps);
};

}  // namespace switcher
#endif
