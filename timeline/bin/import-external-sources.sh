#!/bin/sh

# Copyright (C) 2008 Jonathan Moore Liles                                     #
# Copyright (C) 2023 Stazed                                                   #
#                                                                             #
# This program is free software; you can redistribute it and/or modify it     #
# under the terms of the GNU General Public License as published by the       #
# Free Software Foundation; either version 2 of the License, or (at your      #
# option) any later version.                                                  #
#                                                                             #
# This program is distributed in the hope that it will be useful, but WITHOUT #
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       #
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   #
# more details.                                                               #
#                                                                             #
# You should have received a copy of the GNU General Public License along     #
# with This program; see the file COPYING.  If not,write to the Free Software #
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  #


# USAGE:
#
#   $ import-external-sources ~/.local/share/nsm/'The Best Song Ever'/Non-Timeline-XT.nWCIU --dry-run
#   $ import-external-sources ~/'Ray Sessions'/'The Best Song Ever'/Non-Timeline-XT.nWCIU --dry-run
#
#   -- to print list of SYMLINKS to be imported
#
#   Remove --dry-run to execute.

fatal ()
{
    echo Error: "$1"
    echo 'Aborting!'
    cleanup
    exit 1
}

# The command line project location
PROJECT="$1"

cd "$PROJECT" || fatal "No such project"

[ -f history ] && [ -f info ] || fatal "Not a Non-Timeline project?"

[ -f .lock ] && fatal "Project appears to be in use"

# change into sources directory
cd sources

if [ "$2" = --dry-run ]
    then
        echo "Would import the following files:"
        find . -type l -ls
    else
        # Make a temporary directory for copying.
        mkdir tmp

        # Find all symlinks and copy to temp directory.
        # The copy will convert the symlink to the source of the link.
        find -type l -exec cp -t  tmp/ {} +

        # Remove the symlinks from sources directory.
        find -type l -print0 | xargs -0 rm || echo "Nothing to do..."

        # Move the temporary directory items to sources.
        mv tmp/*.* . || echo "NO SYMLINKS FOUND"

        # Remove the tmp directory.
        rmdir tmp
fi
