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

## remove-unused-sources
#
# April 2008, Jonathan Moore Liles
#
# Simple script to scan a compacted Non-DAW project and remove all
# unused sources from disk.
#
# USAGE:
#
#     $ remove-unused-sources -m ~/.local/share/nsm/'The Best Song Ever'/Non-Timeline-XT.nWCIU
#     $ remove-unused-sources -m ~/'Ray Sessions'/'The Best Song Ever'/Non-Timeline-XT.nWCIU
#
# NOTES:
#
#     This script will not ask for comfirmation! It will ruthlessly
#     delete all unused sources! You have been warned.
#
# OPTIONS:
#     -m default, the unused sources are moved to /project/unused-sources.
#     -d will delete the unused-sources directory.
#     -n will do a dry-run and will list the files to be removed from the sources.
#     -c run the script even if the project is not compacted


DRY_RUN=
ONLY_COMPACTED=1
MOVE=1
ERROR=


TEMP=/tmp

fatal ()
{
    echo Error: "$1"
    echo 'Aborting!'
    ERROR=1
    cleanup
    exit 1
}

cleanup ()
{
    rm -f "${TEMP}/all-sources" "${TEMP}/used-sources"

    if [ "$MOVE" = 1 ]  # -m default copy to unused-sources directory
        then
            if [ "$DRY_RUN" != 1 ] && [ "$ERROR" != 1 ] # don't print if we never moved sources
                then
                    echo "Unused sources moved to ${PROJECT}/unused-sources"
            fi
    fi
}

set_diff ()
{
    diff --new-line-format '' --old-line-format '%L' --unchanged-line-format '' "$1" "$2"
}

remove_sources ()
{
    local TOTAL=0
    local FILE
    local SIZE
    local PSIZE
    while read FILE
    do

        PSIZE=`stat -c '%s' "${FILE}.peak" 2>/dev/null`
        SIZE=`stat -c '%s' "${FILE}" 2>/dev/null`

        PSIZE=${PSIZE:-0}

        if ! [ -f "${FILE}" ]
        then
            echo "Would remove \"${FILE}\", if it existed."
        else
            if [ "$DRY_RUN" = 1 ]
            then
                echo "Would remove: ${FILE}"
            else
                if [ "$MOVE" = 1 ]
                then
                    echo "Moving unused source \"${FILE}\"..."
                    mv -f ./"${FILE}" ./"${FILE}".peak ../unused-sources
                else
                    echo "Removing unused source \"${FILE}\"..."
                    rm -f ./"${FILE}" ./"${FILE}".peak
                fi
            fi

            TOTAL=$(( $TOTAL + $SIZE + $PSIZE ))
        fi

    done

    echo "...Freeing a total of $(($TOTAL / ( 1024 * 1024 ) ))MB"
}

usage ()
{
    fatal "Usage: $0 [-n] [-c] [-m|-d] path/to/project"
}


while getopts "dmnc" o
do
    case "$o" in
	d) MOVE= ;;
        m) MOVE=1 ;;
        n) DRY_RUN=1 ;;
        c) ONLY_COMPACTED= ;;
	\?) usage ;;
        *) echo "$o" && usage ;;
    esac
done

shift $(( $OPTIND - 1 ))
PROJECT="$1"

[ $# -eq 1 ] || usage

cd "$PROJECT" || fatal "No such project"

[ -f history ] && [ -f info ] || fatal "Not a Non-DAW project?"

[ -f .lock ] && fatal "Project appears to be in use"

if [ "$ONLY_COMPACTED" = 1 ]
then
    grep -v '\(^\{\|\}$\)\|create' history && fatal "Not a compacted project"
fi

echo "Scanning \"${PROJECT}\"..."

sed 's/^\s*Audio_Region.* :source "\(.*\)".*/\1/' snapshot | sort | uniq > "${TEMP}/used-sources"

cd sources || fatal "Can't change to source directory"

if [ "$DRY_RUN" != 1 ]  # don't make the unused-sources directory if this is a dry run
    then
        [ "$MOVE" = 1 ] && mkdir ../unused-sources 2>/dev/null
fi

ls -1 | grep -v '\.peak$' | sort > "${TEMP}/all-sources"

set_diff "${TEMP}/all-sources" "${TEMP}/used-sources" | remove_sources

cleanup

echo "Done."
