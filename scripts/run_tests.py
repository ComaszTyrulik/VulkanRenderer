import sys
import pytest

args = ['-s', '--cache-clear']
if sys.argv.__len__() >= 2:
    args.append(sys.argv[1])

pytest.main(args)
