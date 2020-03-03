from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add ds3231 src files.
if GetDepend('PKG_USING_DS3231'):
    src += Glob('ds3231.c')

# add ds3231 include path.
path  = [cwd]

# add src and include to group.
group = DefineGroup('ds3231', src, depend = ['PKG_USING_DS3231'], CPPPATH = path)

Return('group')
