#!/bin/sh

brcfg_dir=$(realpath -s $(dirname "$0"))

cd $brcfg_dir/../buildroot-2020.02.8/
patch -p0 < $brcfg_dir/patches/buildroot/TmpSvnRepo4UbootLinux.patch
patch -p0 < $brcfg_dir/patches/buildroot/SvnRepoSingelCheckIn.patch
patch -p0 < $brcfg_dir/patches/buildroot/SvnRepoIgnore4Uboot.patch
patch -p0 < $brcfg_dir/patches/buildroot/SvnRepoIgnore4Linux.patch

echo "do export CREATE_SVN_REPOS=1 to create local linux and uboot svn repos for better patch creation"
