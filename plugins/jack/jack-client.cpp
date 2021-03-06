/*
 * This file is part of switcher-jack.
 *
 * switcher-myplugin is free software; you can redistribute it and/or
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

#include <glib.h>
#include <jack/statistics.h>
#include <cmath>
#include <string>
#include "./jack-client.hpp"

namespace switcher {
JackClient::JackClient(const char *name,
                       JackProcessCallback process_cb,
                       void *process_user_data,
                       XRunCallback_t xrun_cb,
                       PortCallback_t port_cb) :
    client_ (jack_client_open(name,
                              JackNoStartServer,
                              &status_,
                              nullptr /* server name */),
             &jack_client_close),
    user_cb_(process_cb),
    user_cb_arg_(process_user_data),
    xrun_cb_(xrun_cb),
    port_cb_(port_cb){
  if ((client_ == NULL) && !(status_ & JackServerFailed)) {
    g_warning("jack_client_open() failed, "
              "status = 0x%2.0x", status_);
    return;
  }
  jack_on_shutdown (client_.get(), &JackClient::on_jack_shutdown, this);
  // if (status_ & JackServerStarted) {
  //   g_warning("JACK server started\n");
  // }
  // if (status_ & JackNameNotUnique) {
  // client_name = jack_get_client_name(client);
  // fprintf (stderr, "unique name `%s' assigned\n", client_name);
  // }
  sample_rate_ = jack_get_sample_rate(client_.get());
  buffer_size_ = jack_get_buffer_size(client_.get());
  // g_print("sample rate %" PRIu32 "\n", sample_rate_);
  // g_print("buffer size %" PRIu32 " \n", jack_get_buffer_size(client_.get()));
  jack_set_xrun_callback(client_.get(), on_xrun, this);
  jack_set_process_callback(client_.get(), jack_process, this);
  if(port_cb_)
    jack_set_port_registration_callback (client_.get(), port_callback, this);
  jack_activate(client_.get());
}

void JackClient::port_callback (jack_port_id_t port_id, int yn, void *user_data){
  JackClient *context = static_cast<JackClient *>(user_data);
  jack_port_t *port = jack_port_by_id(context->client_.get(), port_id);
  if (yn)
    context->port_cb_(port);
  // printf ("Port %d %s\n", port_id, (yn ? "registered" : "unregistered"));
  // {
  //   int flags = jack_port_flags (port);
  //   printf ("	properties: ");
  //   if (flags & JackPortIsInput) {
  //     fputs ("input,", stdout);
  //   }
  //   if (flags & JackPortIsOutput) {
  //     fputs ("output,", stdout);
  //   }
  //   if (flags & JackPortCanMonitor) {
  //     fputs ("can-monitor,", stdout);
  //   }
  //   if (flags & JackPortIsPhysical) {
  //     fputs ("physical,", stdout);
  //   }
  //   if (flags & JackPortIsTerminal) {
  //     fputs ("terminal,", stdout);
  //   }
  //   putc ('\n', stdout);
  // }
  // {
  //   printf ("port type :: ");
  //   putc ('\t', stdout);
  //   fputs (jack_port_type (port), stdout);
  //   putc ('\n', stdout);
  // }
  // {
  //   printf("port name %s\n", jack_port_name(port));
  //   printf("port short name %s\n", jack_port_short_name(port));
  // }
  // printf("-------------------------\n");
}

int JackClient::jack_process(jack_nframes_t nframes, void *arg){
  JackClient *context = static_cast<JackClient *>(arg);
  unsigned int samples = context->xrun_count_.load();
  if (0 != samples) {
    // g_print("-- missed samples %u\n", samples);
    if (context->xrun_cb_)
      context->xrun_cb_(samples);
    context->xrun_count_.fetch_sub(samples);
  }
  if (nullptr != context->user_cb_)
    return context->user_cb_(nframes, context->user_cb_arg_);
  return 0;
}


bool JackClient::safe_bool_idiom() const {
  return static_cast<bool>(client_);
}

void JackClient::on_jack_shutdown (void */*arg*/)
{
  g_warning("jack shut down");
  // TODO invalidate client_
}

jack_client_t *JackClient::get_raw(){
  return client_.get();
}

int JackClient::on_xrun(void *arg)
{
  JackClient *context = static_cast<JackClient *>(arg);
  // computing the number of sample missed
  // g_print("xrun delay %f ms\n num sample missed %f",
  //         0.001 * jack_get_xrun_delayed_usecs(context->client_.get()),
  //         (float)context->sample_rate_ * (1e-6 * jack_get_xrun_delayed_usecs(context->client_.get())));
  //g_print("--- XRUN ---\n");
  context->xrun_count_.fetch_add(std::ceil(
      (float)context->sample_rate_
      * (1e-6 * jack_get_xrun_delayed_usecs(context->client_.get()))));
  return 0;
}

JackPort::JackPort(JackClient &client, unsigned int number, bool is_output) :
    port_name_(std::string("output_" + std::to_string(number))),
    port_(jack_port_register(client.get_raw(),
                             port_name_.c_str(),
                             JACK_DEFAULT_AUDIO_TYPE,
                             is_output ? JackPortIsOutput : JackPortIsInput,
                             0),
          [&](jack_port_t *port){
            jack_port_unregister(client.get_raw(), port);
          }){
}

std::string JackPort::get_name() const{
  return port_name_;
}

bool JackPort::safe_bool_idiom() const{
  return static_cast<bool>(port_);
}

}  // namespace switcher
