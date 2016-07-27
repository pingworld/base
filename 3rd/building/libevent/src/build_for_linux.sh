#!/bin/sh

tar xf libevent-2.0.22-stable.tar.gz
cd libevent-2.0.22-stable

#
# 具体编译时需要改变prefix的路径
#
./configure --prefix=/home/liguoqing/codes/pwbase/3rd/building/libevent/bin/linux --datadir=/home/liguoqing/codes/pwbase/3rd/building/libevent/data --disable-openssl --disable-debug-mode

make & make install

cd -


