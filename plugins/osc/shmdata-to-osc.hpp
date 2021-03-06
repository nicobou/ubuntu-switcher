/*
 * This file is part of switcher-osc.
 *
 * switcher-osc is free software; you can redistribute it and/or
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

#ifndef __SWITCHER_SHMDATA_TO_OSC_H__
#define __SWITCHER_SHMDATA_TO_OSC_H__

#include <lo/lo.h>
#include <mutex>
#include <chrono>
#include "switcher/quiddity.hpp"
#include "switcher/custom-property-helper.hpp"
#include "switcher/startable-quiddity.hpp"
#include "switcher/shmdata-connector.hpp"
#include "switcher/shmdata-follower.hpp"

namespace switcher {
class ShmdataToOsc:public Quiddity, public StartableQuiddity {
 public:
  SWITCHER_DECLARE_QUIDDITY_PUBLIC_MEMBERS(ShmdataToOsc);
  ShmdataToOsc(const std::string &);
  ~ShmdataToOsc();
  ShmdataToOsc(const ShmdataToOsc &) = delete;
  ShmdataToOsc &operator=(const ShmdataToOsc &) = delete;

 private:
  // registering connect/disconnect/can_sink_caps:
  ShmdataConnector shmcntr_;
  // shmdata follower
  std::unique_ptr<ShmdataFollower> shm_{nullptr};
  // custom props
  CustomPropertyHelper::ptr custom_props_;
  gint port_{1056};
  std::string host_{"localhost"};
  GParamSpec *port_spec_{nullptr};
  GParamSpec *host_spec_{nullptr};
  lo_address address_{nullptr};
  std::mutex address_mutex_{};

  bool init() final;
  bool start() final;
  bool stop() final;
  bool connect(const std::string &shmdata_path);
  bool disconnect();
  bool can_sink_caps(const std::string &caps);
  void on_shmreader_data(void *data,
                         size_t data_size);
  static void set_port(const gint value, void *user_data);
  static gint get_port(void *user_data);
  static void set_host(const gchar *value, void *user_data);
  static const gchar *get_host(void *user_data);
};

SWITCHER_DECLARE_PLUGIN(ShmdataToOsc);

}  // namespace switcher
#endif
