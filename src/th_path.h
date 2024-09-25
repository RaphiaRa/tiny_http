#ifndef TH_PATH_H
#define TH_PATH_H

#include "th_config.h"
#include "th_dir.h"
#include "th_string.h"

/**
 * @brief th_path provides a bunch of helper functions to work with paths.
 */

/**
 * @brief th_path_resolve resolves a path to a absolute path.
 * @param dir The directory to resolve the path against.
 * @param path The path to resolve.
 * @param out The resolved path.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
TH_PRIVATE(th_err)
th_path_resolve_against(th_string path, th_dir* dir, th_heap_string* out);

TH_PRIVATE(th_err)
th_path_resolve(th_string path, th_heap_string* out);

TH_PRIVATE(bool)
th_path_is_within(th_string path, th_dir* dir);

TH_PRIVATE(bool)
th_path_is_hidden(th_string path);

#endif
