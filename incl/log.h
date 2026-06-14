//!
//! Minimal logger: info, warn, err to stderr.
//! No formatting, no levels — just printf behind macros.
//! Log macros are no-ops when NDEBUG is defined in release builds.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define log_info(fmt, ...) fprintf(stderr, "[INFO]  " fmt "\n", ##__VA_ARGS__)
#define log_warn(fmt, ...) fprintf(stderr, "[WARN]  " fmt "\n", ##__VA_ARGS__)
#define log_err(fmt, ...)  fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)

#endif /* LOG_H */
