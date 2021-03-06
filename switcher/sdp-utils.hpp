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

#ifndef __SWITCHER_SDP_UTILS_H__
#define __SWITCHER_SDP_UTILS_H__

#include <gst/gst.h>
#include <gst/sdp/gstsdpmessage.h>
#include <string>

namespace switcher {
class SDPDescription;

class SDPMedia {
  friend SDPDescription;
 public:
  SDPMedia();
  ~SDPMedia();
  SDPMedia &operator=(const SDPMedia &) = delete;
  SDPMedia(const SDPMedia &) = delete;
  bool set_media_info_from_caps(const GstCaps *media_caps);  // caps from a gst RTP payloader
  bool set_port(uint port);

 private:
  GstSDPMedia *media_{nullptr};
  GstStructure *caps_structure_{nullptr};
  uint port_{0};  // "0" means disabled media
  bool add_to_sdp_description(GstSDPMessage *sdp_description, uint index) const;
};

class SDPDescription {
 public:
  SDPDescription();
  ~SDPDescription();
  SDPDescription &operator=(const SDPDescription &) = delete;
  SDPDescription(const SDPDescription &) = delete;

  bool add_media(const SDPMedia &media);
  std::string get_string();
 private:
  GstSDPMessage *sdp_description_{nullptr};
  uint index_{0};
};

}  // namespace switcher
#endif
