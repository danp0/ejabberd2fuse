#!/usr/bin/env bash

function usage()
{
  echo "usage:"
  echo ""
  echo "$0 <directory>"
  echo ""
  echo "where <directory> is the directory to unmount"
}

if [ "$#" == "1" ]
then
  fusermount -u "$1"
else
  usage
fi
