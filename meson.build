project('mvdsv', 'c')

mvdsv_sources = [
	'src/bothtools.c',
	'src/build.c',
	'src/cmd.c',
	'src/cmodel.c',
	'src/common.c',
	'src/crc.c',
	'src/cvar.c',
	'src/fs.c',
	'src/hash.c',
	'src/mathlib.c',
	'src/md4.c',
	'src/net.c',
	'src/net_chan.c',
	'src/pmove.c',
	'src/pmovetst.c',
	'src/pr2_cmds.c',
	'src/pr2_edict.c',
	'src/pr2_exec.c',
	'src/pr2_vm.c',
	'src/pr_cmds.c',
	'src/pr_edict.c',
	'src/pr_exec.c',
	'src/sha1.c',
	'src/sv_ccmds.c',
	'src/sv_demo.c',
	'src/sv_demo_misc.c',
	'src/sv_demo_qtv.c',
	'src/sv_ents.c',
	'src/sv_init.c',
	'src/sv_login.c',
	'src/sv_main.c',
	'src/sv_master.c',
	'src/sv_mod_frags.c',
	'src/sv_move.c',
	'src/sv_nchan.c',
	'src/sv_phys.c',
	'src/sv_save.c',
	'src/sv_send.c',
	'src/sv_user.c',
	'src/vfs_os.c',
	'src/vfs_pak.c',
	'src/world.c',
	'src/zone.c',
]

c_args = ['-DSERVERONLY', '-DUSE_PR2']
link_args = []

deps = [
	dependency('threads')
]

pcre = dependency('libpcre', required : false)
if pcre.found()
	deps += pcre
else
	mvdsv_sources += [
		'src/pcre/get.c',
		'src/pcre/pcre.c'
	]
endif

curl = dependency('libcurl', required : false)
if curl.found()
	deps += curl
	mvdsv_sources += 'src/central.c'
	c_args += '-DWWW_INTEGRATION'
endif

if target_machine.system() == 'windows'
	mvdsv_sources += [
		'src/sv_sys_win.c',
		'src/sv_windows.c',
		import('windows').compile_resources('src/winquake.rc')
	]
	deps += [
		meson.get_compiler('c').find_library('ws2_32'),
		meson.get_compiler('c').find_library('winmm'),
	]
	if meson.get_compiler('c').get_id() == 'gcc'
		link_args += '-static-libgcc'
	endif
else
	mvdsv_sources += 'src/sv_sys_unix.c'
	deps += [
		meson.get_compiler('c').find_library('m'),
		meson.get_compiler('c').find_library('dl'),
	]
endif

if target_machine.endian() == 'little'
	c_args += '-D__LITTLE_ENDIAN__Q__'
elif target_machine.endian() == 'big'
	c_args += '-D__BIG_ENDIAN__Q__'
else
	c_args += '-D__PDP_ENDIAN__Q__'
endif

if target_machine.cpu_family() == 'x86' and target_machine.system() != 'darwin'
	asm_c_args = ['-x', 'assembler-with-cpp', '-Did386']
	if target_machine.system() != 'windows'
		asm_c_args += '-DELF'
	endif
	deps += declare_dependency(
		link_with : static_library('asm',
			'src/bothtoolsa.s',
			'src/math.s',
			c_args : asm_c_args
		)
	)
endif

executable('mvdsv', mvdsv_sources,
	dependencies : deps,
	c_args : c_args,
	link_args : link_args
)
