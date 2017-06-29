# $FreeBSD: releng/10.3/sys/modules/nullfs/Makefile 89260 2002-01-11 15:49:02Z ru $

.PATH: ${.CURDIR}/../../fs/cryptofs

KMOD=	cryptofs
SRCS=	vnode_if.h \
	crypto_subr.c crypto_vfsops.c crypto_vnops.c

.include <bsd.kmod.mk>
