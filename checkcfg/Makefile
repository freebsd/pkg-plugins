.include <bsd.own.mk>

PREFIX?=	/usr/local
LIBDIR=		${PREFIX}/lib/pkg/
SHLIB_DIR?=	${LIBDIR}/
SHLIB_NAME?=	${PLUGIN_NAME}.so

PLUGIN_NAME=	checkcfg
SRCS=		checkcfg.c

PKGFLAGS!=	pkgconf --cflags pkg
CFLAGS+=	${PKGFLAGS}

PKG=		pkg
PKG_CFG=	pkg.conf

beforeinstall:
	${INSTALL} -d ${LIBDIR}

run: ${SHLIB_NAME}
	${PKG} -C ${PKG_CFG} plugins

test: ${SHLIB_NAME}
	${PKG} -C ${PKG_CFG} plugins | egrep "^checkcfg[[:space:]]+Configuration available: YES[[:space:]]"

.include <bsd.lib.mk>
