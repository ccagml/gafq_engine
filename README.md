cat  /etc/centos-release
    CentOS Linux release 7.3.1611 (Core) 
    CentOS Linux release 7.7.1908 (Core)

g++ (GCC) 4.8.5 20150623 (Red Hat 4.8.5-39)
Copyright © 2015 Free Software Foundation, Inc.
本程序是自由软件；请参看源代码的版权声明。本软件没有任何担保；
包括没有适销性和某一专用目的下的适用性担保。


-- 这里是capi的mysqllib
    wget https://downloads.mysql.com/archives/get/p/19/file/mysql-connector-c-6.1.11-linux-glibc2.12-x86_64.tar.gz


<!-- -- 安装boost
wget https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz
cd ~/b/3rd_source/boost
./bootstrap.sh --help
./bootstrap.sh --prefix=~/b/3rd/boost/prefix --libdir=~/b/3rd/boost/libdir --includedir=~/b/3rd/boost/includedir
./b2
./b2 install -->

安装boost 1.53.0
    yum install boost boost-devel

gdb
    set args --props ./config.json
    set env LD_LIBRARY_PATH /root/gafq_engine/3rd/mysqlcapi/lib
    b main


format
{ BasedOnStyle: LLVM, UseTab: Never, PenaltyBreakFirstLessLess: 120, IndentWidth: 4, TabWidth: 4, BreakBeforeBraces: Allman, AllowShortIfStatementsOnASingleLine: false, IndentCaseLabels: false, ColumnLimit: 200, AccessModifierOffset: -4, NamespaceIndentation: All, FixNamespaceComments: true }