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
#include "vs_log_modules.h"

/* declaring log modules is not mandatory, it allows to have dedicated macros
 * and to have an internal cache for its log level, without looking in the hashmap */

VSLOG_DEF_CACHE(MMAP_LOGNAME)
VSLOG_DEF_CACHE(BASE_LOGNAME)
VSLOG_DEF_CACHE(WINSYS_LOGNAME)
VSLOG_DEF_CACHE(UNIT_LOGNAME)
VSLOG_DEF_CACHE(GAME_LOGNAME)
VSLOG_DEF_CACHE(JOY_LOGNAME)
VSLOG_DEF_CACHE(CONFIG_LOGNAME)
VSLOG_DEF_CACHE(VSFS_LOGNAME)
VSLOG_DEF_CACHE(UNIVERSE_LOGNAME)
VSLOG_DEF_CACHE(PYTHON_LOGNAME)
VSLOG_DEF_CACHE(GL_LOGNAME)
VSLOG_DEF_CACHE(GLTEX_LOGNAME)
VSLOG_DEF_CACHE(GFX_LOGNAME)
