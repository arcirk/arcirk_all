#ifndef BTASKS_GLOBAL_H
#define BTASKS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BTASKS_LIBRARY)
#  define BTASKS_EXPORT Q_DECL_EXPORT
#else
#  define BTASKS_EXPORT Q_DECL_IMPORT
#endif

#endif // BTASKS_GLOBAL_H
