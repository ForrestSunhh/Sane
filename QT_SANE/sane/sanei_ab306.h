/* sane - Scanner Access Now Easy.
   Copyright (C) 1997 Andreas Czechanowski
   This file is part of the SANE package.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

   As a special exception, the authors of SANE give permission for
   additional uses of the libraries contained in this release of SANE.

   The exception is that, if you link a SANE library with other files
   to produce an executable, this does not by itself cause the
   resulting executable to be covered by the GNU General Public
   License.  Your use of that executable is in no way restricted on
   account of linking the SANE library code into it.

   This exception does not, however, invalidate any other reasons why
   the executable file might be covered by the GNU General Public
   License.

   If you submit changes to SANE to the maintainers to be included in
   a subsequent release, you agree by submitting the changes that
   those changes may be distributed with this exception intact.

   If you write modifications of your own for SANE, it is your choice
   whether to permit this exception to apply to your modifications.
   If you do not wish that, delete this exception notice.  */
#ifndef sanei_ab306_h
#define sanei_ab306_h

#include <sys/types.h>

#include <sane/sane.h>

SANE_Status	sanei_ab306_open (const char *dev, int *fd);
void		sanei_ab306_close (int fd);
void		sanei_ab306_exit (void);

SANE_Status	sanei_ab306_get_io_privilege (int fd);
SANE_Status	sanei_ab306_test_ready (int fd);
SANE_Status	sanei_ab306_cmd (int fd, const void *src, size_t src_size,
				 void *dst, size_t *dst_size);
SANE_Status	sanei_ab306_rdata (int fd, int planes,
				   SANE_Byte *buf, int lines, int bpl);

#endif /* sanei_ab306_h */
