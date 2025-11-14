#!/bin/bash



if [ -z "$1" ]; then
  echo "Error: missing parameter"
  echo "Usage: install <git-repo-url>"
  exit 1
fi


GIT_REPO=$1
REPO_NAME="${GIT_REPO%/}"    
REPO_NAME="${REPO_NAME##*/}"     

echo "git clone $GIT_REPO"
echo "sudo ./$REPO_NAME/install"
echo "sudo rm -r $REPO_NAME"


git clone $GIT_REPO
if [ $? -ne 0 ]; then
  echo "Error: unable to clone $GIT_REPO."
  exit 1
fi

cd $REPO_NAME 
./install.sh
if [ $? -ne 0 ]; then
  echo "Error: unable to install $REPO_NAME."
fi

# always remove the repo
cd ..
sudo rm -r $REPO_NAME

exit 0
