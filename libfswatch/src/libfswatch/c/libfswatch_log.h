/*
 * Copyright (c) 2014-2015 Enrico M. Crisostomo
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 3, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBFSW_LOG_H
#  define LIBFSW_LOG_H

void libfsw_logf(const char * format, ...);
void libfsw_log(const char * msg);
void libfsw_perror(const char * msg);

#define FSW_LOG(msg)       libfsw_logf("%s: ", __func__); libfsw_log(msg)
#define FSW_LOGF(msg, ...) libfsw_logf("%s: ", __func__); libfsw_logf(msg, __VA_ARGS__)

#endif  /* LIBFSW_LOG_H */
