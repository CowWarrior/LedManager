#Base Script: Péter Vojnisek
#https://stackoverflow.com/questions/56923895/auto-increment-build-number-using-platformio
#https://gitlab.com/pvojnisek/buildnumber-for-platformio
import datetime
tm = datetime.datetime.today()

FILENAME_BUILDNO = 'versioning'
FILENAME_VERSION_H = 'include/version.h'
V_MAJOR = "1"
V_MINOR = "3"
version_short = 'v' + V_MAJOR + '.' + V_MINOR + '.' + str(tm.year)[-2:]+('0'+str(tm.month))[-2:]+('0'+str(tm.day))[-2:]
version_long = version_short + ' Build '

build_no = 0
try:
    with open(FILENAME_BUILDNO) as f:
        build_no = int(f.readline()) + 1
except:
    print('Starting build number from 1..')
    build_no = 1
with open(FILENAME_BUILDNO, 'w+') as f:
    f.write(str(build_no))
    print('Build number: {}'.format(build_no))

hf = """
#ifndef BUILD_NUMBER
  #define BUILD_NUMBER "{}"
#endif
#ifndef VERSION
  #define VERSION "{} - {}"
#endif
#ifndef VERSION_SHORT
  #define VERSION_SHORT "{}"
#endif
""".format(build_no, version_long+str(build_no), datetime.datetime.now(), version_short+'.'+str(build_no))
with open(FILENAME_VERSION_H, 'w+') as f:
    f.write(hf)
