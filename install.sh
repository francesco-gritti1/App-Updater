#!/bin/bash



GIT_REPO=$1
REPO_NAME=$2
PKG_NAME=$3

git clone $GIT_REPO
sudo dpkg -i $REPO_NAME/install/$PKG_NAME.deb
sudo rm -r $REPO_NAME


exit 0
