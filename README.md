# DualSimplex

Project that implements the Dual Simplex method of
[Operation Research - a.y. 2023-2024](https://www.unibo.it/en/teaching/course-unit-catalogue/course-unit/2023/378226)
([Computer Science and Engineering](https://corsi.unibo.it/1cycle/ComputerScienceEngineering)).

## Author

[@EnryMarch10](https://github.com/EnryMarch10)

## Behavior

### General

This C++ program solves linear programming problems using the Dual Simplex method using
[Eigen](https://eigen.tuxfamily.org/dox/index.html) library.

Files will be processed one by one reading the input file and writing the relative output file(s).

In the following folder paths the repo path is considered as the current folder and referred to as the _._ symbol.

It is ***assumed*** the existence of _./src/DualSimplex/data/in_ and _./src/DualSimplex/data/out_ folders.
Note that you would do better if you clean _./src/DualSimplex/data/out_ folder before to execute the program or you could
not understand the output clearly. For example if you change the program and/or take in input file of name already existent in
solutions.

The program can be run with either a single file or multiple files.
The file names must be specified as arguments of the _DualSimplex_ executable file created by compiling the code.

For example to execute an example of the program you could write in a Unix shell:

```sh
DualSimplex test.dat
```

To execute the _DualSimplex_ file in this way you have to be placed inside the folder in which the executable is placed.
If you compile without modifying the setup in vscode the executable will be placed in the _./src/DualSimplex/build_ folder.

#### Input

The linear programming problems are read from correctly formatted text file(s) taken as input.

The file(s) specified as arguments of the executable by default will be searched inside the _./src/DualSimplex/data/in_ folder
and if they exists they will be interpreted.
In practice you can specify different kind of arguments as arguments: a file, multiple files, a folder path, multiple folder paths
or the * symbol.
So there are tree types of path that the program can interpret:
- file path -> it has to be a valid one;
- folder path -> it means all the files inside the specified folder;
- \* symbol -> means all the files inside the _./src/DualSimplex/data/in_ path.

In the example above only _./src/DualSimplex/data/in/test.dat_ is interpreted.

To execute all files used for testing of the program write (don't forget to quote *):

```sh
DualSimplex '*'
```

If the file(s) are bad formatted the program will abort, otherwise they will be processed by the program.

#### Output

The output files will contain Tableau representations of the different steps of the elaboration (a tabular format discussed in
the documentation).

They will be placed inside folder(s) created by the program with the same name of each input file(s) without the extension.
The output folder(s) will be placed in the _./src/DualSimplex/data/out_ folder.

In the example above the output files will be placed in _./src/DualSimplex/data/out/test_ folder.
Obviously this folder is created automatically by the program.

The files will have increasing Tableau names: _tableau\_1_, _tableau\_2_ and so on until the last solution Tableau.
The first file _tableau\_1_ is always the Tableau as read but formatted in a different way.

The output Tableau can be formatted in different ways depending on the number of equations and variables concerned with the problem.
If it is to large only the objective function value, the reduced costs row and the known terms column will be printed.

## More

For more info read the [documentation](./doc/documentation.md).

## License

[MIT](https://choosealicense.com/licenses/mit/)
