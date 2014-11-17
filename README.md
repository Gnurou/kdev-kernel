KDevelop Linux Kernel Development Plugin
========================================

This plugin offers a "Linux Kernel" project type to KDevelop that makes it easy and comfortable to work on the Linux kernel.

The raw KDevelop comes with extremely useful cross-reference and code parsing tools, but they do not apply well to the Linux kernel. Problems are:

- The kernel source is huge, and parsing all the source code of all drivers is long and unpractical.
- Some implicitly declared macros (like `__KERNEL__`) are needed for the code to be parsed correctly. Without them lots of functions and data structures will appear as undeclared.
- A lot of the code (power management, SMP, etc) is conditionally compiled depending on the kernel configuration which is translated into more C defines. These defines are not seen by KDevelop and a huge part of relevant code is thus not parsed.
- All the same, architecture-dependant code cannot be parsed without a proper configuration

Features
--------

- Parses your kernel configuration file to define the right macros and only show the source code that is actually compiled by your configuration.
- Limits the parser to the kernel files and drivers that are appropriate for your configuration

Installing
----------
This plugin requires KDevelop 4.7.

Just typing `cmake . && make && sudo make install` should be enough to have the project installed. Then in KDevelop, do "Project -> Open/Import Project..." and select the root Makefile of your Linux kernel. When prompted for the type of the project, choose "Linux Kernel".

Then open the project's configuration and in the "Linux Kernel" pane select your kernel configuration file. You will then need to restart KDevelop so the right kernel files are parsed with the right defines.

Limitations
-----------

- KDevelop's parser is C++-oriented and does not support some of the C features used in the kernel like designated initializers. This needs to be fixed upstream.

Contribute & Feedback
---------------------

This plugin is written by Alexandre Courbot and is released under the GPLv3. You can report issues, suggest features or contribute fixes on its Github page:

https://github.com/Gnurou/kdev-kernel

Happy Comfortable Hacking! :)

