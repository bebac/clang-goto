# -----------------------------------------------------------------------------
require 'rake/clean'
require 'rake/tasklib'
# -----------------------------------------------------------------------------
require './rakelib/lib/ctasklib'

# -----------------------------------------------------------------------------
case ENV["variant"]
when "release"
    ENV["CFLAGS"]  = %q(-O2 -Wall -MMD)
    ENV["LDFLAGS"] = %q(-pthread)
else
    ENV["CFLAGS"]  = %q(-g -Wall -MMD)
    ENV["LDFLAGS"] = %q(-g -pthread)
end

ENV["CPPFLAGS"] = %q(-std=c++11)

# -----------------------------------------------------------------------------
program_options = Rake::StaticLibraryTask.new("vendor/program-options/program-options.yml")

# -----------------------------------------------------------------------------
json = Rake::StaticLibraryTask.new("vendor/json/json.yml")

# -----------------------------------------------------------------------------
spec = Rake::ExecutableSpecification.new do |s|
    s.name = 'clang-goto'
    s.includes.add %w(
        vendor/program-options/include
        vendor/json/include
    )
    s.libincludes.add %w(
        build
    )
    s.sources.add %w(
        source/main.cpp
    )
    s.libraries += [ program_options, json ] + %w(clang)
end
# -----------------------------------------------------------------------------
Rake::ExecutableTask.new(:"clang-goto", spec)

# -----------------------------------------------------------------------------
CLEAN.include('build')
# -----------------------------------------------------------------------------
task :default => [ :"clang-goto" ]
task :all => [ :default ]
