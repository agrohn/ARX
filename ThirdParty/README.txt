# This directory will contain platform-specific ARToolkit5 installations.

## Subdirectory layout must be:
{include,lib}/Linux/<arch>/
{include,lib}/Android/<arch>/

## Which in practice, is something like this:

include/Linux/x86_64-unknown-linux-gnu 
lib/Linux/x86_64-unknown-linux-gnu 
include/Android/armeabi-v7a
lib/Android/armeabi-v7a

# What to copy

You need every .LIB and .DLL there is in ARToolkitX, put them under lib.
remember to also copy necessary ones under depends.


