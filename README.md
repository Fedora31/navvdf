# navvdf

This is a personal library whose purpose is to read, load and write
basic VDF files. It doesn't support anything other than the basic nodes
and subnodes.

As it is a library aimed at reading and *writing* VDF files, it's
aim is not efficiency. It tries to make the creation of trees
easy by treating nodes like files and folders, so you
`vdf_touch()` to create "files" (key-value pairs) and you
`vdf_mkdir()` to create folders. You also navigate between
directories with `vdf_nav()`.

As this is a personal library, everything is WIP and no support
is given.

## Limitations

- The separator between folders can't be used in the name of folders.
  By default it is `/` but it can be changed to another character.

- The name for the "here" and "back" folders are hardcoded to `.` and
  `..`.

## Concepts

There are 3 objects:

- Trees. This is where a VDF tree is stored. You can create them
  by hand or load them from a file.

- Positions. One could think of a Pos like a shell/editor whose
  filesystem is a Tree. There can be multiple ones per Tree.

- Entries. Files or folders. You should never work with them
  directly, but let the library handle them.

All functions in the library work either on a Tree or on a Pos.

Read the source and the comments for a better understanding.
