/**************************************************************************
 * Copyright (C) 2018-2019  Junlon2006
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **************************************************************************
 *
 * Description : uni_ringbuf.h
 * Author      : junlon2006@163.com
 * Date        : 2019.03.17
 *
 **************************************************************************/
#ifndef RINGBUF_INC_UNI_DATABUF_H_
#define RINGBUF_INC_UNI_DATABUF_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* RingBufferHandle;

RingBufferHandle RingBufferCreate(int size);
int RingBufferDestroy(RingBufferHandle handle);
int RingBufferPeek(char *dst, int readlen, RingBufferHandle handle);
int RingBufferClear(RingBufferHandle handle);
int RingBufferGetFreeSize(RingBufferHandle handle);
int RingBufferGetDataSize(RingBufferHandle handle);
int RingBufferWrite(RingBufferHandle handle, char *src, int writelen);
int RingBufferRead(char *dst, int readlen, RingBufferHandle handle);

#ifdef __cplusplus
}
#endif
#endif
