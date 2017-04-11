#!/usr/bin/env bash

function usage()
{
  echo "usage:"
  echo ""
  echo "$0 <directory>"
  echo "where <directory> is the directory to mount the ejabberd2fuse example"
}

if [ "$#" == 1 ]
then
  if [ ! -d "$1" ]
  then
    mkdir "$1"
  fi
  build/bin/ejabberd2fuse -d -s -f "$1"
else
  usage
fi
