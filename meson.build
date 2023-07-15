project('h5dsc99', 'c',
  version: '0.2.1',
  default_options: [
    'c_std=c99',
    'buildtype=release',
    'b_ndebug=if-release'
  ]
)

src_lst = []
inc_lst = [
	include_directories('include'),
]
dep_lst = [
  dependency('hdf5'),
]

subdir('include')
subdir('src')

lib_h5ds = library(
  'h5ds',
  src_lst,
  include_directories: inc_lst,
  dependencies: dep_lst,
  install: true,
)

lib_h5ds_dep = declare_dependency(
  include_directories: inc_lst,
  dependencies: dep_lst,
  link_with: lib_h5ds,
)

subdir('tests')
