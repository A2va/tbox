/*!The Treasure Box Library
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (C) 2009-present, TBOOX Open Source Group.
 *
 * @author      ruki
 * @file        pipe.c
 * @ingroup     platform
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME                "pipe"
#define TB_TRACE_MODULE_DEBUG               (1)

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "pipe.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
#if defined(TB_CONFIG_OS_WINDOWS) || defined(__COSMOPOLITAN__)
#   include "windows/pipe.c"
#elif defined(TB_CONFIG_POSIX_HAVE_PIPE) || \
        defined(TB_CONFIG_POSIX_HAVE_PIPE2) || \
            defined(TB_CONFIG_POSIX_HAVE_MKFIFO)
#   include "posix/pipe.c"
#endif

void _get_sys_name(tb_char_t* name) {
    struct utsname buffer;
	if (uname(&buffer) == 0)
	{
		if (tb_strstr(buffer.sysname, "Darwin"))
			tb_strcpy(name, "macosx");
		else if (tb_strstr(buffer.sysname, "Linux"))
			tb_strcpy(name, "linux");
		else if (tb_strstr(buffer.sysname, "Windows"))
			tb_strcpy(name, "windows");
	}
}


tb_pipe_file_ref_t tb_pipe_file_init(tb_char_t const* name, tb_size_t mode, tb_size_t buffer_size)
{
    tb_char_t sysname[16];
	_get_sys_name(sysname);
    if (tb_strcmp(sysname, "windows") == 0)
    {
        return tb_pipe_file_init_win(name, mode, buffer_size);
    } else {
        return tb_pipe_file_init_posix(name, mode, buffer_size);
    }
}
tb_bool_t tb_pipe_file_init_pair(tb_pipe_file_ref_t pair[2], tb_size_t mode[2], tb_size_t buffer_size)
{
    tb_char_t sysname[16];
	_get_sys_name(sysname);
    if (tb_strcmp(sysname, "windows") == 0) return tb_pipe_file_init_pair_win(pair, mode, buffer_size);
    else return tb_pipe_file_init_pair_posix(pair, mode, buffer_size);
}
tb_bool_t tb_pipe_file_exit(tb_pipe_file_ref_t file)
{
    tb_char_t sysname[16];
	_get_sys_name(sysname);
    if (tb_strcmp(sysname, "windows") == 0) return tb_pipe_file_exit_win(file);
    else return tb_pipe_file_exit_posix(file);
}
tb_long_t tb_pipe_file_read(tb_pipe_file_ref_t file, tb_byte_t* data, tb_size_t size)
{
    tb_char_t sysname[16];
	_get_sys_name(sysname);
    if (tb_strcmp(sysname, "windows") == 0) return tb_pipe_file_read_win(file, data, size);
    else return tb_pipe_file_read_posix(file, data, size);
}
tb_long_t tb_pipe_file_write(tb_pipe_file_ref_t file, tb_byte_t const* data, tb_size_t size)
{
    tb_char_t sysname[16];
	_get_sys_name(sysname);
    if (tb_strcmp(sysname, "windows") == 0) return tb_pipe_file_write_win(file, data, size);
    else return tb_pipe_file_write_posix(file, data, size);
}
tb_long_t tb_pipe_file_wait(tb_pipe_file_ref_t file, tb_size_t events, tb_long_t timeout)
{
    tb_char_t sysname[16];
	_get_sys_name(sysname);
    if (tb_strcmp(sysname, "windows") == 0) return tb_pipe_file_wait_win(file, events, timeout);
    else return tb_pipe_file_wait_posix(file, events, timeout);
}
tb_long_t tb_pipe_file_connect(tb_pipe_file_ref_t file)
{
    tb_char_t sysname[16];
	_get_sys_name(sysname);
    if (tb_strcmp(sysname, "windows") == 0) return tb_pipe_file_connect_win(file);
    else return tb_pipe_file_connect_posix(file);
}


tb_bool_t tb_pipe_file_bread(tb_pipe_file_ref_t file, tb_byte_t* data, tb_size_t size)
{
    tb_long_t (*read_func)(tb_pipe_file_ref_t, tb_byte_t*, tb_size_t);
    tb_char_t sysname[16];
    _get_sys_name(sysname);
    if (tb_strcmp(sysname, "windows") == 0) 
        read_func = tb_pipe_file_read_win;
    else 
        read_func = tb_pipe_file_read_posix;

    // read data
    tb_size_t read = 0;
    tb_long_t wait = 0;
    while (read < size)
    {
        // read it

        tb_long_t real = read_func(file, data + read, tb_min(size - read, 8192));

        // has data?
        if (real > 0)
        {
            read += real;
            wait = 0;
        }
        // no data? wait it
        else if (!real && !wait)
        {
            // wait it
            wait = tb_pipe_file_wait(file, TB_PIPE_EVENT_READ, -1);
            tb_check_break(wait > 0);
        }
        // failed or end?
        else break;
    }
    return read == size;
}
tb_bool_t tb_pipe_file_bwrit(tb_pipe_file_ref_t file, tb_byte_t const* data, tb_size_t size)
{
    tb_long_t (*write_func)(tb_pipe_file_ref_t, tb_byte_t const*, tb_size_t);
    tb_char_t sysname[16];
    _get_sys_name(sysname);
    if (tb_strcmp(sysname, "windows") == 0)
        write_func = tb_pipe_file_write_win;
    else 
        read_func = tb_pipe_file_write_posix;

    // writ data
    tb_size_t writ = 0;
    tb_long_t wait = 0;
    while (writ < size)
    {
        // write it
        tb_long_t real = write_func(file, data + writ, tb_min(size - writ, 8192));

        // has data?
        if (real > 0)
        {
            writ += real;
            wait = 0;
        }
        // no data? wait it
        else if (!real && !wait)
        {
            // wait it
            wait = tb_pipe_file_wait(file, TB_PIPE_EVENT_WRIT, -1);
            tb_check_break(wait > 0);
        }
        // failed or end?
        else break;
    }
    return writ == size;
}
