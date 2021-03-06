/*
 * This file is part of switcher.
 *
 * switcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * switcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with switcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <vector>
#include <iostream>
#include <signal.h>
#include <time.h>
#include "switcher/quiddity-manager.hpp"
#include <locale.h>
#ifdef HAVE_GTK
#include <gtk/gtk.h>
#endif

static const gchar *server_name = nullptr;
static const gchar *port_number = nullptr;
static const gchar *load_file = nullptr;
static gchar *osc_port_number = nullptr;
static gboolean display_version;
static gboolean quiet;
static gboolean debug;
static gboolean verbose;
static gboolean listclasses;
static gboolean classesdoc;
static gchar *classdoc = nullptr;
static gchar *listpropbyclass = nullptr;
static gchar *listmethodsbyclass = nullptr;
static gchar *listsignalsbyclass = nullptr;
static gchar *extraplugindir = nullptr;
static switcher::QuiddityManager::ptr manager;
static GOptionEntry entries[15] = {
  {"version", 'V', 0, G_OPTION_ARG_NONE, &display_version,
   "display switcher version number", nullptr},
  {"server-name", 'n', 0, G_OPTION_ARG_STRING, &server_name,
   "server name (default is \"default\")", nullptr},
  {"port-number", 'p', 0, G_OPTION_ARG_STRING, &port_number,
   "port number the server will bind (default is 27182)", nullptr},
  {"load", 'l', 0, G_OPTION_ARG_STRING, &load_file,
   "load state from history file (-l filename)", nullptr},
  {"quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet, "do not display any message",
   nullptr},
  {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
   "display all messages, excluding debug", nullptr},
  {"debug", 'd', 0, G_OPTION_ARG_NONE, &debug,
   "display all messages, including debug", nullptr},
  {"list-classes", 'C', 0, G_OPTION_ARG_NONE, &listclasses,
   "list quiddity classes", nullptr},
  {"list-props-by-class", 'P', 0, G_OPTION_ARG_STRING, &listpropbyclass,
   "list properties of a class", nullptr},
  {"list-methods-by-class", 'M', 0, G_OPTION_ARG_STRING, &listmethodsbyclass,
   "list methods of a class", nullptr},
  {"list-signals-by-class", 'S', 0, G_OPTION_ARG_STRING, &listsignalsbyclass,
   "list signals of a class", nullptr},
  {"classes-doc", 'K', 0, G_OPTION_ARG_NONE, &classesdoc,
   "print classes documentation, JSON-formated", nullptr},
  {"class-doc", 'k', 0, G_OPTION_ARG_STRING, &classdoc,
   "print class documentation, JSON-formated (--class-doc class_name)", nullptr},
  {"osc-port", 'o', 0, G_OPTION_ARG_STRING, &osc_port_number,
   "osc port number (osc enabled only if set)", nullptr},
  {"extra-plugin-dir", 'E', 0, G_OPTION_ARG_STRING, &extraplugindir,
   "directory where to find additional plugins", nullptr}
};

void
leave(int sig) {
  // removing reference to manager in order to delete it
  {
    switcher::QuiddityManager::ptr empty;
    manager.swap(empty);
  }
#ifdef HAVE_GTK
  gtk_main_quit();
#endif
  exit(sig);
}

static void
quiet_log_handler(const gchar * /*log_domain */ ,
                  GLogLevelFlags /*log_level */ ,
                  const gchar * /*message */ ,
                  gpointer /*user_data */ ) {
}

static void
logger_cb(const std::string &/*subscriber_name */ ,
          const std::string &/*quiddity_name */ ,
          const std::string &/*property_name */ ,
          const std::string &value,
          void * /*user_data */ ) {
  g_print("%s\n", value.c_str());
}

int
main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");
  (void) signal(SIGINT, leave);
  (void) signal(SIGABRT, leave);
  (void) signal(SIGQUIT, leave);
  (void) signal(SIGTERM, leave);

  // command line options
  GError *error = nullptr;
  GOptionContext *context;
  context = g_option_context_new("- switcher server");
  g_option_context_add_main_entries(context, entries, nullptr);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_printerr("option parsing failed: %s\n", error->message);
    exit(1);
  }

  if (display_version) {
#ifdef HAVE_CONFIG_H
    g_print("%s\n", VERSION);
#else
    g_print("unknown\n");
#endif
    return 0;
  }

  // running a switcher server
  if (server_name == nullptr)
    server_name = "default";
  if (port_number == nullptr)
    port_number = "27182";

  manager = switcher::QuiddityManager::make_manager(server_name);

  // create logger managing switcher log domain
  manager->create("logger", "internal_logger");
  // manage logs from shmdata
  manager->invoke_va("internal_logger", "install_log_handler", nullptr,
                     "shmdata", nullptr);
  // manage logs from GStreamer
  manager->invoke_va("internal_logger", "install_log_handler", nullptr,
                     "GStreamer", nullptr);
  // manage logs from Glib
  manager->invoke_va("internal_logger", "install_log_handler", nullptr,
                     "GLib", nullptr);
  // manage logs from Glib-GObject
  manager->invoke_va("internal_logger", "install_log_handler", nullptr,
                     "GLib-GObject", nullptr);

  if (quiet)
    manager->set_property("internal_logger", "mute", "true");
  else
    manager->set_property("internal_logger", "mute", "false");
  if (debug)
    manager->set_property("internal_logger", "debug", "true");
  else
    manager->set_property("internal_logger", "debug", "false");
  if (verbose)
    manager->set_property("internal_logger", "verbose", "true");
  else
    manager->set_property("internal_logger", "verbose", "false");

  // subscribe to logs:
  manager->make_property_subscriber("log_sub", logger_cb, nullptr);
  manager->subscribe_property("log_sub", "internal_logger", "last-line");

  // loading plugins from default location // FIXME add an option
#ifdef HAVE_CONFIG_H
  gchar *
      usr_plugin_dir = g_strdup_printf("/usr/%s-%s/plugins", PACKAGE_NAME,
                                       LIBSWITCHER_API_VERSION);
  manager->scan_directory_for_plugins(usr_plugin_dir);
  g_free(usr_plugin_dir);

  gchar *
      usr_local_plugin_dir =
      g_strdup_printf("/usr/local/%s-%s/plugins", PACKAGE_NAME,
                      LIBSWITCHER_API_VERSION);
  manager->scan_directory_for_plugins(usr_local_plugin_dir);
  g_free(usr_local_plugin_dir);
#else
  g_warning("plugins from default location not loaded (config.h missing)");
#endif

  if (extraplugindir != nullptr)
    manager->scan_directory_for_plugins(extraplugindir);

  // checking if this is printing info only
  if (listclasses) {
    g_log_set_default_handler(quiet_log_handler, nullptr);
    std::vector<std::string> resultlist = manager->get_classes();
    for (uint i = 0; i < resultlist.size(); i++)
      g_print("%s\n", resultlist[i].c_str());
    return 0;
  }
  if (classesdoc) {
    g_log_set_default_handler(quiet_log_handler, nullptr);
    g_print("%s\n", manager->get_classes_doc().c_str());
    return 0;
  }
  if (classdoc != nullptr) {
    g_log_set_default_handler(quiet_log_handler, nullptr);
    g_print("%s\n", manager->get_class_doc(classdoc).c_str());
    return 0;
  }
  if (listpropbyclass != nullptr) {
    g_log_set_default_handler(quiet_log_handler, nullptr);
    g_print("%s\n", manager->
            get_properties_description_by_class(listpropbyclass).c_str());
    return 0;
  }
  if (listmethodsbyclass != nullptr) {
    g_log_set_default_handler(quiet_log_handler, nullptr);
    g_print("%s\n",
            manager->
            get_methods_description_by_class(listmethodsbyclass).c_str());
    return 0;
  }
  if (listsignalsbyclass != nullptr) {
    g_log_set_default_handler(quiet_log_handler, nullptr);
    g_print("%s\n",
            manager->
            get_signals_description_by_class(listsignalsbyclass).c_str());
    return 0;
  }

  std::string soap_name = manager->create("SOAPcontrolServer", "soapserver");
  std::vector<std::string> port_arg;
  port_arg.push_back(port_number);
  std::string *result;
  manager->invoke(soap_name, "set_port", &result, port_arg);
  if (g_strcmp0("false", result->c_str()) == 0 && osc_port_number == nullptr)
    return 0;

  // start osc if port number has been set
  if (osc_port_number != nullptr) {
    std::string osc_name = manager->create("OSCctl");
    if (osc_name.compare("") == 0)
      g_warning("osc plugin not found");
    else
      manager->invoke_va(osc_name.c_str(), "set_port", nullptr,
                         osc_port_number, nullptr);
  }

  manager->reset_command_history(false);

  if (load_file != nullptr) {
    switcher::QuiddityManager::CommandHistory histo =
        manager->get_command_history_from_file(load_file);
    std::vector<std::string> prop_subscriber_names =
        manager->get_property_subscribers_names(histo);
    if (!prop_subscriber_names.empty())
      g_warning
          ("creation of property subscriber not handled when loading file %s",
           load_file);

    std::vector<std::string> signal_subscriber_names =
        manager->get_signal_subscribers_names(histo);
    if (!signal_subscriber_names.empty())
      g_warning
          ("creation of signal subscriber not handled when loading file %s",
           load_file);

    manager->play_command_history(histo, nullptr, nullptr, true);
  }

  // manager->create ("videotestsrc", "vid");
  // manager->create("videosink","win");
  // manager->invoke_va ("win",
  //    "connect",
  //                     nullptr,
  //    "/tmp/switcher_default_vid_video",
  //    nullptr);

  // //  g_print ("---- histo testing ------ \n");
  // manager->save_command_history ("trup.switcher");

  // manager->reset_command_history(true);

  // // manager->reboot ();

  // g_print ("---- reset done ----\n");
  // g_print ("--- %s\n",manager->get_quiddities_description ().c_str ());

  // switcher::QuiddityManager::CommandHistory histo =
  //   manager->get_command_history_from_file ("trup.switcher");

  // // std::vector <std::string> prop_subscriber_names =
  // //   manager->get_property_subscribers_names (histo);

  // //    // for (auto &it: prop_subscriber_names)
  // //    //   g_print ("prop sub %s\n", it.c_str ());

  // //    // std::vector <std::string> signal_subscriber_names =
  // //    //   manager->get_signal_subscribers_names (histo);

  // //    // for (auto &it: signal_subscriber_names)
  // //    //   g_print ("signal sub %s\n", it.c_str ());

  //      switcher::QuiddityManager::PropCallbackMap prop_cb_data;
  //      prop_cb_data ["log_sub"] = std::make_pair (logger_cb, (void *)nullptr);
  //      switcher::QuiddityManager::SignalCallbackMap sig_cb_data;
  //      sig_cb_data["create_remove_subscriber"] = std::make_pair (quiddity_created_removed_cb, (void *)nullptr);
  //      manager->play_command_history (histo, &prop_cb_data, &sig_cb_data);
  //      g_print ("--fin-- %s\n",manager->get_quiddities_description ().c_str ());

#ifdef HAVE_GTK
  if (!gtk_init_check(nullptr, nullptr))
    std::cerr << "cannot init gtk in main" << std::endl;
  else
    gtk_main();
#endif

  // waiting for end of life
  timespec delay;
  delay.tv_sec = 1;
  delay.tv_nsec = 0;
  while (1)
    nanosleep(&delay, nullptr);

  return 0;
}
