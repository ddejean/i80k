// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _STDARG_H_
#define _STDARG_H_

typedef char *va_list;
#define va_start(ap, p) (ap = (char *)(&(p) + 1))
#define va_arg(ap, type) ((type *)(ap += sizeof(type)))[-1]
#define va_end(ap)

#endif  // _STDARG_H_
