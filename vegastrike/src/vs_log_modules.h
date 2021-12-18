/*
 * Vega Strike Log Modules Helpers
 * Copyright (C) 2021 Vincent Sallaberry
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef __VS_LOG_MODULES_H_
#define __VS_LOG_MODULES_H_

#include "log.h"

#define MMAP_LOGNAME mmap
VSLOG_DECL_CACHE(MMAP_LOGNAME)
#define MMAP_LOG(level, ...) VS_LOG_M(MMAP_LOGNAME, level, __VA_ARGS__)
#define MMAP_DBG(level, ...) VS_DBG_M(MMAP_LOGNAME, level, __VA_ARGS__)

#define BASE_LOGNAME base
VSLOG_DECL_CACHE(BASE_LOGNAME)
#define BASE_LOG(level, ...) VS_LOG_M(BASE_LOGNAME, level, __VA_ARGS__)
#define BASE_DBG(level, ...) VS_DBG_M(BASE_LOGNAME, level, __VA_ARGS__)

#define WINSYS_LOGNAME winsys
VSLOG_DECL_CACHE(WINSYS_LOGNAME)
#define WINSYS_LOG(level, ...) VS_LOG_M(WINSYS_LOGNAME, level, __VA_ARGS__)
#define WINSYS_DBG(level, ...) VS_DBG_M(WINSYS_LOGNAME, level, __VA_ARGS__)
#define WINSYS_LOG_START(level, ...) VS_LOG_START_M(WINSYS_LOGNAME, level, __VA_ARGS__)
#define WINSYS_LOG_END(level, ...) VS_LOG_END_M(WINSYS_LOGNAME, level, __VA_ARGS__)

#define UNIT_LOGNAME unit
VSLOG_DECL_CACHE(UNIT_LOGNAME)
#define UNIT_LOG(level, ...) VS_LOG_M(UNIT_LOGNAME, level, __VA_ARGS__)
#define UNIT_DBG(level, ...) VS_DBG_M(UNIT_LOGNAME, level, __VA_ARGS__)
#define UNIT_LOG_START(level, ...) VS_LOG_START_M(UNIT_LOGNAME, level, __VA_ARGS__)
#define UNIT_LOG_END(level, ...) VS_LOG_END_M(UNIT_LOGNAME, level, __VA_ARGS__)

#define GAME_LOGNAME game
VSLOG_DECL_CACHE(GAME_LOGNAME)
#define GAME_LOG(level, ...) VS_LOG_M(GAME_LOGNAME, level, __VA_ARGS__)
#define GAME_DBG(level, ...) VS_DBG_M(GAME_LOGNAME, level, __VA_ARGS__)
#define GAME_LOG_START(level, ...) VS_LOG_START_M(GAME_LOGNAME, level, __VA_ARGS__)
#define GAME_LOG_END(level, ...) VS_LOG_END_M(GAME_LOGNAME, level, __VA_ARGS__)

#define JOY_LOGNAME joystick
VSLOG_DECL_CACHE(JOY_LOGNAME)
#define JOY_LOG(level, ...) VS_LOG_M(JOY_LOGNAME, level, __VA_ARGS__)
#define JOY_DBG(level, ...) VS_DBG_M(JOY_LOGNAME, level, __VA_ARGS__)
#define JOY_LOG_START(level, ...) VS_LOG_START_M(JOY_LOGNAME, level, __VA_ARGS__)
#define JOY_LOG_END(level, ...) VS_LOG_END_M(JOY_LOGNAME, level, __VA_ARGS__)

#endif
