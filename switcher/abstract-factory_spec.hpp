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

/**
 * the Abstract factory class
 */

// this file is included from abstract-factory.hpp
// this separation is done in order to make abstract-factory.hpp easier to read


namespace switcher {
template<typename T, typename Key, typename Doc, typename ...ATs>
template<class U> void
AbstractFactory<T, Key, Doc, ATs...>::register_class(Key Id, Doc doc) {
  Creator<T, ATs...> *Fn = (Creator<T, ATs...> *) new DerivedCreator <U, ATs...> ();
  constructor_map_[Id] = Fn;
  classes_documentation_[Id] = std::move(doc);
}

template<typename T, typename Key, typename Doc, typename ...ATs>
void AbstractFactory<T, Key, Doc, ATs...>::register_class_with_custom_factory(
    Key Id,
    Doc doc,
    T *(*custom_create) (ATs...),
    void (*custom_destroy) (T *)) {
  CustomDerivedCreator<T, ATs...> *creator = new CustomDerivedCreator <T, ATs...> ();
  creator->custom_create_ = custom_create;
  Creator<T, ATs...> *Fn = (Creator <T, ATs...> *)creator;
  constructor_map_[Id] = Fn;
  destructor_map_[Id] = custom_destroy;
  classes_documentation_[Id] = std::move(doc);
}

template<typename T, typename Key, typename Doc, typename ...ATs>
std::vector<Key> AbstractFactory <T, Key, Doc, ATs...>::get_keys() {
  std::vector<Key> constructor_names;
  for (auto &it:constructor_map_)
    constructor_names.push_back(it.first);
  return constructor_names;
}

template<typename T, typename Key, typename Doc, typename ...ATs>
std::vector<Doc>
AbstractFactory<T, Key, Doc, ATs...>::get_classes_documentation() {
  std::vector<Doc> tmp;
  typename std::map<Key, Doc>::iterator i =
      classes_documentation_.begin();
  while (i != classes_documentation_.end()) {
    tmp.push_back((*i).second);
    ++i;
  }
  return tmp;
}

template<typename T, typename Key, typename Doc, typename ...ATs>
Doc AbstractFactory<T, Key, Doc, ATs...>::get_class_documentation(Key id){
  typename std::map<Key, Doc>::iterator iter =
      classes_documentation_.find(id);
return iter->second;
}

template<typename T, typename Key, typename Doc, typename ...ATs>
bool AbstractFactory<T, Key, Doc, ATs...>::key_exists(Key Id) {
  return (constructor_map_.find(Id) != constructor_map_.end());
}

template<typename T, typename Key, typename Doc, typename ...ATs>
bool AbstractFactory<T, Key, Doc, ATs...>::unregister_class(Key Id) {
  if (constructor_map_.find(Id) == constructor_map_.end())
    return false;
  else
    delete(constructor_map_.find(Id))->second;
  constructor_map_.erase(Id);
  destructor_map_.erase(Id);
  classes_documentation_.erase(Id);
  return true;
}

template<typename T, typename Key, typename Doc, typename ...ATs>
std::shared_ptr<T> AbstractFactory <T, Key, Doc, ATs...>::create(Key Id, ATs... args) {
  std::shared_ptr<T> pointer;
  if (constructor_map_.find(Id) != constructor_map_.end()) {
    if (destructor_map_.find(Id) != destructor_map_.end())
      pointer.reset(constructor_map_[Id]->Create(std::forward<ATs>(args)...),
                    destructor_map_[Id]);
    else
      pointer.reset(constructor_map_[Id]->Create(std::forward<ATs>(args)...));
  }
  return pointer;
}

template<typename T, typename Key, typename Doc, typename ...ATs>
AbstractFactory<T, Key, Doc, ATs...>::~AbstractFactory() {
  for (auto &it: constructor_map_)
    delete it.second;
  // typename std::map<Key, Creator <T, ATs...> *>::iterator i =
  //     constructor_map_.begin();
  // while (i != constructor_map_.end()) {
  //   delete(*i).second;
  //   ++i;
  // }
}

template<typename T, typename Key, typename Doc, typename ...ATs>
AbstractFactory <T, Key, Doc, ATs...>::AbstractFactory():
    constructor_map_(),
    destructor_map_(), classes_documentation_() {
}

}  // namespace switcher
