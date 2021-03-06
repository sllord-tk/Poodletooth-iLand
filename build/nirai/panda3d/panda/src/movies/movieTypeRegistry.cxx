// Filename: movieTypeRegistry.cxx
// Created by:  rdb (24Aug13)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "movieTypeRegistry.h"
#include "string_utils.h"
#include "config_movies.h"
#include "config_util.h"
#include "load_dso.h"

MovieTypeRegistry *MovieTypeRegistry::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: MovieTypeRegistry::make_audio
//       Access: Published, Static
//  Description: Obtains a MovieVideo that references a file.
////////////////////////////////////////////////////////////////////
PT(MovieAudio) MovieTypeRegistry::
make_audio(const Filename &name) {
  string ext = downcase(name.get_extension());

  // Make sure that the list of audio types has been read in.
  load_audio_types();

  // Maybe we need to load a module?
  if (_deferred_audio_types.count(ext)) {
    load_movie_library(_deferred_audio_types[ext]);
    _deferred_audio_types.erase(ext);
  }

  // Explicit extension is preferred over catch-all.
  if (_audio_type_registry.count(ext)) {
    MakeAudioFunc func = _audio_type_registry[ext];
    return (*func)(name);
  }

  // If we didn't find it, see if there was a type registered
  // with '*' as extension.  This is a catch-all loader.
  if (_deferred_audio_types.count("*")) {
    load_movie_library(_deferred_audio_types["*"]);
    _deferred_audio_types.erase("*");
  }

  if (_audio_type_registry.count("*")) {
    MakeAudioFunc func = _audio_type_registry["*"];
    return (*func)(name);
  }

  movies_cat.error()
    << "Support for audio files with extension ." << ext << " was not enabled.\n";

  return new MovieAudio("Load-Failure Stub");
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTypeRegistry::register_audio_type
//       Access: Published, Static
//  Description: Registers a MovieAudio type, so that files with
//               any of the given extensions will be loaded as this
//               type.  You may use * as a catch-all extension.
////////////////////////////////////////////////////////////////////
void MovieTypeRegistry::
register_audio_type(MakeAudioFunc func, const string &extensions) {
  vector_string words;
  extract_words(downcase(extensions), words);

  vector_string::const_iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    if (_audio_type_registry.count(*wi)) {
      movies_cat->warning()
        << "Attempt to register multiple audio types with extension " << (*wi) << "\n";
    } else {
      movies_cat->debug()
        << "Registered audio type with extension " << (*wi) << "\n";
    }
    _audio_type_registry[*wi] = func;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTypeRegistry::load_audio_types
//       Access: Published, Static
//  Description: Loads the list with audio types, if we haven't
//               already.
////////////////////////////////////////////////////////////////////
void MovieTypeRegistry::
load_audio_types() {
  static bool audio_types_loaded = false;

  if (!audio_types_loaded) {
    int num_unique_values = load_audio_type.get_num_unique_values();

    for (int i = 0; i < num_unique_values; i++) {
      string param = load_audio_type.get_unique_value(i);

      vector_string words;
      extract_words(param, words);

      if (words.size() == 1) {
        // Exactly one word: load the named library immediately.
        string name = words[0];
        Filename dlname = Filename::dso_filename("lib" + name + ".so");
        movies_cat.info()
          << "loading audio type module: " << name << endl;
        void *tmp = load_dso(get_plugin_path().get_value(), dlname);
        if (tmp == (void *)NULL) {
          movies_cat.warning()
            << "Unable to load " << dlname.to_os_specific()
            << ": " << load_dso_error() << endl;
        } else if (movies_cat.is_debug()) {
          movies_cat.debug()
            << "done loading audio type module: " << name << endl;
        }

      } else if (words.size() > 1) {
        // Multiple words: the first n words are filename extensions,
        // and the last word is the name of the library to load should
        // any of those filename extensions be encountered.
        size_t num_extensions = words.size() - 1;
        string library_name = words[num_extensions];

        for (size_t i = 0; i < num_extensions; i++) {
          string extension = downcase(words[i]);
          if (extension[0] == '.') {
            extension = extension.substr(1);
          }

          _deferred_audio_types[extension] = library_name;
        }
      }
    }

    audio_types_loaded = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTypeRegistry::make_video
//       Access: Published, Static
//  Description: Obtains a MovieVideo that references a file.
////////////////////////////////////////////////////////////////////
PT(MovieVideo) MovieTypeRegistry::
make_video(const Filename &name) {
  string ext = downcase(name.get_extension());

  // Make sure that the list of video types has been read in.
  load_video_types();

  // Maybe we need to load a module?
  if (_deferred_video_types.count(ext)) {
    load_movie_library(_deferred_video_types[ext]);
    _deferred_video_types.erase(ext);
  }

  // Explicit extension is preferred over catch-all.
  if (_video_type_registry.count(ext)) {
    MakeVideoFunc func = _video_type_registry[ext];
    return (*func)(name);
  }

  // If we didn't find it, see if there was a type registered
  // with '*' as extension.  This is a catch-all loader.
  if (_deferred_video_types.count("*")) {
    load_movie_library(_deferred_video_types["*"]);
    _deferred_video_types.erase("*");
  }

  if (_video_type_registry.count("*")) {
    MakeVideoFunc func = _video_type_registry["*"];
    return (*func)(name);
  }

  movies_cat.error()
    << "Support for video files with extension ." << ext << " was not enabled.\n";

  return new MovieVideo("Load-Failure Stub");
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTypeRegistry::register_video_type
//       Access: Published, Static
//  Description: Registers a MovieVideo type, so that files with
//               any of the given extensions will be loaded as this
//               type.  You may use * as a catch-all extension.
////////////////////////////////////////////////////////////////////
void MovieTypeRegistry::
register_video_type(MakeVideoFunc func, const string &extensions) {
  vector_string words;
  extract_words(downcase(extensions), words);

  vector_string::const_iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    if (_video_type_registry.count(*wi)) {
      movies_cat->warning()
        << "Attempt to register multiple video types with extension " << (*wi) << "\n";
    } else {
      movies_cat->debug()
        << "Registered video type with extension " << (*wi) << "\n";
    }
    _video_type_registry[*wi] = func;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTypeRegistry::load_video_types
//       Access: Published, Static
//  Description: Loads the list with video types, if we haven't
//               already.
////////////////////////////////////////////////////////////////////
void MovieTypeRegistry::
load_video_types() {
  static bool video_types_loaded = false;

  if (!video_types_loaded) {
    int num_unique_values = load_video_type.get_num_unique_values();

    for (int i = 0; i < num_unique_values; i++) {
      string param = load_video_type.get_unique_value(i);

      vector_string words;
      extract_words(param, words);

      if (words.size() == 1) {
        // Exactly one word: load the named library immediately.
        string name = words[0];
        Filename dlname = Filename::dso_filename("lib" + name + ".so");
        movies_cat.info()
          << "loading video type module: " << name << endl;
        void *tmp = load_dso(get_plugin_path().get_value(), dlname);
        if (tmp == (void *)NULL) {
          movies_cat.warning()
            << "Unable to load " << dlname.to_os_specific()
            << ": " << load_dso_error() << endl;
        } else if (movies_cat.is_debug()) {
          movies_cat.debug()
            << "done loading video type module: " << name << endl;
        }

      } else if (words.size() > 1) {
        // Multiple words: the first n words are filename extensions,
        // and the last word is the name of the library to load should
        // any of those filename extensions be encountered.
        size_t num_extensions = words.size() - 1;
        string library_name = words[num_extensions];

        for (size_t i = 0; i < num_extensions; i++) {
          string extension = downcase(words[i]);
          if (extension[0] == '.') {
            extension = extension.substr(1);
          }

          _deferred_video_types[extension] = library_name;
        }
      }
    }

    video_types_loaded = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTypeRegistry::load_library
//       Access: Published, Static
//  Description: Loads the module.
////////////////////////////////////////////////////////////////////
void MovieTypeRegistry::
load_movie_library(const string &name) {
  Filename dlname = Filename::dso_filename("lib" + name + ".so");
  movies_cat.info()
    << "loading video type module: " << name << endl;
  void *tmp = load_dso(get_plugin_path().get_value(), dlname);

  if (tmp == (void *)NULL) {
    movies_cat.warning()
      << "Unable to load " << dlname.to_os_specific()
      << ": " << load_dso_error() << endl;
  } else if (movies_cat.is_debug()) {
    movies_cat.debug()
      << "done loading video type module: " << name << endl;
  }
}
