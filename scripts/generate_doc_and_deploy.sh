#!/bin/bash
################################################################################
# Title         : generate_doc_and_deploy.sh
# Date created  : 22/12/2020
# Notes         :
__AUTHOR__="Federico Bolelli" # Script adapted from the one designed by "Jeroen de Bruijn" (https://gist.github.com/vidavidorra/548ffbcdae99d752da02)
# Preconditions:
# - Packages doxygen doxygen-doc doxygen-latex doxygen-gui graphviz
#   must be installed.
# - Doxygen configuration file must have the destination directory empty and
#   source code directory with a $(TRAVIS_BUILD_DIR) prefix.
# - An gh-pages branch should already exist. See below for mor info on hoe to
#   create a gh-pages branch.
#
# Required global variables:
# - GH_REPO_NAME        : The name of the repository.
# - GH_REPO_REF         : The GitHub reference to the repository.
# - GH_REPO_TOKEN       : Secure token to the github repository (Repo Secret).
#
# For information on how to encrypt variables for Travis CI please go to
# https://docs.travis-ci.com/user/environment-variables/#Encrypted-Variables
# or https://gist.github.com/vidavidorra/7ed6166a46c537d3cbd2
# For information on how to create a clean gh-pages branch from the master
# branch, please go to https://gist.github.com/vidavidorra/846a2fc7dd51f4fe56a0
#
# This script will generate Doxygen documentation and push the documentation to
# the gh-pages branch of a repository specified by GH_REPO_REF.
# Before this script is used there should already be a gh-pages branch in the
# repository.
# 
################################################################################

################################################################################
##### Setup this script and get the current gh-pages branch.               #####
echo 'Setting up the script...'
# Exit with nonzero exit code if anything fails
set -e

# Get the current gh-pages branch
git clone -b gh-pages https://github.com/prittt/GRAPHGEN.git gh-pages # when public
# git clone -b gh-pages "https://${GH_REPO_TOKEN}@${GH_REPO_REF}" # if private (not working)


################################################################################
##### Configure git for pushing the new doc.                               #####
# Set the push default to simple i.e. push only the current branch.
git config --global push.default simple
# Pretend to be an user called GitHub Actions.
git config user.name "GitHub Actions"
git config user.email "actions@github.org"

################################################################################
##### Generate the Doxygen documentation (from master) and log the output. #####
echo 'Generating Doxygen code documentation...'

cwd=$(pwd)

# Redirect both stderr and stdout to the log file and the console.
cd doc/doxygen
doxygen 2>&1 | tee doxygen.log
cd $cwd

################################################################################
##### Copy generated doc from master folder to gh-pages one.               #####
dir=gh-pages/html
if [ -d "$dir" ]; then rm -Rf $dir; fi
mkdir -p $dir
mv doc/html gh-pages/html

################################################################################
##### Upload the documentation to the gh-pages branch of the repository.   #####
# Only upload if Doxygen successfully created the documentation.
# Check this by verifying that the html directory and the file html/index.html
# both exist. This is a good indication that Doxygen did it's work.
cd gh-pages/$i
if ([ -d "html" ] && [ -f "html/index.html" ]) || [ -f "${zip_name}" ]; then

	echo 'Uploading documentation to the gh-pages branch...'
	# Add everything in this directory (the Doxygen code documentation) to the
	# gh-pages branch.
	# GitHub is smart enough to know which files have changed and which files have
	# stayed the same and will only update the changed files.
	git add --all

	# Commit the added files with a title and description containing the Travis CI
	# build number and the GitHub commit reference that issued this build.
	git commit -m "Deploy code docs to GitHub Pages." -m "Commit: ${GITHUB_SHA}"

	# Force push to the remote gh-pages branch.
	# The ouput is redirected to /dev/null to hide any sensitive credential data
	# that might otherwise be exposed.
	git push "https://${GH_REPO_TOKEN}@${GH_REPO_REF}" > /dev/null 2>&1
else
	echo '' >&2
	echo 'Warning: No documentation (html) files have been found!' >&2
	exit 1
fi

done