.include <bsd.own.mk>

PREFIX?=	/usr/local
LIBDIR=		${PREFIX}/lib/pkg/commands
SHLIB_DIR?=	${LIBDIR}/
SHLIB_NAME?=	${PLUGIN_NAME}.so

PLUGIN_NAME=	mystats
SRCS=		mystats.c

PKGFLAGS!=	pkgconf --cflags pkg
CFLAGS+=	${PKGFLAGS}

beforeinstall:
	${INSTALL} -d ${LIBDIR}

.include <bsd.lib.mk>
