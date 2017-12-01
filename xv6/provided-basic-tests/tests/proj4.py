import sys, os, inspect

import toolspath
from testing import Xv6Test, Xv6Build

ctestsDir = os.path.basename(__file__)[:-3]

curdir = os.path.realpath(os.path.dirname(inspect.getfile(inspect.currentframe())))
def get_description(name):
  cfile = os.path.join(curdir, 'ctests', ctestsDir, name + '.c')
  with open(cfile, 'r') as f:
    desc = f.readline()
  desc = desc.strip()
  desc = desc[2:]
  desc = "Description:" + desc
  if desc[-2:] == '*/':
    desc = desc[:-2]
  return desc.strip()

test_values = [
  #{'name': 'tagFile'},
  #{'name': 'tagFile-bad-fd'},
  #{'name': 'removeFileTag'},
  #{'name': 'updateFileTag'},
  #{'name': 'tagFileLimit'},
  #{'name': 'getAllTags'},
  #{'name': 'getFilesByTag'},
  #{'name': 'reopen'},
  #{'name': 'retag'},
  {'name': 'tagFile'},
  {'name': 'tagFile2'},
  {'name': 'tagFile3'},
  {'name': 'tagFile4'},
  {'name': 'tagFile5'},
  {'name': 'tagFile6'},
  {'name': 'tagFile-bad-fd'},
  {'name': 'tagFile-bad-key'},
  {'name': 'tagFile-bad-val'},
  {'name': 'removeFileTag'},
  {'name': 'remove-bad-key'},
  {'name': 'remove-bad-fd'},
  {'name': 'getTag-long-value'},
  {'name': 'getTag-bad-fd'},
  {'name': 'getTag-bad-key'},
  {'name': 'ec-tagFile7'},
  {'name': 'ec-getAllTags'},
  {'name': 'ec-getAllTags2'},
  {'name': 'ec-getFilesByTag'},
  {'name': 'ec-getFilesByTag2'},
  {'name': 'ec-arbitraryLength'},
  {'name': 'ec-arbitraryLength2'},
  {'name': 'ec-manyBlocks'},
  {'name': 'ec-manyBlocks2'},
  {'name': 'ec-manyBlocks3'},
]

all_tests = []
build_test = Xv6Build
for test in test_values:
  testname = test['name']

  if 'point_value' in test.keys():
    point_value = test['point_value']
  else:
    point_value = 0

  if 'timeout' in test.keys():
    timeout = test['timeout']
  else:
    timeout = 10

  members = {
      'name': testname,
      'tester': 'ctests/' + ctestsDir + '/' + testname + '.c',
      'description': get_description(testname),
      'timeout': timeout,
      'point_value': point_value
      }
  newclass = type(testname, (Xv6Test,), members)
  all_tests.append(newclass)
  setattr(sys.modules[__name__], testname, newclass)

class usertests(Xv6Test):
  name = 'usertests'
  tester = 'ctests/' + ctestsDir + '/usertests.c'
  description = get_description('usertests')
  timeout = 240

# To include full regression test by running usertests, uncomment the line below.
#all_tests.append(usertests)

from testing.runtests import main
main(build_test, all_tests)
