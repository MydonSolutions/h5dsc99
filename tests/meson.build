test('write', executable(
  'write', ['write.c'],
	dependencies: lib_h5dsc99_dep,
	),
	is_parallel: false,
	priority: 100,
)

test('read',executable(
	'read', ['read.c'],
	dependencies: lib_h5dsc99_dep,
	),
	is_parallel: false,
	priority: 99,
)