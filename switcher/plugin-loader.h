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


#ifndef __SWITCHER_PLUGIN_H__
#define __SWITCHER_PLUGIN_H__

#include <memory>
#include <gmodule.h>
#include "json-builder.h"

namespace switcher
{

  class Quiddity;
  class QuiddityDocumentation;

  // the types of the class factories for quiddity pluggins
  typedef switcher::Quiddity *create_t ();
  typedef void destroy_t (switcher::Quiddity *);
  typedef switcher::QuiddityDocumentation get_documentation_t ();
  
  
  class PluginLoader
  {
  public:
    typedef std::shared_ptr<PluginLoader> ptr;
    PluginLoader ();
    ~PluginLoader ();
    PluginLoader (const PluginLoader &) = delete;
    PluginLoader &operator= (const PluginLoader &) = delete;

    bool load (const char *filename);
    bool close ();
    std::string get_class_name (); 
    JSONBuilder::Node get_json_root_node ();

    create_t *create_;
    destroy_t *destroy_;

  private:
    GModule *module_;
    get_documentation_t *get_documentation_; 
    JSONBuilder::Node json_doc_;
    std::string class_name_;
  };
  
}  // end of namespace

#endif // ifndef