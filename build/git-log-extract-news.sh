#!/bin/bash

usage ()
{
  echo "Usage: $0 <version> [<tag prefix>]"
}

find_git_tag ()
{
  local start=$1
  local prefix=$2

  echo $(git tag | grep "^$prefix$start" | sort -r | head -n 1)
}

previous_git_tag ()
{
  local start=$1
  local prefix=$2
  local component=${start##*\.}
  local new_start=${start%\.*}

  if [ $component == $new_start ]; then

    # Processing major version, no more sub-parts.
    if [ $component -eq "0" ]; then
      echo ""
    else
      echo $(find_git_tag $(($component - 1)) $prefix)
    fi

  elif [ $component -eq "0" ]; then

    # Find most recent version with lower higher version number part.
    echo $(previous_git_tag $new_start $prefix)

  else

    echo $(find_git_tag "$new_start."$((component - 1)) $prefix)

  fi
}

previous_version_number ()
{
  local start=$1
  local prefix=$2
  local component=${start##*\.}
  local new_start=${start%\.*}

  if [ $component -eq "0" ]; then
    echo $(previous_git_tag $new_start $prefix)
  else
    echo "$prefix$new_start."$((component - 1))
  fi
}

if [ "$1" = "" ]; then
  echo "Need version string"
  usage
  exit 1
fi

version=$1
tag_prefix=$2
previous_tag=$(previous_version_number $version $tag_prefix)
span=
if [ "$previous_tag" != "" ]; then
  # Print out previous version to stderr.
  echo "Version $previous_tag" 1>&2
  span="$previous_tag.."
fi

echo "Version $version", $(date +"%Y-%m-%d")
echo
git log $span | grep -E "    Fixes|    Feature"
echo

