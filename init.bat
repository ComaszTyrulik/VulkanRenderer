git submodule update --init --recursive
scripts\init.bat^
 & @echo ^@scripts\run_py_cmd.bat scripts\run.py %%* > run.bat^
 & run.bat conan^
 & run.bat cmake configure %*
